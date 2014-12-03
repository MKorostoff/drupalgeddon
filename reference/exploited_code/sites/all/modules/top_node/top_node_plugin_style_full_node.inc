<?php
// $Id: top_node_plugin_style_full_node.inc,v 1.1.2.1.2.1 2011/01/07 00:57:49 eaton Exp $

/**
 * Renders a single node..
 */
class top_node_plugin_style_full_node extends views_plugin_style {

  function query() {
    if (isset($this->view->base_table)) {
      $this->base_field_alias = $this->view->query->add_field($this->view->base_table, $this->view->base_field);
    }
  }

  /**
   * Render the display in this style.
   */
  function render() {
    $output = '';
    if ($this->view->base_table == 'node') {
      if ($node = node_load($this->view->result[0]->{$this->base_field_alias})) {
        // Oh holy god this is wrong
        $_GET['q'] = 'node/' . $node->nid;
        $output = drupal_render(node_page_view($node));
      }
    }
    return $output;
  }
}
