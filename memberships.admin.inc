<?php

/**
 * @file
 * Administration pages and functions for the memberships module.
 */

/**
 * Form builder for the settings page.
 */
function membership_admin_settings_form($form, &$form_state) {
  $form = array();

  return system_settings_form($form);
}
