<?php

/**
 * @file
 * Definition of membership_entity_handler_field_type.
 */

/**
 * Field handler to display the membership type.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_type extends views_handler_field {
  /**
   * Renders the membership type.
   */
  function render($values) {
    $value = $this->get_value($values);
    $types = membership_entity_get_types();
    return $this->sanitize_value($types[$value]->name);
  }
}
