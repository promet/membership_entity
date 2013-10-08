<?php

/**
 * Theme functions for the membership_entity module.
 */

/**
 * Preprocess variables for membership-entity.tpl.php.
 */
function template_preprocess_membership_entity(&$variables) {
  $membership = $variables['membership']['#membership_entity'];

  $variables['view_mode'] = $variables['membership']['#view_mode'];
  $variables['membership_entity'] = $membership;
  $variables['member_id'] = check_plain($membership->member_id);
  $variables['primary_member'] = theme('username', array('account' => user_load($membership->uid)));

  // Add the $content variable.
  $variables['content'] = array();
  foreach (element_children($variables['membership']) as $key) {
    $variables['content'][$key] = $variables['membership'][$key];
  }

  field_attach_preprocess('membership_entity', $membership, $variables['content'], $variables);

  // Add some classes.
  $variables['classes_array'][] = drupal_html_class('membership', 'membership-type-' . $membership->type);
  switch ($membership->status) {
    case MEMBERSHIP_ENTITY_EXPIRED:
      $variables['classes_array'][] = 'membership-expired';
      break;

    case MEMBERSHIP_ENTITY_ACTIVE:
      $variables['classes_array'][] = 'membership-active';
      break;

    case MEMBERSHIP_ENTITY_CANCELLED:
      $variables['classes_array'][] = 'membership-cancelled';
      break;

    case MEMBERSHIP_ENTITY_PENDING:
      $variables['classes_array'][] = 'membership-pending';
      break;
  }
  if ($variables['view_mode'] == 'account') {
    $variables['classes_array'][] = 'membership-account';
  }

  // Add some template suggestions.
  $variables['theme_hook_suggestions'][] = 'membership_entity__' . $membership->type;
  if ($variables['view_mode'] != 'full') {
    $variables['theme_hook_suggestions'][] = 'membership_entity__' . $variables['view_mode'];
  }
}

/**
 * Returns HTML for the available term lengths form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 */
function theme_membership_entity_available_term_lengths($variables) {
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
