<?php

/**
 * @file
 * Handles managing membership types.
 */

/**
 * Page callback to display existing membership types.
 */
function membership_entity_types() {
  $types = array();

  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => '2'));
  $rows = array();
  foreach ($types as $type => $name) {
    $type_url_str = str_replace('_', '-', $type);

    $row = array(
      array('data' => t('@name (Machine name: @type)', array('@name' => $name, '@type' => $type))),
      // Set the edit column.
      array('data' => l(t('edit'), 'admin/structure/membership-types/' . $type_url_str)),
      // Set the delete column.
      array('data' => l(t('delete'), 'admin/structure/membership-types/' . $type_url_str . '/delete')),
    );

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
  // Make the type object available to implementations of hook_form_alter.
  $form['#membership_type'] = $type;

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    //'#default_value' => $type->name,
    '#description' => t('The human-readable name of this membership type. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['type'] = array(
    '#type' => 'machine_name',
    //'#default_value' => $type->type,
    '#maxlength' => 32,
    //'#disabled' => $type->locked,
    '#machine_name' => array(
      'exists' => 'membership_entity_type_load',
    ),
    '#description' => t('A unique machine-readable name for this membership type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    //'#default_value' => $type->description,
    '#description' => t('Describe this membership type.'),
  );

  $form['help']  = array(
    '#type' => 'textarea',
    '#title' => t('Explanation or submission guidelines'),
    //'#default_value' => $type->help,
    '#description' => t('This text will be displayed at the top of the page when creating or editing a membership of this type.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save membership type'),
    '#weight' => 40,
  );

  if (!empty($type->type)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete membership type'),
      '#weight' => 45,
    );
  }

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['expiration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership expiration settings'),
    '#collapsible' => TRUE,
    '#group' => 'additional_settings',
  );

  $form['expiration']['expires'] = array(
    '#title' => t('Membership expires'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('Enter the date this type of membership will expire. You can enter a relative dates such as \'+1 year\''),
  );

  $form['user'] = array(
    '#type' => 'fieldset',
    '#title' => t('Member options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['user']['cardinality'] = array(
    '#type' => 'select',
    '#title' => t('Cardinality'),
    '#options' => array(
      '0' => t('Unlimited'),
      '1' => t('1'),
      '2' => t('2'),
      '3' => t('3'),
    ),
    '#description' => t('Select the number of users that may belong to a single membership of this type.'),
  );
  $form['user']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Allowed roles.'),
    '#options' => array(
      'role1' => t('Role 1'),
      'role2' => t('Role 2'),
      'role3' => t('Role 3'),
      'role4' => t('Role 4'),
    ),
    '#description' => t('Select the user roles that may be part of this type of membership.'),
  );


  return $form;
}

/**
 * Form validation handler for membership_type_form().
 *
 * @see membership_type_form_submit()
 */
function membership_type_form_validate($form, &$form_state) {
  $type = new stdClass();
  $type->type = trim($form_state['values']['type']);
  $type->name = trim($form_state['values']['name']);

  /*
  $types = node_type_get_names();
  $names = array_flip($types);

  if (isset($names[$type->name])) {
    form_set_error('name', t('The human-readable name %name is already taken.', array('%name' => $type->name)));
  }
  */
}

/**
 * Form submission handler for membership_type_form().
 *
 * @see membership_type_form_validate()
 */
function membership_type_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  //$type = node_type_set_defaults();

  $type->type = trim($form_state['values']['type']);
  $type->name = trim($form_state['values']['name']);
  $type->description = $form_state['values']['description'];
  $type->help = $form_state['values']['help'];

  if ($op == t('Delete content type')) {
    $form_state['redirect'] = 'admin/structure/membership-types/' . str_replace('_', '-', $type->type) . '/delete';
    return;
  }

  $form_state['redirect'] = 'admin/structure/memberhsip-types';
  return;
}

/**
 * Menu callback; delete a single content type.
 */
/*
function node_type_delete_confirm($form, &$form_state, $type) {
  $form['type'] = array('#type' => 'value', '#value' => $type->type);
  $form['name'] = array('#type' => 'value', '#value' => $type->name);

  $message = t('Are you sure you want to delete the content type %type?', array('%type' => $type->name));
  $caption = '<p>' . t('This action cannot be undone.') . '</p>';

  return confirm_form($form, $message, 'admin/structure/membership-types', $caption, t('Delete'));
}
*/

/**
 * Process content type delete confirm submissions.
 */
/*
function node_type_delete_confirm_submit($form, &$form_state) {
  node_type_delete($form_state['values']['type']);

  $t_args = array('%name' => $form_state['values']['name']);
  drupal_set_message(t('The membership type %name has been deleted.', $t_args));
  watchdog('membership_entity', 'Deleted membership type %name.', $t_args, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/structure/membership-types';
  return;
}
*/
