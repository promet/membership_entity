<?php

/**
 * @file
 * Contains the MembershipEntityType class.
 */

/**
 * Defines a membership type.
 */
class MembershipEntityType extends Entity {
  public function __construct($values = array()) {
    parent::__construct($values, 'membership_entity_type');
  }
}
