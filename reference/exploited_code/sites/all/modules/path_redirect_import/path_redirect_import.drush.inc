<?php

/**
 * @file
 *   Drush commands for the pat_redirect_import module
 */

/**
 * Implements hook_drush_commands().
 */
function path_redirect_import_drush_command() {
  $items = array();

  $items['path-redirect-import'] = array(
    'description' => "Import list of URL redirects",
    'arguments' => array(
      'file' => 'Path to the CSV file that includes the redirect info',
    ),
    'options' => array(
      'no-headers' =>
        'Indicates if the first line must be taken into account. Default TRUE',
      'override' =>
        'Indicates if existent redirect should be overriden or not. Default FALSE',
      'status-code' =>
        'Indicates the redirection status code. Default 301',
      'delimiter' =>
        'Indicates the delimiter between fields. Default \',\'',
    ),
    'examples' => array(
      'drush -l http://example.com path-redirect-import sites/all/modules/custom/import_tienda/import/consulta_fabricantes_espana.csv',
      'drush -l http://example.com primport sites/all/modules/custom/import_tienda/import/consulta_fabricantes_espana.csv',
    ),
    'aliases' => array('primport'),
  );

  return $items;
}

/**
 * Implements the path-redirect-import drush command.
 */
function drush_path_redirect_import($file) {

  if (!file_exists($file)) {
    drush_log("File $file doesn't exist \n", 'error');
    exit;
  }

  $options = array();
  $option_values = array('no-headers', 'override', 'status-code');
  foreach ($option_values as $value) {
    if (drush_get_option($value)) {
      $options[$value] = drush_get_option($value);
    }
  }
  $result = path_redirect_import_read_file($file, $options);

  if ($result['success']) {
    drush_log(dt(implode(PHP_EOL, $result['message'])), 'success');
  }
  else {
    drush_log(dt(implode(PHP_EOL, $result['message'])), 'error');
  }
}
