<?php
/**
 * @file
 * whaleocalypse_side_bar.features.fe_block_boxes.inc
 */

/**
 * Implements hook_default_fe_block_boxes().
 */
function whaleocalypse_side_bar_default_fe_block_boxes() {
  $export = array();

  $fe_block_boxes = new stdClass();
  $fe_block_boxes->info = 'About';
  $fe_block_boxes->format = 'full_html';
  $fe_block_boxes->machine_name = 'about';
  $fe_block_boxes->body = '<a href="/about"><img src="/sites/all/modules/whaleocalypse_side_bar/images/about.jpg" /></a>';

  $export['about'] = $fe_block_boxes;

  $fe_block_boxes = new stdClass();
  $fe_block_boxes->info = 'About the author';
  $fe_block_boxes->format = 'full_html';
  $fe_block_boxes->machine_name = 'about_the_author';
  $fe_block_boxes->body = '<a href="/about-author"><img src="/sites/all/modules/whaleocalypse_side_bar/images/author.jpg" /></a>';

  $export['about_the_author'] = $fe_block_boxes;

  $fe_block_boxes = new stdClass();
  $fe_block_boxes->info = 'Also by Matt Korostoff';
  $fe_block_boxes->format = 'full_html';
  $fe_block_boxes->machine_name = 'also_by_matt_korostoff';
  $fe_block_boxes->body = '<a target="_blank" href="http://forimmediateoccupancy.com"><img src="/sites/all/modules/whaleocalypse_side_bar/images/fio-banner.png"></a>

<a target="_blank" href="http://roswellchamberofcommerce.org"><img width="200" height="104" src="/sites/all/modules/whaleocalypse_side_bar/images/roswell-banner.png"></a>';

  $export['also_by_matt_korostoff'] = $fe_block_boxes;

  return $export;
}
