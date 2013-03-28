<?php

/**
 * @file
 * Membership entity controller class definition.
 */

/**
 * MembershipEntityControllerInterface definition.
 */
interface MembershipEntityControllerInterface
  extends DrupalEntityControllerInterface {
  public function create($type);
  public function save($membership);
  public function delete_multiple($mids);
}

/**
 * Entity controller class for memberhip entities.
 */
class MembershipEntityController
  extends DrupalDefaultEntityController
  implements MembershipEntityControllerInterface {

  /**
   * Return a new membership entity with default values.
   *
   * @param $type
   *  The membership type for the new membership entity.
   */
  public function create($type) {
    $user = $GLOBALS['user'];

    return (object) array(
      'entity_type' => 'membership_entity',
      'mid' => 0,
      'uid' => $user->uid,
      'type' => $type,
      'created' => REQUEST_TIME,
      'status' => MEMBERSHIP_ENTITY_ACTIVE,
      'terms' => array(),
    );
  }

  /**
   * Loads a membership entity from the database.
   *
   * @param $mids
   *  An array of membership_entity mids to load.
   * @param $conditions
   *  An array of conditions to match.
   */
  public function load($mids = array(), $conditions = array()) {
    $memberships = parent::load($mids, $conditions);

    // Load memberships terms.
    foreach ($memberships as &$membership) {
      $membership->terms = db_select('membership_entity_terms', 'met')
        ->fields('met')
        ->condition('mid', $membership->mid)
        ->orderBy('delta', 'ASC')
        ->execute()
        ->fetchAllAssoc('delta', PDO::FETCH_ASSOC);
    }
    // Unset foreach reference.
    unset($membership);

    return $memberships;
  }

  /**
   * Saves membership_entity fields using drupal_write_record().
   *
   * @param $membership
   *  The membership object to save. If $membership->mid is omitted or is 0, a
   *  new membership will be created.
   */
  public function save($membership) {
    field_attach_presave('membership_entity', $membership);

    // Determine if we will be inserting a new membership.
    $is_new = empty($membership->mid);
    $membership->changed = REQUEST_TIME;

    // Let modules modify the membership before it is saved to the database.
    module_invoke_all('entity_presave', $membership, 'membership_entity');

    // Save the membership.
    if ($is_new) {
      drupal_write_record('membership_entity', $membership);
      $op = 'insert';
    }
    else {
      drupal_write_record('membership_entity', $membership, 'mid');
      $op = 'update';
    }

    // Save the membership terms.
    foreach ($membership->terms as $term) {
      if (empty($term['mid'])) {
        $term['mid'] = $membership->mid;
        drupal_write_record('membership_entity_terms', $term);
      }
      else {
        drupal_write_record('membership_entity_terms', $term, array('mid', 'delta'));
      }
    }

    // Save fields.
    $function = 'field_attach_' . $op;
    $function('membership_entity', $membership);

    // Envoke insert/update hooks.
    module_invoke_all('entity_' . $op, $membership, 'membership_entity');
    module_invoke_all('membership_entity_' . $op, $membership);

    return $membership;
  }

  /**
   * Delete one or more membership entities.
   *
   * @param $mid
   *   An array of membership ids to delete.
   */
  public function delete_multiple($mids) {
    if (empty($mids)) {
      return;
    }

    $memberships = membership_entity_load_multiple($mids);
    $transaction = db_transaction();
    try {
      foreach ($memberships as $mid => $membership) {
        module_invoke_all('entity_delete', $membership, 'membership_entity');
        field_attach_delete('membership_entity', $membership);
      }

      // Delete the membership.
      db_delete('membership_entity')
        ->condition('mid', $mids, 'IN')
        ->execute();

      // Delete the membership terms.
      db_delete('membership_entity_terms')
        ->condition('mid', $mids, 'IN')
        ->execute();
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('membership_entity', $e);
      throw $e;
    }

    $this->resetCache($mids);
  }
}
