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

  $form['membership_settings']['lifetime'] = array(
    '#type' => 'checkbox',
    '#title' => t('Lifetime membership'),
    '#default_value' => variable_get('membership_entity_lifetime_' . $type->type, ''),
    '#description' => t('Lifetime memberships will never expire.'),
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
    '#states' => array(
      'visible' => array(
        ':input[name="lifetime"]' => array('checked' => FALSE),
      ),
      'required' => array(
        ':input[name="lifetime"]' => array('checked' => FALSE),
      ),
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
      '0' => t('Unlimited'),
      '1' => t('1'),
      '2' => t('2'),
      '3' => t('3'),
      '4' => t('4'),
      '5' => t('5'),
      '6' => t('6'),
      '7' => t('9'),
      '8' => t('8'),
      '9' => t('9'),
      '10' => t('10'),
    ),
    '#default_value' => variable_get('membership_entity_cardinality_' . $type->type, ''),
    '#description' => t('Select the number of users that may belong to a single membership of this type.'),
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
  );

  if (!empty($type->type)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete membership type'),
    );
  }

  return $form;
}

/**
 * Form validation handler for membership_type_form().
 *
 * @see membership_type_form_submit()
 */
function membership_entity_type_form_validate($form, &$form_state) {
  $values = $form_state['values'];
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
  foreach (array('additional_settings__active_tab', 'submit', 'delete', 'form_build_id', 'form_token', 'form_id', 'op') as $key) {
    unset($values[$key]);
  }

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
