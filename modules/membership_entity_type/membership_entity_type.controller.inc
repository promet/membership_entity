<?php

/**
 * @file
 * Contains the MembershipEntityTypeController class.
 */

/**
 * Entity controller class for membership types.
 */
class MembershipEntityTypeController extends EntityAPIControllerExportable {
  /**
   * Overrides EntityAPIControllerExportable::create().
   */
  public function create(array $values = array()) {
    $values += array(
      'label' => '',
      'type' => '',
      'description' => '',
      'weight' => 0,
      'data' => array(
        'cardinality' => '1',
        'show_on_profile' => 1,
        'all_edit' => 0,
        'roles' => array(),
        'primary_role' => '',
        'secondary_role' => '',
      ),
    );
    return parent::create($values);
  }
}
