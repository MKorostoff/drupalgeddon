<?php

/**
 * @file
 * Implementation of migration from WordPress into Drupal
 */

abstract class WordPressMigration extends DynamicMigration {
  /**
   * The filespec of the WXR file this migration is based on.
   *
   * @var string
   */
  protected $wxrFile;

  /**
   * The blog object representing a set of migrations.
   *
   * @var WordPressBlog
   */
  protected $blog;

  /**
   * Constructor - general setup for WordPress migrations.
   *
   * @param array $arguments
   *  'filename' => WXR file managed by this migration
   */
  public function __construct(array $arguments) {
    if (!isset($arguments['filename'])) {
      throw new Exception(t('Filename is a required parameter for WordPress migrations'));
    }

    // Must be set before calling the parent constructor, which will call
    // generateMachineName() below.
    $filename = $arguments['filename'];
    $this->wxrFile = $filename;
    $this->blog = wordpress_migrate_blog($filename);
    parent::__construct(MigrateGroup::getInstance($this->blog->getTitle()));
  }

  /**
   * Construct the machine name from the blog title
   */
  protected function generateMachineName($class_name = NULL) {
    if (!$class_name) {
      $class_name = get_class($this);
    }

    // The machine name is the cleansed blog title, followed by the portion
    // of the class name after WordPress. For example, for
    // category migration (class WordPressCategory) of "Mike's Blog",
    // the generated machine name will be MikesBlogCategory.
    return $this->blog->machineName($class_name);
  }

  /**
   * Called after completion of each migration.
   */
  public function postImport() {
    parent::postImport();
    // Clear ignored rows from the map, so as not to confuse reporting.
    $map_table = $this->map->getMapTable();
    db_delete($map_table)
      ->condition('needs_update', MigrateMap::STATUS_IGNORED)
      ->execute();
  }

  /**
   * Outputs a progress message, reflecting the current status of a migration process.
   *
   * @param int $result
   *  Status of the process, represented by one of the Migration::RESULT_* constants.
   */
  protected function progressMessage($result) {
    $time = microtime(TRUE) - $this->lastfeedback;
    if ($time > 0) {
      $perminute = round(60*$this->processed_since_feedback/$time);
      $time = round($time, 1);
    }
    else {
      $perminute = '?';
    }

    if ($this->status == Migration::STATUS_IMPORTING) {
      switch ($result) {
        case Migration::RESULT_COMPLETED:
          $basetext = "Imported !numitems in !time sec (!perminute/min) - done with '!name'";
          $type = 'completed';
          break;
        case Migration::RESULT_FAILED:
          $basetext = "Imported !numitems in !time sec (!perminute/min) - failure with '!name'";
          $type = 'failed';
          break;
        case Migration::RESULT_INCOMPLETE:
          $basetext = "Imported !numitems in !time sec (!perminute/min) - continuing with '!name'";
          $type = 'ok';
          break;
        case Migration::RESULT_STOPPED:
          $basetext = "Imported !numitems in !time sec (!perminute/min) - stopped '!name'";
          $type = 'warning';
          break;
      }
      $numitems = $this->destination->getCreated();
    }
    else {
      switch ($result) {
        case Migration::RESULT_COMPLETED:
          $basetext = "Rolled back !numitems in !time sec (!perminute/min) - done with '!name'";
          $type = 'completed';
          break;
        case Migration::RESULT_FAILED:
          $basetext = "Rolled back !numitems in !time sec (!perminute/min) - failure with '!name'";
          $type = 'failed';
          break;
        case Migration::RESULT_INCOMPLETE:
          $basetext = "Rolled back !numitems in !time sec (!perminute/min) - continuing with '!name'";
          $type = 'ok';
          break;
        case Migration::RESULT_STOPPED:
          $basetext = "Rolled back !numitems in !time sec (!perminute/min) - stopped '!name'";
          $type = 'warning';
          break;
      }
      $numitems = $this->processed_since_feedback + $this->source->getIgnored();
    }

    $message = t($basetext,
        array('!numitems' => $numitems,
              '!time' => $time,
              '!perminute' => $perminute,
              '!name' => $this->machineName));
    self::displayMessage($message, $type);
    if ($result == Migration::RESULT_INCOMPLETE) {
      $this->lastfeedback = time();
      $this->processed_since_feedback = $this->successes_since_feedback = 0;
      $this->source->resetStats();
      $this->destination->resetStats();
    }
  }

  /**
   * Work-around for http://drupal.org/node/936222 - make sure our node_save()
   * calls not governed by the node destination class do not overwrite aliases.
   *
   * @param $node
   */
  protected function disablePathauto($node) {
    $node->path['pathauto'] = 0;
    if (!isset($node->path['alias'])) {
      $node->path['alias'] = '';
    }
  }
}

class WordPressBlog {
  protected $blogID;
  public function getBlogID() {
    return $this->blogID;
  }
  protected $filename;
  public function getFilename() {
    return $this->filename;
  }
  protected $wxrVersion = '1.0';
  public function getWxrVersion() {
    return $this->wxrVersion;
  }
  protected $title;
  public function getTitle() {
    return $this->title;
  }
  protected $blog_url;
  public function getBlogUrl() {
    return $this->blog_url;
  }
  protected $link;
  public function getLink() {
    return $this->link;
  }
  protected $uid;
  public function getUid() {
    return $this->uid;
  }

  protected $migrations = array();

  public function __construct($filename) {
    $this->filename = $filename;

    // Make sure the upload directory is properly protected
    file_create_htaccess('wordpress://', TRUE);

    // Suppress errors during parsing, so we can pick them up after
    libxml_use_internal_errors(TRUE);

    // Get the blog_url, which is our unique determiner of which blog we're
    // talking about
    $title = '';
    $reader = new XMLReader;
    $status = $reader->open($this->filename);
    if ($status) {
      $this->blog_url = '';
      while ($reader->read()) {
        if ($reader->nodeType == XMLREADER::ELEMENT) {
          switch ($reader->name) {
            case 'title':
              $title = WordPressBlog::readString($reader);
              break;
            case 'wp:wxr_version':
              $this->wxrVersion = WordPressBlog::readString($reader);
              break;
            case 'wp:base_blog_url':
              $this->blog_url = WordPressBlog::readString($reader);
              break;
            case 'link':
              $this->link = WordPressBlog::readString($reader);
              // Catch only the first link
              if (empty($this->link)) {
                $this->link = $reader->readString();
              };
              break;
          }
        }
        if (!empty($title) && !empty($this->blog_url) && !empty($this->link)) {
          break;
        }
      }
    }
    else {
      throw new Exception(t('Could not open XML file !url',
                            array('!url' => $this->filename)));
    }

    // Validate that it really is a WXR file
    if (empty($this->blog_url)) {
      // Older WP versions did not have a blog_url but used link instead.
      if (!empty($this->link)) {
        $this->blog_url = $this->link;
      }
      else {
        throw new Exception(t('The uploaded file is not a valid WordPress export'));
      };
    }
    // Keep only alphabetic characters
    $this->title = preg_replace('/[^A-Za-z]/', '', $title);
    if (!$this->title) {
      $this->title = preg_replace('/[^A-Za-z]/', '', $this->blog_url);
    }

    global $user;
    $this->uid = $user->uid;

    $status = db_merge('wordpress_migrate')
      ->key(array('blog_url' => $this->blog_url))
      ->fields(array(
                'title' => $this->title,
                'uid' => $this->uid,
                'link' => $this->link,
                'filename' => $this->filename,
                'wxr_version' => $this->wxrVersion,
               ))
      ->execute();
    $this->blogID = db_select('wordpress_migrate', 'wm')
                    ->fields('wm', array('blog_id'))
                    ->condition('blog_url', $this->blog_url)
                    ->execute()
                    ->fetchField();

    foreach ($this->migrationClasses() as $class) {
      // Skip authors with WXR 1.0
      if ($class == 'WordPressAuthor' && $this->wxrVersion == '1.0') {
        continue;
      }
      // Skip comments if comment module is not enabled
      if ($class == 'WordPressComment' && !module_exists('comment')) {
        continue;
      }
      // Skip tags/categories if taxonomy is not enabled
      if ($class == 'WordPressTag' || $class == 'WordPressCategory') {
        if (!module_exists('taxonomy')) {
          continue;
        }
      }
      // Skip tags/categories if there's no destination vocabulary
      if ($class == 'WordPressTag' && !variable_get('wordpress_migrate_tag_vocabulary', '') ||
          $class == 'WordPressCategory' && !variable_get('wordpress_migrate_category_vocabulary', '')) {
        continue;
      }
      Migration::registerMigration($class, $this->machineName($class), array('filename' => $filename));
    }
  }

  public function machineName($class_name) {
    // If the default classes have been overridden, $class_name might be either
    // the default class name, or the name of the overridden class. Check first
    // for the former case, then the latter
    $classes = $this->migrationClasses();
    if (!isset($classes[$class_name])) {
      $flipped = array_flip($classes);
      $class_name = $flipped[$class_name];
    }
    return $this->title . substr($class_name, strlen('WordPress'),
      strlen($class_name) - strlen('WordPress'));
  }

  /**
   * The implemented WordPress migrations, in the order they should be run.
   */
  public function migrationClasses() {
    return array(
      'WordPressAuthor' => 'WordPressAuthor',
      'WordPressCategory' => 'WordPressCategory',
      'WordPressTag' => 'WordPressTag',
      'WordPressBlogEntry' => 'WordPressBlogEntry',
      'WordPressPage' => 'WordPressPage',
      'WordPressAttachment' => 'WordPressAttachment',
      'WordPressComment' => 'WordPressComment',
    );
  }

  public function migrations() {
    if (empty($this->migrations)) {
      $this->migrations = array();
      foreach ($this->migrationClasses() as $base_class => $actual_class) {
        // Skip authors with WXR 1.0
        if ($base_class == 'WordPressAuthor' && $this->wxrVersion == '1.0') {
          continue;
        }
        // Skip comments if comment module is not enabled
        if ($base_class == 'WordPressComment' && !module_exists('comment')) {
          continue;
        }
        // Skip tags/categories if taxonomy is not enabled
        if ($base_class == 'WordPressTag' || $base_class == 'WordPressCategory') {
          if (!module_exists('taxonomy')) {
            continue;
          }
        }
        // Skip tags/categories if there's no destination vocabulary
        if ($base_class == 'WordPressTag' && !variable_get('wordpress_migrate_tag_vocabulary', '') ||
            $base_class == 'WordPressCategory' && !variable_get('wordpress_migrate_category_vocabulary', '')) {
          continue;
        }
        $this->migrations[$actual_class] = MigrationBase::getInstance($this->machineName($actual_class), $actual_class,
          array('filename' => $this->filename));
      }
    }
    return $this->migrations;
  }

  static public function blogs() {
    $blogs = array();
    $result = db_select('wordpress_migrate', 'wm')
               ->fields('wm', array('filename'))
               ->execute();
    foreach ($result as $row) {
      $blogs[] = wordpress_migrate_blog($row->filename);
    }
    return $blogs;
  }

  /**
   * WXR files typically need some cleanup to be successfully parsed - perform
   * that here.
   *
   * @param $sourcefile
   *  The raw WXR file as uploaded.
   * @param $destination
   *  Filespec to which to write the cleaned-up WXR file.
   */
  static public function preprocessFile($sourcefile, $destination) {
    // Cleanup some stuff in the process of moving the file to its final
    // destination
    $source_handle = fopen($sourcefile, 'r');
    $dest_handle = fopen($destination, 'w');

    // First, get the header (everything before the <channel> element) to
    // rewrite the namespaces
    $header = '';
    while ($line = fgets($source_handle)) {
      $header .= $line;
      if (strpos($line, '<channel>') !== FALSE) {
        break;
      }
    }

    // The excerpt namespace is sometimes omitted, stuff it in if necessary
    $excerpt_ns = 'xmlns:excerpt="http://wordpress.org/export/1.0/excerpt/"';
    $excerpt_signature = 'xmlns:excerpt="http://wordpress.org/export/';
    $content_ns = 'xmlns:content="http://purl.org/rss/1.0/modules/content/"';
    if (!strpos($header, $excerpt_signature)) {
      $header = str_replace($content_ns, $excerpt_ns . "\n\t" . $content_ns, $header);
    }
    // Add the Atom namespace, in case it's referenced
    $atom_ns = 'xmlns:atom="http://www.w3.org/2005/Atom"';
    $header = str_replace($content_ns, $atom_ns . "\n\t" . $content_ns, $header);

    // What the hell, throw in iTunes too
    $itunes_ns = 'xmlns:itunes="http://www.itunes.com/dtds/podcast-1.0.dtd"';
    $header = str_replace($content_ns, $itunes_ns . "\n\t" . $content_ns, $header);

    // Replace HTML entities with XML entities
    $header = strtr($header, self::$entityReplacements);
    fputs($dest_handle, $header);

    // Now, do some line-by-line fix-ups fix unencoded ampersands and bogus characters on a line-by-line basis
    while ($line = fgets($source_handle)) {
      // Handle unencoded ampersands
      $line = preg_replace('/&(?![\w\d#]+;)/', '&amp;', $line);
      // Remove control characters (the regex removes the newline, so tack it back on)
      $line = preg_replace('~\p{C}+~u', '', $line) . "\n";
      // WordPress export doesn't properly format embedded CDATA sections - our
      // quick-and-dirty fix is to remove the terminator of the embedded section
      $line = preg_replace('|// \]\]|', '', $line);
      // Replace HTML entities with XML entities
      $line = strtr($line, self::$entityReplacements);
      fputs($dest_handle, $line);
    }

    fclose($dest_handle);
    fclose($source_handle);
    unlink($sourcefile);
  }

  /**
   * Translation table between HTML entities and XML entities; some WP blogs
   * use HTML entities in XML.
   *
   * @var array
   */
  static protected $entityReplacements = array(
    '&AElig;'    => '&#198;',  '&Aacute;'   => '&#193;',
    '&Acirc;'    => '&#194;',  '&Agrave;'   => '&#192;',
    '&Alpha;'    => '&#913;',  '&Aring;'    => '&#197;',
    '&Atilde;'   => '&#195;',  '&Auml;'     => '&#196;',
    '&Beta;'     => '&#914;',  '&Ccedil;'   => '&#199;',
    '&Chi;'      => '&#935;',  '&Dagger;'   => '&#8225;',
    '&Delta;'    => '&#916;',  '&ETH;'      => '&#208;',
    '&Eacute;'   => '&#201;',  '&Ecirc;'    => '&#202;',
    '&Egrave;'   => '&#200;',  '&Epsilon;'  => '&#917;',
    '&Eta;'      => '&#919;',  '&Euml;'     => '&#203;',
    '&Gamma;'    => '&#915;',  '&Iacute;'   => '&#205;',
    '&Icirc;'    => '&#206;',  '&Igrave;'   => '&#204;',
    '&Iota;'     => '&#921;',  '&Iuml;'     => '&#207;',
    '&Kappa;'    => '&#922;',  '&Lambda;'   => '&#923;',
    '&Mu;'       => '&#924;',  '&Ntilde;'   => '&#209;',
    '&Nu;'       => '&#925;',  '&OElig;'    => '&#338;',
    '&Oacute;'   => '&#211;',  '&Ocirc;'    => '&#212;',
    '&Ograve;'   => '&#210;',  '&Omega;'    => '&#937;',
    '&Omicron;'  => '&#927;',  '&Oslash;'   => '&#216;',
    '&Otilde;'   => '&#213;',  '&Ouml;'     => '&#214;',
    '&Phi;'      => '&#934;',  '&Pi;'       => '&#928;',
    '&Prime;'    => '&#8243;', '&Psi;'      => '&#936;',
    '&Rho;'      => '&#929;',  '&Scaron;'   => '&#352;',
    '&Sigma;'    => '&#931;',  '&THORN;'    => '&#222;',
    '&Tau;'      => '&#932;',  '&Theta;'    => '&#920;',
    '&Uacute;'   => '&#218;',  '&Ucirc;'    => '&#219;',
    '&Ugrave;'   => '&#217;',  '&Upsilon;'  => '&#933;',
    '&Uuml;'     => '&#220;',  '&Xi;'       => '&#926;',
    '&Yacute;'   => '&#221;',  '&Yuml;'     => '&#376;',
    '&Zeta;'     => '&#918;',  '&aacute;'   => '&#225;',
    '&acirc;'    => '&#226;',  '&acute;'    => '&#180;',
    '&aelig;'    => '&#230;',  '&agrave;'   => '&#224;',
    '&alefsym;'  => '&#8501;', '&alpha;'    => '&#945;',
    '&and;'      => '&#8743;', '&ang;'      => '&#8736;',
    '&aring;'    => '&#229;',  '&asymp;'    => '&#8776;',
    '&atilde;'   => '&#227;',  '&auml;'     => '&#228;',
    '&bdquo;'    => '&#8222;', '&beta;'     => '&#946;',
    '&brvbar;'   => '&#166;',  '&bull;'     => '&#8226;',
    '&cap;'      => '&#8745;', '&ccedil;'   => '&#231;',
    '&cedil;'    => '&#184;',  '&cent;'     => '&#162;',
    '&chi;'      => '&#967;',  '&circ;'     => '&#710;',
    '&clubs;'    => '&#9827;', '&cong;'     => '&#8773;',
    '&copy;'     => '&#169;',  '&crarr;'    => '&#8629;',
    '&cup;'      => '&#8746;', '&curren;'   => '&#164;',
    '&dArr;'     => '&#8659;', '&dagger;'   => '&#8224;',
    '&darr;'     => '&#8595;', '&deg;'      => '&#176;',
    '&delta;'    => '&#948;',  '&diams;'    => '&#9830;',
    '&divide;'   => '&#247;',  '&eacute;'   => '&#233;',
    '&ecirc;'    => '&#234;',  '&egrave;'   => '&#232;',
    '&empty;'    => '&#8709;', '&emsp;'     => '&#8195;',
    '&ensp;'     => '&#8194;', '&epsilon;'  => '&#949;',
    '&equiv;'    => '&#8801;', '&eta;'      => '&#951;',
    '&eth;'      => '&#240;',  '&euml;'     => '&#235;',
    '&euro;'     => '&#8364;', '&exist;'    => '&#8707;',
    '&fnof;'     => '&#402;',  '&forall;'   => '&#8704;',
    '&frac12;'   => '&#189;',  '&frac14;'   => '&#188;',
    '&frac34;'   => '&#190;',  '&frasl;'    => '&#8260;',
    '&gamma;'    => '&#947;',  '&ge;'       => '&#8805;',
    '&hArr;'     => '&#8660;', '&harr;'     => '&#8596;',
    '&hearts;'   => '&#9829;', '&hellip;'   => '&#8230;',
    '&iacute;'   => '&#237;',  '&icirc;'    => '&#238;',
    '&iexcl;'    => '&#161;',  '&igrave;'   => '&#236;',
    '&image;'    => '&#8465;', '&infin;'    => '&#8734;',
    '&int;'      => '&#8747;', '&iota;'     => '&#953;',
    '&iquest;'   => '&#191;',  '&isin;'     => '&#8712;',
    '&iuml;'     => '&#239;',  '&kappa;'    => '&#954;',
    '&lArr;'     => '&#8656;', '&lambda;'   => '&#955;',
    '&lang;'     => '&#9001;', '&laquo;'    => '&#171;',
    '&larr;'     => '&#8592;', '&lceil;'    => '&#8968;',
    '&ldquo;'    => '&#8220;', '&le;'       => '&#8804;',
    '&lfloor;'   => '&#8970;', '&lowast;'   => '&#8727;',
    '&loz;'      => '&#9674;', '&lrm;'      => '&#8206;',
    '&lsaquo;'   => '&#8249;', '&lsquo;'    => '&#8216;',
    '&macr;'     => '&#175;',  '&mdash;'    => '&#8212;',
    '&micro;'    => '&#181;',  '&middot;'   => '&#183;',
    '&minus;'    => '&#8722;', '&mu;'       => '&#956;',
    '&nabla;'    => '&#8711;', '&nbsp;'     => '&#160;',
    '&ndash;'    => '&#8211;', '&ne;'       => '&#8800;',
    '&ni;'       => '&#8715;', '&not;'      => '&#172;',
    '&notin;'    => '&#8713;', '&nsub;'     => '&#8836;',
    '&ntilde;'   => '&#241;',  '&nu;'       => '&#957;',
    '&oacute;'   => '&#243;',  '&ocirc;'    => '&#244;',
    '&oelig;'    => '&#339;',  '&ograve;'   => '&#242;',
    '&oline;'    => '&#8254;', '&omega;'    => '&#969;',
    '&omicron;'  => '&#959;',  '&oplus;'    => '&#8853;',
    '&or;'       => '&#8744;', '&ordf;'     => '&#170;',
    '&ordm;'     => '&#186;',  '&oslash;'   => '&#248;',
    '&otilde;'   => '&#245;',  '&otimes;'   => '&#8855;',
    '&ouml;'     => '&#246;',  '&para;'     => '&#182;',
    '&part;'     => '&#8706;', '&permil;'   => '&#8240;',
    '&perp;'     => '&#8869;', '&phi;'      => '&#966;',
    '&pi;'       => '&#960;',  '&piv;'      => '&#982;',
    '&plusmn;'   => '&#177;',  '&pound;'    => '&#163;',
    '&prime;'    => '&#8242;', '&prod;'     => '&#8719;',
    '&prop;'     => '&#8733;', '&psi;'      => '&#968;',
    '&rArr;'     => '&#8658;', '&radic;'    => '&#8730;',
    '&rang;'     => '&#9002;', '&raquo;'    => '&#187;',
    '&rarr;'     => '&#8594;', '&rceil;'    => '&#8969;',
    '&rdquo;'    => '&#8221;', '&real;'     => '&#8476;',
    '&reg;'      => '&#174;',  '&rfloor;'   => '&#8971;',
    '&rho;'      => '&#961;',  '&rlm;'      => '&#8207;',
    '&rsaquo;'   => '&#8250;', '&rsquo;'    => '&#8217;',
    '&sbquo;'    => '&#8218;', '&scaron;'   => '&#353;',
    '&sdot;'     => '&#8901;', '&sect;'     => '&#167;',
    '&shy;'      => '&#173;',  '&sigma;'    => '&#963;',
    '&sigmaf;'   => '&#962;',  '&sim;'      => '&#8764;',
    '&spades;'   => '&#9824;', '&sub;'      => '&#8834;',
    '&sube;'     => '&#8838;', '&sum;'      => '&#8721;',
    '&sup1;'     => '&#185;',  '&sup2;'     => '&#178;',
    '&sup3;'     => '&#179;',  '&sup;'      => '&#8835;',
    '&supe;'     => '&#8839;', '&szlig;'    => '&#223;',
    '&tau;'      => '&#964;',  '&there4;'   => '&#8756;',
    '&theta;'    => '&#952;',  '&thetasym;' => '&#977;',
    '&thinsp;'   => '&#8201;', '&thorn;'    => '&#254;',
    '&tilde;'    => '&#732;',  '&times;'    => '&#215;',
    '&trade;'    => '&#8482;', '&uArr;'     => '&#8657;',
    '&uacute;'   => '&#250;',  '&uarr;'     => '&#8593;',
    '&ucirc;'    => '&#251;',  '&ugrave;'   => '&#249;',
    '&uml;'      => '&#168;',  '&upsih;'    => '&#978;',
    '&upsilon;'  => '&#965;',  '&uuml;'     => '&#252;',
    '&weierp;'   => '&#8472;', '&xi;'       => '&#958;',
    '&yacute;'   => '&#253;',  '&yen;'      => '&#165;',
    '&yuml;'     => '&#255;',  '&zeta;'     => '&#950;',
    '&zwj;'      => '&#8205;', '&zwnj;'     => '&#8204;',
  );

  /**
   * With earlier versions of libxml, XMLReader has no readString() method -
   * mock it up if necessary.
   *
   * @param $reader
   *  XMLReader instance being iterated for XML parsing.
   *
   * @return string
   */
  static public function readString(XMLReader $reader) {
    if (method_exists('XMLReader', 'readString')) {
      return $reader->readString();
    }
    else {
      $node = $reader->expand();
      return $node->textContent;
    }
  }
}
