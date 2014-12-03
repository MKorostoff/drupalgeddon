<?php

/**
 * @file
 *
 * Support for migrating authors from a WordPress blog into Drupal.
 */

/**
 * Implementation of WordPressMigration, for authors
 */
class WordPressAuthor extends WordPressMigration {
  /**
   * Set it up
   */
  public function __construct(array $arguments = array()) {
    parent::__construct($arguments);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'author_login' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'WordPress author username',
        )
      ),
      WordPressAuthorDestination::getKeySchema()
    );

    $fields = array(
      'author_id' => 'Unique WordPress ID of the author',
      'author_login' => 'Username (for login) of the author',
      'author_email' => 'Author email address',
      'author_display_name' => 'Displayed author name',
      'author_first_name' => 'Author first name',
      'author_last_name' => 'Author last name',
    );

    // Construct the source and destination objects.
    $source_options = array(
      'reader_class' => 'WordPressXMLReader',
      'cache_counts' => TRUE,
    );
    $this->source = new WordPressSourceXML($this->wxrFile, '/rss/channel/wp:author',
      'wp:author_login', $fields, $source_options);
    $this->destination = new WordPressAuthorDestination();

    // The basic mappings
    $this->addFieldMapping('name', 'author_login')
         ->dedupe('users', 'name');
    $this->addFieldMapping('mail', 'author_email');
    $this->addFieldMapping('roles')
         ->defaultValue(DRUPAL_AUTHENTICATED_RID);
    $this->addFieldMapping('status')
         ->defaultValue(1);

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('pass', 'theme', 'signature',
      'signature_format', 'created', 'access', 'login', 'timezone', 'language',
      'picture', 'init', 'is_new'));
    if (module_exists('path')) {
      $this->addUnmigratedDestinations(array('path'));
    }

    // Unmapped source fields
    $this->addUnmigratedSources(array('author_display_name', 'author_first_name',
                                'author_last_name', 'author_id'));
  }
}

class WordPressAuthorDestination extends MigrateDestinationUser {
  /**
   * Override of MigrateDestinationUser::import().
   *
   * On initial import, if the email address already exists we want to link to
   * that account, and remember that we did so. On updates, if we see that it
   * is a linked account, we don't want to update it.
   *
   * @param stdClass $account
   * @param stdClass $row
   */
  public function import(stdClass $account, stdClass $row) {
    if (isset($row->migrate_map_destid1)) {
      // Updating the account - if it's linked, just return the ID of the linked
      // account
      $uid = db_select('wordpress_migrate_linked_authors', 'a')
             ->fields('a', array('uid'))
             ->condition('mail', $account->mail)
             ->execute()
             ->fetchField();
      if ($uid) {
        $this->numUpdated++;
        return array($uid);
      }
    }
    else {
      // Initial import - if already in the users table, add to our linked_authors
      // list and return the uid of the existing account.
      $uid = db_select('users', 'u')
             ->fields('u', array('uid'))
             ->condition('mail', $account->mail)
             ->execute()
             ->fetchField();
      if (!$uid) {
        // This user does not yet exist on the site. See if we're supposed to
        // create it.
        $create_new_users = variable_get('wordpress_migrate_create_new_users', 1);
        if (!$create_new_users) {
          // Link this content to the default user chosen in the import settings.
          $uid = variable_get('wordpress_migrate_default_author_uid', 1);
        }
      }
      if ($uid) {
        db_merge('wordpress_migrate_linked_authors')
          ->key(array('mail' => $account->mail))
          ->fields(array('uid' => $uid))
          ->execute();
        $this->numCreated++;
        return array($uid);
      }
    }
    // If there's no linkage, do the normal thing and create a new user.
    return parent::import($account, $row);
  }

  /**
   * Override of MigrateDestinationUser::bulkRollback().
   *
   * We want to make sure we don't delete any users who existed before we first
   * imported.
   *
   * @param $uids
   *  Array of user IDs to be deleted.
   */
  public function bulkRollback(array $uids) {
    // Make sure we only attempt each uid once - if multiple authors had the
    // same email address in WordPress, the corresponding Drupal uid will be in
    // the array twice, we'll delete the linked_authors row the first time we
    // see it, and the second pass on that uid will delete the account.
    $uids = array_unique($uids, SORT_NUMERIC);
    $delete_uids = array();
    foreach ($uids as $uid) {
      $mail = db_select('wordpress_migrate_linked_authors', 'a')
              ->fields('a', array('mail'))
              ->condition('uid', $uid)
              ->execute()
              ->fetchField();
      if ($mail) {
        db_delete('wordpress_migrate_linked_authors')
          ->condition('mail', $mail)
          ->execute();
      }
      else {
        $delete_uids[] = $uid;
      }
    }
    parent::bulkRollback($delete_uids);
  }
}
