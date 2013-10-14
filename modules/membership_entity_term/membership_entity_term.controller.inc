<?php

/**
 * @file
 * Containse the MembershipEntityTermController class.
 */

/**
 * Entity controller class for membership term entities.
 */
class MembershipEntityTermController extends EntityAPIController {
  /**
   * Overrides EntityAPIController::create().
   */
  public function create(array $values = array()) {
    // Add some defaults.
    $values += array(
      'id' => 0,
      'mid' => 0,
      'term' => '',
      'modifiers' => array(),
      'start' => format_date(REQUEST_TIME, 'custom', DATE_FORMAT_DATETIME),
    );
    return parent::create($values);
  }

  /**
   * Load all terms for a given membership.
   *
   * @param $membership
   *  A membership object or mid.
   */
  public function load_by_membership($membership) {
    // Support mid or entire membership object.
    $mid = is_object($membership) ? $membership->mid : $membership;

    $ids = array();
    $efq = new EntityFieldQuery();
    $result = $efq->entityCondition('entity_type', 'membership_entity_term')
      ->propertyCondition('mid', $mid)
      ->propertyOrderBy('start')
      ->execute();
    if (!empty($result)) {
      $ids = array_merge($ids, array_keys($result['membership_entity_term']));
    }
    return $this->load($ids);
  }
}

