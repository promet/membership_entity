<?php

/**
 * @file
 * Page callbacks for membership entities.
 */

/**
 * Menu callback: Add a membership entity.
 */
function membership_entity_add() {
  // Create a generic membership entity.
  $membership = membership_entity_create();
  drupal_set_title(t('Create membership');
  return drupal_get_form('membership_entity_edit_form', $membership);
}

/**
 * Menu callback: Edit a membership.
 */
function membership_entity_edit($membership) {
  drupal_set_title(t('Edit membership: @id', array('@id' => $membership->member_id)));
  return drupal_get_form('membership_entity_edit_form', $membership);
}

/**
 * Form builder to create/edit membership entities.
 *
 * @see membership_entity_form_validate()
 * @see membership_entity_form_submit()
 */
function membership_entity_edit_form($form, &$form_state, $membership) {
  kpr($membership);

  $form['#attributes']['class'][] = 'membership-entity-edit-form';
  if (!empty($membership->type)) {
    $form['#attributes']['class'][] = 'membership-entity-' . $membership->type . '-edit-form';
  }

  // Add the membership entity form fields.
  $form['#membership_entity'] = $membership;
  membership_entity_form($form, $form_state);
  $form['#validate'][] = 'membership_entity_form_validate';

  // Add the buttons.
  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 9999,
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('membership_entity_edit_form_submit'),
    ),
  );

  if (!empty($membership->mid) && user_access('delete memberships')) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('membership_entity_edit_form_delete'),
    );
  }

  // Add type specific submit handlers.
  if (function_exists($membership->type . '_membership_entity_edit_form_submit')) {
    $form['#submit'][] = $membership->type . '_membership_entity_edit_form_submit';
  }

  kpr($form);

  return $form;
}

/**
 * Menu callback: Join as member of the site.
 */
function membership_entity_join($form, &$form_state) {
  $current_step = !empty($form_state['storage']['current_step']) ? $form_state['storage']['current_step'] : 1;
  $form_state['storage']['current_step'] = $current_step;

  $max_step = !empty($form_state['storage']['max_step']) ? $form_state['storage']['max_step'] : 2;
  $form_state['storage']['max_step'] = $max_step;

  $storage = isset($form_state['storage']['step' . $current_step]) ? $form_state['storage']['step' . $current_step] : array();

  $form = array(
    '#prefix' => '<div id="membership-entity-join-wrapper">',
    '#suffix' => '</div>',
    '#attributes' => array(
      'class' => array('membership-entity-join-form'),
    ),
    '#logged_in' => user_is_logged_in(),
  );

  $form['#user'] = empty($GLOBALS['user']->uid) ? drupal_anonymous_user() : $GLOBALS['user'];
  switch ($current_step) {
    case 1:
      if (!$form['#logged_in']) {
        $form['#user_category'] = 'register';

        // Display account info as a fieldset.
        $form['account_info'] = array(
          '#type' => 'fieldset',
          '#title' => t('Account information'),
          '#description' => t('If you already have an account on @site_name please <a href="!login">login</a> before completing this form.', array(
            '@site_name' => variable_get('site_name', ''),
            '!login' => url('user', array('query' => array('destination' => 'membership_entity/join'))),
          )),
          '#user' => $form['#user'],
        );

        // Add the user register form.
        user_account_form($form['account_info'], $form_state);
        $form['#validate'][] = 'user_register_validate';

        // Add custom account fields.
        $langcode = entity_language('user', $form['#user']);
        field_attach_form('user', $form['#user'], $form['account_info'], $form_state, $langcode);
        foreach (field_info_instances('user', 'user') as $field_name => $instance) {
          if (empty($instance['settings']['user_register_form'])) {
            $form['account_info'][$field_name]['#access'] = FALSE;
          }
        }

        // Load stored values.
        if (!empty($storage)) {
          foreach ($storage as $field_name => $value) {
            if (isset($form['account_info'][$field_name])) {
              $form['account_info'][$field_name]['#default_value'] = $value;
            }
          }
        }
      }

      $types = membership_entity_get_types();
      $options = array();
      foreach ($types as $type) {
        $options[$type->type] = $type->name;
      }

      $form['step1'] = array(
        '#type' => 'fieldset',
        '#title' => t('Step 1: Select a Membership type'),
      );

      $form['step1']['membership_type'] = array(
        '#type' => 'radios',
        '#title' => t('Membership Type'),
        '#default_value' => isset($storage['membership_type']) ? $storage['membership_type'] : '',
        '#required' => TRUE,
        '#options' => $options,
      );
      break;

    case 2:
      $type = $form_state['storage']['step1']['membership_type'];

      // Create a default membership and build the form.
      $form['#membership_entity'] = entity_get_controller('membership_entity')
        ->create($type);

      // Display membership info as a fieldset.
      $form['membership_info'] = array(
        '#type' => 'fieldset',
        '#title' => t('Step 2: Membership information'),
        '#membership_entity' => $form['#membership_entity'],
      );
      membership_entity_form($form['membership_info'], $form_state);
      $form['#validate'][] = 'membership_entity_form_validate';
      break;
  }

  $form['actions'] = array('#type' => 'actions');
  if ($current_step > 1) {
    $form['actions']['back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#name' => 'back',
      '#submit' => array('membership_entity_join_multistep_submit'),
      '#limit_validation_errors' => array(),
    );
  }

  if ($current_step == $max_step) {
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
      '#submit' => array(
        'membership_entity_join_multistep_submit',
        'membership_entity_join_submit',
      ),
    );
  }
  else {
    $form['actions']['next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#name' => 'next',
      '#submit' => array('membership_entity_join_multistep_submit'),
    );
  }

  // Add ajax settings.
  foreach (array('next', 'back') as $button) {
    $form['actions'][$button]['#ajax'] = array(
      'wrapper' => 'membership-entity-join-wrapper',
      'callback' => 'membership_entity_join_multistep_callback',
      'effect' => 'fade',
    );
  }

  return $form;
}

/**
 * Ajax callback to reload the membership join form.
 */
function membership_entity_join_multistep_callback($form, $form_state) {
  return $form;
}

/**
 * Sumbmit handler for the multistep join form.
 */
function membership_entity_join_multistep_submit($form, &$form_state) {
  $button = $form_state['triggering_element'];
  $current_step = &$form_state['storage']['current_step'];

  $form_state['rebuild'] = TRUE;
  $values = $form_state['values'];

  switch ($current_step) {
    case 1:
      $form_state['storage']['step1'] = $values;
      break;

    case 2:
      $form_state['storage']['step2'] = $values;
      break;
  }

  // Update the step counter.
  switch ($button['#name']) {
    case 'next':
      $current_step++;
      break;

    case 'back':
      $current_step--;
      break;
  }
}

/**
 * Submit handler to save the membership information.
 */
function membership_entity_join_submit($form, &$form_state) {
  $storage = $form_state['storage'];
  $account = $GLOBALS['user'];

  if (!$form['#logged_in']) {
    // Save Step 1 account information.
    $form_state['values'] = $storage['step1'];
    $form_state['values']['administer_users'] = user_access('administer users');
    user_register_submit($form, $form_state);
    $account = $form_state['user'];
  }

  // Save the membership.
  $form_state['values'] = $storage['step2'];
  $form_state['values']['uid'] = $account->uid;

  if (empty($form_state['values']['member_id'])) {
    $form_state['values']['member_id'] = membership_entity_next_member_id();
  }

  membership_entity_edit_form_submit($form, $form_state);

  $form_state['rebuild'] = FALSE;
  $form_state['redirect'] = '<front>';
}

/**
 * Menu callback: Renew an existing membership.
 */
function membership_entity_renew($form, &$form_state, $membership) {
  $membership->is_renewal = TRUE;

  $form = array(
    '#prefix' => '<div id="membership-entity-renew-wrapper">',
    '#suffix' => '</div>',
    '#attributes' => array(
      'class' => array('membership-entity-renew-form'),
    ),
  );

  // Create a default membership and build the form.
  $form['#membership_entity'] = $membership;

  // Display membership info as a fieldset.
  $form['membership_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership information'),
    '#membership_entity' => $form['#membership_entity'],
  );
  membership_entity_form($form['membership_info'], $form_state);

  // Set the membership to active.
  $form['membership_info']['membership']['status']['#default_value'] = MEMBERSHIP_ENTITY_ACTIVE;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Renew'),
  );

  $form['#submit'][] = 'membership_entity_renew_submit';
  $form['#validate'][] = 'membership_entity_form_validate';

  return $form;
}

/**
 * Submit the renewal form.
 */
function membership_entity_renew_submit($form, &$form_state) {
  $membership = $form['#membership_entity'];
  membership_entity_edit_form_submit($form, $form_state);
  $form_state['redirect'] = 'membership/' . $membership->mid;
}

/**
 * Helper to add default and field api membership entity fields.
 */
function membership_entity_form(&$form, &$form_state) {
  $membership = $form['#membership_entity'];
  $membership = membership_entity_prepare_form($membership);

  $form['#entity_type'] = 'membership_entity';
  $form['#bundle'] = $membership->type;

  // Basic information.
  foreach (array('mid', 'type', 'created', 'entity_type') as $key) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => isset($membership->{$key}) ? $membership->{$key} : NULL,
    );
  }
  $form['changed'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($membership->changed) ? $membership->changed : NULL,
  );

  $form['membership'] = array(
    '#type' => 'container',
    '#weight' => -10,
  );

  if (!empty($membership->member_id)) {
    $form['membership']['member_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Member ID'),
      '#description' => t('Your unique member id.'),
      '#default_value' => $membership->member_id,
      '#required' => TRUE,
      '#disabled' => !user_access('administer memberships'),
    );
  }

  $form['membership']['uid'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Member'),
    '#description' => t('Enter the username of the primary member account.'),
    '#access' => user_access('administer memberships'),
    '#default_value' => !empty($membership->uid) ? $membership->uid : NULL,
    '#required' => TRUE,
    '#autocomplete_path' => 'membership-entity/' . $membership->mid . '/user/autocomplete/' . $membership->type,
    '#value_callback' => 'membership_entity_user_autocomplete_value',
  );

  $form['membership']['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#options' => array(
      MEMBERSHIP_ENTITY_ACTIVE => t('Active'),
      MEMBERSHIP_ENTITY_PENDING => t('Pending'),
      MEMBERSHIP_ENTITY_EXPIRED => t('Expired'),
      MEMBERSHIP_ENTITY_CANCELLED => t('Cancelled'),
    ),
    '#default_value' => $membership->status,
    '#required' => TRUE,
    '#access' => user_access('administer memberships'),
  );

  // For joins and renewals we create a new term for the membership. All other
  // edits will only update the most recent membership term.
  $join = empty($membership->mid);
  $renewal = !empty($membership->is_renewal);
  $form['membership']['current_term_length'] = array(
    '#type' => 'radios',
    '#title' => ($join || $renewal) ? t('Term length') : t('Current term length'),
    '#description' => ($join || $renewal) ? t('Select the desired term length.') : t('This will change your current term length. To renew this membership, go to the <a href="@url">renew page</a>', array('@url' => url('membership-entity/renew/' . $membership->mid))),
    '#default_value' => isset($membership->current_term_length) ? $membership->current_term_length : NULL,
    '#required' => TRUE,
    '#options' => drupal_map_assoc(variable_get('membership_entity_term_lengths_' . $membership->type, array())),
    '#access' => ($join || $renewal || user_access('administer memberships')),
  );

  // Add field UI items.
  field_attach_form('membership_entity', $membership, $form, $form_state);

  // Additional members access.
  $additional_members = &$form[$membership->type . '_members'];
  if (!isset($additional_members['#access'])) {
    $additional_members['#access'] = TRUE;
  }
  $additional_members['#access'] = membership_entity_access('manage', $membership) && $additional_members['#access'];
}

/**
 * Validate the membership entity form.
 *
 * @see membership_entity_form()
 */
function membership_entity_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $membership = $form['#membership_entity'];

  // Member ID must be unique.
  if (!empty($values['member_id'])) {
    if ((bool) db_select('membership_entity', 'me')->fields('me', array('mid'))->condition('mid', $membership->mid, '<>')->condition('member_id', db_like($values['member_id']), 'LIKE')->range(0, 1)->execute()->fetchField()) {
      form_set_error('member_id', t('The member ID %member_id already exists.', array(
        '%member_id' => $values['member_id'],
      )));
    }
  }

  // Extract the uid.
  if (!empty($values['uid'])) {
    preg_match('/^(?:\s*|(.*) )?\[\s*uid\s*:\s*(\d+)\s*\]$/', $values['uid'], $matches);
    array_shift($matches);
    if (!empty($matches)) {
      // Check that name matches the uid.
      list($name, $uid) = $matches;
      $result = db_select('users', 'u')
        ->fields('u', array('uid'))
        ->condition('u.uid', $uid)
        ->condition('u.name', $name)
        ->countQuery()
        ->execute()
        ->fetchField();
      if (!empty($result)) {
        $values['uid'] = $uid;
      }
      else {
        form_set_error('uid', t('Primary member name mismatch. Please check your selection.'));
      }
    }
    else {
      form_set_error('uid', t('Invalid selection for Primary member.'));
    }
  }

  // Validate custom fields.
  entity_form_field_validate('membership_entity', $form, $form_state);
}

/**
 * Menu callback: Autocomplete suggestions for existing users by role.
 *
 * @param $type
 *  The mebmership type.
 */
function membership_entity_user_autocomplete($type, $string = '') {
  $matches = &drupal_static(__FUNCTION__, array());

  // Create unique id for static cache.
  if (!isset($matches[$type])) {
    $roles = array_filter(variable_get('membership_entity_roles_' . $type, array()));

    // Build the query.
    $query = db_select('users', 'u')
      // Select the whole record, so that format_username() has enough
      // information.
      ->fields('u', array('uid', 'name'))
      ->addMetaData('id', 'membership_entity_user_autocomplete')
      ->condition('u.name', '%' . db_like($string) . '%', 'LIKE');

    // No role filter if authenticated user is selected.
    if (!empty($roles) && !in_array(DRUPAL_AUTHENTICATED_RID, $roles)) {
      $query->join('users_roles', 'r', 'u.uid = r.uid');
      $query->condition('r.rid', $roles, 'IN');
    }

    // Explicitly exclude the anonymous user.
    $result = $query->condition('u.uid', 0, '<>')
      ->orderBy('u.name')
      ->execute()
      ->fetchAll();

    foreach ($result as $account) {
      $name = check_plain(format_username($account));
      // Remove hyperlinks.
      $matches[$type][$account->name . " [uid:$account->uid]"] = preg_replace('/<a href="([^<]*)">([^<]*)<\/a>/', '$2', $name);
    }
  }

  drupal_json_output($matches[$type]);
}

/**
 * Value callback for a membership_entity user autocomplete element.
 */
function membership_entity_user_autocomplete_value($element, $input = FALSE, $form_state) {
  if ($input === FALSE) {
    // Expand the raw uid into 'username [uid:n]'.
    $uid = $element['#default_value'];
    if (!empty($uid)) {
      $query = db_select('users', 'u');
      $query->addField('u', 'name');
      $result = $query->condition('u.uid', $uid)
        ->range(0, 1)
        ->execute();
      $value = $result->fetchField();
      $value .= " [uid:$uid]";
      return $value;
    }
  }
}

/**
 * Submit the membership entity form.
 *
 * @see membership_entity_form()
 * @see membership_entity_form_validate()
 */
function membership_entity_edit_form_submit($form, &$form_state) {
  $membership = $form['#membership_entity'];
  entity_form_submit_build_entity('membership_entity', $membership, $form, $form_state);

  $new = empty($membership->mid);
  if ($new) {
    $membership->member_id = membership_entity_next_member_id();
  }
  $membership = membership_entity_prepare_save($membership);
  membership_entity_save($membership);

  $t_args = array(
    '@type' => membership_entity_type_get_name($membership),
    '%member_id' => $membership->member_id,
  );
  if ($new && user_access('administer memberships')) {
    drupal_set_message(t('@type %member_id has been created.', $t_args));
  }
  elseif (!$new) {
    drupal_set_message(t('@type %member_id has been updated.', $t_args));
  }

  $form_state['redirect'] = 'membership/' . $membership->mid;
}

/**
 * Submit handler to delete a membership.
 */
function membership_entity_edit_form_delete($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $membership = $form['#membership_entity'];
  $form_state['redirect'] = array(
    'membership/' . $membership->mid . '/delete',
    array('query' => $destination),
  );
}

/**
 * Dispaly a confirmation page before delteing a membership.
 */
function membership_entity_delete_confirm($form, &$form_state, $membership) {
  $form_state['memberships'] = array(
    $membership->mid => $membership,
  );

  $form['membership'] = array(
    '#theme_wrappers' => array('container'),
    '#attributes' => array(),
    '#markup' => t('You are about to delete membership %member_id.', array(
      '%member_id' => $membership->member_id,
    )),
  );

  $form['#submit'][] = 'membership_entity_multiple_delete_confirm_submit';
  $confirm_message = t('Are you sure you want to delete this membership?');
  return confirm_form($form, $confirm_message, 'membership/' . $membership->mid, t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Display a confirmation page before deleting memberships.
 */
function membership_entity_multiple_delete_confirm($form, &$form_state, $memberships) {
  $form_state['memberships'] = $memberships;

  $form['memberships'] = array(
    '#theme' => 'item_list',
    '#type' => 'ul',
    '#items' => array(),
    '#attributes' => array(),
  );
  foreach ($memberships as $mid => $value) {
    $member_id = db_query('SELECT member_id FROM {membership_entity} WHERE mid = :mid', array(':mid' => $mid))
      ->fetchField();
    $form['memberships']['#items'][] = check_plain($member_id);
  }
  $form['operation'] = array(
    '#type' => 'hidden',
    '#value' => 'delete',
  );
  $form['#submit'][] = 'membership_entity_multiple_delete_confirm_submit';

  $confirm_question = format_plural(count($memberships), 'Are you sure you want to delete this item?', 'Are you sure you want to delete these items?');
  return confirm_form($form, $confirm_question, 'admin/memberships', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit the memberships delete confirm form.
 */
function membership_entity_multiple_delete_confirm_submit($form, &$form_state) {
  $memberships = $form_state['memberships'];
  if ($form_state['values']['confirm']) {
    membership_entity_delete_multiple(array_keys($memberships));
    $count = count($memberships);
    drupal_set_message(format_plural($count, 'Deleted 1 membership.', 'Deleted @count memberships.'));
  }
  $form_state['redirect'] = 'admin/memberships';
}
