<?php

/**
 * @file
 * Page callbacks for membership entity types.
 */

/**
 * Page callback: Displays join links for available membership types.
 *
 * Redirects to user/join/[type] if only one type is available.
 */
function membership_entity_type_membership_add_page() {
  $item = menu_get_item();
  // Trick the system module into thinking this is not a local task.
  $item['tab_root'] = $item['path'];
  $content = system_admin_menu_block($item);

  // Bypass the type listing if only one type is available.
  if (count($content) == 1) {
    $item = array_shift($content);
    drupal_goto($item['href']);
  }
  return theme('membership_entity_type_add_list', array('content' => $content));
}

/**
 * Page callback: Displays renew links for available membership types.
 *
 * Redirects to user/renew/[type] if only one type is available.
 */
function membership_entity_type_membership_renew_page() {
  $item = menu_get_item();
  // Trick the system module into thinking this is not a local task.
  $item['tab_root'] = $item['path'];
  $content = system_admin_menu_block($item);

  foreach ($content as $id => $menu_item) {
    $arguments = unserialize($menu_item['page_arguments']);
    $result = db_query('SELECT mid FROM membership_entity WHERE uid = :uid AND type = :type', array(
      ':uid' => $GLOBALS['user']->uid,
      ':type' => $arguments[2],
    ))->fetchColumn();
    if (empty($result)) {
      unset($content[$id]);
    }
  }

  if (empty($content)) {
    return t('You do not have any membership to renew. Click <a href="@url">here</a> to join.', array(
      '@url' => url('user/join'),
    ));
  }
  elseif (count($content) == 1) {
    $item = array_shift($content);
    drupal_goto($item['href']);
  }
  return theme('membership_entity_type_add_list', array('content' => $content));
}

/**
 * Returns HTML for a list of available membership types.
 */
function theme_membership_entity_type_add_list($variables) {
  $content = $variables['content'];
  $output = '';
  if ($content) {
    $output = '<dl class="membership-entity-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href'], $item['localized_options']) . '</dt>';
      $output .= '<dd>' . filter_xss($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  elseif (user_access('administer membership types')) {
    $output = '<p>' . t('You have not created any membership types yet. Go to the <a href="@create-content">membership type creation page</a> to add a new membership type.', array(
      ' @create-content' => url('admin/memberships/types/add'),
    )) . '</p>';
  }
  else {
    $output = '<p>' . t('There are no membership types available. Please contact a site administrator.') . '</p>';
  }
  return $output;
}
