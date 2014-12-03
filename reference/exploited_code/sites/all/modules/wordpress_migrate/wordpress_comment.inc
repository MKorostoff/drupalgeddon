<?php

/**
 * @file
 *
 * Support for migrating comments from a WordPress blog into Drupal.
 */

/**
 * Override of MigrateXMLReader, so we can track the parent posts for
 * comments.
 */
class WordPressCommentXMLReader extends MigrateXMLReader {
  /**
   * The last post ID we've seen - saved so we can apply it to comments.
   * @var int
   */
  protected $postId;

  /**
   * Implementation of Iterator::next(). We need to preserve the ID of
   * the parent element.
   *
   * @return void
   */
  public function next() {
    migrate_instrument_start('WordPressCommentXMLReader::next');
    $this->currentElement = $this->currentId = NULL;

    // Loop over each node in the XML file, looking for elements at a path
    // matching the input query string (represented in $this->elementsToMatch).
    while ($this->reader->read()) {
      if ($this->reader->nodeType == XMLREADER::ELEMENT) {
        $this->currentPath[$this->reader->depth] = $this->reader->localName;
        // Save the last post_id, so comments can use it to find their parent
        if ($this->reader->name == 'wp:post_id') {
          $this->postId = WordPressBlog::readString($this->reader);
        }
        if ($this->currentPath == $this->elementsToMatch) {
          // We're positioned to the right element path - if filtering on an
          // attribute, check that as well before accepting this element.
          if (empty($this->attributeName) ||
              ($this->reader->getAttribute($this->attributeName) == $this->attributeValue)) {
            // We've found a matching element - get a SimpleXML object representing it.
            // We must associate the DOMNode with a DOMDocument to be able to import
            // it into SimpleXML.
            // Despite appearances, this is almost twice as fast as
            // simplexml_load_string($this->readOuterXML());
            $node = $this->reader->expand();
            if ($node) {
              $dom = new DOMDocument();
              $node = $dom->importNode($node, TRUE);
              $dom->appendChild($node);
              $this->currentElement = simplexml_import_dom($node);
              if ($this->reader->name == 'wp:comment') {
                $this->currentElement->post_id = $this->postId;
              }
              $idnode = $this->currentElement->xpath($this->idQuery);
              $this->currentId = (string)reset($idnode);
              break;
            }
            else {
              foreach (libxml_get_errors() as $error) {
                $error_string = MigrateItemsXML::parseLibXMLError($error);
                if ($migration = Migration::currentMigration()) {
                  $migration->saveMessage($error_string);
                }
                else {
                  Migration::displayMessage($error_string);
                }
              }
            }
          }
        }
      }
      elseif ($this->reader->nodeType == XMLREADER::END_ELEMENT) {
        // Trim currentPath as we exit each element
        unset($this->currentPath[$this->reader->depth]);
      }
    }

    // Copy of code from WordPressXMLReader - turn the SimpleXML object
    // into a flat list of fields.
    if (is_a($this->currentElement, 'SimpleXMLElement')) {
      $item = $this->currentElement;
      $currentRow = new stdClass;
      // Pull non-namespaced items
      foreach ($item as $name => $value) {
        $currentRow->$name = (string)$value;
      }
      $namespaces = $item->getNameSpaces(TRUE);
      foreach ($namespaces as $ns => $nsuri) {
        $item_ns = $item->children($namespaces[$ns]);
        foreach ($item_ns as $name => $value) {
          // Special-case content:encoded and excerpt:encoded, which otherwise
          // would both be saved as "encoded"
          if ($name == 'encoded') {
            $currentRow->$ns = (string)$value;
          }
          else {
            $currentRow->$name = (string)$value;
          }
        }
      }
      $this->currentElement = $currentRow;
    }
    migrate_instrument_stop('WordPressCommentXMLReader::next');
  }
}

/**
 * Implementation of WordPressMigration, for comments
 */
class WordPressComment extends WordPressMigration {
  /**
   * Set it up
   */
  public function __construct(array $arguments = array()) {
    $arguments['post_type'] = 'comment_node_' . variable_get('wordpress_migrate_post_type', 'blog');
    parent::__construct($arguments);
    $this->dependencies = array($this->generateMachineName('WordPressBlogEntry'));
    $this->softDependencies = array($this->generateMachineName('WordPressPage'));

    // comment_id is the unique ID of items in WordPress
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'comment_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'WordPress comment ID',
        )
      ),
      MigrateDestinationComment::getKeySchema()
    );

    $fields = array(
        'post_id' => 'Unique ID of the item the comment is attached to',
        'comment_id' => 'Unique ID of the comment',
        'comment_author' => 'Name of comment author',
        'comment_author_email' => 'Email address of comment author',
        'comment_author_url' => 'URL of comment author',
        'comment_author_IP' => 'IP address from which comment was posted',
        'comment_date' => 'Date of comment (what timezone?)',
        'comment_date_gmt' => 'Date of comment (GMT)',
        'comment_content' => 'Body of comment',
        'comment_approved' => '1/0/spam - spam comments will not be imported',
        'comment_type' => '?',
        'comment_parent' => 'comment_id (?) of parent comment',
        'comment_user_id' => 'WordPress user ID of commenter (?)',
      );

    $source_options = array(
      'reader_class' => 'WordPressCommentXMLReader',
      'cache_counts' => TRUE,
    );
    $this->source = new WordPressSourceXML($this->wxrFile, '/rss/channel/item/comment',
      'wp:comment_id', $fields, $source_options);
    $this->destination = new MigrateDestinationComment($arguments['post_type']);

    // The basic mappings
    $this->addFieldMapping('nid', 'post_id')
         ->sourceMigration(array($this->generateMachineName('WordPressBlogEntry'), $this->generateMachineName('WordPressPage')));
    $this->addFieldMapping('pid', 'comment_parent')
         ->sourceMigration($this->generateMachineName('WordPressComment'));
    $this->addFieldMapping('uid')
         ->description('Use email to match Drupal account; if no match, default to anonymous');
    $this->addFieldMapping('subject')
         ->description('No comment subjects in WP')
         ->issueGroup('DNM');
    $this->addFieldMapping('hostname', 'comment_author_IP');
    $this->addFieldMapping('created', 'comment_date');
    $this->addFieldMapping('changed', 'comment_date');
    $this->addFieldMapping(NULL, 'comment_date_gmt')
         ->description('Using comment_date')
         ->issueGroup('DNM');
    $this->addFieldMapping('status', 'comment_approved')
         ->description('Do not import those with values of "spam"');
    $this->addFieldMapping('thread')
         ->issueGroup('DNM');
    $this->addFieldMapping('name', 'comment_author')
         ->callbacks(array($this, 'truncateName'));
    $this->addFieldMapping('mail', 'comment_author_email');
    $this->addFieldMapping('homepage', 'comment_author_url');
    $this->addFieldMapping('language');
    $this->addFieldMapping('path');
    $this->addFieldMapping(NULL, 'comment_id')
         ->description('Source primary key')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'comment_user_id')
         ->description('Always 0?')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'comment_type')
         ->description('Always empty?')
         ->issueGroup('DNM');
    $text_format = variable_get('wordpress_migrate_text_format_comment', 'filtered_html');
    $arguments = array('format' => $text_format);
    $this->addFieldMapping('comment_body', 'comment_content')
         ->arguments($arguments);
  }

  /**
   * Names from Wordpress can be longer than 60 characters, truncate them.
   */
  protected function truncateName($value) {
    $value = substr($value, 0, 60);
    return $value;
  }

  public function prepareRow($row) {
    // Reject spam
    if ($row->comment_approved == 'spam') {
      return FALSE;
    }

    // Ignore pingbacks
    if ($row->comment_type == 'pingback') {
      return FALSE;
    }
    $row->comment_date = strtotime($row->comment_date);

    return TRUE;
  }

  /**
   * Prepare comment - called just before comment_save().
   *
   * @param stdClass $comment
   * @param stdClass $row
   */
  public function prepare(stdClass $comment, stdClass $row) {
    // Match creator email to Drupal account if possible; otherwise, use anonymous
    static $drupal_static_fast;
    if (!isset($drupal_static_fast)) {
      $drupal_static_fast['user_map'] = &drupal_static(__FUNCTION__);
    }
    $user_map = &$drupal_static_fast['user_map'];
    if (!isset($user_map[$row->comment_author_email])) {
      $user_map[$row->comment_author_email] = db_select('users', 'u')
                                              ->fields('u', array('uid'))
                                              ->condition('mail', $row->comment_author_email)
                                              ->execute()
                                              ->fetchField();
      if (!$user_map[$row->comment_author_email]) {
        $user_map[$row->comment_author_email] = 0;
      }
    }
    $comment->uid = $user_map[$row->comment_author_email];
  }
}
