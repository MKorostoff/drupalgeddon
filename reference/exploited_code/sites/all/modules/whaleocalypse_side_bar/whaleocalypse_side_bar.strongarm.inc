<?php
/**
 * @file
 * whaleocalypse_side_bar.strongarm.inc
 */

/**
 * Implements hook_strongarm().
 */
function whaleocalypse_side_bar_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'views_defaults';
  $strongarm->value = array(
    'delete_comments' => TRUE,
    'archive' => TRUE,
  );
  $export['views_defaults'] = $strongarm;

  return $export;
}
