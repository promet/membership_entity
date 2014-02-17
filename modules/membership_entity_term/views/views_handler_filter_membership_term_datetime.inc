<?php

/**
 * @file
 * Definition of views_handler_filter_membership_term_datetime.
 */

/**
 * A handler to filter on the membership term dates.
 */
class views_handler_filter_membership_term_datetime extends views_handler_filter_date {
  function get_field($field = NULL) {
    return parent::get_field(strtotime($this->table_alias . '.' . $this->real_field));
  }
}
