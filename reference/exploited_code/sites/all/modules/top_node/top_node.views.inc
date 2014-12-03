<?php
// $Id: top_node.views.inc,v 1.1.2.1 2010/01/17 19:26:14 eaton Exp $

/**
 * Implementation of hook_views_plugins().
 */
function top_node_views_plugins() {
  $plugins = array(
    'display' => array(
      'single_node_page' => array(
        'parent' => 'page',
        'title' => t('Node page'),
        'help' => t('Display a page with a single node.'),
        'handler' => 'top_node_plugin_display_single_node_page',
        'theme' => 'views_view',
        'uses hook menu' => TRUE,
        'use ajax' => FALSE,
        'use more' => FALSE,
        'use pager' => FALSE,
        'accept attachments' => FALSE,
      ),
    ),
    'style' => array(
      'full_node' => array(
        'no ui' => TRUE,
        'title' => t('Single node'),
        'help' => t('Display a single node in page view mode.'),
        'handler' => 'top_node_plugin_style_full_node',
        'uses row plugin' => FALSE,
        'uses grouping' => FALSE,
        'uses options' => FALSE,
        'type' => 'normal',
        'help topic' => 'style-unformatted',
      ),
    ),
  );

  return $plugins;
}
