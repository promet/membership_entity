<?php

/**
 * @file
 * Hooks and functions for Views.
 */

/**
 * Implements hook_views_data_alter().
 */
function membership_entity_views_data_alter(&$data) {
  $data['users']['table']['join']['membership_entity'] = array(
    'left_field' => 'uid',
    'field' => 'uid',
  );

  $data['users']['uid_membership'] = array(
    'title' => t('Primary user membership'),
    'help' => t('Relate user to the primary user membership.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'membership_entity',
      'base field' => 'uid',
      'field' => 'uid',
    ),
  );
}

/**
 * Implements hook_views_data().
 */
function membership_entity_views_data() {
  $data['membership_entity']['table']['group'] = t('Membership');

  $data['membership_entity']['table']['base'] = array(
    'field' => 'mid',
    'title' => t('Memberships'),
    'access query tag' => 'membership_entity_access',
    'defaults' => array(
      'field' => 'member_id',
    ),
  );

  $data['membership_entity']['join'] = array(
    'users' => array(
      'left_table' => 'membership_entity',
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );

  $data['membership_entity']['table']['entity type'] = 'membership_entity';

  $data['membership_entity']['mid'] = array(
    'title' => t('Mid'),
    'help' => t('Membership entity ID.'),
    'field' => array(
      'handler' => 'membership_entity_handler_field_membership_entity',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'mid',
      'numeric' => TRUE,
      'validate type' => 'mid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['membership_entity']['type'] = array(
    'title' => t('Type'),
    'help' => t('The membership type.'),
    'field' => array(
      'handler' => 'membership_entity_handler_field_membership_entity_type',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'membership_entity_handler_filter_membership_entity_type',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['membership_entity']['member_id'] = array(
    'title' => t('Member ID'),
    'help' => t('The member ID'),
    'field' => array(
      'handler' => 'membership_entity_handler_field_membership_entity',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['membership_entity']['status'] = array(
    'title' => t('Status'),
    'help' => t('Whether or not the membership is active'),
    'field' => array(
      'handler' => 'membership_entity_handler_field_membership_entity_status',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'membership_entity_handler_filter_membership_entity_status',
      'numeric' => TRUE,
      'allow empty' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['membership_entity']['created'] = array(
    'title' => t('Created date'),
    'help' => t('The date the membership record was created.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  $data['membership_entity']['changed'] = array(
    'title' => t('Updated date'),
    'help' => t('The date the membership record was last updated.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  $data['membership_entity']['uid'] = array(
    'title' => t('Primary member'),
    'help' => t('The primary member attached to the membership.'),
    'relationship' => array(
      'title' => t('Primary member'),
      'help' => t('Relate the primary member to their user account.'),
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
      'label' => t('primary member'),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
    'field' => array(
      'handler' => 'views_handler_field_user',
    ),
  );

  $data['views_entity_membership_entity']['table']['group'] = t('Membership');

  $data['membership_entity']['view_membership']['moved to'] = array('views_entity_membership_entity', 'view_membership');
  $data['views_entity_membership_entity']['view_membership'] = array(
    'field' => array(
      'title' => t('Link'),
      'help' => t('Provide a link to the membership.'),
      'handler' => 'membership_entity_handler_field_membership_entity_link',
    ),
  );

  $data['membership_entity']['edit_membership']['moved to'] = array('views_entity_membership_entity', 'edit_membership');
  $data['views_entity_membership_entity']['edit_membership'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a link to edit the membership.'),
      'handler' => 'membership_entity_handler_field_membership_entity_link_edit',
    ),
  );

  $data['membership_entity']['delete_membership']['moved to'] = array('views_entity_membership_entity', 'delete_membership');
  $data['views_entity_membership_entity']['delete_membership'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a link to delete the membership.'),
      'handler' => 'membership_entity_handler_field_membership_entity_link_delete',
    ),
  );

  return $data;
}
