<?php

/**
 * @file
 * Definition of membership_entity_handler_field_link_edit.
 */

/**
 * Field handler to display a link to edit a membership.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_link_edit extends membership_entity_handler_field_link {

  /**
   * Renders the link.
   */
  function render_link($membership, $values) {
    if (!membership_entity_access('edit', $membership)) {
      return;
    }

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "membership/$membership->mid/edit";
    $this->options['alter']['query'] = drupal_get_destination();

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    return $text;
  }
}
