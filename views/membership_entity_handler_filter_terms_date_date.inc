<?php

/**
 * @file
 * Definition of membership_entity_handler_filter_terms_date_date.
 */

/**
 * A handler to provide proper filtering for membership dates.
 *
 * @ingroup views_filter_handlers
 */
if (class_exists('date_views_filter_handler')) {
  class membership_entity_handler_filter_terms_date_date extends date_views_filter_handler {
    /**
     * Provide defaults for form fields.
     */
    function option_definition() {
      $options = parent::option_definition();
      $options['date_multiple'] = array('default' => 'most recent');
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
          ),
          '#default_value' => isset($this->options['date_multiple']) ? $this->options['date_multiple'] : 'most recent',
          '#weight' => 10,
        );
      }

      parent::value_form($form, $form_state);
    }

    /**
     * Make changes to the views query object to support display of membership
     * date fields.
     */
    function query() {
      parent::query();
      $this->ensure_my_table();

      // Check if derived table relationship has already been added.
      if (!isset($this->query->relationships[$this->table_alias . '_derived'])) {
         switch ($this->options['date_multiple']) {
          case 'all':
            break;

          case 'most recent':
          default:
            // Set derived table alias.
            $alias = $this->table_alias . '_derived';

            // Derived table query.
            $query = db_select('membership_entity_terms', 'st');
            $query->addField('st', 'mid');
            $query->addExpression('MAX(st.delta)', 'delta');
            $query->groupBy('st.mid');

            // Join definition.
            $def = $this->definition;
            $def['field'] = 'mid';
            $def['left_field'] = 'mid';
            $def['type'] = 'INNER';
            $def['derived_table'] = $query;
            $def['condition'] = "$alias.mid = $this->table_alias.mid AND $alias.delta = $this->table_alias.delta";

            // Create the join object.
            $join = new membership_entity_views_join_derived_table();
            $join->definition = $def;
            $join->construct();
            $join->adjusted = TRUE;

            // Add the join relationship to the views query.
            $this->query->add_relationship($alias, $join, $this->definition['base'], $this->relationship);
        }
      }
    }
  }
}
