<?php

/**
 * Shutdown function executed at cron time.
 */
function antispam_cron_shutdown() {
  // Expired content spam that we have to remove from each content repository.
  $expired_content_spam = array('nids' => array(), 'cids' => array());
  // Spam marks that we have to remove from the 'spam marks' table.
  $obsolete_spam_marks = array('nids' => array(), 'cids' => array());

  // Retrieve the list of expired content spam, based on the age specified in the settings panel.
  $expire_spam_age = variable_get('antispam_remove_spam_age', 259200);
  if ($expire_spam_age > 0) {
    $result = db_query('SELECT content_type, content_id FROM {antispam_spam_marks} WHERE spam_created < :spam_created', array(':spam_created' => time() - $expire_spam_age));
    foreach ($result as $s) {
      $key = ($s->content_type == 'node' ? 'nids' : 'cids');
      $expired_content_spam[$key][] = $s->content_id;
      $obsolete_spam_marks[$key][] = $s->content_id;
    }
  }

  // Deal with possible spam marks for content that have already been removed from database.
  // Note: when Drupal deletes a node, all its comments are deleted, but no hook is invoked,
  // so that may lead to orphans in the 'spam marks' table.
  // This is why this cron task is being more complex that it could really be. Anyway, these
  // queries shouldn't be too heavy.
  $result = db_query('SELECT s.content_id FROM {antispam_spam_marks} s LEFT JOIN {node} n ON s.content_id = n.nid WHERE s.content_type = \'node\' AND n.nid IS NULL');
  foreach ($result as $s) {
    if (!in_array($s->content_id, $obsolete_spam_marks['nids'])) {
      $obsolete_spam_marks['nids'][] = $s->content_id;
    }
  }
  $result = db_query('SELECT s.content_id FROM {antispam_spam_marks} s LEFT JOIN {comment} c ON s.content_id = c.cid WHERE s.content_type = \'comment\' AND c.cid IS NULL');
  foreach ($result as $s) {
    if (!in_array($s->content_id, $obsolete_spam_marks['cids'])) {
      $obsolete_spam_marks['cids'][] = $s->content_id;
    }
  }

  // From this point on is where we really will delete stuff from database.
  // Drupal cache will need to be cleared so anonymous users get updated views.
  $clear_cache = FALSE;

  // Remove expired spam from each content repository.
  $expired_nids_removed = count($expired_content_spam['nids']);
  $expired_cids_removed = count($expired_content_spam['cids']);
  if ($expired_nids_removed > 0) {
    $deleted_items = array();
    $delete_count = 0;
    foreach ($expired_content_spam['nids'] as $nid) {
      if (antispam_content_delete('node', $nid)) {
        $deleted_items[] = $nid;
        $delete_count++;
      }
    }
    if ($delete_count > 0) {
      $message = t('AntiSpam housekeeping') .': '. format_plural($delete_count, '1 expired spam node removed from database', '@count expired spam nodes removed from database').
        '<br />'.t('Node ID List: %nids', array('%nids' => implode(',', $deleted_items)));
      watchdog('cron', $message);
      $clear_cache = TRUE;
    }
  }
  if ($expired_cids_removed > 0) {
    $deleted_items = array();
    $delete_count = 0;
    foreach ($expired_content_spam['cids'] as $cid) {
      if (antispam_content_delete('comment', $cid)) {
        $deleted_items[] = $cid;
        $delete_count++;
      }
    }
    if ($delete_count > 0) {
      $message = t('AntiSpam housekeeping') .': '. format_plural($delete_count, '1 expired spam comment removed from database', '@count expired spam comments removed from database').
        '<br />'.t('Comment ID List: %cids', array('%cids' => implode(',', $deleted_items)));
      watchdog('cron', $message);
      $clear_cache = TRUE;
    }
  }

  // Remove obsolete spam marks from database.
  $spam_nids_removed = count($obsolete_spam_marks['nids']);
  $spam_cids_removed = count($obsolete_spam_marks['cids']);
  $spam_marks_removed = $spam_nids_removed + $spam_cids_removed;
  if ($spam_nids_removed > 0) {
    // $spam_nids_list = implode(',', $obsolete_spam_marks['nids']);
    $spam_nids_list = $obsolete_spam_marks['nids'];
    db_delete('antispam_spam_marks')
      ->condition('content_type', 'node')
      ->condition('content_id', $spam_nids_list, 'IN')
      ->execute();
  }
  if ($spam_cids_removed > 0) {
    // $spam_cids_list = implode(',', $obsolete_spam_marks['cids']);
    $spam_cids_list = $obsolete_spam_marks['cids'];
    db_delete('antispam_spam_marks')
      ->condition('content_type', 'comment')
      ->condition('content_id', $spam_cids_list, 'IN')
      ->execute();
  }
  if ($spam_marks_removed > 0) {
    $message = t('AntiSpam housekeeping') .': '. format_plural($spam_marks_removed, '1 spam mark removed from database', '@count spam marks removed from database');
    if (isset($spam_nids_list)) {
      $message .= '<br />'. t('Node ID List: %nids', array('%nids' => $spam_nids_list));
    }
    if (isset($spam_cids_list)) {
      $message .= '<br />'. t('Comment ID List: %cids', array('%cids' => $spam_cids_list));
    }
    watchdog('cron', $message);
    $clear_cache = TRUE;
  }

  // If anything was removed, then clear Drupal cache.
  if ($clear_cache) {
    antispam_clear_cache();
  }
}
