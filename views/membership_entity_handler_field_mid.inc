<?php

/**
 * @file
 * Contains the basic membership entity field handler.
 */

/**
 * Field handler to provide simple renderer that allows linking to a membership.
 *
 * Definition terms:
 * - link_to_membership default: Should this field have the checkbox
 *   "link to membership" enabled by default.
 *
 * @ingroup views_field_handlers
 */
class membership_entity_handler_field_mid extends views_handler_field {
  /**
   * Initialize.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    // Don't add the additional fields to groupby.
    if (!empty($this->options['link_to_membership'])) {
      $this->additional_fields['mid'] = array('table' => 'membership_entity', 'field' => 'mid');
    }
  }

  /**
   * Option_definition function.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_membership'] = array('default' => isset($this->definition['link_to_membership default']) ? $this->definition['link_to_membership default'] : FALSE, 'bool' => TRUE);
    return $options;
  }

  /**
   * Provide link to membership option.
   */
  function options_form(&$form, &$form_state) {
    $form['link_to_membership'] = array(
      '#title' => t('Link this field to its membership.'),
      '#description' => t("Enable to override this field's links."),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_membership']),
    );

    parent::options_form($form, $form_state);
  }

  /**
   * Render whatever the data is as a link to the membership.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_membership']) && !empty($this->additional_fields['mid'])) {
      if ($data !== NULL && $data !== '') {
        $this->options['alter']['make_link'] = TRUE;
        $this->options['alter']['path'] = "membership/" . $this->get_value($values, 'mid');
        if (isset($this->aliases['language'])) {
          $languages = language_list();
          $language = $this->get_value($values, 'language');
          if (isset($languages[$language])) {
            $this->options['alter']['language'] = $languages[$language];
          }
          else {
            unset($this->options['alter']['language']);
          }
        }
      }
      else {
        $this->options['alter']['make_link'] = FALSE;
      }
    }
    return $data;
  }

  /**
   * Render link.
   */
  function render($values) {
    $value = $this->get_value($values);
    return $this->render_link($this->sanitize_value($value), $values);
  }
}
