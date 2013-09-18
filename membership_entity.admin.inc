<?php

/**
 * @file
 * Administration pages and helpers for the membership entity module.
 */

/**
 * Form builder: membership settings form.
 */
function membership_entity_settings_form($form, &$form_state, $data = array()) {
  if (empty($data)) {
    $data = variable_get('membership_entity_settings', array());
  }

  // Provide some defaults.
  $data += array(
    'available_term_lengths' => array(),
    'cardinality' => 1,
    'show_on_profile' => 1,
    'all_edit' => 0,
    'primary_role' => '',
    'secondary_role' => '',
  );

  // Add membership term lengths.
  if (!isset($form_state['available_term_lengths'])) {
    $form_state['available_term_lengths'] = $data['available_term_lengths'];
  }

  $form['term_lengths'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership Term Lengths'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#description' => t('Select the term lengths that will be available for this membership type.'),
  );

  $wrapper_id = 'available-term-lengths-wrapper';
  $form['term_lengths']['available_term_lengths'] = array(
    '#theme' => 'membership_entity_available_term_lengths',
    '#id' => 'membership-entity-term-lengths',
    '#prefix' => '<div id="'. $wrapper_id . '">',
    '#suffix' => '</div>',
  );

  foreach ($form_state['available_term_lengths'] as $delta => $term_length) {
    $form['term_lengths']['available_term_lengths'][$delta] = array(
      'term_length' => array(
        '#type' => 'value',
        '#value' => $term_length,
      ),
      'example' => array(
        '#markup' => ($term_length == 'lifetime') ? t('N/A') : format_date(strtotime($term_length), 'custom', 'Y-m-d H:i:s O'),
      ),
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight for term length @term_length', array('@term_lenght' => $term_length)),
        '#title_display' => 'invisible',
        '#default_value' => $delta,
      ),
      'remove' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#name' => 'remove_' . $delta,
        '#delta' => $delta,
        '#submit' => array('membership_entity_remove_term_length'),
        '#limit_validation_errors' => array(),
        '#ajax' => array(
          'callback' => 'membership_entity_term_length_callback',
          'wrapper' => $wrapper_id,
          'effect' => 'fade',
        ),
      ),
    );
  }

  $form['term_lengths']['term_length_options'] = array(
    '#type' => 'select',
    '#title' => t('Term length'),
    '#required' => TRUE,
    '#options' => array(
      '1 day' => t('1 day'),
      '1 week' => t('1 week'),
      '1 month' => t('1 month'),
      '3 months' => t('3 months'),
      '6 months' => t('6 months'),
      '1 year' => t('1 year'),
      '2 years' => t('2 years'),
      '3 years' => t('3 years'),
      '4 years' => t('4 years'),
      '5 years' => t('5 years'),
      '10 years' => t('10 years'),
      'lifetime' => t('Lifetime'),
      'other' => t('Other'),
    ),
  );

  $form['term_lengths']['custom_term_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Other term length'),
    '#size' => 32,
    '#description' => t('Enter a term length including number and unit (eg. 15 years).'),
    '#states' => array(
      'visible' => array(
       ':input[name="term_lengths[term_length_options]"]' => array('value' => 'other'),
      ),
      'required' => array(
        ':input[name="term_lengths[term_length_options]"]' => array('value' => 'other'),
      ),
    ),
  );

  $form['term_lengths']['add_term_length'] = array(
    '#type' => 'submit',
    '#value' => t('Add term length'),
    '#validate' => array('membership_entity_add_term_length_validate'),
    '#submit' => array('membership_entity_add_term_length'),
    '#limit_validation_errors' => array(array('term_lengths')),
    '#ajax' => array(
      'callback' => 'membership_entity_term_length_callback',
      'wrapper' => $wrapper_id,
      'effect' => 'fade',
    ),
  );

  $form['user_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Member options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['user_settings']['cardinality'] = array(
    '#type' => 'select',
    '#title' => t('Number of users per membership'),
    '#options' => array(
      FIELD_CARDINALITY_UNLIMITED => t('Unlimited'),
      '1' => t('1'),
      '2' => t('2'),
      '3' => t('3'),
      '4' => t('4'),
      '5' => t('5'),
      '6' => t('6'),
      '7' => t('7'),
      '8' => t('8'),
      '9' => t('9'),
      '10' => t('10'),
    ),
    '#default_value' => $data['cardinality'],
    '#description' => t('Select the number of users that may belong to a single membership of this type. (Including the primary member.)'),
  );

  $roles = user_roles(TRUE);
  $form['user_settings']['primary_role'] = array(
    '#title' => t('Primary member role'),
    '#type' => 'select',
    '#options' => $roles,
    '#default_value' => $data['primary_role'],
    '#required' => TRUE,
    '#description' => t('Select the role that will be assigned to the primary member when the membership is active.'),
  );

  $form['user_settings']['secondary_role'] = array(
    '#title' => t('Secondary member role'),
    '#type' => 'select',
    '#options' => array('primary' => 'Same as primary member') + $roles,
    '#default_value' => ($data['secondary_role'] == $data['primary_role']) ? 'primary' : $data['secondary_role'],
    '#description' => t('Select the role that will be assigned to secondary members when the membership is active.'),
    '#states' => array(
      'invisible' => array(
       ':input[name="cardinality"]' => array('value' => '1'),
      ),
    ),
  );

  $form['user_settings']['show_on_profile'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display membership information on the user profile.'),
    '#default_value' => $data['show_on_profile'],
  );

  $form['user_settings']['all_edit'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow secondary members to edit the membership.'),
    '#default_value' => $data['all_edit'],
    '#description' => t('If not checked, only the primary member will be able to edit membership fields.'),
    '#states' => array(
      'invisible' => array(
       ':input[name="cardinality"]' => array('value' => '1'),
      ),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array('membership_entity_settings_form_submit'),
    '#limit_validation_errors' => array(
      array('primary_role'),
      array('secondary_role'),
      array('term_lengths', 'available_term_lengths'),
      array('cardinality'),
      array('show_on_profile'),
      array('all_edit'),
    ),
  );

  return $form;
}

/**
 * Ajax callback to update term lengths.
 */
function membership_entity_term_length_callback($form, $form_state) {
  return $form['term_lengths']['available_term_lengths'];
}

/**
 * Validate handler to add a term length.
 */
function membership_entity_add_term_length_validate($form, &$form_state) {
  $values = &$form_state['values']['term_lengths'];
  $term_length = $values['term_length_options'];

  // Validate a custom term length.
  if ($term_length == 'other') {
    $term_length = trim($values['custom_term_length']);
    $regex = '/^((\d+ (sec|second|min|minute|hour|day|week|month|year)s?)\s?)+$/i';
    if (!preg_match($regex, $term_length)) {// || strtotime($term_length) === FALSE) {
      form_set_error('term_lengths][custom_term_length', t('%term_length is not a valid term length. Please enter term lengths as a number and unit (eg. 15 years)', array(
        '%term_length' => $term_length,
      )));
    }
  }

  // Check for duplicates.
  if (in_array($term_length, $form_state['available_term_lengths'])) {
    form_set_error('term_lengths][term_length_options', t('The term length %term_length has already been added.', array(
      '%term_length' => $term_length,
    )));
  }
}

/**
 * Submit handler to add a term length.
 */
function membership_entity_add_term_length($form, &$form_state) {
  $values = $form_state['values']['term_lengths'];
  $term = ($values['term_length_options'] != 'other') ? $values['term_length_options'] : $term = $values['custom_term_length'];
  $form_state['available_term_lengths'][] = check_plain($term);
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler to remove a term length.
 */
function membership_entity_remove_term_length($form, &$form_state) {
  $button = $form_state['triggering_element'];
  if (isset($button['#delta'])) {
    unset($form_state['available_term_lengths'][$button['#delta']]);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 *  Form validation handler for membership_entity_settings_form().
 */
function membership_entity_settings_form_validate($form, &$form_state) {
  // Make sure at least one term length is selected.
  if (empty($form_state['values']['term_lengths']['available_term_lengths'])) {
    form_set_error('term_lengths][available_term_lengths', t('You must add at least one term length.'));
  }
}

/**
 * Form submission handler for membership_entity_settings_form().
 */
function membership_entity_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  // Prepare available term_lengths.
  $term_lengths = array();
  foreach ($values['term_lengths']['available_term_lengths'] as $item) {
    $term_lengths[$item['weight']] = $item['term_length'];
  }
  ksort($term_lengths);
  $values['available_term_lengths'] = $term_lengths;
  unset($values['term_lengths']);

  // Prepare roles.
  if ($values['secondary_role'] == 'primary') {
    $values['secondary_role'] = $values['primary_role'];
  }

  // Remove extra fields.
  unset($values['submit']);
  unset($values['op']);

  variable_set('membership_entity_settings', $values);
  drupal_set_message(t('The configuration options have been saved'));
}

/**
 * Returns HTML for the available term lengths form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 */
function theme_membership_entity_available_term_lengths($variables) {
  $element = $variables['element'];
  $order_class = $element['#id'] . '-term-weight';

  // Sort items according to weight.
  $items = array();
  foreach (element_children($element) as $key) {
    $items[] = &$element[$key];
  }

  if (!empty($items)) {
    usort($items, '_membership_entity_sort_items_by_weight');
  }

  $table_id = 'available-term-lengths';
  $header = array(t('Available Term Lengths'), t('Example (relative to now)'), '');
  $rows = array();

  if (!empty($items)) {
    $header[] = t('Order');

    // Add the items as table rows.
    foreach ($items as $key => $item) {
      $item['weight']['#attributes']['class'][] = $order_class;
      $rows[] = array(
        'data' => array(
          // We use #type => value instead of markup so that term_length will
          // be included in $form_state['values'].
          array('data' => $item['term_length']['#value']),
          array('data' => drupal_render($item['example'])),
          array('data' => drupal_render($item['remove'])),
          array('data' => drupal_render($item['weight']), 'class' => array('term-weight')),
        ),
        'class' => array('draggable'),
      );
    }
    drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);
  }
  else {
    $rows[]['data'] = array(
      array(
        'data' => t('Select a term length and click "Add term length" to add it to the list of available term lengths.'),
        'colspan' => 3,
      ),
    );
  }

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
  ));
}

/**
 * Menu callback: membership administration.
 */
function membership_entity_admin_memberships($form, &$form_state) {
  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    if (user_access('delete memberships')) {
      return membership_entity_multiple_delete_confirm($form, $form_state, array_filter($form_state['values']['memberships']));
    }
    else {
      drupal_set_message(t('You do not have permission to delete memberships.'), 'error');
    }
  }
  $form['filter'] = membership_entity_filter_form();
  $form['#submit'][] = 'membership_entity_filter_form_submit';
  $form['admin'] = membership_entity_admin_list();

  return $form;
}

/**
 * Validate membership overview form operations.
 */
function membership_entity_admin_list_validate($form, &$form_state) {
  // Make sure there are items selected.
  if (!is_array($form_state['values']['memberships']) || !count(array_filter($form_state['values']['memberships']))) {
    form_set_error('', t('No items selected.'));
  }
}

/**
 * Submit handler for membership overview form operations.
 */
function membership_entity_admin_list_submit($form, &$form_state) {
  $operations = module_invoke_all('membership_entity_operations');
  $operation = $operations[$form_state['values']['operation']];

  // Filter out unchecked items.
  $mids = array_filter($form_state['values']['memberships']);
  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array_merge(array($mids), $operation['callback arguments']);
    }
    else {
      $args = array($mids);
    }
    call_user_func_array($function, $args);
    cache_clear_all();
    // @TODO membership_entity_cache_clear_all();
  }
  else {
    // We need to rebuild the form to go to a second step. For example, to
    // show the confirmation form for the deletion of memberships.
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Build the membership administration overview.
 */
function membership_entity_admin_list() {
  $admin = user_access('manage memberships');
  $admin_delete = user_access('delete memberships');

  // Build the 'Update options' form.
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#attributes' => array('class' => array('container-inline')),
    '#access' => $admin,
  );
  $options = array();
  foreach (module_invoke_all('membership_entity_operations') as $operation => $array) {
    $options[$operation] = $array['label'];
  }
  $form['options']['operation'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#title_display' => 'invisible',
    '#options' => $options,
  );
  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array('membership_entity_admin_list_validate'),
    '#submit' => array('membership_entity_admin_list_submit'),
  );

  // Build the sortable table header.
  $header = array(
    'member_id' => array('data' => t('Member ID'), 'field' => 'me.type'),
    'status' => array('data' => t('Status'), 'field' => 'me.status'),
    'created' => array('data' => t('Created'), 'field' => 'me.created', 'sort' => 'desc'),
    'changed' => array('data' => t('Last Updated'), 'field' => 'me.updated'),
    'operations' => array('data' => t('Operations')),
  );

  // Build the query.
  $query = db_select('membership_entity', 'me')
    ->extend('PagerDefault')
    ->extend('TableSort');
  _membership_entity_build_filter_query($query);
  $mids = $query
    ->fields('me', array('mid'))
    ->limit(50)
    ->orderByHeader($header)
    ->execute()
    ->fetchCol();
  $memberships = membership_entity_load_multiple($mids);

  // Prepare the list of memberships.
  $destination = drupal_get_destination();
  $options = array();
  $statuses = membership_entity_get_status_list();
  foreach ($memberships as $membership) {
    $options[$membership->mid] = array(
      'member_id' => l(check_plain($membership->member_id), 'membership/' . $membership->mid),
      'status' => $statuses[$membership->status],
      'created' => format_date($membership->created, 'short'),
      'changed' => format_date($membership->changed, 'short'),
    );

    // Build a list of all the accessible operations for the current membership.
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => 'membership/' . $membership->mid . '/edit',
      'query' => $destination,
    );

    if ($admin_delete) {
      $operations['delete'] = array(
        'title' => t('delete'),
        'href' => 'membership/' . $membership->mid . '/delete',
        'query' => $destination,
      );
    }
    if (count($operations) > 1) {
      $options[$membership->mid]['operations'] = array(
        'data' => array(
          '#theme' => 'links__membership_entity_operations',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
    }
    else {
      $link = reset($operations);
      $options[$membership->mid]['operations'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        ),
      );
    }
  }

  $form['memberships'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No content available.'),
  );

  $form['pager'] = array('#markup' => theme('pager'));
  return $form;
}

/**
 * Return form for membership administration filters.
 */
function membership_entity_filter_form() {
  $session = isset($_SESSION['membership_entity_overview_filter']) ? $_SESSION['membership_entity_overview_filter'] : array();
  $filters = membership_entity_filters();

  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Show only memberships where'),
    '#theme' => 'exposed_filters__membership_entity',
  );

  $i = 0;
  foreach ($session as $filter) {
    list($type, $value) = $filter;
    $value = $filters[$type]['options'][$value];

    $t_args = array('%property' => $filters[$type]['title'], '%value' => $value);
    if ($i++) {
      $form['filters']['current'][] = array('#markup' => t('and where %property is %value', $t_args));
    }
    else {
      $form['filters']['current'][] = array('#markup' => t('where %property is %value', $t_args));
    }

    if (in_array($type, array('type', 'status'))) {
      // Remove the option if it is already being filtered on.
      unset($filters[$type]);
    }
  }

  $form['filters']['status'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
    '#prefix' => ($i ? '<div class="additional-filters">' . t('and where') . '</div>' : ''),
  );
  $form['filters']['status']['filters'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('filters')),
  );
  foreach ($filters as $key => $filter) {
    $form['filters']['status']['filters'][$key] = array(
      '#type' => 'select',
      '#options' => $filter['options'],
      '#title' => $filter['title'],
      '#default_value' => '[any]',
    );
  }

  $form['filters']['status']['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['filters']['status']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => count($session) ? t('Refine') : t('Filter'),
  );
  if (count($session)) {
    $form['filters']['status']['actions']['undo'] = array('#type' => 'submit', '#value' => t('Undo'));
    $form['filters']['status']['actions']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  drupal_add_js('misc/form.js');

  return $form;
}

/**
 * Process result from membership administration filter form.
 */
function membership_entity_filter_form_submit($form, &$form_state) {
  $filters = membership_entity_filters();
  switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      // Apply every filter that has a choice selected other than 'any'.
      foreach ($filters as $filter => $options) {
        if (isset($form_state['values'][$filter]) && $form_state['values'][$filter] != '[any]') {
          // Flatten the options array to accommodate hierarchical/nested options.
          $flat_options = form_options_flatten($filters[$filter]['options']);
          // Only accept valid selections offered on the dropdown, block bad input.
          if (isset($flat_options[$form_state['values'][$filter]])) {
            $_SESSION['membership_entity_overview_filter'][] = array($filter, $form_state['values'][$filter]);
          }
        }
      }
      break;

    case t('Undo'):
      array_pop($_SESSION['membership_entity_overview_filter']);
      break;

    case t('Reset'):
      $_SESSION['membership_entity_overview_filter'] = array();
      break;
  }
}

/**
 * List administration filters that can be applied.
 */
function membership_entity_filters() {
  $filters['status'] = array(
    'title' => t('status'),
    'options' => array(
      '[any]' => t('any'),
    ) + membership_entity_get_status_list(),
  );
  return $filters;
}

/**
 * Apply filters for membership administration list based on session.
 *
 * @param $query
 *   A SelectQuery to which the filters should be applied.
 */
function _membership_entity_build_filter_query(SelectQueryInterface $query) {
  $filter_data = isset($_SESSION['membership_entity_overview_filter']) ? $_SESSION['membership_entity_overview_filter'] : array();
  foreach ($filter_data as $index => $filter) {
    list($key, $value) = $filter;
    switch ($key) {
      case 'status':
        $query->condition('me.' . $key, $value);
        break;
    }
  }
}

/**
 * Sort by weight value.
 *
 * Callback for usort() within theme_membership_entity_available_term_lengths().
 */
function _membership_entity_sort_items_by_weight($a, $b) {
  $a_weight = (is_array($a) && isset($a['weight']['#value']) ? $a['weight']['#value'] : 0);
  $b_weight = (is_array($b) && isset($b['weight']['#value']) ? $b['weight']['#value'] : 0);
  return $a_weight - $b_weight;
}
