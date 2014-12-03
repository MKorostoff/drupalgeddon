<?php
/**
 * @file
 * whaleocalypse_side_bar.features.menu_custom.inc
 */

/**
 * Implements hook_menu_default_menu_custom().
 */
function whaleocalypse_side_bar_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-sidebar-menu.
  $menus['menu-sidebar-menu'] = array(
    'menu_name' => 'menu-sidebar-menu',
    'title' => 'Sidebar Menu',
    'description' => '',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Sidebar Menu');


  return $menus;
}
