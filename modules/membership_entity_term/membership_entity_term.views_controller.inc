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
 * A handler to filter on the membership join and start dates.
 */
class membership_entity_term_views_handler_filter_datetime extends views_handler_filter_date {
  /**
   * Provide defaults for form fields.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['date_multiple'] = array('default' => 'all');
    return $options;
  }

  /**
   * Extend the filter form options.
   */
  function value_form(&$form, &$form_state) {
    // Don't display in exposed form.
    if (empty($form_state['exposed'])) {
      $form['date_multiple'] = array(
        '#type' => 'select',
        '#title' => t('Membership Term Filter'),
        '#description' => t('Select how to handle filtering of membership term dates.'),
        '#options' => array(
          'all' => t('All Terms'),
          'most recent' => t('Most Recent Term'),
          'first' => t('First Term'),
        ),
        '#default_value' => isset($this->options['date_multiple']) ? $this->options['date_multiple'] : 'all',
        '#weight' => 10,
      );
    }

    parent::value_form($form, $form_state);
  }

  /**
   * Overrides views_handler_filter_numeric::operators().
   */
  function operators() {
    $operators = parent::operators();

    // Remove operators we don't need.
    foreach (array('not between', 'empty', 'not empty', 'regular_expression') as $op) {
      unset($operators[$op]);
    }

    return $operators;
  }

  /**
   * Overrides views_handler_filter_date::get_field().
   */
  function get_field($field = NULL) {
    return parent::get_field(strtotime($this->table_alias . '.' . $this->real_field));
  }

  /**
   * Overrides views_handler_filter_date::op_simple().
   */
  function op_simple($field) {
    $value = intval(strtotime($this->value['value'], 0));
    $value = format_date($value, 'custom', 'Y-m-d');

    if ($this->options['date_multiple'] == 'all') {
      parent::op_simple($field);
    }
    else {
      $op = $this->operator;
      $agg = $this->options['date_multiple'] == 'first' ? 'MIN' : 'MAX';
      $where = "(SELECT $agg($this->field) FROM {membership_entity_term} WHERE mid = $this->table_alias.mid) $op :val";
      $this->query->add_where_expression($this->options['group'], $where, array(':val' => $value));
    }
  }

  /**
   * Overrides views_handler_filter_date::op_between().
   */
  function op_between($field) {
    $min = intval(strtotime($this->value['min'], 0));
    $max = intval(strtotime($this->value['max'], 0));

    $min = format_date($min, 'custom', 'Y-m-d');
    $max = format_date($max, 'custom', 'Y-m-d');

    if ($this->options['date_multiple'] == 'all') {
      parent::op_between();
    }
    else {
      $op = drupal_strtoupper($this->operator);
      $agg = $this->options['date_multiple'] == 'first' ? 'MIN' : 'MAX';
      $where = "(SELECT $agg($this->field) FROM {membership_entity_term} WHERE mid = $this->table_alias.mid) $op :min AND :max";
      $this->query->add_where_expression($this->options['group'], $where, array(':min' => $min, ':max' => $max));
    }
  }
}
