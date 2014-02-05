<?php

/**
 * @file
 * Defines the MembershipEntityTermViews Controller class.
 */

/**
 * Membership entity term views controller class.
 */
class MembershipEntityTermViewsController extends EntityDefaultViewsController {
  /**
   * Overrides EntityDefaultViewsController::views_data().
   */
  public function views_data() {
    $info = parent::views_data();
    $data = &$info['membership_entity_term'];

    foreach (array('start', 'end') as $field) {
      $data[$field]['field']['handler'] = 'views_handler_field_membership_term_datetime';
      $data[$field]['filter']['handler'] = 'views_handler_filter_membership_term_datetime';
    }

    $data['edit'] = array(
      'title' => t('Edit'),
      'help' => t('A link to edit the membership term.'),
      'field' => array(
        'op' => 'edit',
        'handler' => 'views_handler_field_membership_link',
      ),
    );

    $data['delete'] = array(
      'title' => t('Delete'),
      'help' => t('A link to delete the membership term.'),
      'field' => array(
        'op' => 'delete',
        'handler' => 'views_handler_field_membership_link',
      ),
    );

    return $info;
  }
}

/**
 * Implements hook_views_data_alter().
 */
function membership_entity_term_views_data_alter(&$data) {
  $data['membership_entity']['renew'] = array(
    'title' => t('Renew'),
    'help' => t('A link to renew the membership.'),
    'field' => array(
      'op' => 'renew',
      'handler' => 'views_handler_field_membership_link',
    ),
  );
}