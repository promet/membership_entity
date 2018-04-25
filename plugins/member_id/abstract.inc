<?php

/**
 * @file
 * Contains the abstract member_id plugin class.
 */

/**
 * Additional member_id formats for a membership type.
 */
interface MembershipEntityMemberIdInterface {

  /**
   * Determines the next available member id.
   *
   * @param MembershipEntity $membership
   *   The membership object used for tokens.
   *
   * @return string
   *   The next available member id.
   */
  public function next(MembershipEntity $membership);

  /**
   * Builds the settings form this handler.
   *
   * @param array $form_state
   *   The Form API form_state array.
   *
   * @return array
   *   The Form API form array.
   *
   * @todo determine if form_state parameter is actually needed, since it
   * is not used within the method.
   */
  public function settingsForm(array &$form_state);

  /**
   * Validates the plugin settings form.
   *
   * @param array $element
   *   Form API element array.
   * @param array $form_state
   *   Form API form_state array.
   */
  public function validateSettings(array &$element, array &$form_state);

}

/**
 * An abstract implementation of MembershipEntityMemberId.
 */
abstract class MembershipEntityMemberIdAbstract implements MembershipEntityMemberIdInterface {
  /**
   * The settings options.
   *
   * @var array
   */
  protected $settings;

  /**
   * Default constructor.
   *
   * @param array $settings
   *   An array of default plugin settings.
   */
  public function __construct(array $settings) {
    $this->settings = $settings;
  }

  /**
   * {@inheritdoc}
   */
  public function next(MembershipEntity $membership) {}

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array &$form_state) {
    return array();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings(array &$element, array &$form_state) {}

}

/**
 * A broken implementation of MembershipEntityMemberId.
 */
class MembershipEntityMemberIdBroken extends MembershipEntityMemberIdAbstract {

  /**
   * {@inheritdoc}
   */
  public function next(MembershipEntity $membership) {}

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array &$form_state) {
    $form['member_id_broken'] = array(
      '#markup' => t('The selected member_id handler is broken.'),
    );
    return $form;
  }

}
