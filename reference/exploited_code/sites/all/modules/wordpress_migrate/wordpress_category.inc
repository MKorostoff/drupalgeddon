<?php

/**
 * @file
 *
 * Support for migrating categories from a WordPress blog into Drupal.
 */

/**
 * Implementation of WordPressMigration, for categories
 */
class WordPressCategory extends WordPressMigration {
  /**
   * Set it up
   */
  public function __construct(array $arguments = array()) {
    parent::__construct($arguments);

    // The "nicename" is what we would call the machine name, the unique
    // identifier of a category.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'category_nicename' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'WordPress category machine name',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    $fields = array(
        'category_nicename' => 'Unique "machine name" of the category',
        'category_parent' => 'Category parent (nicename?)',
        'cat_name' => 'User-friendly category name',
        'category_description' => 'Description of category',
    );

    // Construct the source and destination objects.
    $source_options = array(
      'reader_class' => 'WordPressXMLReader',
      'cache_counts' => TRUE,
    );
    $this->source = new WordPressSourceXML($this->wxrFile, '/rss/channel/category',
      'wp:category_nicename', $fields, $source_options);
    $this->destination = new MigrateDestinationTerm(
      variable_get('wordpress_migrate_category_vocabulary', ''));

    // The basic mappings
    $this->addFieldMapping('name', 'cat_name');
    $this->addFieldMapping('description', 'category_description');
    $this->addFieldMapping('parent', 'category_parent')
         ->sourceMigration($this->machineName);

    // Unmapped destination fields
    $this->addFieldMapping('parent_name');
    $this->addFieldMapping('format');
    $this->addFieldMapping('weight');
    $this->addFieldMapping('path');
  }
}
