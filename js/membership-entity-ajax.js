/**
 * @file
 * Adds a 'attribute' ajax command.
 */

(function($) {
  /**
   * Command to change an elment attribute.
   */
  Drupal.ajax.prototype.commands.membership_entity_attr = function(ajax, response, status) {
    $(response.selector).attr(response.attribute, response.value);
  }
})(jQuery);


