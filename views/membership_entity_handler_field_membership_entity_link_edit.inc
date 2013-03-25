<?php

/**
 * @file
 * Definition of views_handler_field_membership_entity_link_edit.
 */

/**
 * Field handler to present a link membership edit.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_membership_entity_link_edit extends membership_entity_handler_field_membership_entity_link {

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
