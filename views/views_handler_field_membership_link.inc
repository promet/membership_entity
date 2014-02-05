<?php

/**
 * @file
 * Definition of views_handler_field_membership_link.
 */

/**
 * Field handler to show a link to the membership.
 */
class views_handler_field_membership_link extends views_handler_field_entity {
  public $entity_info;

  public $op;

  public function init(&$view, $options) {
    parent::init($view, $options);
    $this->entity_info = entity_get_info($this->entity_type);
    $this->op = $this->definition['op'];

    // Add the primary key field.
    $key = $this->entity_info['entity keys']['id'];
    $this->additional_fields[$key] = $key;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => $this->op, 'translatable' => TRUE);
    return $options;
  }

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

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  // An example of field level access control.
  function render($values) {
    if ($entity = $this->get_value($values)) {
      return $this->render_link($entity, $values);
    }
  }

  function render_link($entity, $values) {
    if (entity_access('view', $this->entity_type, $entity)) {
      $this->options['alter']['make_link'] = TRUE;
      $uri = entity_uri($this->entity_type, $entity);
      $this->options['alter']['path'] = $uri['path'];
      if ($this->op != 'view') {
        $this->options['alter']['path'] .= "/$this->op";
        $this->options['alter']['query'] = drupal_get_destination();
      }
      $text = !empty($this->options['text']) ? $this->options['text'] : $this->op;
      return $text;
    }
  }
}
