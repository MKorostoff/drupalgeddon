<?php

/**
 * @file
 * Drush support for the wordpress_migrate module
 */

/**
 * Implementation of hook_drush_help().
 */
function wordpress_migrate_drush_help($section) {
  switch ($section) {
    case 'drush:wordpress-migrate-import':
      return dt('Import a WordPress blog');
  }
}

/**
 * Implementation of hook_drush_command().
 */
function wordpress_migrate_drush_command() {
  static $commands = FALSE;

  $items['wordpress-migrate-import'] = array(
    'description' => 'Import a WordPress blog',
    'arguments' => array(
      'filename' => 'Filename of blog export to import',
    ),
    'examples' => array(
      'wordpress-migrate-import public://wordpress/blog.xml' => 'Import blog data',
    ),
    'drupal dependencies' => array('migrate'),
  );
  return $items;
}

function drush_wordpress_migrate_import($filename) {
  // Capture non-informational output for mailing
  ob_start();
  try {
    $blog = wordpress_migrate_blog($filename);
    $mail_key = 'import_complete';
    $migrations = $blog->migrations();
    foreach ($migrations as $migration) {
      $result = $migration->processImport();
      if ($result != MigrationBase::RESULT_COMPLETED) {
        $mail_key = 'import_failure';
        break;
      }
    }
  }
  catch (Exception $e) {
    drush_log($e->getMessage(), 'error');
    $mail_key = 'import_failure';
  }

  // Notify user
  if (variable_get('wordpress_migrate_notification', 0) == 1) {
    $account = user_load($blog->getUid());
    $params['account'] = $account;
    $params['output'] = ob_get_contents();
    ob_end_flush();
    drupal_mail('wordpress_migrate', $mail_key, $account->mail,
      user_preferred_language($account), $params);
  }
}
