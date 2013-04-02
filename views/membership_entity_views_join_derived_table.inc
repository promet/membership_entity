<?php

/**
 * @file
 * Defines handler objects to extend views queries.
 */

/**
 * Join handler for derived table relationships.
 */
class membership_entity_views_join_derived_table extends views_join {
  /**
   * Class constructor.
   *
   * Calls parent constructor and sets variables.
   */
  function construct($left_field = NULL, $field = NULL, $type = 'LEFT') {
    parent::construct(NULL, $left_field, NULL, $field, array(), $type);
    $this->derived_table = $this->definition['derived_table'];
    $this->condition = $this->definition['condition'];
    $this->arguments = $this->definition['arguments'];
  }

  /**
   * Build the SQL for the derived table.
   *
   * @param $select_query
   *   An implementation of SelectQueryInterface.
   * @param $table
   *   The base table to join.
   * @param $view_query
   *   The source query, implementation of views_plugin_query.
   */
  function build_join($select_query, $table, $view_query) {
    if (count($this->arguments) == 0) {
      $this->arguments = array();
    }

    $select_query->addJoin($this->type, $this->derived_table, $table['alias'], $this->condition, $this->arguments);
  }
}
