<?php

/**
 * @file
 * Handles managing membership types.
 */

/**
 * Page callback to display existing membership types.
 */
function membership_entity_types() {
  $types = membership_entity_get_types();
  $field_ui = module_exists('field_ui');

  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => $field_ui ? '4' : '2'));
  $rows = array();
  foreach ($types as $type) {
    $type_url_str = str_replace('_', '-', $type->type);
    $row = array(check_plain($type->name) . ' <small>(Machine name: ' . $type->type . ')</small><div class="description">' . $type->description . '</div>');
    // Set the edit column.
    $row[] = l(t('edit'), 'admin/structure/membership-types/manage/' . $type_url_str);

    if ($field_ui) {
      // Manage fields.
      $row[] = array('data' => l(t('manage fields'), 'admin/structure/membership-types/manage/' . $type_url_str . '/fields'));

      // Display fields.
      $row[] = array('data' => l(t('manage display'), 'admin/structure/membership-types/manage/' . $type_url_str . '/display'));
    }

    // Set the delete column.
    $row[] = l(t('delete'), 'admin/structure/membership-types/manage/' . $type_url_str . '/delete');
    $rows[] = $row;
  }

  $output['membership_types_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No membership types available. <a href="@link">Add membership type</a>.', array('@link' => url('admin/structure/membership-types/add'))),
  );

  return $output;
}

/**
 * Form builder for the membership type form.
 *
 * @param $type
 *   (optional) An object representing the membership type.
 *
 * @see membership_type_form_validate()
 * @see membership_type_form_submit()
 */
function membership_entity_type_form($form, &$form_state, $type = NULL) {
  $roles = user_roles(TRUE);

  if (empty($type)) {
    // Set some defaults.
    $type = (object) array('type' => '', 'name' => '', 'description' => '');
  }

  // Make the type object available to implementations of hook_form_alter.
  $form['#membership_type'] = $type;

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => $type->name,
    '#description' => t('The human-readable name of this membership type. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => $type->type,
    '#maxlength' => 32,
    '#disabled' => !empty($type->type),
    '#machine_name' => array(
      'exists' => 'membership_entity_type_load',
    ),
    '#description' => t('A unique machine-readable name for this membership type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $type->description,
    '#description' => t('Describe this membership type.'),
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['membership_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership settings'),
    '#collapsible' => TRUE,
    '#group' => 'additional_settings',
  );

  $form['membership_settings']['active_role'] = array(
    '#title' => t('Member active role'),
    '#type' => 'select',
    '#options' => $roles,
    '#default_value' => variable_get('membership_entity_active_role_' . $type->type, ''),
    '#required' => TRUE,
    '#description' => t('Select the role that will be assigned to users when the membership is active.'),
  );

  $form['membership_settings']['expired_role'] = array(
    '#title' => t('Member expired role'),
    '#type' => 'select',
    '#options' => $roles,
    '#default_value' => variable_get('membership_entity_expired_role_' . $type->type, ''),
    '#description' => t('Select the role that will be assigned to users when the membership expires.'),
  );

  $wrapper_id = 'available-term-lengths-wrapper';
  $form['membership_settings']['term_lengths'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership Term Lengths'),
    '#tree' => TRUE,
    '#prefix' => '<div id="'. $wrapper_id . '">',
    '#suffix' => '</div>',
    '#description' => t('Select the term lengths that will be available for this membership type.'),
    '#states' => array(

    ),
  );

  $form['membership_settings']['term_lengths']['available_term_lengths'] = array(
    '#theme' => 'membership_entity_type_available_term_lengths',
    '#id' => 'membership-entity-term-lengths',
  );

  if (!isset($form_state['available_term_lengths'])) {
    $form_state['available_term_lengths'] = variable_get('membership_entity_term_lengths_' . $type->type, array());
  }
  foreach ($form_state['available_term_lengths'] as $weight => $term_length) {
    $form['membership_settings']['term_lengths']['available_term_lengths'][$weight] = array(
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
        '#default_value' => $weight,
      ),
      'remove' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#name' => 'remove_' . $weight,
        '#membership_entity_term_length_weight' => $weight,
        '#submit' => array('membership_entity_types_remove_term_length'),
        '#limit_validation_errors' => array(),
        '#ajax' => array(
          'callback' => 'membership_entity_types_term_length_callback',
          'wrapper' => $wrapper_id,
          'effect' => 'fade',
        ),
      ),
    );
  }

  $form['membership_settings']['term_lengths']['term_length_options'] = array(
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

  $form['membership_settings']['term_lengths']['custom_term_length'] = array(
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

  $form['membership_settings']['term_lengths']['add_term_length'] = array(
    '#type' => 'submit',
    '#value' => t('Add term length'),
    '#validate' => array('membership_entity_types_add_term_length_validate'),
    '#submit' => array('membership_entity_types_add_term_length'),
    '#limit_validation_errors' => array(array('term_lengths')),
    '#ajax' => array(
      'callback' => 'membership_entity_types_term_length_callback',
      'wrapper' => $wrapper_id,
      'effect' => 'fade',
    ),
  );

  $form['user_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Member options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
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
    '#default_value' => variable_get('membership_entity_cardinality_' . $type->type, ''),
    '#description' => t('Select the number of users that may belong to a single membership of this type. (Including the primary member.)'),
  );

  $form['user_settings']['show_on_profile'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display membership information on the user profile.'),
    '#default_value' => variable_get('membership_entity_show_on_profile_' . $type->type, 1),
  );

  $form['user_settings']['all_edit'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow all members to edit the memberships.'),
    '#default_value' => variable_get('membership_entity_all_edit_' . $type->type, 0),
    '#description' => t('If not checked, only the primary member will be able to edit membership fields.'),
  );

  $form['user_settings']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Allowed roles'),
    '#options' => $roles,
    '#default_value' => variable_get('membership_entity_roles_' . $type->type, array()),
    '#description' => t('Select the user roles that may be part of this type of membership. If you select no roles, the membership will be available to all users.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save membership type'),
    '#submit' => array('membership_entity_type_form_submit'),
    '#limit_validation_errors' => array(
      array('name'),
      array('type'),
      array('description'),
      array('active_role'),
      array('expired_role'),
      array('term_lengths', 'available_term_lengths'),
      array('cardinality'),
      array('show_on_profile'),
      array('all_edit'),
      array('roles'),
    ),
  );

  if (!empty($type->type)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete membership type'),
      '#submit' => array('membership_entity_type_form_submit'),
      '#limit_validation_errors' => array(),
    );
  }

  return $form;
}

/**
 * Returns HTML for the available term lengths form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 */
function theme_membership_entity_type_available_term_lengths($variables) {
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
 * Ajax callback to update term lengths.
 */
function membership_entity_types_term_length_callback($form, $form_state) {
  return $form['membership_settings']['term_lengths'];
}

/**
 * Validate handler to add a term length.
 */
function membership_entity_types_add_term_length_validate($form, &$form_state) {
  $values = &$form_state['values']['term_lengths'];

  // Validate a custom term length.
  if ($values['term_length_options'] == 'other') {
    if (empty($values['custom_term_length'])) {
      form_set_error('term_lengths][custom_term_length', t('<em>Other term length</em> is required.'));
    }

    $term_length = trim($values['custom_term_length']);
    $regex = '/^((\d+ (sec|second|min|minute|hour|day|week|month|year)s?)\s?)+$/i';
    if (!preg_match($regex, $term_length)) {// || strtotime($term_length) === FALSE) {
      form_set_error('term_lengths][custom_term_length', t('%term_length is not a valid term length. Please enter term lengths as a number and unit (eg. 15 years)', array(
        '%term_length' => $term_length,
      )));
    }
  }
}

/**
 * Submit handler to add a term length.
 */
function membership_entity_types_add_term_length($form, &$form_state) {
  $values = $form_state['values']['term_lengths'];
  $term = ($values['term_length_options'] != 'other') ? $values['term_length_options'] : $term = $values['custom_term_length'];
  $weight = isset($form_state['available_term_lengths']) ? count($form_state['available_term_lengths']) : 0;
  $form_state['available_term_lengths'][$weight] = check_plain($term);
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler to remove a term length.
 */
function membership_entity_types_remove_term_length($form, &$form_state) {
  $button = $form_state['triggering_element'];
  if (isset($button['#membership_entity_term_length_weight'])) {
    $weight = $button['#membership_entity_term_length_weight'];
    unset($form_state['available_term_lengths'][$weight]);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Form validation handler for membership_type_form().
 *
 * @see membership_type_form_submit()
 */
function membership_entity_type_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  $type = trim($values['type']);
  $name = trim($values['name']);

  // Check for duplicate name.
  $names = membership_entity_get_type_names();
  if (in_array($name, $names) && $names[$type] != $name) {
    form_set_error('name', t('The human-readable name %name is already taken.', array('%name' => $name)));
  }
}

/**
 * Form submission handler for membership_type_form().
 *
 * @see membership_type_form_validate()
 */
function membership_entity_type_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $op = isset($values['op']) ? $values['op'] : '';

  $type = $form_state['complete form']['#membership_type'];
  $type->type = trim($values['type']);
  $type->name = trim($values['name']);
  $type->description = $values['description'];

  if ($op == t('Delete membership type')) {
    $form_state['redirect'] = 'admin/structure/membership-types/manage/' . str_replace('_', '-', $type->type) . '/delete';
    return;
  }

  // Remove database fields. Everything that's left is a variable.
  foreach ($values as $key => $value) {
    if (isset($type->$key)) {
      unset($values[$key]);
    }
  }

  // Remove extra fields.
  foreach (array('submit', 'op') as $key) {
    unset($values[$key]);
  }

  // Save term lengths.
  $term_data = array();
  foreach ($values['term_lengths']['available_term_lengths'] as $item) {
    $term_data[$item['weight']] = $item['term_length'];
  }
  ksort($term_data);
  variable_set('membership_entity_term_lengths_' . $type->type, $term_data);
  unset($values['term_lengths']);

  // Save the variables.
  foreach ($values as $key => $value) {
    $variable = 'membership_entity_' . $key . '_' . $type->type;

    if (is_array($value)) {
      $value = array_keys(array_filter($value));
    }
    variable_set($variable, $value);
  }

  // Save the type.
  $status = membership_entity_type_save($type);

  $t_args = array('%name' => $type->name);
  switch ($status) {
    case SAVED_NEW:
      drupal_set_message(t('The membership type %name has been created.', $t_args));
      membership_entity_add_fields($type);
      break;

    case SAVED_UPDATED:
      drupal_set_message(t('The membership type %name has been updated.', $t_args));
      break;
  }

  $form_state['redirect'] = 'admin/structure/membership-types';
  return;
}

/**
 * Form builder for the membership type delete form.
 *
 * @see membership_entity_type_confirm_delete_submit().
 */
function membership_entity_type_confirm_delete($form, &$form_state, $type) {
  $form['#membership_type'] = $type;
  $message = t('Are you sure you want to delete the membership type %type?', array('%type' => $type->name));
  $caption = '<p>' . t('This action cannot be undone.') . '</p>';
  return confirm_form($form, $message, 'admin/structure/membership-types', $caption, t('Delete'));
}

/**
 * Submit handler for the memberhip type delete form.
 */
function membership_entity_type_confirm_delete_submit($form, &$form_state) {
  $type = $form_state['complete form']['#membership_type'];
  membership_entity_type_delete($type);
  drupal_set_message(t('The membership type %name has been deleted.', array('%name' => $type->name)));
  $form_state['redirect'] = 'admin/structure/membership-types';
  return;
}

/**
 * Sort callback to sort by weight.
 */
function _membership_entity_sort_items_by_weight($a, $b) {
  $a_weight = (is_array($a) && isset($a['weight']['#value']) ? $a['weight']['#value'] : 0);
  $b_weight = (is_array($b) && isset($b['weight']['#value']) ? $b['weight']['#value'] : 0);
  return $a_weight - $b_weight;
}
