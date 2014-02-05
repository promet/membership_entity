<?php

/**
 * @file
 * Provides entity views integration.
 */

/**
 * Provides views integration for memberships.
 */
class MembershipEntityViewsController extends EntityDefaultViewsController {
  /**
   * Overrides EntityDefaultViewsController::views_data().
   */
  public function views_data() {
    $info = parent::views_data();
    $data = &$info['membership_entity'];

    $data['primary_member']['title'] = t('Primary member');
    $data['primary_member']['field'] = array(
      'type' => 'user',
      'handler' => 'views_handler_field_membership_primary_member',
      'click sortable' => TRUE,
    );

    $data['url'] = array(
      'title' => t('Link'),
      'help' => t('A link to view the membership.'),
      'field' => array(
        'op' => 'view',
        'handler' => 'views_handler_field_membership_link',
      ),
    );

    $data['edit'] = array(
      'title' => t('Edit'),
      'help' => t('A link to edit the membership.'),
      'field' => array(
        'op' => 'edit',
        'handler' => 'views_handler_field_membership_link',
      ),
    );

    $data['delete'] = array(
      'title' => t('Delete'),
      'help' => t('A link to delete the membership.'),
      'field' => array(
        'op' => 'delete',
        'handler' => 'views_handler_field_membership_link',
      ),
    );

    return $info;
  }
}
