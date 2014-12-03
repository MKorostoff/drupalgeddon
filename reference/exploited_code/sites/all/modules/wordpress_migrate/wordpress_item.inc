<?php

/**
 * @file
 *
 * Support for migrating posts and pages from a WordPress blog into Drupal.
 */

/**
 * Implementation of MigrateSource, to handle migrating items from WordPress XML dumps.
 */
class WordPressItemSource extends WordPressSourceXML {
  /**
   * The <wp:post_type> value we're looking for in this migration
   * (post/page/attachment).
   *
   * @var string
   */
  protected $postType;

  /**
   * List of available source fields.
   *
   * @var array
   */
  protected $fields = array(
    'title' => 'Item title',
    'link' => 'WordPress URL of the item',
    'pubDate' => 'Published date',
    'creator' => 'WordPress username of the item author',
    'guid' => 'Alternate URL of the item (?)',
    'description' => '?',
    'content' => 'Body of the item',
    'excerpt' => 'Teaser for the item',
    'post_id' => 'Unique ID of the item within the blog',
    'post_date' => 'Date posted (author\s timezone?)',
    'post_date_gmt' => 'Date posted (GMT)',
    'comment_status' => 'Whether comments may be posted to this item (open/closed)',
    'ping_status' => '?',
    'post_name' => 'Trailing component of link',
    'status' => 'Item status (publish/draft/inherit)',
    'post_parent' => 'Parent item ID (?)',
    'menu_order' => 'Equivalent to Drupal weight?',
    'post_type' => 'Item type (post/page/attachment)',
    'post_password' => '?',
    'is_sticky' => 'Equivalent to Drupal sticky flag',
    'category' => 'Categories (as nicename) assigned to this item',
    'tag' => 'Tags (as nicename) assigned to this item',
  );

  /**
   * Simple initialization.
   *
   */
  public function __construct($filename, $post_type, $cache_key) {
    $source_options = array(
      'reader_class' => 'WordPressXMLReader',
      'cache_counts' => TRUE,
      'cache_key' => $cache_key,
    );
    parent::__construct($filename, '/rss/channel/item', 'wp:post_id',
      $this->fields, $source_options);
    $this->postType = $post_type;
  }

  /**
   * Return a count of all available source records.
   */
  public function computeCount() {
    $count = 0;
    foreach ($this->sourceUrls as $url) {
      $reader = new $this->readerClass($url, $this->elementQuery, $this->idQuery);
      foreach ($reader as $element) {
        // Only count relevant postType
        if ($element->post_type == $this->postType) {
          $count++;
        }
      }
    }

    return $count;
  }
}

/**
 * Intermediate Migration class, implementing behavior common across different
 * types (post_type) of items.
 */
abstract class WordPressItemMigration extends WordPressMigration {
  /**
   * The <wp:post_type> value we're looking for in this migration
   * (post/page/attachment).
   *
   * @var string
   */
  protected $postType;

  /**
   * Indicates to the complete() method that the nid of this item needs to be
   * saved in linksToFix for later processing.
   *
   * @var boolean
   */
  protected $linksNeedFixing = FALSE;

  /**
   * List of nids which have links needing fixing.
   *
   * @var array
   */
  static protected $linksToFix = array();

  /**
   * Track the fields for the term references, so we can fix up if necessary
   * in prepare().
   *
   * @var string
   */
  protected $tagField = NULL;
  protected $categoryField = NULL;

  /**
   * Set it up
   */
  public function __construct(array $arguments = array()) {
    parent::__construct($arguments);

    if (module_exists('taxonomy')) {
      if (variable_get('wordpress_migrate_category_vocabulary', '')) {
        $this->dependencies[] = $this->generateMachineName('WordPressCategory');
      }
      if (variable_get('wordpress_migrate_tag_vocabulary', '')) {
        $this->dependencies[] = $this->generateMachineName('WordPressTag');
      }
    }

    if ($this->blog->getWxrVersion() != '1.0') {
      $this->dependencies[] = $this->generateMachineName('WordPressAuthor');
    }

    // WordPress post type
    $this->postType = $arguments['post_type'];
    // Drupal content type (bundle)
    $bundle = $arguments['bundle'];

    // post_id is the unique ID of items in WordPress
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'post_id' => array(
          'type' => 'int',
          'not null' => TRUE,
          'unsigned' => TRUE,
          'description' => 'WordPress post ID',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Construct the source objects.
    $this->source = new WordPressItemSource($this->wxrFile, $this->postType, $this->machineName);
    $this->destination = new MigrateDestinationNode($bundle);

    // Default mappings, applying to most or all migrations
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('created', 'post_date')
         ->description('Empty dates handled in prepare()');
    $this->addFieldMapping('changed', 'post_date')
         ->description('Empty dates handled in prepare()');
    // If we have a separate author migration, use it here
    $uid_mapping = $this->addFieldMapping('uid', 'creator')
         ->description('Use matching username if any, otherwise current user');
    if ($this->blog->getWxrVersion() != '1.0') {
      $uid_mapping->sourceMigration($this->generateMachineName('WordPressAuthor'));
    }
    $text_format = variable_get('wordpress_migrate_text_format', 'filtered_html');
    $this->addFieldMapping('body', 'content')
         ->arguments($arguments);
    $this->addFieldMapping('body:summary', 'excerpt');
    $this->addFieldMapping('body:format')
         ->defaultValue($text_format);
    if (module_exists('comment')) {
      $this->addFieldMapping('comment', 'comment_status')
           ->description('WP "open" mapped to Drupal COMMENT_NODE_OPEN');
    }
    $this->addFieldMapping('status', 'status')
         ->description('Set Drupal status to 1 iff wp:status=publish');
    $this->addFieldMapping(NULL, 'post_parent')
         ->description('Only applies to attachments');
    $this->addFieldMapping('sticky', 'is_sticky');
    if (module_exists('path')) {
      switch (variable_get('wordpress_migrate_path_action', 1)) {
        // Do not set path aliases
        case 0:
          $this->addFieldMapping('path');
          if (!module_exists('redirect')) {
            $this->addFieldMapping(NULL, 'link');
          }
          if (module_exists('pathauto')) {
            $this->addFieldMapping('pathauto')
                 ->defaultValue(0);
          }
          break;
        // Set path aliases to their original WordPress values
        case 1:
          $this->addFieldMapping('path', 'link');
          if (module_exists('pathauto')) {
            $this->addFieldMapping('pathauto')
                 ->defaultValue(0);
          }
          break;
        // Have pathauto generate new aliases
        case 2:
          $this->addFieldMapping('path');
          if (!module_exists('redirect')) {
            $this->addFieldMapping(NULL, 'link');
          }
          if (module_exists('pathauto')) {
            $this->addFieldMapping('pathauto')
                 ->defaultValue(1);
          }
          break;
      }
    }

    if (module_exists('redirect')) {
      if (variable_get('wordpress_migrate_generate_redirects', 0)) {
        $this->addFieldMapping('migrate_redirects', 'link');
      }
      else {
        $this->addFieldMapping('migrate_redirects');
        $this->addFieldMapping(NULL, 'link');
      }
    }

    if (module_exists('taxonomy')) {
      // Map the source fields to the configured vocabularies
      $vocabs = array(
        'tag' => variable_get('wordpress_migrate_tag_vocabulary', ''),
        'category' => variable_get('wordpress_migrate_category_vocabulary', ''),
      );

      // Look through this content type's fields for term references
      foreach ($this->destination->fields() as $machine_name => $description) {
        if (preg_match('/\(taxonomy_term_reference\)$/', $description)) {
          $field_info = field_info_field($machine_name);
          // Check this field against each of the configured vocabularies - if
          // a match is found, make the mapping
          foreach ($vocabs as $source_field => $vocab_name) {
            if ($vocab_name == $field_info['settings']['allowed_values'][0]['vocabulary']) {
              $vocab = taxonomy_vocabulary_machine_name_load($vocab_name);
              if ($source_field == 'tag') {
                $sourceMigration = $this->generateMachineName('WordPressTag');
                $this->tagField = $machine_name;
                $this->tagVid = $vocab->vid;
              }
              else {
                $sourceMigration = $this->generateMachineName('WordPressCategory');
                $this->categoryField = $machine_name;
                $this->categoryVid = $vocab->vid;
              }
              $this->addFieldMapping($machine_name, $source_field)
                   ->arguments(array('source_type' => 'tid'))
                   ->sourceMigration($sourceMigration);
              unset($vocabs[$source_field]);
            }
          }
        }
      }

      // If we didn't map one or both of the tag/category fields, indicate so with
      // a DNM mapping.
      foreach ($vocabs as $source_field => $vocab_name) {
        if ($vocab_name) {
          $message = t('!vocab_name vocabulary is not assigned to node type !bundle',
            array('!vocab_name' => $vocab_name, '!bundle' => $bundle));
        }
        else {
          $message = t('No vocabulary assigned for this field');
        }
        $this->addFieldMapping(NULL, $source_field)
             ->description($message)
             ->issueGroup(t('DNM'));
      }
    }

    // If podcast migration is requested, add the mapping.
    $podcast_field = variable_get('wordpress_migrate_podcast_field', '');
    if ($podcast_field) {
      $this->addFieldMapping($podcast_field, 'enclosure')
           ->callbacks(array($this, 'handleEnclosure'));
    }

    // If an attachment field is configured, document the mapping.
    $attachment_field = variable_get('wordpress_migrate_attachment_field', '');
    if ($attachment_field) {
      $this->addFieldMapping($attachment_field)
           ->description('Attachment field populated later by attachment migration');
    }

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('is_new', 'revision', 'language',
                                     'promote', 'revision_uid', 'log', 'tnid'));

    // Unmapped source fields
    $this->addUnmigratedSources(array('post_id', 'menu_order', 'post_type'));
    $this->addFieldMapping(NULL, 'guid')
         ->description('same as link, plus isPermaLink attribute?')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping(NULL, 'description')
         ->description('Always empty?')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping(NULL, 'pubDate')
         ->description('Use post_date')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping(NULL, 'post_date_gmt')
         ->description('Use post_date')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping(NULL, 'ping_status')
         ->description('What does this mean?')
         ->issueGroup(t('Open issues'))
         ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM);
    $this->addFieldMapping(NULL, 'post_name')
         ->description('Looks like last component of path')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping(NULL, 'post_password')
         ->description('???')
         ->issueGroup(t('DNM'));
  }

  /**
   * Data manipulations to be performed before the migrate module applies mappings.
   *
   * @param stdClass $row
   * @return string
   */
  public function prepareRow($row) {
    // Skip any of the wrong post type
    if ($row->post_type != $this->postType) {
      return FALSE;
    }

    // Only publish those with wp:status == 'publish'
    if (isset($row->status)) {
      switch ($row->status) {
        case 'publish':
          $row->status = NODE_PUBLISHED;
          break;
        case 'trash':
          return FALSE;
        default:
          $row->status = NODE_NOT_PUBLISHED;
      }
    }
    else {
      $row->status = NODE_NOT_PUBLISHED;
    }

    // If incoming date is zero (indicates unpublished content), use the current time
    if ($row->post_date == '0000-00-00 00:00:00') {
      $row->post_date = time();
    }

    // If the link has a query string, don't produce a path
    if (strpos($row->link, '?')) {
      unset($row->link);
    }
    else {
      // Otherwise, strip the domain portion of the URL
      $matches = array();
      if (preg_match('|http://[^/]+/(.*)|', $row->link, $matches)) {
        $row->link = $matches[1];
        // Strip the last slash off of the URL (the Path module can't handle this)
        $row->link = rtrim($row->link, '/');
      }
      else {
        unset($row->link);
      }
    }

    // Translate WordPress comment_status to Drupal values
    if (module_exists('comment')) {
      if ($row->comment_status == 'open') {
        $row->comment_status = COMMENT_NODE_OPEN;
      }
      else {
        $row->comment_status = COMMENT_NODE_CLOSED;
      }
    }

    // Pull out teasers out based on <!--more--> tags
    $broken = explode('<!--more-->', $row->content);
    if (count($broken) == 2) {
      $row->excerpt = $broken[0];
    }

    // Interpret the [caption] tags
    $row->content = preg_replace_callback('|(\[caption.*?\])(.*?)(\[/caption\])|i',
      array($this, 'replaceCaptions'), $row->content);

    // Handle [youtube] tags - convert them to <object> tags. If the media module is
    // installed, the next step will then convert them to media tags
    $replacement = '<object width="425" height="344">' .
      '<param name="movie" value="http://www.youtube.com/v/$1&#038;fs=1" />' .
      '<param name="allowFullScreen" value="true" />' .
      '<param name="allowscriptaccess" value="always" />' .
      '<embed src="http://www.youtube.com/v/$1&#038;fs=1" type="application/x-shockwave-flash" allowscriptaccess="always" allowfullscreen="true" width="425" height="344">' .
      '</embed></object>';
    // One form is [youtube dQw4w9WgXcQ]
    $row->content = preg_replace('|\[youtube (.+?)\]|i', $replacement, $row->content);

    // Another form is [youtube=https://www.youtube.com/watch?v=dQw4w9WgXcQ]
    $row->content = preg_replace('|\[youtube=.+?=([a-z0-9_-]+)[^\]]*\]|i', $replacement, $row->content);

    // Rewrite embedded video references to media tags
    if (module_exists('media')) {
      $row->content = preg_replace_callback('|<object [^>]*>.*?(<embed [^>]*>).*?</object>|i',
        array($this, 'replaceEmbeds'), $row->content);
    }

    // Rewrite (or remember to rewrite) links of the form
    // http://example.wordpress.com/?p=19 to local links of the form /node/35
    $row->content = $this->fixLocalLinks($row->content);

    // Handle Embedit HTML embed
    if (isset($row->HTML1)) {
      $row->content = $this->replaceHTMLEmbeds($row, $row->content);
    }

    // Replace Kimilli Flash Embed tags with <object>
    $row->content = preg_replace_callback(
      '|\[kml_flashembed *(.*?)/\]|i',
      array($this, 'replaceFlashEmbeds'), $row->content);

    return TRUE;
  }

  /**
   * Rewrite [caption] tags into HTML representing a caption.
   * [caption] itself ($matches[1]) will become an opening <div>,
   * the content within the tag ($matches[2]) will be passed through unchanged,
   * and the closing [/caption] ($matches[3]) will become a <p> containing the
   * caption followed by a closing </div>.
   *
   * @param array $matches
   */
  protected function replaceCaptions(array $matches) {
    $caption_open = $matches[1];
    $content = $matches[2];
    $caption_close = $matches[3];

    preg_match('|width="(.*?)"|i', $caption_open, $matches);
    $width = (int)$matches[1] + 10;
    $style = "width: {$width}px;";

    preg_match('|align="(.*?)"|i', $caption_open, $matches);
    $align = $matches[1];
    switch ($align) {
      case 'aligncenter':
        $style .= "display:block;margin:0 auto;";
        break;
      case 'alignleft':
        $style .= "float:left;";
        break;
      case 'alignright':
        $style .= "float:right;";
        break;
      default:
        break;
    }
    preg_match('|caption="(.*?)"|i', $caption_open, $matches);
    $caption = $matches[1];

    $result = '<div style="' . $style . '">';
    $result .= $content;
    $result .= "<p>$caption</p></div>";

    return $result;
  }

  /**
   * If we have a YouTube or other media reference, replace it with media tags.
   *
   * @param array $matches
   */
  protected function replaceEmbeds(array $matches) {
    // Default to the original <object> tag.
    $result = $matches[0];

    // If an <embed> tag is present, attempt to parse it.
    if ($matches[1]) {
      if (preg_match('|src=[\'"](.*?)[\'"]|i', $matches[1], $src_matches)) {
        $src = $src_matches[1];
      }
      else {
        return $result;
      }

      // Attempt to parse embedded media automatically through the media module.
      try {
        $uri = media_parse_to_uri($src);
        // Sometimes, at least with oembed, it doesn't like the /v/ form
        $uri = str_replace('youtube.com/v/', 'youtube.com/watch?v=', $uri);
      }
      catch (Exception $e) {
        return $result;
      }
      if (!isset($uri)) {
        return $result;
      }

      // Extract the width & height for the media tag.
      if (preg_match('|width=[\'"](.*?)[\'"]|i', $matches[1], $width_matches)) {
        $width = $width_matches[1];
      }
      else {
        return $result;
      }
      if (preg_match('|height=[\'"](.*?)[\'"]|i', $matches[1], $height_matches)) {
        $height = $height_matches[1];
      }
      else {
        return $result;
      }

      // Build a file object suitable for saving.
      if (function_exists('file_uri_to_object')) {
        $file = file_uri_to_object($uri, TRUE);
        if (!isset($file->fid)) {
          // Save the media.
          file_save($file);
        }
      }
      else {
        // We shouldn't get here. But just in case...
        return $result;
      }

      // Build the media tag
      $video_info = array(
        'type' => 'media',
        'view_mode' => 'media_large',
        'fid' => $file->fid,
        'attributes' => array(
          'class' => 'media-image',
          'typeof' => 'foaf:Image',
          'height' => $height,
          'width' => $width,
          'style' => '',
        ),
      );
      $result = '[[' . drupal_json_encode($video_info) . ']]';
    }

    return $result;
  }

  /**
   * Replace a kml_flashembed tag with an HTML <object> tag.
   *
   [kml_flashembed movie="http://example.com/video/soundslider.swf"
    FVARS="size=0" height="368" width="420" /]
   <object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
   			id="fm_soundslider_1640275925"
   			class="flashmovie"
   			width="420"
   			height="368">
   	<param name="movie" value="http://example.com/video/soundslider.swf"" />
   	<param name="flashvars" value="size=0" />
   	<!--[if !IE]>-->
   	<object	type="application/x-shockwave-flash"
   			data="http://example.com/video/soundslider.swf""
   			name="fm_soundslider_1640275925"
   			width="420"
   			height="368">
   		<param name="flashvars" value="size=0" />
   	<!--<![endif]-->

   	<!--[if !IE]>-->
   	</object>
   	<!--<![endif]-->
   </object>
   * @param array $matches
   */
  protected function replaceFlashEmbeds(array $matches) {
    $attribute_matches = array();
    $attribute_string = '';
    preg_match_all('|(?P<name>\w+)="(?P<value>.*?)"|', $matches[0], $attribute_matches);
    foreach ($attribute_matches['name'] as $delta => $name) {
      $value = $attribute_matches['value'][$delta];
      switch (drupal_strtolower($name)) {
        case 'movie':
          // Sometimes they've got their own player in their ahead of the movie,
          // strip it out
          $url_position = strpos($value, 'http://');
          if ($url_position) {
            $movie = substr($value, $url_position);
            // Strip parameters
            $movie = substr($movie, 0, strpos($movie, '&'));
          }
          else {
            $movie = $value;
          }
          break;
        case 'fvars':
          $flashvars = $value;
          break;
        default:
          $attribute_string .= ' ' . $name . '="' . $value . '"';
          break;
      }
    }
    $result = '<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" class="flashmovie"'
      . $attribute_string . ">\n";
    $result .= '<param name="movie" value="' . $movie . "\" />\n";
    if (isset($flashvars)) {
      $result .= '<param name="flashvars" value="' . $flashvars . "\" />\n";
    }
    $result .= "<!--[if !IE]>-->\n";
    $result .= '<object type="application/x-shockwave-flash" data="' . $movie .
               '"' . $attribute_string. ">\n";
    if (isset($flashvars)) {
      $result .= '<param name="flashvars" value="' . $flashvars . "\" />\n";
    }
    $result .= "<!--<![endif]-->\n";
    $result .= "<!--[if !IE]>--></object><!--<![endif]-->\n";
    $result .= "</object>\n";
    return $result;
  }

  /**
   * Replace any hrefs to links of the form http://example.wordpress.com/?=23
   * to local links to a node.
   *
   * @param string $body
   */
  protected function fixLocalLinks($body) {
    $this->linksNeedFixing = FALSE;
    $site_url = $this->blog->getLink();
    $pattern = '|href="' . $site_url . '/\?p=([0-9]+)"|i';
    $body = preg_replace_callback($pattern, array($this, 'replaceLinks'), $body);
    return $body;
  }

  /**
   * If we have a local link of the form ?p=34, translate the WordPress ID into
   * a Drupal nid, and rewrite the link.
   *
   * @param array $matches
   */
  protected function replaceLinks(array $matches) {
    // Default to the existing string
    $return = $matches[0];

    $wordpress_id = (int)$matches[1];

    // Check the blog entry and page maps to see if we can map this to a nid
    static $maps = array();
    if (empty($maps)) {
      $machines = array($this->generateMachineName('WordPressBlogEntry'),
        $this->generateMachineName('WordPressPage'));
      foreach ($machines as $machine) {
        $maps[] = MigrationBase::getInstance($machine)->getMap();
      }
    }

    foreach ($maps as $map) {
      $destination_id = $map->lookupDestinationID(array($wordpress_id), $this);
      if (!empty($destination_id)) {
        // Got a hit! Stop looking...
        $destination_id = reset($destination_id);
        break;
      }
    }

    // Remember if we didn't get a hit, complete() will set up for later review
    if (empty($destination_id)) {
      $this->linksNeedFixing = TRUE;
    }
    else {
      $return = 'href="/node/' . $destination_id . '"';
    }
    return $return;
  }

  protected function replaceHTMLEmbeds($row, $text) {
    for ($i = 1; $i <= 9; $i++) {
      $field = "HTML$i";
      if (isset($row->$field)) {
        $text = str_replace("[$field]", $row->$field, $text);
      }
    }
    return $text;
  }

  /**
   * Blubrry PowerPress podcast values are of the form
   *  http://www.example.com/audio/example.mp3
   *  7662957
   *  audio/mpeg
   *  a:1:{s:8:"duration";s:8:"00:05:19";}
   * We will extract and return the first line, the URL of the audio file.
   *
   * @param $value
   * @return string
   */
  protected function handleEnclosure($value) {
    $value_array = explode("\n", $value);
    return reset($value_array);
  }

  /**
   * Prepare node - called just before node_save().
   *
   * @param stdClass $node
   * @param stdClass $row
   */
  public function prepare(stdClass $node, stdClass $row) {
    // With WXR version 1.0, match creator username to Drupal username if
    // possible; otherwise, use the user that initiated the import. With later
    // versions, we've already got the right uid via the author migration.
    if ($this->blog->getWxrVersion() == '1.0') {
      static $drupal_static_fast;
      if (!isset($drupal_static_fast)) {
        $drupal_static_fast['user_map'] = &drupal_static(__FUNCTION__);
        $drupal_static_fast['default_user'] = &drupal_static(__FUNCTION__ . 'DefaultUser');
      }
      $user_map = &$drupal_static_fast['user_map'];
      if (!isset($user_map[$row->creator])) {
        $user_map[$row->creator] = db_select('users', 'u')
                                   ->fields('u', array('uid'))
                                   ->condition('name', $row->creator)
                                   ->execute()
                                   ->fetchField();
        if (!$user_map[$row->creator]) {
          $default_user = &$drupal_static_fast['default_user'];
          if (!isset($default_user)) {
            $default_user = db_select('wordpress_migrate', 'wpm')
                            ->fields('wpm', array('uid'))
                            ->condition('filename', $this->wxrFile)
                            ->execute()
                            ->fetchField();
          }
          $user_map[$row->creator] = $default_user;
        }
      }
      $node->uid = $user_map[$row->creator];
    }

    // If any term relationships were unresolved, create them the hard way
    foreach (array('tag', 'category') as $term_type) {
      $meta_field_name = $term_type . 'Field';
      if ($this->$meta_field_name) {
        $field_name = $this->$meta_field_name;
        $value_name = $term_type . '_value';
        // Shortcut - if the counts match, don't need to dig deeper
        $field_values = field_get_items('node', $node, $field_name);
        if (!empty($field_values)) {
          $node_count = count($field_values);
        }
        else {
          $node_count = 0;
        }
        if (isset($row->$term_type)) {
          $row_count = count($row->$term_type);
        }
        else {
          $row_count = 0;
        }
        if ($node_count != $row_count) {
          $vid_field = $term_type . 'Vid';
          $vid = $this->$vid_field;
          // Get any terms already in the field
          $done_terms = array();
          if (is_array($field_values)) {
            foreach ($field_values as $value_array) {
              $terms = taxonomy_term_load_multiple($value_array);
              foreach ($terms as $term) {
                $done_terms[] = $term->name;
              }
            }
          }
          if (isset($row->$value_name)) {
            $diff = array_diff($row->$value_name, $done_terms);
            $field_language = field_language('node', $node, $field_name);
            foreach ($diff as $new_term_name) {
              // Let's see if the term already exists
              $matches = taxonomy_term_load_multiple(array(),
                array('name' => trim($new_term_name), 'vid' => $vid));
              if ($matches) {
                $node->{$field_name}[$field_language][] = array('tid' => key($matches));
              }
              else {
                $term = new stdClass;
                $term->name = $new_term_name;
                $term->vid = $vid;
                taxonomy_term_save($term);
                $node->{$field_name}[$field_language][] = array('tid' => $term->tid);
              }
            }
          }
        }
      }
    }

    // If we have an attached podcast, replace [powerpress] in the body with
    // a link to the audio file.
    $podcast_field = variable_get('wordpress_migrate_podcast_field', '');
    if ($podcast_field && isset($node->$podcast_field)) {
      $podcast_fid = $node->{$podcast_field}[LANGUAGE_NONE][0]['fid'];
      if ($podcast_fid) {
        $podcast = file_load($podcast_fid);
        if ($podcast) {
          // file_create_url() gives us a full URL, which when running from
          // drush will often start with http://default/, which is not what we
          // want. Strip the prefix for a relative link.
          $url = file_create_url($podcast->uri);
          $url = str_replace($GLOBALS['base_url'] . '/', '/', $url);
          $link = '<p><a href="' . $url .
                  '"><img src="/modules/file/icons/audio-x-generic.png" /></a></p>';
          $body_language = field_language('node', $node, 'body');
          $node->body[$body_language][0]['value'] = str_replace('[powerpress]',
            $link, $node->body[$body_language][0]['value']);
        }
      }
    }
  }

  /**
   * Complete node - called just after node_save().
   *
   * @param stdClass $node
   * @param stdClass $row
   */
  public function complete(stdClass $node, stdClass $row) {
    // Remember the nid of any node where we weren't able to resolve ?p=23
    // links yet - by the time the page migration's postImport() is called, we
    // should have resolved all references.
    if ($this->linksNeedFixing) {
      self::$linksToFix[] = $node->nid;
    }
  }
}

/**
 * Implementation of WordPressMigration, for blog entries
 */
class WordPressBlogEntry extends WordPressItemMigration {
  public function __construct(array $arguments = array()) {
    $arguments['post_type'] = 'post';
    $arguments['bundle'] = variable_get('wordpress_migrate_post_type', '');
    parent::__construct($arguments);
  }
}

/**
 * Implementation of WordPressMigration, for pages
 */
class WordPressPage extends WordPressItemMigration {
  public function __construct(array $arguments = array()) {
    $arguments['post_type'] = 'page';
    $arguments['bundle'] = variable_get('wordpress_migrate_page_type', '');
    parent::__construct($arguments);
    $this->dependencies += array($this->generateMachineName('WordPressBlogEntry'));
  }

  /**
   * Called after completion of the page migration. We review any nodes with links
   * that couldn't be resolved at migration time (presumably because they refer to
   * nodes not yet migrated) and see if we can resolve them now.
   */
  public function postImport() {
    parent::postImport();
    foreach (self::$linksToFix as $nid) {
      $node = node_load($nid);
      // Maintain the original update datestamp
      $changed = $node->changed;
      $node->body[LANGUAGE_NONE][0]['value'] = $this->fixLocalLinks($node->body[LANGUAGE_NONE][0]['value']);
      $this->disablePathauto($node);
      node_save($node);
      db_update('node')
        ->fields(array('changed' => $changed))
        ->condition('nid', $node->nid)
        ->execute();
    }
  }
}
