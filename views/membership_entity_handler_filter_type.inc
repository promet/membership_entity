<?php

/**
 * @file
 * Definition of membership_entity_handler_filter_type.
 */

/**
 * Filter by membership type.
 *
 * @ingroup views_filter_handlers
 */
class membership_entity_handler_filter_type extends views_handler_filter_in_operator {
  /**
   * Get types options.
   */
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Membership types');
      $types = membership_entity_type_get_types();
      $options = array();
      foreach ($types as $type => $info) {
        $options[$type] = t($info->name);
      }
      asort($options);
      $this->value_options = $options;
    }
  }
}
