<?php

/**
 * @file
 * Devel integration for the membership_entity module.
 */

/**
 * Form builder to generate membership content.
 */
function membership_entity_devel_generate_memberships_form($form, &$form_state) {
  $form['#bundle_settings'] = membership_entity_get_bundle_settings();
  if (empty($form['#bundle_settings'])) {
    return;
  }
  $form['num'] = array(
    '#type' => 'textfield',
    '#title' => t('How many memberships would you like to generate?'),
    '#description' => t('You must have at least the same number of user object available. Click <a href="!url">here</a> to generate users.', array(
      '!url' => url('admin/config/development/generate/user'),
    )),
    '#default_value' => 50,
    '#size' => 10,
  );
  $form['kill'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete all memberships before generating new memberships.'),
    '#default_value' => FALSE,
  );
  $options = array(1 => t('Now'));
  foreach (array(86400, 604800, 2592000, 31536000, 63072000, 94608000, 126144000, 157680000, 315360000, 473040000) as $interval) {
    $options[$interval] = format_interval($interval, 1) . ' ' . t('ago');
  }
  $form['time_range'] = array(
    '#type' => 'select',
    '#title' => t('How old should memberships be?'),
    '#description' => t('Memberships will be distributed randomly from the current time, back to the selected time.'),
    '#options' => $options,
    '#default_value' => 94608000,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );

  return $form;
}

/**
 * Submit the form to generate memberships.
 */
function membership_entity_devel_generate_memberships_form_submit($form, &$form_state) {
  form_state_values_clean($form_state);
  membership_entity_devel_create_memberships($form_state['values']);
}

/**
 * Generate some random memberships.
 *
 * @param $values
 *   An assoicative array with the following keys:
 *   - num: Number of users to generate.
 *   - kill: Boolean that indicates if existing users should be removed first.
 *   - time_range: The max join date of each membership term in seconds.
 */
function membership_entity_devel_create_memberships($values) {
  // Load some devel generate helpers.
  module_load_include('inc', 'devel_generate');
  module_load_include('inc', 'devel_generate', 'devel_generate.fields');

  if ($values['kill']) {
    $mids = db_select('membership_entity')
      ->fields('membership_entity', array('mid'))
      ->execute()
      ->fetchAllAssoc('mid');
    membership_entity_delete_multiple(array_keys($mids));
    drupal_set_message(format_plural(count($mids), '1 membership deleted', '@count memberships deleted'));
  }

  $result = db_select('users', 'u')
    ->fields('u', array('uid'))
    ->condition('u.uid', 0, '<>')
    ->execute()
    ->fetchAllAssoc('uid');
  $users = drupal_map_assoc(array_keys($result));

  $num = min(count($users), $values['num']);
  for ($i = 0; $i < $num; $i++) {
    $created = REQUEST_TIME - mt_rand(0, $values['time_range']);
    $uid = array_rand($users);
    unset($users[$uid]);

    // Create the user.
    $name = devel_generate_word(mt_rand(6, 12));
    $membership = (object) array(
      'mid' => NULL,
      'uid' => $uid,
      'member_id' => membership_entity_next_member_id(),
      'type' => 'membership',
      'created' => $created,
      'status' => MEMBERSHIP_ENTITY_ACTIVE,
    );

    // Populate all core fields on behalf of field.module
    devel_generate_fields($membership, 'membership_entity', $membership->type);

    // Save the membership.
    membership_entity_save($membership);
  }

  drupal_set_message(t('!num created.', array(
    '!num' => format_plural($num, '1 membership', '@count memberships'),
  )));
}
