<?php

/**
 * @file
 * Devel integration for the membership_entity module.
 */

/**
 * Form builder to generate membership content.
 */
function membership_entity_devel_generate_memberships_form($form, &$form_state) {
  $types = membership_entity_get_types();
  foreach ($types as $type) {
    $options[$type->type] = $type->name;
  }
  if (empty($options)) {
    drupal_set_message(t('You do not have any membership types that can be generated. <a href="@create-type">Create a new membership type</a> already!</a>', array(
      '@create-type' => url('admin/structure/membership-types/add'),
    )), 'error', FALSE);
    return;
  }
  $form['types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('What types of memberships would you like to generate?'),
    '#options' => $options,
    '#default_value' => array_keys($options),
  );
  $form['num'] = array(
    '#type' => 'textfield',
    '#title' => t('How many memberships would you like to generate?'),
    '#description' => t('You must have at least this many user object available.'),
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
  $form['max_terms'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum number of terms (renewals) per membership.'),
    '#default_value' => 0,
    '#size' => 3,
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
 *  An assoicative array with the following keys:
 *  - types: The types of memberships to create.
 *  - num: Number of users to generate.
 *  - kill: Boolean that indicates if existing users should be removed first.
 *  - time_range: The max join date of each membership term in seconds.
 *  - max_terms: The max number of renewals to generate for the membership.
 */
function membership_entity_devel_create_memberships($values) {
  // Load some devel generate helpers.
  module_load_include('inc', 'devel_generate');
  module_load_include('inc', 'devel_generate', 'devel_generate.fields');

  if ($values['kill']) {
    $mids = db_select('membership_entity', 'm')
      ->fields('m', array('mid'))
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
      'type' => array_rand($values['types']),
      'created' => $created,
      'status' => MEMBERSHIP_ENTITY_ACTIVE,
    );

    // Populate all core fields on behalf of field.module
    devel_generate_fields($membership, 'membership_entity', $membership->type);

    // Generate membership terms.
    $terms = drupal_map_assoc(variable_get('membership_entity_term_lengths_' . $membership->type, array()));
    $num_terms = !empty($values['max_terms']) ? mt_rand(0, $values['max_terms'] - 1) : 0;
    for ($j = 0; $j <= $num_terms; $j++) {
      $term_length = array_rand($terms);
      $start = ($j < 1) ? $membership->created : $membership->terms[$j - 1]['end'];
      if ($start == MEMBERSHIP_ENTITY_LIFETIME_TERM) {
        // Don't add another term if the previous was lifetime.
        break;
      }
      $end = ($term_length == 'lifetime') ? MEMBERSHIP_ENTITY_LIFETIME_TERM : strtotime($term_length, $start);
      $membership->terms[] = array(
        'delta' => $j,
        'start' => $start,
        'end' => $end,
        'term_length' => $term_length,
      );
    }

    // Save the membership.
    membership_entity_save($membership);
  }

  drupal_set_message(t('!num created.', array(
    '!num' => format_plural($num, '1 membership', '@count memberships'),
  )));
}
