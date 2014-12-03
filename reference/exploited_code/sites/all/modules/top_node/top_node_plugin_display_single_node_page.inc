<?php
// $Id: top_node_plugin_display_single_node_page.inc,v 1.1.2.1.2.1 2011/01/07 00:57:49 eaton Exp $
/**
 * @file
 * Contains the single-node display plugin.
 */

/**
 * The plugin that displays a single node in full-page mode. Only works with
 * nodes, obviously.
 */
class top_node_plugin_display_single_node_page extends views_plugin_display_page {
  function get_style_type() { return 'full_node'; }

  function option_definition() {
    $options = parent::option_definition();

    $options['displays'] = array('default' => array());

    // Overrides for standard stuff:
    $options['style_plugin']['default'] = 'full_node';
    $options['style_options']['default']  = array('description' => '');
    $options['sitename_title']['default'] = FALSE;
    $options['row_plugin']['default'] = '';
    $options['defaults']['default']['style_plugin'] = FALSE;
    $options['defaults']['default']['style_options'] = FALSE;
    $options['defaults']['default']['row_plugin'] = FALSE;
    $options['defaults']['default']['row_options'] = FALSE;

    return $options;
  }

  function defaultable_sections($section = NULL) {
    if (in_array($section, array('style_options', 'style_plugin', 'row_options', 'row_plugin',))) {
      return FALSE;
    }

    $sections = parent::defaultable_sections($section);

    return $sections;
  }

  function validate() {
    $errors = parent::validate();

    if ($this->view->base_table != 'node') {
      $errors[] = t('Display @display is only compatible with views of nodes.', array('@display' => $this->display->display_title));
    }

    return $errors;
  }
}
