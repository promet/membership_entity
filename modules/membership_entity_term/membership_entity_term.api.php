<?php

/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * Alter a membership term end date before it is saved.
 *
 * Membership term end dates are calculated automatically based on start date
 * and term length. This hook allows you to apply special rules when
 * calculating the end date.
 *
 * @param DateObject $end
 *   The end date based on term start date and term length.
 * @param MembershipEntityTerm $term
 *   The full membership term object.
 */
function hook_membership_entity_term_end_date_alter(&$end, &$term) {
  // Add term modifiers.
  foreach ($term->modifiers as $modifier) {
    $end = _membership_entity_term_modify_date($end, $modifier);
  }
}
