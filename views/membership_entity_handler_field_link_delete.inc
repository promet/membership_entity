<?php

/**
 * @file
 * Definition of membership_entity_handler_field_link_delete.
 */

/**
 * Field handler to present a link to delete a membership.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_link_delete extends membership_entity_handler_field_link {

  /**
   * Renders the link.
   */
  function render_link($membership, $values) {
    if (!membership_entity_access('delete', $membership)) {
      return;
    }

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "membership/$membership->mid/delete";
    $this->options['alter']['query'] = drupal_get_destination();

    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return $text;
  }
}
