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

    $properties['start'] = array(
      'type' => 'date',
      'label' => t('Start date'),
      'description' => t('The date the membership term began.'),
      'getter callback' =>  'membership_entity_term_get_properties',
      'computed' => TRUE,
      'entity views field' => TRUE,
    );

    $properties['end'] = array(
      'type' => 'date',
      'label' => t('End date'),
      'description' => t('The date membership term will end.'),
      'getter callback' =>  'membership_entity_term_get_properties',
      'computed' => TRUE,
      'entity views field' => TRUE,
    );

    return $info;
  }
}

/**
 * Implements hook_entity_property_info_alter().
 */
function membership_entity_term_entity_property_info_alter(&$info) {
  // Add term based membership entity properties.
  $properties = &$info['membership_entity']['properties'];

  $properties['expiration'] = array(
    'type' => 'date',
    'label' => t('Expiration date'),
    'description' => t('The date the latest membership term will expire.'),
    'getter callback' => 'membership_entity_term_get_properties',
    'computed' => TRUE,
    'entity views field' => TRUE,
  );

  $properties['renew_url'] = array(
    'type' => 'uri',
    'label' => t('Renew URL'),
    'description' => t("The URL of the membership's renew page."),
    'getter callback' => 'membership_entity_term_get_properties',
    'computed' => TRUE,
    'entity views field' => TRUE,
  );
}

