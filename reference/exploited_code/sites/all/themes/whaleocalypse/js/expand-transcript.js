(function($) {
$('.field-name-field-transcript .field-items').hide();
$('.field-name-field-transcript h3.field-label').text('View Transcript ▼')
$('.field-name-field-transcript h3.field-label').click(function() {
	$('.field-name-field-transcript .field-items').slideToggle();
});
$('.field-name-field-transcript h3.field-label').toggle(function() {
    $(this).text('Hide Transcript ▲');
}, function() {
    $(this).text('View Transcript ▼');
});
})(jQuery);