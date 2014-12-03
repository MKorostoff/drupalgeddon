<?php

/**
 * Truncate size of content body for the tooltip
 */
define('BODY_TOOLTIP_LEN', 128);

function antispam_settings_form_validate($form, &$form_state) {

  if ($form_state['values']['antispam_connection_enabled']) {
    // if connection enabled, then check and validate the entered API key

    $service_provider = $form_state['values']['antispam_service_provider'];
    switch ($service_provider) {
      case AKISMET_SERVICE:
        $apikey = $form_state['values']['antispam_wpapikey'];
        $apikey_form_element = 'antispam_wpapikey';
        if (empty($apikey)) {
          form_set_error('antispam_wpapikey', t('You must enter an API key from the service provider to use AntiSpam module'));
        }
        break;
      case TYPEPAD_SERVICE:
        $apikey = $form_state['values']['antispam_tpapikey'];
        $apikey_form_element = 'antispam_tpapikey';
        if (empty($apikey)) {
          form_set_error('antispam_tpapikey', t('You must enter an API key from the service provider to use AntiSpam module'));
        }
        break;
      case DEFENSIO_SERVICE:
        $apikey = $form_state['values']['antispam_deapikey'];
        $apikey_form_element = 'antispam_deapikey';
        break;
    }
    if (empty($apikey)) {
      form_set_error($apikey_form_element, t('You must enter an API key from the service provider to use AntiSpam module'));
    }
    if (antispam_api_cmd_verify_key($apikey, $service_provider) == ANTISPAM_API_RESULT_ERROR) {
      form_set_error($apikey_form_element, t('The API key you entered is not valid. Check the email from the service provider and enter a valid API key'));
    }
  }
}

/**
 * Build the antispam settings form.
 */
function antispam_settings_form() {
  $form = array();

  $enable_options = array('1' => t('Enabled'), '0' => t('Disabled'));

  $antispam_wpapikey = variable_get('antispam_wpapikey', '');
  $antispam_tpapikey = variable_get('antispam_tpapikey', '');
  $antispam_deapikey = variable_get('antispam_deapikey', '');
  $service_provider = antispam_get_service_provider();

  // Provide the AntiSpam administration JavaScript.
  drupal_add_js(drupal_get_path('module', 'antispam') .'/antispam.js');

  $service_fieldset_collapsed = FALSE; // always show 

  $form['service'] = array(
    '#type' => 'fieldset', '#title' => t('AntiSpam Service Options'),
    '#collapsible' => TRUE, '#collapsed' => $service_fieldset_collapsed
  );
  $form['service']['antispam_service_provider'] = array(
    '#type' => 'radios',
    '#title' => t('Service Provider'),
    '#options' => array(
      t('Akismet'),
      t('TypePad AntiSpam'),
      t('Defensio'),
    ),
    '#default_value' => $service_provider,
    '#description' => t('Please select the anti spam service provider. Although they provide the similar services, TypePad AntiSpam service is free even for commercial sites while Akismet and Defensio have some limitations on the commercial usage. Do not forget to enter an API key of the select service provider below.'),
  );
  $form['service']['antispam_wpapikey'] = array(
    '#type' => 'textfield', '#title' => t('Akismet API key'),
    '#size' => 30, '#maxlength' => 60,
    '#default_value' => $antispam_wpapikey,
    '#description' => t('Please, enter here your <a href="!wpapikey">Akismet API key</a>. If you don\'t have one already, you can get it by simply signing up for a free account at <a href="!akismet-com">Akismet.com</a>. Note that this information is required in order to use the <a href="!akismet">Akismet Service</a>. Please, consult the <a href="!akismet-faq">Akismet FAQ</a> for further information.',
      array(
        '!wpapikey' => url('http://akismet.com/signup/'),
        '!akismet-com' => url('http://akismet.com'),
        '!akismet' => url('http://akismet.com'),
        '!akismet-faq' => url('http://akismet.com/faq/'),
      )
    ),
    '#prefix' => '<div id="antispam-wrapper-0" class="antispam-wrapper">',
    '#suffix' => '</div>',
  );
  $form['service']['antispam_tpapikey'] = array(
    '#type' => 'textfield', '#title' => t('TypePad AntiSpam API key'),
    '#size' => 50, '#maxlength' => 90,
    '#default_value' => $antispam_tpapikey,
    '#description' => t('Please, enter here your <a href="!tpapikey">TypePad.com AntiSpam API key</a>. If you don\'t have one already, you can get it by simply signing up for a free account at <a href="!typepad-com">Typepad.com</a>. Once you get a free acount, visit the <a href="!tpapikey">TypePad AntiSpam Service</a> and get your free TypePad AntiSpam API Key there. Please, consult the <a href="!antispam-faq">TypePad AntiSpam FAQ</a> for further information.',
      array(
        '!tpapikey' => url('http://antispam.typepad.com/info/get-api-key.html'),
        '!typepad-com' => url('http://typepad.com/connect/register'),
        '!antispam-faq' => url('http://antispam.typepad.com/info/faq.html'),
      )
    ),
    '#prefix' => '<div id="antispam-wrapper-1" class="antispam-wrapper">',
    '#suffix' => '</div>',
  );
  $form['service']['antispam_deapikey'] = array(
    '#type' => 'textfield', '#title' => t('Defensio API key'),
    '#size' => 50, '#maxlength' => 90,
    '#default_value' => $antispam_deapikey,
    '#description' => t('Please, enter here your <a href="!defensio-com">Defensio API key</a>. If you don\'t have one already, you can get it by simply signing up for a free account at <a href="!defensio-key">Defensio.com</a>. Then you will get your API key to use <a href="!defensio-com">Defensio</a> antispam service. Please, consult the <a href="!antispam-faq">Defensio FAQ</a> for further information.',
      array(
        '!defensio-com' => url('http://defensio.com/'),
        '!defensio-key' => url('http://defensio.com/signup/'),
        '!antispam-faq' => url('http://defensio.com/faq'),
      )
    ),
    '#prefix' => '<div id="antispam-wrapper-2" class="antispam-wrapper">',
    '#suffix' => '</div>',
  );

  $form['service']['antispam_connection_enabled'] = array(
    '#type' => 'radios', '#title' => t('Service provider connections'),
    '#options' => $enable_options,
    '#default_value' => variable_get('antispam_connection_enabled', 1),
    '#description' => t('<strong>This option must be enabled in order to perform real requests to the specified antispam service provider.</strong> You may want to disable this option for testing purposes, however. In this case, the <em>antispam module</em> will operate as normal, except sending real requests to the service provider. ie. no automatic spam detection will be performed and no remote requests will be made when content is manually <em>marked</em>/<em>unmarked</em> as spam.<br />Note: regardless of this option, the <em>antispam module</em> will still connect, from this panel, to validate your API key, if specified.'),
  );
  $timeout_options = array();
  for ($n = 1; $n <= 30; $n++) {
    $timeout_options[$n] = $n;
  }
  $form['service']['antispam_connection_timeout'] = array(
    '#type' => 'select', '#title' => t('Connection timeout'),
    '#default_value' => variable_get('antispam_connection_timeout', 10),
    '#options' => $timeout_options,
    '#description' => t('This option allows you to specify the connection timeout in seconds that is used for real time antispam service connections.')
  );

  $form['general'] = array(
    '#type' => 'fieldset', '#title' => t('General Options'),
    '#collapsible' => TRUE, '#collapsed' => TRUE
  );
  $age_options = drupal_map_assoc(array(0, 3600, 10800, 21600, 32400, 43200, 86400, 172800, 259200, 604800, 1209600, 1814400), 'format_interval');
  $age_options[0] = t('never');
  $age_options[2592000] = t('1 month');
  $age_options[5184000] = t('2 months');
  $age_options[7776000] = t('3 months');
  $age_options[10368000] = t('4 months');
  $age_options[15768000] = t('6 months');
  $age_options[31536000] = t('1 year');
  $form['general']['antispam_remove_spam_age'] = array(
    '#type' => 'select', '#title' => t('Remove spam older than'),
    '#default_value' => variable_get('antispam_remove_spam_age', 259200),
    '#options' => $age_options,
    '#description' => t('Content marked as <em>spam</em> is still saved into database so it can be reviewed by content administrators. This option allows you to specify how long this information will be kept in the database. <em>Spam</em> older than the age specified here will be automatically removed. Requires crontab.')
  );
  $form['general']['antispam_records_per_page'] = array(
    '#type' => 'select', '#title' => t('Records per page'),
    '#default_value' => variable_get('antispam_records_per_page', 50),
    '#options' => drupal_map_assoc(array(10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200)),
    '#description' => t('The maximum number of records per page on moderation queue.')
  );
  $form['general']['antispam_blocks_counter'] = array(
    '#type' => 'select', '#title' => t('Number of blocks'),
    '#default_value' => variable_get('antispam_blocks_counter', 1),
    '#options' => array(0 => t('none'), 1 => 1, 2 => 2, 3 => 3, 4 => 4, 5 => 5),
    '#description' => t('The antispam module may generate a number of blocks for you to display the current spam counter anywhere on your site. The number of blocks is variable to help you keep your <a href="!admin-block">blocks administration panel</a> as clean as possible. This option allows you to specify how many blocks you wish to use. If you do not plan to show the spam counter to your visitors, set this option to <em>none</em>.',
      array(
        '!admin-block' => url('admin/block'),
      ))
  );
  $form['general']['antispam_email_enabled'] = array(
    '#type' => 'radios', '#title' => t('E-mail notifications'),
    '#options' => $enable_options,
    '#default_value' => variable_get('antispam_email_enabled', 1),
    '#description' => t('Use this option to <em>enable</em> or <em>disable</em> e-mail notifications to content moderators. If enabled, users with proper permissions are allowed to set, from their user profiles, whether they wish to receive e-mail notications for all new (or updated) posts, just for content needing approval or no notifications at all. Users are notified about content types they are allowed to moderate only.')
  );

  $form['node_options'] = array(
    '#type' => 'fieldset', '#title' => t('Node Options'),
    '#collapsible' => TRUE, '#collapsed' => TRUE
  );
  $form['node_options']['antispam_check_nodetypes'] = array(
    '#type' => 'checkboxes', '#title' => t('Check for spam in these node types'),
    '#options' => node_type_get_names(),
    '#default_value' => variable_get('antispam_check_nodetypes', array()),
    '#description' => t('Use this option to <em>enable</em> or <em>disable</em> spam check for nodes of types specified here. When this option is enabled, a request will be sent to the selected antispam service, in real time. If the service was down, nodes would simply be queued for manual moderation. Users with <a href="!admin-access">@admin-nodes</a> permission and <a href="!admin-access">spam moderators</a> are exempt from this check.',
      array(
        '@admin-nodes' => t('administer nodes'),
        '!admin-access' => url('admin/access'),
      ))
  );
  $form['node_options']['antispam_node_publish_links'] = array(
    '#type' => 'radios', '#title' => t('Show publish/unpublish links'),
    '#options' => $enable_options,
    '#default_value' => variable_get('antispam_node_publish_links', 0),
    '#description' => t('Use this option to <em>enable</em> or <em>disable</em> links for <em>publish</em>/<em>unpublish</em> operations in nodes. If enabled, these links will only be displayed to <a href="!admin-access">spam moderators</a> and users with <a href="!admin-access">@admin-nodes</a> permission.',
      array(
        '@admin-nodes' => t('administer nodes'),
        '!admin-access' => url('admin/access'),
      ))
  );
  $form['node_options']['antispam_node_spam_links'] = array(
    '#type' => 'radios', '#title' => t('Show submit spam/ham links'),
    '#options' => $enable_options,
    '#default_value' => variable_get('antispam_node_spam_links', 0),
    '#description' => t('Use this option to <em>enable</em> or <em>disable</em> links for <em>submit spam</em>/<em>ham</em> operations in nodes. If enabled, these links will only be displayed to <a href="!admin-access">spam moderators</a> and users with <a href="!admin-access">@admin-nodes</a> permission.',
      array(
        '@admin-nodes' => t('administer nodes'),
        '!admin-access' => url('admin/access'),
      ))
      .'<br />'. t('<strong>Note:</strong> To interact fully with the antispam service, you really should try putting data back into the system as well as just taking it out. If it is at all possible, please use these links to submit missed spam and false positives (ham), otherwise antispam service will never learn from its mistakes. Thank you.'),
  );

  if (module_exists('comment')) {
    $form['comment_options'] = array(
      '#type' => 'fieldset', '#title' => t('Comment Options'),
      '#collapsible' => TRUE, '#collapsed' => TRUE
    );
    $form['comment_options']['antispam_check_comments'] = array(
      '#type' => 'radios', '#title' => t('Check for spam in comments'),
      '#options' => $enable_options,
      '#default_value' => variable_get('antispam_check_comments', 1),
      '#description' => t('Use this option to <em>enable</em> or <em>disable</em> spam check for comments. When this option is enabled, a request will be sent to the selected antispam service, in real time. If the antispam service was down, comments would simply be queued for manual moderation. Users with <a href="!admin-access">@admin-comments</a> permission and <a href="!admin-access">spam moderators</a> are exempt from this check.',
        array(
          '@admin-comments' => t('administer comments'),
          '!admin-access' => url('admin/access'),
        ))
    );
    $form['comment_options']['antispam_comment_publish_links'] = array(
      '#type' => 'radios', '#title' => t('Show publish/unpublish links'),
      '#options' => $enable_options,
      '#default_value' => variable_get('antispam_comment_publish_links', 1),
      '#description' => t('Use this option to <em>enable</em> or <em>disable</em> links for <em>publish</em>/<em>unpublish</em> operations in comments. If enabled, these links will only be displayed to <a href="!admin-access">spam moderators</a> and users with <a href="!admin-access">@admin-comments</a> permission.',
        array(
          '@admin-comments' => t('administer comments'),
          '!admin-access' => url('admin/access'),
        ))
    );
    $form['comment_options']['antispam_comment_spam_links'] = array(
      '#type' => 'radios', '#title' => t('Show submit spam/ham links'),
      '#options' => $enable_options,
      '#default_value' => variable_get('antispam_comment_spam_links', 1),
      '#description' => t('Use this option to <em>enable</em> or <em>disable</em> links for <em>submit spam</em>/<em>ham</em> operations in comments. If enabled, these links will only be displayed to <a href="!admin-access">spam moderators</a> and users with <a href="!admin-access">@admin-comments</a> permission.',
        array(
          '@admin-comments' => t('administer comments'),
          '!admin-access' => url('admin/access'),
        ))
        .'<br />'. t('<strong>Note:</strong> To interact fully with the selected antispam service, you really should try putting data back into the system as well as just taking it out. If it is at all possible, please use these links to submit missed spam and false positives (ham), otherwise the antispam service will never learn from its mistakes. Thank you.'),
    );
  }

  $date_formats = array(
    'F j, Y', 'j F, Y', 'Y, F j',
    'M j, Y', 'j M, Y', 'Y, M j',
    'Y/m/d', 'm/d/Y', 'd/m/Y',
    'Y-m-d', 'm-d-Y', 'd-m-Y'
  );
  $date_options = array();
  $now = time();
  foreach ($date_formats as $format) {
    $date_options[$format] = format_date($now, 'custom', $format);
  }

  $form['counter_options'] = array(
    '#type' => 'fieldset', '#title' => t('Spam Counter Options'),
    '#collapsible' => TRUE, '#collapsed' => TRUE
  );
  $form['counter_options']['antispam_counter_since'] = array(
    '#type' => 'date', '#title' => t('Counting since'),
    '#default_value' => variable_get('antispam_counter_since', array('day' => date('j'), 'month' => date('n'), 'year' => date('Y'))),
    '#description' => t('This is the date that will tell your visitors when your spam counter started to increment.'),
  );
  $form['counter_options']['antispam_counter_date_format'] = array(
    '#type' => 'select', '#title' => t('Date format'),
    '#default_value' => variable_get('antispam_counter_date_format', $date_formats[0]),
    '#options' => $date_options,
    '#description' => t('Date format used to render the <em>Counting since</em> date.'),
  );

  $form['statistics_options'] = array(
    '#type' => 'fieldset', '#title' => t('Statistics Chart Options'),
    '#collapsible' => TRUE, '#collapsed' => TRUE
  );
  $form['statistics_options']['antispam_chart_width'] = array(
    '#type' => 'select', '#title' => t('Chart width'),
    '#default_value' => variable_get('antispam_chart_width', 480),
    '#options' => drupal_map_assoc(array(320, 400, 480, 540, 600)),
    '#description' => t('This is the width of charts in the statictics page. Use the width that matches to the width of the admin page of your site. The height of charts are automatically determined based on the width.')
  );

  $form['anti_spambot'] = array(
    '#type' => 'fieldset', '#title' => t('Anti-Spambot Options'),
    '#collapsible' => TRUE, '#collapsed' => TRUE,
    '#description' => t('The goal of this section is not to replace anything that the antispam service itself can do a lot better than us, but to provide a set of simple rules aimed to prevent Denial of Service (DoS) situations that could be caused by certain spambots.'),
  );
  $delay_options = drupal_map_assoc(array(0, 30, 60, 90, 120, 150, 180), 'format_interval');
  $delay_options[0] = t('none');
  $form['anti_spambot']['antispam_antispambot_delay'] = array(
    '#type' => 'select', '#title' => t('Delay when spam is detected'),
    '#default_value' => variable_get('antispam_antispambot_delay', 60),
    '#options' => $delay_options,
    '#description' => t('Use this option to delay the response when content has been identified as spam or to requests that match the rules defined below.')
  );
  $anti_spambot_rules = array(
    'ip' => t('IP addresses used by known spammers.'),
    'mail' => t('E-mail addresses used by known spammers.'),
    'body' => t('Content that has already been identified as spam.'),
  );
  $form['anti_spambot']['antispam_antispambot_rules'] = array(
    '#type' => 'checkboxes', '#title' => t('Identify spambots by'),
    '#options' => $anti_spambot_rules,
    '#default_value' => variable_get('antispam_antispambot_rules', array()),
    '#description' => t('These rules will be applied before sending any request to the antispam service. If a request to send content matches any of these rules, the actions defined below will be triggered. Requests to send content are checked against spam that is stored locally (visible from the <a href="!moderation-queue">moderation queue</a>).',
      array(
        '!moderation-queue' => url('admin/content/antispam'),
      ))
  );
  $anti_spambot_actions = array(
    'none' => t('None (only the delay specified above, if any).'),
    '503' => t('HTTP error 503 (Service Unavailable), showing a simple blank page.'),
    '403' => t('HTTP error 403 (Forbidden), showing a simple blank page.'),
    '403d' => t('HTTP error 403 (Forbidden), showing a Drupal generated page.')
  );
  $form['anti_spambot']['antispam_antispambot_action'] = array(
    '#type' => 'radios', '#title' => t('Actions against spambots'),
    '#options' => $anti_spambot_actions,
    '#default_value' => variable_get('antispam_antispambot_action', '503'),
    '#description' => t('Use this option to specify what to do against spambots identified by any of the above rules. When a <em>HTTP error</em> is generated (403 or 503), no request to the antispam service will be made, the request to post content will not be stored into database and no further moderator notifications will be sent. In any case, when a rule matches, a record of the event will be <a href="!admin-reports">logged</a> for further analysis.',
      array(
        '!admin-reports' => url('admin/reports'),
      ))
  );

  return system_settings_form($form);
}

/**
 * Moderation queue operations.
 */
function antispam_moderator_operations($mode, $submode) {
  // Build operations array; based on current mode.
  if ($mode == 'nodes') {
    $operations = array(
      'submit-spam' => array(
        'title' => (variable_get('antispam_connection_enabled', 1) ? t('Submit selected nodes as spam') : t('Mark selected nodes as spam')),
        'confirm' => (variable_get('antispam_connection_enabled', 1) ? t('Are you sure you want to submit these nodes as spam?') : t('Are you sure you want to mark these nodes as spam?')),
        'button' => (variable_get('antispam_connection_enabled', 1) ? t('Submit nodes as spam') : t('Mark nodes as spam')),
      ),
      'submit-ham' => array(
        'title' => (variable_get('antispam_connection_enabled', 1) ? t('Submit selected nodes as ham') : t('Mark selected nodes as ham')),
        'confirm' => (variable_get('antispam_connection_enabled', 1) ? t('Are you sure you want to submit these nodes as ham?') : t('Are you sure you want to mark these nodes as ham?')),
        'button' => (variable_get('antispam_connection_enabled', 1) ? t('Submit nodes as ham') : t('Mark nodes as ham')),
      ),
      'publish' => array(
        'title' => t('Publish selected nodes'),
        'confirm' => t('Are you sure you want to publish these nodes?'),
        'button' => t('Publish nodes'),
      ),
      'unpublish' => array(
        'title' => t('Unpublish selected nodes'),
        'confirm' => t('Are you sure you want to unpublish these nodes?'),
        'button' => t('Unpublish nodes'),
      ),
      'delete' => array(
        'title' => t('Delete selected nodes'),
        'confirm' => t('Are you sure you want to delete these nodes and all their comments?'),
        'button' => t('Delete nodes'),
        'warning' => t('This action cannot be undone.')
      )
    );
  }
  else if ($mode == 'comments') {
    $operations = array(
      'submit-spam' => array(
        'title' => (variable_get('antispam_connection_enabled', 1) ? t('Submit selected comments as spam') : t('Mark selected comments as spam')),
        'confirm' => (variable_get('antispam_connection_enabled', 1) ? t('Are you sure you want to submit these comments as spam?') : t('Are you sure you want to mark these comments as spam?')),
        'button' => (variable_get('antispam_connection_enabled', 1) ? t('Submit comments as spam') : t('Mark comments as spam')),
      ),
      'submit-ham' => array(
        'title' => (variable_get('antispam_connection_enabled', 1) ? t('Submit selected comments as ham') : t('Mark selected comments as ham')),
        'confirm' => (variable_get('antispam_connection_enabled', 1) ? t('Are you sure you want to submit these comments as ham?') : t('Are you sure you want to mark these comments as ham?')),
        'button' => (variable_get('antispam_connection_enabled', 1) ? t('Submit comments as ham') : t('Mark comments as ham')),
      ),
      'publish' => array(
        'title' => t('Publish selected comments'),
        'confirm' => t('Are you sure you want to publish these comments?'),
        'button' => t('Publish comments'),
      ),
      'unpublish' => array(
        'title' => t('Unpublish selected comments'),
        'confirm' => t('Are you sure you want to unpublish these comments?'),
        'button' => t('Unpublish comments'),
      ),
      'delete' => array(
        'title' => t('Delete selected comments'),
        'confirm' => t('Are you sure you want to delete these comments and all their replies?'),
        'button' => t('Delete comments'),
        'warning' => t('This action cannot be undone.')
      )
    );
  }
  else { // Unknown mode!
    return array();
  }

  // Unset redundant operations; based on current submode.
  if ($submode == 'spam') {
    unset($operations['submit-spam']);
  }
  else if ($submode == 'unpublished') {
    unset($operations['unpublish']);
  }
  else if ($submode == 'published') {
    unset($operations['publish']);
  }
  else { // Unknown submode!
    return array();
  }

  return $operations;
}

/**
 * Generate a graph of service statistics using Google Chart API
 */
function antispam_generate_statistics_graph() {
  /**
   * Chart 1: Element Ratio Pie Chart
   */
  // get sum
  $counts = antispam_get_total_counter();
  // construct URL for Google Chart API
  $total = $counts['total_spam'] + $counts['total_ham'];
  $counts['total_spam'] -= $counts['total_fpositive'];
  $counts['total_ham'] -= $counts['total_fnegative'];
  $chart_width = variable_get('antispam_chart_width', 480);
  $output = '<p><img src="http://chart.apis.google.com/chart?'
            . 'chtt=' . t('Total Statistics')
            . '&cht=p'  // char type: line chart
            . '&chs=' . $chart_width . 'x' . ($chart_width / 2) // chart size
            . '&chds=0,' . $total
            . '&chl=' . t('Spam') . '|' . t('Ham') .'|'
              . t('False Negative') . '|' . t('False Positive') // legend
            . '&chd=t:' . $counts['total_spam'] . ',' . $counts['total_ham'] . ','
              . $counts['total_fnegative'] . ',' . $counts['total_fpositive']
            . '&chco=dd6666,ffdd33,444444,bbbbbb' // line colors
            . '" alt="Total Statistics Chart" class="antispam-chart" /></p>' . "\n";

  /**
   * Chart 2: Daily Statistics Line Chart (max 1 year)
   */
  // get max
  $max_counts = antispam_get_max_counter();
  $y_max = max($max_counts['max_spam'], $max_counts['max_ham']) + 1;


  // get oldest and latest date within the range
  $rec = db_query("SELECT MIN(date) AS oldest, MAX(date) AS latest FROM {antispam_counter} ORDER BY date DESC LIMIT 365")->fetchObject();
  $oldest = $rec ? date('M-d', strtotime($rec->oldest)) : '';
  $latest = $rec ? date('M-d', strtotime($rec->latest)) : date('M-d');

  // construct URL for Google Chart API
  $output .= '<p><img src="http://chart.apis.google.com/chart?'
            . 'chtt=' . t('Daily Statistics')
            . '&cht=lc' // char type: line chart
            . '&chs=' . $chart_width . 'x' . (int)($chart_width *2 / 3) // chart size
            . '&chdl=' . t('Spam') . '|' . t('Ham') // legend
            . '&chdlp=t' // legend at top
            . '&chxt=x,y' // axis
            . '&chxl=0:|' . $latest . '|' . $oldest // x-axis label
            . '&chxr=0,0,365|1,0,' . $y_max // x-axis, y-axis range
            . '&chds=0,' . $y_max
            . '&chco=dd6666,ffdd33' // line colors
            . '&chd=t:';
  $spam = $ham = array();
  // max 1 year
  $result = db_query("SELECT * FROM {antispam_counter} ORDER BY date DESC LIMIT 365");
  foreach ($result as $rec) {
    $spam[] = $rec->spam_detected;
    $ham[] = $rec->ham_detected;
  }
  foreach ($spam as $spam_count) {
    $output .= $spam_count . ',';
  }
  $output = rtrim($output, ','); // drop trailing comma
  foreach ($ham as $ham_count) {
    $output .= $ham_count . ',';
  }
  $output = rtrim($output, ','); // drop trailing comma
  $output .= '" alt="Daily Statistics Chart" class="antispam-chart" /></p>' . "\n";

  return $output;
}

/**
 * Menu callback; Moderation queue.
 *
 * @param string Mode: overview (default), nodes, comments, statistics
 * @param string Submode: spam (default), unpublished, published.
 */
function antispam_callback_queue($mode = '', $submode = '') {
  // Make sure we're dealing with a valid mode and submode.
  $valid_modes = array('nodes', 'comments', 'statistics');
  $valid_submodes = array(
    'spam' => t('Spam'),
    'unpublished' => t('Unpublished'),
    'published' => t('Published')
  );
  if (empty($mode)) {
    $mode = 'overview';
  }
  else if (!in_array($mode, $valid_modes)) {
    drupal_not_found();
    return;
  }
  if (empty($submode)) {
    $submode = 'spam';
  }
  else if (!isset($valid_submodes[$submode])) {
    drupal_not_found();
    return;
  }

  // Compute exactly what the current user is allowed to moderate.
  $moderator_types = antispam_get_moderator_types();
  $moderator_types_count = count($moderator_types);
  $allowed_comments = (isset($moderator_types['comments']) ? TRUE : FALSE);
  $allowed_nodes = $moderator_types;
  if ($allowed_comments) {
    unset($allowed_nodes['comments']);
  }
  $allowed_nodes_count = count($allowed_nodes);

  // Make sure the user has some kind of content administration/moderation permission.
  if ($allowed_nodes_count <= 0 && !$allowed_comments) {
    drupal_access_denied();
    return;
  }

  // Present the overview page (default).
  if ($mode == 'overview') {
    $items = array();
    //------ node ------
    if ($allowed_nodes_count > 0) {
      $subitems = array();
      foreach ($valid_submodes as $key => $title) {
        $query = db_select('node', 'n');
        $query->leftJoin('antispam_spam_marks', 's', 's.content_id = n.nid AND s.content_type = :type', array(':type' => 'node'));
        $query->fields('n');
        if ($key == 'spam') {
          $query->isNotNull('s.content_id');            
        }
        else if ($key == 'unpublished') {
          $query->condition('n.status', COMMENT_NOT_PUBLISHED);
        }
        else if ($key == 'published') {
          $query->condition('n.status', COMMENT_PUBLISHED);
        }
        $query->addTag('node_access');
        $count = $query->countQuery()->execute()->fetchField();
/*
        $sql_cnt = str_replace('%cond', $sql_nodes_cond[$key], $sql_nodes_cnt);
        $count = db_query(db_rewrite_sql($sql_cnt))->fetchField();
*/
        $path = 'admin/content/antispam/nodes'. ($key == 'spam' ? '' : '/'. $key);
        $label = ($count > 0 ? l($title, $path) : $title);
        $subitems[] = '<p><strong>'. $label .': '. $count .'</strong></p>';
      }
      $items[] = '<h4>'. t('Nodes') .'</h4>'. theme('item_list', array('items' => $subitems));
    }
    //------ comment ------
    if ($allowed_comments) {
      $subitems = array();
      foreach ($valid_submodes as $key => $title) {
        $query = db_select('comment', 'c');
        $query->leftJoin('antispam_spam_marks', 's', 's.content_id = c.cid AND s.content_type = :type', array(':type' => 'comment'));
        $query->fields('c');
        if ($key == 'spam') {
          $query->isNotNull('s.content_id');            
        }
        else if ($key == 'unpublished') {
          $query->condition('c.status', 0);
        }
        else if ($key == 'published') {
          $query->condition('c.status', 1);
        }
        $query->addTag('node_access');
        $count = $query->countQuery()->execute()->fetchField();
        $path = 'admin/content/antispam/comments'. ($key == 'spam' ? '' : '/'. $key);
        $label = ($count > 0 ? l($title, $path) : $title);
        $subitems[] = '<p><strong>'. $label .': '. $count .'</strong></p>';
      }
      $items[] = '<h4>'. t('Comments') .'</h4>'. theme('item_list', array('items' => $subitems));
    }
    return '<h3>' . t('Summary of content:') . '</h3>' . theme('item_list', array('items' => $items));
  }
  // Present the statistics page (default).
  if ($mode == 'statistics') {
    $items = array();
    $provider_name = antispam_get_provider_name(antispam_get_service_provider(), TRUE);
    $items[] = $provider_name;
    $output = '<h3>' . t('Current Service Provider') . '</h3>' . theme('item_list', array('items' => $items)) . '<br />';
    
    $items = array();
    $counts = antispam_get_total_counter();
    $total_checked = $counts['total_spam'] + $counts['total_ham'];
    $total_false = $counts['total_fnegative'] + $counts['total_fpositive'];
    $accuracy = $total_checked  ? round(100.0 -((float)$total_false / (float)$total_checked) * 100, 2) : 0;
    $since = variable_get('antispam_counter_since', array('day' => date('j'), 'month' => date('n'), 'year' => date('Y')));
    $start = mktime(0, 0, 0, $since['month'], $since['day'], $since['year']);
    $days = (int)((time() - $start) / (60 * 60 * 24) + 1);
    $subitems = array();
    $subitems[] = t('Total Spams: !total (avg: !avg/day)', array('!total' => $counts['total_spam'], '!avg' => round($counts['total_spam'] / $days, 2)));
    $subitems[] = t('Total Hams: !total (avg: !avg/day)', array('!total' => $counts['total_ham'], '!avg' => round($counts['total_ham'] / $days, 2)));
    $items[] = '<p>' . t('Total Checked: !total (avg: !avg/day)', array('!total' => $total_checked, '!avg' => round($total_checked / $days, 2))) . theme('item_list', array('items' => $subitems)) . '</p>';

    $items[] = '<p>' . t('Total False Negatives: !total (avg: !avg/day)', array('!total' => $counts['total_fnegative'], '!avg' => round($counts['total_fnegative'] / $days, 2))) . '</p>';
    $items[] = '<p>' . t('Total False Positives: !total (avg: !avg/day)', array('!total' => $counts['total_fpositive'], '!avg' => round($counts['total_fpositive'] / $days, 2))) . '</p>';
    $items[] = '<p><strong>' . t('Accuracy: !accuracy %', array('!accuracy' => $accuracy)) . '</strong></p>';
    $output .= '<h3>' . t('Statistics since @since (!days)', 
      array('@since' => antispam_get_counting_since(), '!days' => format_plural($days, '1 day', '@count days'))) . '</h3>' . theme('item_list', array('items' => $items));
    // generate graph using Google Chart API
    $output .= antispam_generate_statistics_graph();
    // footnotes
    $output .= '<p><br /><i>' . t('Note: <strong>False negatives</strong> is the number of spams that were incorrectly tagged as hams, while <strong>false positives</strong> is the number of hams that were incorrectly tagged as spams. These numbers totally depends on your manual operation to retrain the antispam service using <strong>submit as spam</strong> and <strong>submit as ham</strong> feature.') . '</i></p>';
    $output .= '<p><i>' . t('Note: <strong>Accuracy</strong> is calculated by the following formula:<br /> accuracy(%) = 100 - (((false negative + false positive) / total checked) * 100)') . '</i></p>';
   
    return $output;
  }

  if (isset($_POST) && isset($_POST['items']) && count($_POST['items']) > 0) {
    return drupal_get_form('antispam_confirm_multiple_operation', $mode, $submode);
  }
  else {
    return drupal_get_form('antispam_moderation_form', $mode, $submode);
  }
}

function _antispam_spaminess_bar($spaminess) {
  // $spaminess - float value between (0 to 1) or null
  if (empty($spaminess)) {
    return;
  }
  else {
    $ispaminess = (int)($spaminess * 100);
  }
  $output  = '<div style="width:100px; background-color: #eee;">';
  $output .= '<div style="width:' . $ispaminess . 'px; color: #000; background: url('
              . base_path() . drupal_get_path('module', 'antispam') 
              . '/spaminess.jpg) center left; padding:0px 2px; font-size: 9px">' 
              . $spaminess;
  $output .= '</div></div>';
  return $output;
}

function antispam_get_submode_menu($mode, $submode) {
  $out  = '<div><ul class="tabs secondary">';

  $spam_li = $unpublished_li = $published_li = '<li>';
  if ($submode == 'spam') {
    $spam_li = '<li class="active">'; 
  }
  else if ($submode == 'unpublished') {
    $unpublished_li = '<li class="active">'; 
  }
  else if ($submode == 'published') {
    $published_li = '<li class="active">'; 
  }
  $out .= $spam_li . l(t('Spam'), 'admin/content/antispam/'.$mode.'/spam') . '</li>';
  $out .= $unpublished_li . l(t('Unpublished'), 'admin/content/antispam/'.$mode.'/unpublished') . '</li>';
  $out .= $published_li . l(t('Published'), 'admin/content/antispam/'.$mode.'/published') . '</li>';
  $out .= '</ul></div>';
  return $out;
}

function antispam_moderation_form($form, &$form_state, $mode = '', $submode = '') {
  drupal_add_js('misc/tableselect.js');

  // Build the moderation queue form.
  $form = array();

  $submode_menu = antispam_get_submode_menu($mode, $submode);
  $form['submode_menu'] = array(
    '#type' => 'item',
    '#markup' => $submode_menu,
  );

  $form['options'] = array(
    '#type' => 'fieldset', 
    '#title' => t('Moderator actions'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="container-inline">', '#suffix' => '</div>'
  );

  $options = array('' => t('- select operation -'));

  foreach (antispam_moderator_operations($mode, $submode) as $key => $operation_info) {
    $options[$key] = $operation_info['title'];
  }

  $form['options']['operation'] = array(
    '#type' => 'select', 
    '#options' => $options, 
    '#default_value' => '');
  $form['options']['submit'] = array('#type' => 'submit', '#value' => t('Moderate'));

  if ($mode == 'nodes') {

    $allowed_nodes = antispam_get_moderator_types();
    if (isset($allowed_nodes['comments'])) {
      unset($allowed_nodes['comments']);
    }
    $sql_nodetypes = array();
    foreach ($allowed_nodes as $type => $name) {
      $sql_nodetypes[] = $type;
    }

    $query = db_select('node', 'n');
    $query->join('users', 'u', 'u.uid = n.uid');
    $query->join('field_data_body', 'e', 'e.entity_id = n.nid');
    $query->leftJoin('antispam_spam_marks', 's', 's.content_id = n.nid AND s.content_type = :type', array(':type' => 'node'));
    $query->fields('n');
    $query->addField('u', 'name', 'registered_name');
    $query->addField('s', 'spaminess');
    $query->addField('s', 'content_id', 'spam_mark');
    $query->addField('e', 'body_value', 'body');
    $query->condition('n.type', $sql_nodetypes, 'IN');
    if ($submode == 'spam') {
      $query->isNotNull('s.content_id');            
    }
    else if ($submode == 'unpublished') {
      $query->condition('n.status', 0);
    }
    else if ($submode == 'published') {
      $query->condition('n.status', 1);
    }
    if (antispam_get_service_provider() == DEFENSIO_SERVICE) {
      $form['header'] = array('#type' => 'value', '#value' => array(
        // theme('table_select_header_cell'),
        array('class' => 'select-all'),
        array('data' => t('Title'), 'field' => 'title'),
        array('data' => t('Type'), 'field' => 'type'),
        array('data' => t('Author'), 'field' => 'name'),
        array('data' => t('Status'), 'field' => 'status'),
        array('data' => t('Spaminess'), 'field' => 'spaminess', 'sort' => 'desc'),
        array('data' => t('Last changed'), 'field' => 'changed', 'sort' => 'desc')
      ));
    }
    else {
      $form['header'] = array('#type' => 'value', '#value' => array(
        // theme('table_select_header_cell'),
        array('class' => 'select-all'),
        array('data' => t('Title'), 'field' => 'title'),
        array('data' => t('Type'), 'field' => 'type'),
        array('data' => t('Author'), 'field' => 'name'),
        array('data' => t('Status'), 'field' => 'status'),
        array('data' => t('Last changed'), 'field' => 'changed', 'sort' => 'desc')
      ));
    }
  }
  else { // comments
    $query = db_select('comment', 'c');
    $query->join('users', 'u', 'u.uid = c.uid');
    $query->join('field_data_comment_body', 'e', 'e.entity_id = c.cid');
    $query->leftJoin('antispam_spam_marks', 's', 's.content_id = c.cid AND s.content_type = :type', array(':type' => 'comment'));
    $query->fields('c');
    $query->addField('u', 'name', 'registered_name');
    $query->addField('s', 'spaminess');
    $query->addField('s', 'content_id', 'spam_mark');
    $query->addField('e', 'comment_body_value', 'body');
    if ($submode == 'spam') {
      $query->isNotNull('s.content_id');            
    }
    else if ($submode == 'unpublished') {
      $query->condition('c.status', COMMENT_NOT_PUBLISHED);
    }
    else if ($submode == 'published') {
      $query->condition('c.status', COMMENT_PUBLISHED);
    }
    if (antispam_get_service_provider() == DEFENSIO_SERVICE) {
      $form['header'] = array('#type' => 'value', '#value' => array(
        // theme('table_select_header_cell'),
        array('class' => 'select-all'),
        array('data' => t('Subject'), 'field' => 'subject'),
        array('data' => t('Author'), 'field' => 'name'),
        array('data' => t('Status'), 'field' => 'status'),
        array('data' => t('Spaminess'), 'field' => 'spaminess', 'sort' => 'desc'),
        array('data' => t('Last changed'), 'field' => 'created', 'sort' => 'desc')
      ));
    }
    else {
      $form['header'] = array('#type' => 'value', '#value' => array(
        // theme('table_select_header_cell'),
        array('class' => 'select-all'),
        array('data' => t('Subject'), 'field' => 'subject'),
        array('data' => t('Author'), 'field' => 'name'),
        array('data' => t('Status'), 'field' => 'status'),
        array('data' => t('Last changed'), 'field' => 'created', 'sort' => 'desc')
      ));
    }
  }

  $records_per_page = variable_get('antispam_records_per_page', 50);

  $query = $query->extend('PagerDefault')->limit($records_per_page)->extend('TableSort')->orderByHeader($form['header']['#value']);
  $result = $query->execute();
  $items = array();
  $now = time();
  foreach ($result as $content) {
    if ($mode == 'nodes') {
      $items[$content->nid] = '';
      $content->name = $content->uid ? $content->registered_name : $content->name;
      $form['title'][$content->nid] = array(
        '#value' => l($content->title, 'node/'. $content->nid, array(
          'attributes' => array(
            'title' => truncate_utf8(render($content->body), BODY_TOOLTIP_LEN),
          )
        )
      ) .' '. theme('mark', array('type' => node_mark($content->nid, $content->changed))));
      $form['type'][$content->nid] = array('#value' => node_type_get_name($content));
      $form['author'][$content->nid] = array('#value' => theme('username', array('name' => $content->name)));
      $form['status'][$content->nid] = array('#value' => ($content->status ? t('published') : t('not published')));
      if(empty($content->spam_mark)) $content->spam_mark = 0;
      if ($content->spam_mark) {
        $form['status'][$content->nid]['#value'] .= '/' . t('spam');
      }
      if (antispam_get_service_provider() == DEFENSIO_SERVICE) {
        $form['spaminess'][$content->nid] = array('#type' => 'item', '#markup' => _antispam_spaminess_bar($content->spaminess));
      }
      $form['created'][$content->nid] = array('#value' => t('%time ago', array('%time' => format_interval($now - $content->changed))));
    }
    else { // comments
      $items[$content->cid] = '';
      $content->name = $content->uid ? $content->registered_name : $content->name;
      $form['title'][$content->cid] = array(
        '#value' => l($content->subject, 'node/'. $content->nid, array(
          'attributes' => array(
            'title' => truncate_utf8(render($content->body), BODY_TOOLTIP_LEN)
          ), 
          'fragment' => 'comment-' . $content->cid,
         )) .' '. theme('mark', array('type' => node_mark($content->cid, $content->created))));
      $form['author'][$content->cid] = array('#value' => theme('username', array('name' => $content->name)));
      $form['status'][$content->cid] = array('#value' => ($content->status == COMMENT_PUBLISHED ? t('published') : t('not published')));
      if ($content->spam_mark) {
        $form['status'][$content->cid]['#value'] .= '/' . t('spam');
      }
      if (antispam_get_service_provider() == DEFENSIO_SERVICE) {
        $form['spaminess'][$content->cid] = array('#type' => 'item', '#markup' => _antispam_spaminess_bar($content->spaminess));
      }
      $form['created'][$content->cid] = array('#value' => t('%time ago', array('%time' => format_interval($now - $content->created))));
    }
  }
  $form['mode'] = array('#type' => 'hidden', '#value' => $mode);
  $form['submode'] = array('#type' => 'hidden', '#value' => $submode);
  $form['items'] = array('#type' => 'checkboxes', '#options' => $items);
  $form['pager'] = array('#value' => theme('pager'));

  return $form;
}

/**
 * Theme callback; render the moderation queue form.
 */
function theme_antispam_moderation_form($variables) {
  $form = $variables['form'];
  $mode = $form['mode']['#value'];
  $submode = $form['submode']['#value'];

  $output = drupal_render($form['submode_menu']);
  $output .= drupal_render($form['options']);

  $rows = array();
  if (isset($form['author']) && is_array($form['author'])) {
    foreach (element_children($form['author']) as $key) {
      $row = array();
      $row[] = render($form['items'][$key]);
      $row[] = render($form['title'][$key]['#value']);
      if ($mode == 'nodes') {
        $row[] = render($form['type'][$key]['#value']);
      }
      $row[] = render($form['author'][$key]['#value']);
      $row[] = render($form['status'][$key]['#value']);
      if (antispam_get_service_provider() == DEFENSIO_SERVICE) {
        $row[] = render($form['spaminess'][$key]['#value']);
      }
      $row[] = render($form['created'][$key]['#value']);
      $rows[] = $row;
    }
  }
  else {
    if ($submode == 'spam') {
      $message = ($mode == 'nodes' ? t('There is no spam in the nodes moderation queue.') : t('There is no spam in the comments moderation queue.')) .'<br />'. t('It must be your lucky day! ;-)');
    }
    else if ($submode == 'unpublished') {
      $message = ($mode == 'nodes' ? t('There are no unpublished nodes in the moderation queue.') : t('There are no unpublished comments in the moderation queue.'));
    }
    else { // published
      $message = ($mode == 'nodes' ? t('There are no published nodes.') : t('There are no published comments.'));
    }
    if (antispam_get_service_provider() == DEFENSIO_SERVICE) {
      $rows[] = array(array('data' => $message, 'align' => 'center', 'colspan' => ($mode == 'nodes' ? '7' : '6')));
    }
    else {
      $rows[] = array(array('data' => $message, 'align' => 'center', 'colspan' => ($mode == 'nodes' ? '6' : '5')));
    }
  }
  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows));
  $output .= render($form['pager']['#value']);
  return $output;
}

/**
 * Form API callback; Validate the moderation queue form.
 */
function antispam_moderation_form_validate($form, &$form_state) {
  $mode = $form_state['values']['mode'];
  $submode = $form_state['values']['submode'];
  $operation = $form_state['values']['operation'];
  $valid_operations = antispam_moderator_operations($mode, $submode);

  if (!isset($valid_operations[$operation])) {
    form_set_error('', t('Please, choose a valid operation.'));
  }
  $form_state['values']['items'] = array_diff($form_state['values']['items'], array(0));
  if (count($form_state['values']['items']) == 0) {
    if ($operation == 'delete') {
      form_set_error('', t('Please, select some items to perform the delete operation.'));
    }
    else {
      form_set_error('', t('Please, select some items to perform the action on.'));
    }
  }
}

/**
 * List the selected items and verify that the admin really wants to delete them.
 */
// function antispam_confirm_multiple_operation() {
function antispam_confirm_multiple_operation($form, &$form_state, $mode = '', $submode = '') {
  $edit = $_POST;
  $operation = $edit['operation'];
  $valid_operations = antispam_moderator_operations($mode, $submode);

  // Make sure we deal with a valid combination of mode, submode and operation.
  if (!isset($valid_operations[$operation])) {
    return;
  }

  $confirm_message = '<strong>'. $valid_operations[$operation]['confirm'] .'</strong>';
  $confirm_button = $valid_operations[$operation]['button'];
  $confirm_warning = '<p>'. (isset($valid_operations[$operation]['warning']) ? $valid_operations[$operation]['warning'] : '') .'</p>';
  $content_type = ($mode == 'nodes' ? 'node' : 'comment');

  $form = array();
  $form['items'] = array('#prefix' => '<ul>', '#suffix' => '</ul>', '#tree' => TRUE);
  // array_filter() returns only elements with actual values
  foreach (array_filter($edit['items']) as $content_id => $value) {
    if ($content = antispam_content_load($content_type, $content_id)) {
      $title = '&quot;'. check_plain($content_type == 'node' ? $content->title : $content->subject) .'&quot;, '. t('by') .' '. theme('username', array('name' => $content->name));
      $form['items'][$content_id] = array('#type' => 'hidden', '#value' => $content_id, '#prefix' => '<li>', '#suffix' => $title .'</li>');
    }
  }
  $form['mode'] = array('#type' => 'hidden', '#value' => $mode);
  $form['submode'] = array('#type' => 'hidden', '#value' => $submode);
  $form['operation'] = array('#type' => 'hidden', '#value' => $operation);

  // Redirect to a non-existent menu item to make tabs disappear.
  menu_set_active_item('');

  $path = 'admin/content/antispam/'. $mode;
  if ($submode != 'spam') {
    $path .= '/'. $submode;
  }
  return confirm_form($form, $confirm_message, $path, $confirm_warning, $confirm_button, t('Cancel'));
}

/**
 * confirm_form callback; perform the actual operation against selected content.
 */
function antispam_confirm_multiple_operation_submit($form, &$form_state) {
  $mode = $form_state['values']['mode'];
  $submode = $form_state['values']['submode'];
  $operation = $form_state['values']['operation'];
  $valid_operations = antispam_moderator_operations($mode, $submode);

  // Make sure we deal with a valid combination of mode, submode and operation.
  if (!isset($valid_operations[$operation])) {
    $form_state['redirect'] = 'admin/content/antispam';
    return;
  }

  if ($form_state['values']['confirm']) {
    $content_type = ($mode == 'nodes' ? 'node' : 'comment');
    foreach ($form_state['values']['items'] as $content_id => $value) {
      if ($operation == 'delete') {
        antispam_content_delete($content_type, $content_id);
        $message = ($mode == 'nodes' ? t('The nodes have been deleted.') : t('The comments have been deleted.'));
      }
      else if ($content = antispam_content_load($content_type, $content_id)) {
        if ($content_type == 'node') {
          $is_published = ($content->status ? TRUE : FALSE);
        }
        else { // comment
          $is_published = ($content->status == COMMENT_PUBLISHED ? TRUE : FALSE);
        }
        $is_spam = antispam_content_is_spam($content_type, $content_id);

        if ($operation == 'submit-spam' && !$is_spam) {
          antispam_content_spam_operation($content_type, $content, 'submit-spam', TRUE);
          antispam_increase_counter(ANTISPAM_COUNT_FALSE_NEGATIVE);
        }
        else if ($operation == 'submit-ham' && $is_spam) {
          antispam_content_spam_operation($content_type, $content, 'submit-ham', TRUE);
          antispam_increase_counter(ANTISPAM_COUNT_FALSE_POSITIVE);
        }

        if (in_array($operation, array('unpublish', 'submit-spam')) && $is_published) {
          antispam_content_publish_operation($content_type, $content, 'unpublish');
        }
        else if (in_array($operation, array('publish', 'submit-ham')) && !$is_published) {
          antispam_content_publish_operation($content_type, $content, 'publish');
        }

        $message = ($mode == 'nodes' ? t('The nodes have been updated.') : t('The comments have been updated.'));
      }
    }
    drupal_set_message($message);
  }

  $form_state['redirect'] = 'admin/content/antispam/'. $mode;
  if ($submode != 'spam') {
    $form_state['redirect'] .= '/'. $submode;
  }
  return;
}
