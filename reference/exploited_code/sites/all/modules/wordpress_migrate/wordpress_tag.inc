<?php

/**
 * @file
 *
 * Support for migrating tags from a WordPress blog into Drupal.
 */

/**
 * Implementation of WordPressMigration, for tags
 */
class WordPressTag extends WordPressMigration {
  /**
   * Set it up
   */
  public function __construct(array $arguments = array()) {
    parent::__construct($arguments);

    // The slug is what we would call the machine name, the unique
    // identifier of a tag.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'tag_slug' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'WordPress tag machine name',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    $fields = array(
        'tag_slug' => 'Unique "machine name" of the tag',
        'tag_name' => 'User-friendly tag name',
        'tag_description' => 'Description of tag',
    );

    // Construct the source and destination objects.
    $source_options = array(
      'reader_class' => 'WordPressXMLReader',
      'cache_counts' => TRUE,
    );
    $this->source = new WordPressSourceXML($this->wxrFile, '/rss/channel/tag',
      'wp:tag_slug', $fields, $source_options);
    $this->destination = new MigrateDestinationTerm(
      variable_get('wordpress_migrate_tag_vocabulary', ''));

    // The basic mappings
    $this->addFieldMapping('name', 'tag_name');
    $this->addFieldMapping('description', 'tag_description');
    $this->addFieldMapping('parent_name')
         ->issueGroup('DNM');

    // Unmapped destination fields
    $this->addFieldMapping('parent');
    $this->addFieldMapping('format');
    $this->addFieldMapping('weight');
    $this->addFieldMapping('path');
  }
}
