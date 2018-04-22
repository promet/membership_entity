<?php

/**
 * @file
 * Contains the MembershipEntity class.
 */

/**
 * Defines a membership entity.
 */
class MembershipEntity extends Entity {
  /**
   * The primary identifier for a membership.
   *
   * @var int
   */
  public $mid;

  /**
   * A user enterable unique member id.
   *
   * @var int
   */
  public $member_id; // @codingStandardsIgnoreLine

  /**
   * The type (bundle) of membership.
   *
   * @var string
   */
  public $type;

  /**
   * The primary member.
   *
   * @var int
   */
  public $uid;

  /**
   * An array of secondary members.
   *
   * @var array
   */
  public $secondary_members; // @codingStandardsIgnoreLine

  /**
   * Integer code indicating the membership status.
   *
   * @var int
   *
   * @see membership_entity_get_status_list().
   */
  public $status;

  /**
   * The Unix timestamp when the membership was created.
   *
   * @var int
   */
  public $created;

  /**
   * The Unix timestamp when the membership was most recently saved.
   *
   * @var int
   */
  public $changed;

  /**
   * Build a new membership entity.
   */
  public function __construct($values = array()) {
    return parent::__construct($values, 'membership_entity');
  }

  /**
   * Returns the primary member.
   */
  public function primaryMember() {
    return user_load($this->uid);
  }

  /**
   * Set the primary member for the membership.
   *
   * @param object|int $account
   *   The loaded account object or member uid.
   */
  public function setPrimaryMember($account) {
    $this->uid = is_object($account) ? $account->uid : $account;
  }

  /**
   * Returns the full url to the membership.
   */
  public function url() {
    $uri = $this->uri();
    return url($uri['path'], $uri);
  }

  /**
   * Returns the Drupal path to the membership.
   */
  public function path() {
    $uri = $this->uri();
    return $uri['path'];
  }

  /**
   * Overrides Entity::defaultUri().
   */
  public function defaultUri() {
    return array('path' => 'membership/' . $this->mid);
  }

  /**
   * Overrides Entity::save().
   */
  public function save() {
    $this->changed = REQUEST_TIME;

    // If this is a new entity.
    if (!isset($this->mid)) {
      $this->is_new_revision = TRUE;
      $this->created = REQUEST_TIME;
      return parent::save();
    }

    // Load unchanged entity data.
    if (!isset($this->original)) {
      $original = entity_load_unchanged('membership_entity', $this->mid);
    } else {
      $original = $this->original;
      unset($this->original);
    }

    // The entity_type property cannot be changed in the UI, so we ignore 
    // it when comparing current and unchanged states.
    if (isset($this->entity_type)) {
      $original->entity_type = $this->entity_type;
    }

    // Compare new and old objects to check if any change was made.
    if (md5(json_encode($this)) !== md5(json_encode($original))) {
      // Flag to add a new revision.
      $this->is_new_revision = TRUE;
    }

    // EntityAPIController will load original again if left unset.
    $this->original = $original;
    return parent::save();
  }

}
