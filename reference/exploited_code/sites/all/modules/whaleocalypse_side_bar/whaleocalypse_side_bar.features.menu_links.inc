<?php
/**
 * @file
 * whaleocalypse_side_bar.features.menu_links.inc
 */

/**
 * Implements hook_menu_default_menu_links().
 */
function whaleocalypse_side_bar_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: menu-sidebar-menu_archive:archive
  $menu_links['menu-sidebar-menu_archive:archive'] = array(
    'menu_name' => 'menu-sidebar-menu',
    'link_path' => 'archive',
    'router_path' => 'archive',
    'link_title' => 'Archive',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
      'identifier' => 'menu-sidebar-menu_archive:archive',
    ),
    'module' => 'menu',
    'hidden' => 0,
    'external' => 0,
    'has_children' => 0,
    'expanded' => 0,
    'weight' => 0,
    'customized' => 1,
  );
  // Exported menu link: menu-sidebar-menu_random:random
  $menu_links['menu-sidebar-menu_random:random'] = array(
    'menu_name' => 'menu-sidebar-menu',
    'link_path' => 'random',
    'router_path' => 'random',
    'link_title' => 'Random',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
      'identifier' => 'menu-sidebar-menu_random:random',
    ),
    'module' => 'menu',
    'hidden' => 0,
    'external' => 0,
    'has_children' => 0,
    'expanded' => 0,
    'weight' => 0,
    'customized' => 1,
  );
  // Exported menu link: menu-sidebar-menu_subscribe:main.xml
  $menu_links['menu-sidebar-menu_subscribe:main.xml'] = array(
    'menu_name' => 'menu-sidebar-menu',
    'link_path' => 'main.xml',
    'router_path' => 'main.xml',
    'link_title' => 'Subscribe',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
      'identifier' => 'menu-sidebar-menu_subscribe:main.xml',
    ),
    'module' => 'menu',
    'hidden' => 0,
    'external' => 0,
    'has_children' => 0,
    'expanded' => 0,
    'weight' => 0,
    'customized' => 1,
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Archive');
  t('Random');
  t('Subscribe');


  return $menu_links;
}
