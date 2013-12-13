<?php

/**
 * @file
 * Defines the MembershipEntityTermViews Controller class.
 */

/**
 * Membership entity term views controller class.
 */
class MembershipEntityTermViewsController extends EntityDefaultViewsController {
  /**
   * Overrides EntityDefaultViewsController::views_data().
   */
  public function views_data() {
    $data = parent::views_data();
    foreach (array('start', 'end') as $field) {
      $data['membership_entity_term'][$field]['field']['handler'] = 'membership_entity_term_views_handler_field_datetime';
      $data['membership_entity_term'][$field]['filter']['handler'] = 'membership_entity_term_views_handler_filter_datetime';
    }
    return $data;
  }
}

/**
 * Field handler for datetime fields.
 */
class membership_entity_term_views_handler_field_datetime extends views_handler_field_date {
  /**
   * Overrides views_handler_field_date::get_value().
   */
  function get_value($values, $field = NULL) {
    return strtotime(parent::get_value($values, $field));
  }
}

/**
 * Filter handler for datetime fields.
 */
class membership_entity_term_views_handler_filter_datetime extends views_handler_filter_date {
  /**
   * Overrides views_handler_filter_date::get_field().
   */
  function get_field($field = NULL) {
    return parent::get_field(strtotime($this->table_alias . '.' . $this->real_field));
  }
}
