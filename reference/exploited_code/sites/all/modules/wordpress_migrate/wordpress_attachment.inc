<?php

/**
 * Override WordPressItemSource, to add additional fields for attachments.
 */
class WordPressAttachmentSource extends WordPressItemSource {
  /**
   * List of additional source fields for attachments.
   *
   * @var array
   */
  protected $attachmentFields = array(
    'attachment_url' => 'The URL of the attached file',
    '_wp_attached_file' => 'Local (to WordPress) filespec',
    '_wp_attachment_metadata' => 'Serialized metadata (image size etc.)',
  );

  /**
   * Simple initialization.
   *
   */
  public function __construct($filename, $cache_key) {
    parent::__construct($filename, 'attachment', $cache_key);
    $this->fields += $this->attachmentFields;
  }
}

/**
 * Implementation of WordPressMigration, for attachments
 */
class WordPressAttachment extends WordPressMigration {
  /**
   * Set it up
   */
  public function __construct(array $arguments = array()) {
    parent::__construct($arguments);

    $this->dependencies = array($this->generateMachineName('WordPressBlogEntry'),
      $this->generateMachineName('WordPressPage'));

    // Construct the source object.
    $this->source = new WordPressAttachmentSource($this->wxrFile, $this->machineName);
    $this->destination = new MigrateDestinationFile();

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
      MigrateDestinationFile::getKeySchema()
    );

    $this->addFieldMapping('value', 'attachment_url');
    $this->addFieldMapping('destination_file', 'attachment_url')
         ->callbacks(array($this, 'fixDestinationFile'));
    $this->addFieldMapping('uid', 'creator')
         ->description('Use matching username if any, otherwise current user');
    $this->addFieldMapping('timestamp', 'post_date');
    $this->addFieldMapping(NULL, 'post_parent')
         ->description('Mapping is handled dynamically');

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('path', 'destination_dir',
                                     'file_replace', 'preserve_files', 'source_dir'));

    // Unmapped source fields
    $this->addUnmigratedSources(array('link', 'guid', 'description', 'excerpt',
      'post_id', 'comment_status', 'ping_status', 'post_name', 'status',
      'menu_order', 'post_type', 'post_password', 'is_sticky', 'tag', 'category',
      '_wp_attached_file', '_wp_attachment_metadata', 'title', 'content'));
    $this->addFieldMapping(NULL, 'pubDate')
         ->description('Use post_date')
         ->issueGroup(t('DNM'));
    $this->addFieldMapping(NULL, 'post_date_gmt')
         ->description('Use post_date')
         ->issueGroup(t('DNM'));
  }

  protected function fixDestinationFile($value) {
    $url_info = parse_url($value);
    return $url_info['path'];
  }
  /**
   * Data manipulations to be performed before migrate module applies mappings.
   *
   * @param stdClass $row
   * @return string
   */
  public function prepareRow($row) {
    // Skip any of the wrong post type
    if ($row->post_type != 'attachment') {
      return FALSE;
    }

    // If incoming date is zero (indicates unpublished content), use the current time
    if ($row->post_date == '0000-00-00 00:00:00') {
      $row->post_date = time();
    }

    // Sometimes URLs have doubled-up slashes - reduce to a single.
    $pieces = explode('://', $row->attachment_url);
    if (count($pieces) == 1) {
      $row->attachment_url = str_replace('//', '/', $row->attachment_url);
    }
    else {
      $row->attachment_url = $pieces[0] . '://' . str_replace('//', '/', $pieces[1]);
    }

    // Make sure we actually have a URL
    if ($row->attachment_url) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Prepare node - called just before node_save().
   *
   * @param stdClass $node
   * @param stdClass $row
   */
  public function prepare(stdClass $file, stdClass $row) {
    // Match creator username to Drupal username if possible; otherwise, use
    // the user that initiated the import
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
    $file->uid = $user_map[$row->creator];
  }

  /**
   * Called after file object is saved - maintain a mapping from the URL on the
   * original WordPress blog to the URI in Drupal.
   *
   * @param stdClass $file
   * @param stdClass $row
   */
  public function complete(stdClass $file, stdClass $row) {
    // Find the parent nid
    $row->post_parent = $this->handleSourceMigration($this->dependencies, $row->post_parent);
    // Populate the parent node's attachment field, if applicable
    $attachment_field = variable_get('wordpress_migrate_attachment_field', '');
    if ($attachment_field && $row->post_parent && $file->fid) {
      if ($row->post_parent) {
        $parent_node = node_load($row->post_parent);
        if ($parent_node) {
          $file->display = 1;
          $file->description = '';
          // node_load will give us an empty value here, so adding a value with
          // [] will mess things up - remove it.
          if (isset($parent_node->{$attachment_field}[LANGUAGE_NONE]) &&
              !is_array($parent_node->{$attachment_field}[LANGUAGE_NONE][0])) {
            unset($parent_node->{$attachment_field}[LANGUAGE_NONE][0]);
          }
          $parent_node->{$attachment_field}[LANGUAGE_NONE][] = (array)$file;
          $this->disablePathauto($parent_node);
          // Maintain the original update datestamp
          $changed = $parent_node->changed;
          node_save($parent_node);
          db_update('node')
            ->fields(array('changed' => $changed))
            ->condition('nid', $parent_node->nid)
            ->execute();
          file_usage_add($file, 'file', 'node', $parent_node->nid);
        }
      }
    }

    // Record the attachment, so we can fix up the parent bodies later
    $fields['new_uri'] = parse_url(file_create_url($file->uri), PHP_URL_PATH);
    $fields['new_fid'] = $file->fid;
    if ($row->post_parent) {
      $fields['parent_nid'] = $row->post_parent;
    }
    db_merge('wordpress_migrate_attachment')
      ->key(array('blog_id' => $this->blog->getBlogID(),
                  'original_url' => $row->attachment_url))
      ->fields($fields)
      ->execute();

    // If media_gallery is enabled, add this image to the user's gallery.
    // Lazy-create the gallery node if it doesn't already exist
    // TODO: Needs generalization, takes for granted blog module
    // TODO: Cache fids to add, do them all at once
    if (module_exists('media_gallery')) {
      global $user;
      $blog_title = t("@name's blog", array('@name' => format_username($user)));
      $gallery_nid = db_select('node', 'n')
                     ->fields('n', array('nid'))
                     ->condition('type', 'media_gallery')
                     ->condition('title', $blog_title)
                     ->execute()
                     ->fetchField();
      if ($gallery_nid) {
        $gallery_node = node_load($gallery_nid);
      }
      else {
        $gallery_node = new stdClass;
        $gallery_node->type = 'media_gallery';
        $gallery_node->title = $blog_title;
        $gallery_node->uid = $user->uid;
        $gallery_node->language = LANGUAGE_NONE;
      }
      $gallery_node->media_gallery_media[LANGUAGE_NONE][] = array('fid' => $file->fid);
      $this->disablePathauto($gallery_node);
      node_save($gallery_node);
    }
  }

  /**
   * Called after all attachments are imported - fix up references to the imported
   * attachments in node bodies.
   */
  public function postImport() {
    parent::postImport();
    migrate_instrument_start('WordPressAttachment postImport');
    // We'd like to just go through nodes with attachments (i.e., loop
    // through wordpress_migrate_attachment), but a node may reference
    // other posts' attachments without having any itself.
    $map_table = 'migrate_map_' . strtolower($this->generateMachineName('WordPressBlogEntry'));
    // Replicate the Migrate truncation to make sure we match the real map name.
    $map_table = drupal_substr($map_table, 0, 63);
    $nids = db_select($map_table, 'be')
            ->fields('be', array('destid1'))
            ->isNotNull('destid1')
            ->execute();
    foreach ($nids as $nid_row) {
      $node = node_load($nid_row->destid1);
      if ($node && is_array($node->body)) {
        foreach ($node->body as $language => $body) {
          $body = $body[0]['value'];
          // If we have the media module, rewrite the img tags to media tags if
          // we can.
          if (module_exists('media')) {
            $body = preg_replace_callback('|<img +(.*?)>|i',
              array($this, 'replaceImgs'), $body);
          }
          // See if any remaining images can be directly replaced.
          $result = db_select('wordpress_migrate_attachment', 'a')
                              ->fields('a', array('original_url', 'new_uri', 'new_fid'))
                              ->condition('parent_nid', $nid_row->destid1)
                              ->execute();
          foreach ($result as $row) {
            $body = str_replace($row->original_url, $row->new_uri, $body);
          }
          $node->body[$language][0]['value'] = $body;
        }

        // Maintain the original update datestamp
        $changed = $node->changed;
        $this->disablePathauto($node);
        node_save($node);
        db_update('node')
          ->fields(array('changed' => $changed))
          ->condition('nid', $node->nid)
          ->execute();
      }
    }

    migrate_instrument_stop('WordPressAttachment postImport');
  }

  /**
   * If we have an image reference, replace it with media tags.
   *
   * @param array $matches
   */
  protected function replaceImgs(array $matches) {
    // Default to the original <img> tag.
    $result = $matches[0];

    // The src parameter is required
    if (preg_match('|src=[\'"](.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
    }
    else {
      return $result;
    }

    // Get the fid, if any. If none, let the img tag stand
    $fid = db_select('wordpress_migrate_attachment', 'a')
           ->fields('a', array('new_fid'))
           ->condition('original_url', $src)
           ->execute()
           ->fetchField();
    if (!$fid) {
      // We have an image that wasn't pulled over as an attachment - if it's not
      // on the existing WordPress site, leave it be; if it is, copy it directly
      // and create the file entity.
      $blog_url = $this->blog->getBlogUrl();
      $prefix_pattern = '|^' . $blog_url. '/|';
      if (preg_match($prefix_pattern, $src)) {
        $file = array(
          'destination_file' => preg_replace($prefix_pattern, '', $src),
          'file_replace' => FILE_EXISTS_RENAME,
        );
        $source = new MigrateFileUri($file);
        // @todo: uid from parent node
        $file = $source->processFile($src, 1);
        $fid = $file->fid;
      }
      else {
        return $result;
      }
    }

    // Extract the width, height, and classes for the media tag, if present
    $attributes = array(
      'class' => 'media-image',
      'typeof' => 'foaf:Image',
      'style' => '',
    );
    if (preg_match('|width=[\'"](.*?)[\'"]|i', $result, $width_matches)) {
      $attributes['width'] = $width_matches[1];
    }
    if (preg_match('|height=[\'"](.*?)[\'"]|i', $result, $height_matches)) {
      $attributes['height'] = $height_matches[1];
    }
    if (preg_match('|title=[\'"](.*?)[\'"]|i', $result, $title_matches)) {
      $attributes['title'] = $title_matches[1];
    }
    if (preg_match('|alt=[\'"](.*?)[\'"]|i', $result, $alt_matches)) {
      $attributes['alt'] = $alt_matches[1];
    }
    if (preg_match('|class=[\'"](.*?)[\'"]|i', $result, $class_matches)) {
      $attributes['class'] .= ' ' . $class_matches[1];
    }

    // Build the media tag
    $img_info = array(
      'type' => 'media',
      'view_mode' => 'media_large',
      'fid' => $fid,
      'attributes' => $attributes,
    );

    $result = '[[' . drupal_json_encode($img_info) . ']]';

    return $result;
  }

  /**
   * Remove all attachment records
   */
  public function postRollback() {
    db_delete('wordpress_migrate_attachment')
    ->condition('blog_id', $this->blog->getBlogID())
    ->execute();
  }
}
