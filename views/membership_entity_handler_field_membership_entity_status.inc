<?php

/**
 * @file
 * Contains the membership field handler for the status field.
 */

/**
 * Handle display of membership status field.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_membership_entity_status extends views_handler_field {
  /**
   * Render the membership status field.
   */
  function render($values) {
    $value = $this->get_value($values);

    if (!is_null($value)) {
      $status = membership_entity_get_status_list();
      return $status[$value];
    }
  }
}
