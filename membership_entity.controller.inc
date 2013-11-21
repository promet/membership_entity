<?php

/**
 * @file
 * Containse the MembershipEntityController class.
 */

/**
 * Entity controller class for membership entities.
 */
class MembershipEntityController extends EntityAPIController {
  /**
   * Overrides EntityAPIController::create().
   */
  public function create(array $values = array()) {
    // Add some defaults.
    $values += array(
      'mid' => 0,
      'uid' => 0,
      'type' => 'membership',
      'created' => REQUEST_TIME,
      'status' => MEMBERSHIP_ENTITY_ACTIVE,
      'secondary_members' => array(),
    );
    return parent::create($values);
  }

  /**
   * Overrides EntityAPIController::load().
   */
  public function load($mids = array(), $conditions = array()) {
    $memberships = parent::load($mids, $conditions);

    if (empty($memberships)) {
      // Early exit if membership loading failed.
      return array();
    }

    // Load secondary members.
    foreach ($memberships as $mid => $membership) {
      $membership->secondary_members = array();
      $secondary_members = db_select('membership_entity_secondary_member')
        ->fields('membership_entity_secondary_member', array('uid', 'weight'))
        ->condition('mid', $membership->mid)
        ->orderBy('weight')
        ->execute();
      foreach ($secondary_members as $member) {
        $membership->secondary_members[$member->weight] = $member->uid;
      }
    }
    return $memberships;
  }

  /**
   * Load all memberships for a given user.
   *
   * @param $account
   *  A user account object or uid.
   */
  public function load_by_user($account) {
    // Support uid or entire user object.
    $uid = is_object($account) ? $account->uid : $account;
    $mids = array();

    $efq = new EntityFieldQuery();
    $result = $efq->entityCondition('entity_type', 'membership_entity')
      ->propertyCondition('uid', $uid)
      ->execute();
    if (!empty($result)) {
      $mids = array_merge($mids, array_keys($result['membership_entity']));
    }
    return $this->load($mids);
  }

  /**
   * Overrides EntityAPIController::save().
   */
  public function save($membership, DatabaseTransaction $transaction = NULL) {
    $transaction = isset($transaction) ? $transaction : db_transaction();
    $data = variable_get('membership_entity_settings', array());

    try {
      $roles = array();

      // Add or remove old roles depending on membership status.
      $roles[$membership->uid] = array(
        'rid' => $data['primary_role'],
        'op' => ($membership->status == MEMBERSHIP_ENTITY_ACTIVE) ? 'add' : 'remove',
      );
      foreach ($membership->secondary_members as $uid) {
        $roles[$uid] = array(
          'rid' => $data['secondary_role'],
          'op' => ($membership->status == MEMBERSHIP_ENTITY_ACTIVE) ? 'add' : 'remove',
        );
      }

      // Remove old roles.
      $original_uid = db_select('membership_entity')->fields('membership_entity', array('uid'))->condition('mid', $membership->mid)->execute()->fetchField();
      if ($membership->uid != $original_uid) {
        $roles[$original_uid] = array(
          'rid' => $data['primary_role'],
          'op' => 'remove',
        );
      }
      $original_secondary = array_keys(db_select('membership_entity_secondary_member')->fields('membership_entity_secondary_member', array('uid'))->condition('mid', $membership->mid)->execute()->fetchAllAssoc('uid'));
      foreach (array_diff($original_secondary, $membership->secondary_members) as $uid) {
        $roles[$uid] = array(
          'rid' => $data['secondary_role'],
          'op' => 'remove',
        );
      }
      _membership_entity_update_roles($roles);
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('membership_entity', $e);
      throw $e;
    }

    // Save the membership.
    $status = parent::save($membership, $transaction);

    try {
      // Update secondary members.
      db_delete('membership_entity_secondary_member')->condition('mid', $membership->mid)->execute();
      if (!empty($membership->secondary_members)) {
        $query = db_insert('membership_entity_secondary_member')->fields(array('mid', 'uid', 'weight'));
        foreach ($membership->secondary_members as $weight => $uid) {
          $query->values(array(
            'mid' => $membership->mid,
            'uid' => $uid,
            'weight' => $weight,
          ));
        }
        $query->execute();
      }
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('membership_entity', $e);
      throw $e;
    }

    return $status;
  }

  /**
   * Overrides EntityAPIController::delete().
   */
  public function delete($mids, DatabaseTransaction $transaction = NULL) {
    $memberships = $mids ? $this->load($mids) : FALSE;
    if (!$memberships) {
      // Early exit if no memberships were loaded.
      return;
    }

    try {
      // Remove roles.
      $data = variable_get('membership_entity_settings', array());
      $roles = array();
      foreach ($memberships as $membership) {
        $roles[$membership->uid] = array(
          'rid' => $data['primary_role'],
          'op' => 'remove',
        );
        foreach ($membership->secondary_members as $uid) {
          $roles[$uid] = array(
            'rid' => $data['secondary_role'],
            'op' => 'remove',
          );
        }
      }
      _membership_entity_update_roles($roles);

      // Delete secondary members.
      db_delete('membership_entity_secondary_member')
        ->condition('mid', $mids, 'IN')
        ->execute();
    }
    catch (Exception $e) {
      if (isset($transaction)) {
        $transaction->rollback();
      }
      watchdog_exception($this->entityType, $e);
      throw $e;
    }

    // Finish deleting the memberships;
    parent::delete($mids, $transaction);
  }
}
