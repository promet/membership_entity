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
   * Build a new membership entity term.
   */
  public function __construct($values = array()) {
    return parent::__construct($values, 'membership_entity_term');
  }

  /**
   * Overrides Entity::save().
   */
  public function save() {
    $this->calculateEndDate();
    return parent::save();
  }

  /**
   * Calculate the term end date.
   */
  protected function calculateEndDate() {
    if ($this->term == 'lifetime') {
      $this->end = NULL;
      return;
    }

    $end = new DateObject($this->start, new DateTimezone('UTC'));

    // Add the term length.
    $end->add(DateInterval::createFromDateString($this->term));

    // Add modifiers.
    foreach ($this->modifiers as $modifier) {
      $end->add(DateInterval::createFromDateString($modifier));
    }
    $this->end = date_format_date($end, 'custom', DATE_FORMAT_DATETIME);
  }
}

