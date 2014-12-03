<?php

/**
 * Implementation of hook_migrate_ui_wizard().
 *
 * @return array
 */
function wordpress_migrate_migrate_ui_wizard() {
  return array(
    'wordpress' => array(
      'source_title' => t('WordPress'),
      'filename' => drupal_get_path('module', 'wordpress_migrate') . '/wordpress_migrate.migrate.inc',
      'overview' => t(''),
      'steps' => array(
        1 => array(
          'name' => t('Credentials'),
          'form' => 'wordpress_migrate_credentials',
        ),
      ),
    ),
  );
}

/*
 * Implementation of hook_migrate_api().
 */
function wordpress_migrate_migrate_api() {
  $api = array(
    'api' => 2,
  );
  return $api;
}
