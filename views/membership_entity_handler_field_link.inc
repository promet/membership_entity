<?php

/**
 * @file
 * Definition of membership_entity_handler_field_link.
 */

/**
 * Field handler to present a link to the membership.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_link extends views_handler_field_entity {

  /**
   * Option_defintion function.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  /**
   * Field options form.
   */
  function options_form(&$form, &$form_state) {
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
    parent::options_form($form, $form_state);

    // The path is set by render_link function so don't allow to set it.
    $form['alter']['path'] = array('#access' => FALSE);
    $form['alter']['external'] = array('#access' => FALSE);
  }

  /**
   * Render function.
   */
  function render($values) {
    if ($entity = $this->get_value($values)) {
      return $this->render_link($entity, $values);
    }
  }

  /**
   * Render link.
   */
  function render_link($membership, $values) {
    if (membership_entity_access('view', $membership)) {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = "membership/$membership->mid";
      $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
      return $text;
    }
  }
}
