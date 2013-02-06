<?php

/**
 * @file
 * Page callbacks for membership entities.
 */

/**
 * Menu callback: Display a membership.
 *
 * Invokes a number of hooks to prepare the membershp for display.
 *
 * @see hook_entity_prepare_view()
 * @see hook_entity_view()
 * @see hook_entity_view_alter()
 */
function membership_entity_view($membership, $view_mode = 'full') {
  $entity_type = 'membership_entity';
  $membership->content = array(
    '#view_mode' => $view_mode,
  );

  if ($view_mode == 'full') {
    drupal_set_title(t('Membership: @id', array('@id' => $membership->member_id)));
  }

  // Membership fields.
  $membership->content['member_id'] = array(
    '#type' => 'item',
    '#title' => t('Member ID'),
    '#markup' => check_plain($membership->member_id),
  );

  // Build custom field content.
  field_attach_prepare_view($entity_type, array($membership->mid => $membership), $view_mode);
  entity_prepare_view($entity_type, array($membership->mid => $membership));
  $membership->content += field_attach_view($entity_type, $membership, $view_mode);

  // Invoke hook_entity_view() and hook_entity_view_alter().
  $langcode = $GLOBALS['language']->language ;
  module_invoke_all('entity_view', $membership, $entity_type, $view_mode, $langcode);
  drupal_alter(array('entity_membership_entity_view', 'entity_view'), $membership->content, $entity_type);

  return $membership->content;
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
  $is_new = empty($membership->mid);

  $form['#attributes']['class'][] = 'membership-entity-edit-form';
  if (!empty($membership->type)) {
    $form['#attributes']['class'][] = 'membership-entity-' . $membership->type . '-edit-form';
  }

  // Add the membership and entity info for future reference.
  $form['#entity_type'] = 'membership_entity';
  $form['#bundle'] = $membership->type;
  $form['#membership_entity'] = $membership;
  $form_state['membership_entity'] = $membership;

  // Basic information.
  foreach (array('mid', 'type', 'created', 'entity_type') as $key) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => isset($membership->$key) ? $membership->$key : NULL,
    );
  }

  $form['changed'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($membership->changed) ? $membership->changed : NULL,
  );

  $form['member_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Member ID'),
    '#description' => t('A unique member id for this membership.'),
    '#default_value' => isset($membership->member_id) ? $membership->member_id : NULL,
    '#required' => TRUE,
  );

  // Add field UI items.
  field_attach_form('membership_entity', $membership, $form, $form_state);

  // If this is a new membership, the expiration date must be in the future.
  if ($is_new) {
    foreach (element_children($form['membership_entity_expire'][LANGUAGE_NONE]) as $key) {
      $form['membership_entity_expire'][LANGUAGE_NONE][$key]['#element_validate'][] = 'membership_entity_date_validate_future';
    }
  }

  // Add the buttons.
  $form['actions'] = array(
    '#type' => 'actions',
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

  $form['#validate'][] = 'membership_entity_edit_form_validate';
  $form['#submit'] = array();
  if (function_exists($membership->type . '_membership_entity_edit_form_submit')) {
    $form['#submit'][] = $membership->type . '_membership_entity_edit_form_submit';
  }

  return $form;
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
 * Validate the membership entity form.
 *
 * @see membership_entity_form()
 * @see membership_entity_form_submit()
 */
function membership_entity_edit_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $membership = $form_state['membership_entity'];

  // Member ID must be unique.
  $query = db_select('membership_entity', 'me')
    ->fields('me', array('mid'));
  if (!empty($membership->mid)) {
    $query->condition('mid', $membership->mid, '<>');
  }
  $count = $query->condition('member_id', check_plain($values['member_id']))
    ->countQuery()
    ->execute()
    ->fetchField();
  if ($count > 0) {
    form_set_error('member_id', t('The member ID %member_id already exists.', array(
      '%member_id' => $values['member_id'],
    )));
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
  $membership = $form_state['membership_entity'];
  entity_form_submit_build_entity('membership_entity', $membership, $form, $form_state);

  $is_new = empty($membershp->mid);
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
