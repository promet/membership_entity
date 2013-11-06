<?php

/**
 * @file
 * Contains the MembershipEntityTypeUIController.
 */

/**
 * Membership type UI controller.
 */
class MembershipEntityTypeUIController extends EntityDefaultUIController {
  /**
   * Overrides EntityDefaultUIController::hook_menu().
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage membership types and fields.';
    return $items;
  }
}

/**
 *  Creates the membership entity type edit form.
 */
function membership_entity_type_form($form, &$form_state, $type, $op = 'edit') {
  if ($op == 'clone') {
    $type->label .= ' (cloned)';
    $type->type .= '_clone';
  }

  // Make the type object available to implementations of hook_form_alter.
  $form['#membership_entity_type'] = $type;

  $form['label'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => $type->label,
    '#description' => t('The human-readable name of this membership type. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => $type->type,
    '#maxlength' => 32,
    '#disabled' => !empty($type->type) && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'membership_entity_type_load',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this membership type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $type->description,
    '#description' => t('Describe this membership type.'),
  );

  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $type->weight,
    '#description' => t('When showing membership types, those with lighter (smaller) weights get listed before membership types with heavier (larger) weights.'),
  );

  form_load_include($form_state, 'inc', 'membership_entity', 'membership_entity.pages');
  $form = membership_entity_settings_form($form, $form_state, $type->data);

  $form['#validate'][] = 'membership_entity_type_form_validate';
  $form['actions']['submit']['#submit'][] = 'membership_entity_type_form_submit';
  $form['actions']['submit']['#limit_validation_errors'][] = array('label');
  $form['actions']['submit']['#limit_validation_errors'][] = array('type');
  $form['actions']['submit']['#limit_validation_errors'][] = array('description');
  $form['actions']['submit']['#limit_validation_errors'][] = array('weight');

  return $form;
}

/**
 * Form validation handler for membership_type_form().
 */
function membership_entity_type_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  $type = trim($values['type']);
  $label = trim($values['label']);

  // Check for duplicate name.
  $names = membership_entity_type_get_names();
  if (in_array($label, $names) && $names[$type] != $label) {
    form_set_error('name', t('The human-readable name %name is already taken.', array('%name' => $label)));
  }
}

/**
 * Form submission handler for membership_type_form().
 *
 * @see membership_type_form_validate()
 */
function membership_entity_type_form_submit($form, &$form_state) {
  $type = $form['#membership_entity_type'];

  // Remove extra fields.
  form_state_values_clean($form_state);

  // Remove database fields. Everything that's left goes in data.
  foreach ($form_state['values'] as $key => $value) {
    if (!isset($type->$key)) {
      $form_state['values']['data'][$key] = $value;
      unset($form_state['values'][$key]);
    }
  }
  entity_form_submit_build_entity('membership_entity_type', $type, $form, $form_state);
  $status = membership_entity_type_save($type);

  // Add the saved membership object to $form_state for other modules.
  $form_state['#membership_entity_type'] = $type;

  $t_args = array('%label' => $type->label);
  switch ($status) {
    case SAVED_NEW:
      drupal_set_message(t('The membership type %label has been created.', $t_args));
      membership_entity_add_fields($type);
      break;

    case SAVED_UPDATED:
      drupal_set_message(t('The membership type %label has been updated.', $t_args));
      break;
  }
}

/**
 * Form builder for the membership type delete form.
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