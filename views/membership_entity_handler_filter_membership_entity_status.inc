<?php

/**
 * @file
 * Contains the membership filter handler for the status field.
 */

/**
 * Handle display of membership status filter.
 *
 * @ingroup views_filter_handlers
 */
class membership_entity_handler_filter_membership_entity_status extends views_handler_filter_many_to_one {
  /**
   * Set options to filter on.
   */
  function get_value_options() {
    $this->value_options = membership_entity_get_status_list();
  }

  /**
   * Remove empty and not empty operators.
   */
  function operators() {
    $operators = parent::operators();
    unset($operators['empty'], $operators['not empty']);
    return $operators;
  }
}
