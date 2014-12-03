<?php
/**
 * @file
 * Shield module's admin funcions.
 */

/**
 * Administration settings form
 */
function shield_admin_settings($form, &$form_state) {
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('Shield settings'),
    '#description' => t('Set up credentials for an authenticated user. You can also decide whether you want to print out the credentials or not.'),
  );

  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
   );

  $form['general']['shield_allow_cli'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow command line access'),
    '#description' => t('When the site is accessed from command line (e.g. from Drush, cron), the shield should not work.'),
    '#default_value' => variable_get('shield_allow_cli', 1),
  );

  $form['credentials'] = array(
    '#type' => 'fieldset',
    '#title' => t('Credentials'),
   );

  $form['credentials']['shield_user'] = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#default_value' => variable_get('shield_user', ''),
    '#description' => t('Live it blank to disable authentication.')
  );

  $form['credentials']['shield_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('shield_pass', ''),
  );

  $form['shield_print'] = array(
    '#type' => 'textfield',
    '#title' => t('Authentication message'),
    '#description' => t("The message to print in the authentication request popup. You can use [user] and [pass] to print the user and the password respectively. You can leave it empty, if you don't want to print out any special message to the users."),
    '#default_value' => variable_get('shield_print', 'Hello, user: [user], pass: [pass]!'),
  );

  return system_settings_form($form);
}
