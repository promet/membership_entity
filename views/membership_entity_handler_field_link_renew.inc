<?php

/**
 * @file
 * Definition of membership_entity_handler_field_link_renew.
 */

/**
 * Field handler to present a link to renew a membership.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_link_renew extends membership_entity_handler_field_link {
  /**
   * Renders the renew link.
   */
  function render_link($membership, $values) {
    if (!membership_entity_access('view', $membership)) {
      return;
    }

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "membership/$membership->mid/renew";
    $this->options['alter']['query'] = drupal_get_destination();

    return !empty($this->options['text']) ? $this->options['text'] : t('renew');
  }
}
