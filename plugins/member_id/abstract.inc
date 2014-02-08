<?php

/**
 * @file
 * Contains the abstract member_id plugin class.
 */

/**
 * Additional member_id formats for a membership type.
 */
interface MembershipEntityMemberIdInterface {
  public function __construct($settings);

  /**
   * Return the next available member id.
   */
  public function next();

  /**
   * Builds the settings form this handler.
   */
  public function settingsForm(&$form_state);
}

/**
 * An abstract implementation of MembershipEntityMemberId.
 */
abstract class MembershipEntityMemberIdAbstract implements MembershipEntityMemberIdInterface {
  /**
   * The settings options.
   */
  protected $settings;

  public function __construct($settings) {
    $this->settings = $settings;
  }

  public function next() {}

  public function settingsForm(&$form_state) {
    return array();
  }
}

/**
 * A broken implementation of MembershipEntityMemberId.
 */
class MembershipEntityMemberIdBroken extends MembershipEntityMemberIdAbstract {
  public function settingsForm(&$form_state) {
    $form['member_id_broken'] = array(
      '#markup' => t('The selected member_id handler is broken.'),
    );
    return $form;
  }
}
