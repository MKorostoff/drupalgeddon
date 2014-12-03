<?php

/**
 * @file
 * Page and form to import drupal path redirects
 */

/**
 *  form for import redirect path
 */
function path_redirect_import_form() {
  $form = array();
  $form['csv'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import from .csv or .txt file'),
    '#description' => t('To import redirects, you must create a CSV or TXT.'),
  );

  $form['csv']['delimiter'] = array(
    '#type' => 'textfield',
    '#title' => t('Delimiter'),
    '#description' => t('Add your delimiter.'),
    '#default_value' => ',',
    '#maxlength' => 2,
  );
  $form['csv']['no_headers'] = array(
    '#type' => 'checkbox',
    '#title' => t('No headers'),
    '#description' =>
      t('Check if the imported file does not start with a header row.'),
  );

  $form['csv']['override'] = array(
    '#type' => 'checkbox',
    '#title' => t('Override sources'),
    '#description' => t('Check to override stored redirects.'),
  );

  $form['csv']['csv_file'] = array(
    '#type' => 'file',
    '#description' =>
      t('The CSV file must include columns in the following order: 
      "From URL","To URL","Redirect Status","Redirect Language". Defaults for status and language can be set in the advanced options.'),
  );

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['advanced']['status_code'] = array(
    '#type' => 'select',
    '#title' => t('Redirect status'),
    '#description' =>
      t('You can find more information about HTTP redirect status codes at 
      <a href="@status-codes">@status-codes</a>.',
      array('@status-codes' =>
        'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection',
      )
    ),
    '#default_value' => 301,
    '#options' => redirect_status_code_options(),
  );
  if (module_exists('locale')) {
    $form['advanced']['language'] = array(
      '#type' => 'select',
      '#title' => t('Redirect language'),
      '#description' => t('A redirect set for a specific language will always be used when requesting this page in that language, and takes precedence over redirects set for <em>All languages</em>.'),
      '#default_value' => LANGUAGE_NONE,
      '#options' => array(LANGUAGE_NONE => t('All languages')) + locale_language_list('name'),
    );
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Import'));
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  return $form;
}

/**
 * validate form function
 */
function path_redirect_import_form_validate($form, &$form_state) {
  $validators = array('file_validate_extensions' => array('csv txt'));
  if ($file = file_save_upload('csv_file', $validators)) {
    $form_state['uploaded_file'] = $file;
  }
  else {
    form_set_error('form', t('File upload failed.'));
  }
}

/**
 * Submit form function
 */
function path_redirect_import_form_submit($form, &$form_state) {
  ini_set('auto_detect_line_endings', TRUE);
  if (!isset($form_state['uploaded_file'])) {
    return;
  }
  $result = path_redirect_import_read_file(
    $form_state['uploaded_file']->uri, $form_state['values']);
  if ($result['success']) {
    drupal_set_message(implode('<br />', $result['message']));
  }
  else {
    drupal_set_message(implode('<br />', $result['message']), 'error');
  }

  file_delete($form_state['uploaded_file']);
}


/**
 * Export redirects page form
 */
function path_redirect_import_export_form() {
  $form['markup'] = array(
    '#type' => 'item',
    '#markup' => t('You can export all your redirects in path_redirect_import format'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export redirects'),
  );

  return $form;
}
/**
 * Dump current redirects for download as csv.
 */
function path_redirect_import_export_form_submit($form, &$form_state) {
  $efq = new EntityFieldQuery();
  $result = $efq->entityCondition('entity_type', 'redirect')
    ->entityCondition('bundle', 'redirect')
    ->execute();

  if (isset($result['redirect'])) {
    path_redirect_import_export_redirects($result['redirect']);
  }
  else {
    drupal_set_message('Sorry, no redirects to export available at this moment');
  }
}

