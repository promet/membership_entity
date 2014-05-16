<?php

/**
 * @file
 * Definition of views_handler_filter_membership_term_datetime.
 */

/**
 * A handler to filter on the membership term dates.
 */
class views_handler_filter_membership_term_datetime extends views_handler_filter_date {
  public function op_between($field) {
    if (!empty($this->value['type']) && $this->value['type'] == 'offset') {
      $a = new DateObject('now', 'UTC');
      $b = new DateObject('now', 'UTC');

      $a = _membership_entity_term_modify_date($a, $this->value['min']);
      $b = _membership_entity_term_modify_date($b, $this->value['max']);
    }
    else {
      $a = new DateObject($this->value['min'], 'UTC');
      $b = new DateObject($this->value['max']. 'UTC');
    }

    // Format as DATETIME.
    $a = date_format_date($a, 'custom', DATE_FORMAT_DATETIME);
    $b = date_format_date($b, 'custom', DATE_FORMAT_DATETIME);

    $op = strtoupper($this->operator);
    $where = "$field $op :min AND :max";
    $this->query->add_where_expression($this->options['group'], $where, array(':min' => $a, ':max' => $b));
  }

  public function op_simple($field) {
    if (!empty($this->value['type']) && $this->value['type'] == 'offset') {
      $value = new DateObject('now', 'UTC');
      $value = _membership_entity_term_modify_date($value, $this->value['min']);
    }
    $value = date_format_date($value, 'custom', DATE_FORMAT_DATETIME);
    $this->query->add_where_expression($this->options['group'], "$field $this->operator :value", array(':value' => $value));
  }
}
