<?php

/**
 * @file
 * Definition of membership_entity_handler_field_date.
 */

/**
 * A handler to provide proper displays for membership dates.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_terms_date extends views_handler_field_date {
  /**
   * Class constructor.
   */
  function construct() {
    parent::construct();

    // Define additional fields to add to the query.
    $this->additional_fields['mid'] = 'mid';
  }

  /**
   * Add the additional fields to the query.
   */
  function query() {
    parent::query();
    $this->add_additional_fields();
  }

  /**
   * Combine multiple rows of membership term dates into a single row for
   * display.
   */
  function pre_render(&$values) {
    $last = 0;

    $mid_field = $this->table_alias . '_mid';

    foreach ($values as $key => $value) {
      if ($value->{$mid_field} == $last) {
        foreach ($this->view->field as $k => $field) {
          // Check that each field value we alter has the correct handler.
          if ($field->definition['handler'] == $this->definition['handler']) {
            $alias = $field->table_alias . '_' . $field->real_field;
            $values[$key - 1]->{$alias} .= ',' . $value->{$alias};
          }
        }

        // Remove extra rows.
        unset($values[$key]);
      }

      $last = $value->{$mid_field};
    }
  }

  /**
   * Determine output of membership date fields.
   */
  function render(&$values) {
    // Need to set the field alias here since the value gets lost when
    // implementing the query method in this class.
    $this->field_alias = $this->table_alias . '_' . $this->real_field;

    $dates = explode(',', $this->get_value($values));

    foreach ($dates as $key =>$date) {
      $values->{$this->field_alias} = $date;
      $dates[$key] = parent::render($values);
    }

    return implode(', ', $dates);
  }
}
