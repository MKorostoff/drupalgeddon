
/**
 * @file
 * Provides the administration JavaScript for the AntiSpam module.
 */

/**
 * The AntiSpam administration JavaScript behavior.
 */
(function ($) {
  Drupal.behaviors.antispam = {
    attach : function(context, settings) {
      // Hide everything but the active provider.
      var active = $('input[name="antispam_service_provider"]:checked').attr('value');
      $('.antispam-wrapper').hide();
      $('#antispam-wrapper-' + active).show();
  
      // Switch the active provider with user input.
      $('input[name="antispam_service_provider"]').click(function() {
        var active = $('input[name="antispam_service_provider"]:checked').attr('value');
        $('.antispam-wrapper').hide();
        $('#antispam-wrapper-' + active).show();
      });
    }
  };
})(jQuery);
