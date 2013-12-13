<?php

/**
 * @file
 * Provides entity metadata integration.
 */

/**
 * Provides metadata for memberships.
 */
class MembershipEntityMetadataController extends EntityDefaultMetadataController {
  /**
   * Overrides EntityDefaultMetadataController::entityPropertyInfo().
   */
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    $properties['mid'] = array(
      'label' => t('Internal id'),
      'description' => t('The unique membership identifier used internally and in URLs.'),
      'required' => TRUE,
    ) + $properties['mid'];

    $properties['member_id'] = array(
      'label' => t('Member id'),
      'description' => t('The Member ID assigned to the membership.'),
      'required' => TRUE,
    ) + $properties['member_id'];

    $properties['type'] = array(
      'label' => t('Type'),
      'description' => t('The membership type.'),
      'required' => TRUE,
    ) + $properties['type'];

    unset($properties['uid']);
    $properties['primary_member'] = array(
      'label' => t('Primary member'),
      'type' => 'user',
      'description' => t('The primary member on the membership.'),
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_setter_method',
      'setter permission' => 'administer memberships',
      'required' => TRUE,
      'schema field' => 'uid',
    );

    $properties['status'] = array(
      'label' => t('Membership Status'),
      'description' => t('The membership status.'),
      'options list' => 'membership_entity_get_status_list',
    ) + $properties['status'];

    $properties['created'] = array(
      'type' => 'date',
      'label' => t('Date created'),
      'description' => t('The date the membership was created.'),
      'setter callback' => 'entity_property_verbatim_set',
      'setter permission' => 'administer memberships',
    ) + $properties['created'];

    $properties['changed'] = array(
      'type' => 'date',
      'label' => t('Date changed'),
      'description' => t('The date the membership was most recently updated.'),
    ) + $properties['changed'];

    $properties['url'] = array(
      'description' => t('The URL of the Membership'),
    ) + $properties['url'];

    $properties['edit_url'] = array(
      'type' => 'uri',
      'label' => t('Edit URL'),
      'description' => t("The URL of the membership's edit page."),
      'getter callback' => 'membership_entity_get_properties',
      'computed' => TRUE,
      'entity views field' => TRUE,
    );

    $properties['secondary_members'] = array(
      'type' => 'list<user>',
      'label' => t('Secondary Members'),
      'description' => t('Secondary members that belong to the membership.'),
      'getter callback' => 'entity_property_verbatim_get',
      'setter callback' => 'entity_property_verbatim_set',
      'computed' => TRUE,
      'entity views field' => TRUE,
    );

    return $info;
  }
}

/**
 * Implements hook_entity_property_info_alter().
 */
function membership_entity_entity_property_info_alter(&$info) {
  // Add related memberships to the user object.
  $properties = &$info['user']['properties'];
  if (!module_exists('membership_entity_type')) {
    $properties['membership_entity_membership'] = array(
      'type' => 'membership_entity',
      'label' => t('Membership'),
      'description' => t("The user's membership."),
      'getter callback' => 'membership_entity_user_get_properties',
      'bundle' => 'membership',
    );
  }
}
