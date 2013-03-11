<?php

/**
 * @file
 * Page callbacks for membership entities.
 */

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

        // Add the user register form.
        user_account_form($form, $form_state);
        $form['#validate'][] = 'user_register_validate';

        // Display account info as a fieldset.
        $form['account']['#type'] = 'fieldset';
        $form['account']['#title'] = t('Account information');
        $form['account']['#description'] = t('If you already have an account on @site_name please <a href="@login">login</a> before completing this form.', array(
          '@site_name' => variable_get('site_name', ''),
          '@login' => url('user', array('query' => array('destination' => 'membership_entity/join'))),
        ));

        $langcode = entity_language('user', $form['#user']);
        field_attach_form('user', $form['#user'], $form, $form_state, $langcode);
        foreach (field_info_instances('user', 'user') as $field_name => $instance) {
          if (empty($instance['settings']['user_register_form'])) {
            $form[$field_name]['#access'] = FALSE;
          }
        }

        // Load stored values.
        if (isset($storage['account'])) {
          foreach ($storage['account'] as $field_name => $value) {
            $form['account'][$field_name]['#default_value'] = $value;
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
      $membership = entity_get_controller('membership_entity')
        ->create($type);
      $form['#membership_entity'] = $membership;
      membership_entity_form($form, $form_state);

      // Remove the uid field if not logged in. This will be added later from
      // the new account.
      if (!$form['#logged_in']) {
        $form['membership_entity']['uid']['#required'] = FALSE;
        $form['membership_entity']['uid']['#access'] = FALSE;
      }

      // Add fieldset info.
      $form['membership_entity']['#type'] = 'fieldset';
      $form['membership_entity']['#title'] = t('Step 2: Membership information');
      $form['membership_entity']['#parents'] = array('membership_entity');
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

  // Set the expiration date.
  $expire = format_date(strtotime($storage['step2']['term_length']), 'custom', DATE_FORMAT_DATETIME);
  $form_state['values']['membership_entity_expire'][LANGUAGE_NONE][0]['value'] = $expire;
  $form_state['values']['membership_entity_expire'][LANGUAGE_NONE][0]['value2'] = $expire;

  membership_entity_edit_form_submit($form, $form_state);
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
      ->condition('u.name', '%' . check_plain($string) . '%', 'LIKE');

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
 * Menu callback: Edit a membership.
 */
function membership_entity_edit($membership) {
  drupal_set_title(t('Edit membership: @id', array('@id' => $membership->member_id)));
  return drupal_get_form($membership->type . '_membership_entity_edit_form', $membership);
}

/**
 * Menu callback: Displays add membership links.
 */
function membership_entity_add_page() {
  $types = membership_entity_get_types();

  $build = array();
  if (!empty($types)) {
    // Only one membership type. Go directly there (do not pass go,
    // do not collect $200)
    if (count($types) == 1) {
      $type = current($types);
      drupal_goto('admin/memberships/add/' . str_replace('_', '-', $type->type));
    }

    $build['membership_types'] = array(
      '#theme' => 'item_list',
      '#type' => 'ul',
      '#items' => array(),
      '#attributes' => array(
        'class' => array('admin-list', 'membership-types-list'),
      ),
    );

    foreach ($types as $type) {
      $path = 'admin/memberships/add/' . str_replace('_', '-', $type->type);
      $build['membership_types']['#items'][] = l($type->name, $path) . '<div class="description">' . filter_xss_admin($type->description) . '</div>';
    }
  }
  else {
    $build = array(
      '#markup' => '<p>' . t('You have not created any membership types yet. Go to the <a href="@url">membership type creation page</a> to add a new membership type.', array('@url' => url('admin/structure/membership-types/add'))) . '</p>',
    );
  }

  return $build;
}

/**
 * Menu callback: Add a memberhsip entity.
 */
function membership_entity_add($type) {
  // Create a generic membership entity.
  $membership = entity_get_controller('membership_entity')
    ->create($type);
  return drupal_get_form($type . '_membership_entity_edit_form', $membership);
}

/**
 * Form builder to create/edit membership entities.
 *
 * @see membership_entity_form_validate()
 * @see membership_entity_form_submit()
 */
function membership_entity_edit_form($form, &$form_state, $membership) {
  $form['#attributes']['class'][] = 'membership-entity-edit-form';
  if (!empty($membership->type)) {
    $form['#attributes']['class'][] = 'membership-entity-' . $membership->type . '-edit-form';
  }

  // Add the membership entity form fields.
  $form['#membership_entity'] = $membership;
  membership_entity_form($form, $form_state);

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

  return $form;
}

/**
 * Helper to add default and field api membership entity fields.
 */
function membership_entity_form(&$form, &$form_state) {
  $membership = $form['#membership_entity'];
  $is_new = empty($membership->mid);

  $form['#validate'][] = 'membership_entity_form_validate';

  $form['membership_entity'] = array(
    '#type' => 'container',
    '#entity_type' => 'membership_entity',
    '#bundle' => $membership->type,
  );
  $membership_form = &$form['membership_entity'];

  // Basic information.
  foreach (array('mid', 'type', 'created', 'entity_type') as $key) {
    $membership_form[$key] = array(
      '#type' => 'value',
      '#value' => isset($membership->$key) ? $membership->$key : NULL,
    );
  }

  $membership_form['changed'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($membership->changed) ? $membership->changed : NULL,
  );

  if (empty($membership->member_id)) {
    $membership->member_id = membership_entity_next_member_id();
  }
  if (user_access('administer memberships')) {
    $membership_form['member_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Member ID'),
      '#description' => t('A unique member id for this membership.'),
      '#default_value' => $membership->member_id,
      '#required' => TRUE,
    );
  }
  else {
    $membership_form['member_id'] = array(
      '#type' => 'value',
      '#value' => $membership->member_id,
    );

    $membership_form['member_id_view'] = array(
      'label' => array(
        '#theme' => 'html_tag',
        '#tag' => 'label',
        '#value' => t('Member ID'),
        '#attributes' => array('for' => 'membership-entity-member-id'),
      ),
      'value' => array(
        '#theme_wrappers' => array('container'),
        '#attributes' => array('id' => 'membership-entity-member-id'),
        '#markup' => check_plain($membership->member_id),
      ),
    );
  }

  $membership_form['uid'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Member'),
    '#default_value' => !empty($membership->uid) ? $membership->uid : NULL,
    '#required' => TRUE,
    '#autocomplete_path' => 'membership-entity/' . $membership->mid . '/user/autocomplete/' . $membership->type,
    '#value_callback' => 'membership_entity_user_autocomplete_value',
  );

  $membership_form['term_length'] = array(
    '#type' => 'radios',
    '#title' => t('Term length'),
    '#default_value' => !empty($membership->term_length) ? $membership->term_length : NULL,
    '#required' => TRUE,
    '#options' => drupal_map_assoc(variable_get('membership_entity_term_lengths_' . $membership->type, array())),
  );

  // Add field UI items.
  field_attach_form('membership_entity', $membership, $membership_form, $form_state);

  // Admin fields access.
  foreach (array('uid', 'membership_entity_join') as $field) {
    $membership_form[$field]['#disabled'] = !user_access('administer memberships');
  }

  // Additional members access.
  if (!isset($membership_form[$membership->type . '_members']['#access'])) {
    $membership_form[$membership->type . '_members']['#access'] = TRUE;
  }
  $membership_form[$membership->type . '_members']['#access'] = membership_entity_access('manage', $membership) && $membership_form[$membership->type . '_members']['#access'];

  // If this is a new membership, the expiration date must be in the future.
  if ($is_new) {
    foreach (element_children($membership_form['membership_entity_expire'][LANGUAGE_NONE]) as $delta) {
      $form['membership_entity']['membership_entity_expire'][LANGUAGE_NONE][$delta]['#element_validate'][] = 'membership_entity_date_validate_future';
    }
  }
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
 * Validate the membership entity form.
 *
 * @see membership_entity_form()
 */
function membership_entity_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $membership = $form['#membership_entity'];

  // Member ID must be unique.
  if ((bool) db_select('membership_entity', 'me')->fields('me', array('mid'))->condition('mid', $membership->mid, '<>')->condition('member_id', db_like($values['member_id']), 'LIKE')->range(0, 1)->execute()->fetchField()) {
    form_set_error('member_id', t('The member ID %member_id already exists.', array(
      '%member_id' => $values['member_id'],
    )));
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
      form_set_error('uid', t('Invalid selection for Priamry member.'));
    }
  }

  // Validate custom fields.
  entity_form_field_validate('membership_entity', $form, $form_state);
}

/**
 * Validate that a given date element is >= today.
 */
function membership_entity_date_validate_future(&$element) {
  if (strtotime($element['value']['date']['#value']) < strtotime(format_date(REQUEST_TIME, 'custom', $element['value']['#date_format']))) {
    form_error($element, t('@field must be a current or future date.', array(
      '@field' => $element['#title'],
    )));
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

  $is_new = empty($membership->mid);
  membership_entity_save($membership);

  $t_args = array(
    '@type' => membership_entity_type_get_name($membership),
    '%member_id' => $membership->member_id,
  );
  if ($is_new) {
    drupal_set_message(t('@type %member_id has been created.', $t_args));
  }
  else {
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
  $membership = $form_state['membership_entity'];
  $form_state['redirect'] = array(
    'membership/' . $membership->mid . '/delete',
    array('query' => $destination),
  );
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
    'type' => array('data' => t('Type'), 'field' => 'me.type'),
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
  foreach ($memberships as $membership) {
    $options[$membership->mid] = array(
      'member_id' => l(check_plain($membership->member_id), 'membership/' . $membership->mid),
      'type' => check_plain(membership_entity_type_get_name($membership)),
      'status' => $membership->status ? t('Active') : t('Expired'),
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
      MEMBERSHIP_ENTITY_ACTIVE => t('Active'),
      MEMBERSHIP_ENTITY_EXPIRED => t('Expired'),
      MEMBERSHIP_ENTITY_CANCELLED => t('Cancelled'),
    ),
  );

  $types = array('[any]' => t('any'));
  foreach (membership_entity_get_types() as $type) {
    $types[$type->type] = $type->name;
  }
  $filters['type'] = array(
    'title' => t('type'),
    'options' => $types,
  );

  return $filters;
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
      case 'type':
        $query->condition('me.' . $key, $value);
        break;
    }
  }
}
