<?php

/**
 * @file
 * Contains the MembershipEntity class.
 */

/**
 * Defines a membership entity.
 */
class MembershipEntityTerm extends Entity {
  /**
   * The membership identifier.
   */
  public $mid;

  /**
   * The human readable term length. eg. 1 year.
   */
  public $term;

  /**
   * An array term length modifiers. eg. +1 month.
   */
  public $modifiers;

  /**
   * The UNIX timestamp when this term starts.
   */
  public $start;

  /**
   * The UNIX timestamp when this term ends.
   */
  public $end;

  /**
   * Build a new membership entity.
   */
  public function __construct($values = array()) {
    return parent::__construct($values, 'membership_entity_term');
  }
}

