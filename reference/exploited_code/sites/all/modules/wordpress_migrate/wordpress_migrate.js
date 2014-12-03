Drupal.behaviors.wordpressMigrate = Drupal.behaviors.wordpressMigrate || {};

Drupal.behaviors.wordpressMigrate.attach = function (context, settings) {
  var $ = jQuery;
  // "Map content" section
  $('fieldset#edit-wordpress-migrate-content-mapping', context).drupalSetSummary(function (context) {
    var pages = $('#edit-wordpress-migrate-page-type option:selected').text();
    var posts = $('#edit-wordpress-migrate-post-type option:selected').text();
    return 'Pages &rarr; ' + pages + '<br>Posts &rarr; ' + posts;
  });
  $('fieldset#edit-wordpress-migrate-text-formats', context).drupalSetSummary(function (context) {
    var post = $('#edit-wordpress-migrate-text-format option:selected').text();
    var comment = $('#edit-wordpress-migrate-text-format-comment option:selected').text();
    return 'Text &rarr; ' + post + '<br>Comments &rarr; ' + comment;
  });
  $('fieldset#edit-wordpress-migrate-taxonomy', context).drupalSetSummary(function (context) {
    var tags = $('#edit-wordpress-migrate-tag-vocabulary option:selected').text();
    var categories = $('#edit-wordpress-migrate-category-vocabulary option:selected').text();
    return 'Tags &rarr; ' + tags + '<br>Categories&rarr; ' + categories;
  });
  $('fieldset#edit-wordpress-migrate-path-aliases', context).drupalSetSummary(function (context) {
    var handling = $('input[name=wordpress_migrate_path_action]:checked + label').text();
    var redirects = $('#edit-wordpress-migrate-generate-redirects').is(':checked');
    if (redirects) {
      return $.trim(handling) + "; generate redirects";
    }
    else {
      return handling;
    }
  });
};
