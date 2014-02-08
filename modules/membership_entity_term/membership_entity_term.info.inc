<?php

/**
 * @file
 * Provides entity metadata integration.
 */

/**
 * Provides metadata for membership terms.
 */
class MembershipEntityTermMetadataController extends EntityDefaultMetadataController {
  /**
   * Overrides EntityDefaultMetadataController::entityPropertyInfo().
   */
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    unset($properties['url']);

    unset($properties['mid']);
    $properties['membership'] = array(
      'label' => t('Membership'),
      'type' => 'membership_entity',
      'description' => t('The membership for this term.'),
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_setter_method',
      'setter permission' => 'administer memberships',
      'required' => TRUE,
      'schema field' => 'mid',
    );

    $properties['term'] = array(
      'label' => t('Term length'),
      'description' => t('The membership term length.'),
    ) + $properties['term'];

     $properties['status'] = array(
      'label' => t('Term Status'),
      'description' => t('The membership term status.'),
      'options list' => 'membership_entity_get_status_list',
    ) + $properties['status'];

    $properties['start'] = array(
      'type' => 'date',
      'label' => t('Start date'),
      'description' => t('The date the membership term begins.'),
      'required' => TRUE,
      'schema field' => 'start',
    );

    $properties['end'] = array(
      'type' => 'date',
      'label' => t('End date'),
      'description' => t('The date membership term ends.'),
      'required' => TRUE,
      'schema field' => 'end',
    );

    return $info;
  }
}
