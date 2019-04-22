<?php

/**
 * @file
 * PhpSpreadsheet style plugin.
 */

class views_data_export_phpspreadsheet_plugin_style_export extends views_data_export_plugin_style_export {

  /**
   * Render the display in this style.
   */
  function render() {
    if ($this->uses_row_plugin() && empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }

    $output = '';
    $output .= $this->render_body();
    return $output;
  }

  /**
   * Implements parent::option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    return $options;
  }

  /**
   * Implements parent::options_form().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = array(
        '#markup' => '<div class="error messages">' . t('You need at least one field before you can configure your table settings') . '</div>',
      );
      return;
    }

    // Note: views UI registers this theme handler on our behalf. Your module
    // will have to register your theme handlers if you do stuff like this.
    $form['color'] = array(
      '#title' => t('Color column'),
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );

    $columns = $this->sanitize_columns($this->options['columns']);

    // Create an array of allowed columns from the data we know.
    $field_names = $this->display->handler->get_field_labels();
    foreach ($columns as $field => $column) {
      $option_default = (!empty($this->options['color'][$field])) ? $this->options['color'][$field] : '';
      $form['color'][$field] = array(
        '#title' => $field_names[$field],
        '#type' => 'textfield',
        '#default_value' => $option_default,
        '#attributes' => ['class'=> ['color'],]
      );
    }
    drupal_add_js(drupal_get_path('module', 'views_data_export_phpspreadsheet') . "/js/color.js");
  }
  /**
   * Normalize a list of columns based upon the fields that are
   * available. This compares the fields stored in the style handler
   * to the list of fields actually in the view, removing fields that
   * have been removed and adding new fields in their own column.
   *
   * - Each field must be in a column.
   * - Each column must be based upon a field, and that field
   *   is somewhere in the column.
   * - Any fields not currently represented must be added.
   * - Columns must be re-ordered to match the fields.
   *
   * @param array $columns
   *   An array of all fields; the key is the id of the field and the
   *   value is the id of the column the field should be in.
   * @param array $fields
   *   The fields to use for the columns. If not provided, they will
   *   be requested from the current display. The running render should
   *   send the fields through, as they may be different than what the
   *   display has listed due to access control or other changes.
   *
   * @return array
   *    An array of all the sanitized columns.
   */
  public function sanitize_columns($columns, $fields = NULL) {
    $sanitized = array();
    if ($fields === NULL) {
      $fields = $this->display->handler->get_option('fields');
    }
    // Preconfigure the sanitized array so that the order is retained.
    foreach ($fields as $field => $info) {
      // Set to itself so that if it isn't touched, it gets column
      // status automatically.
      $sanitized[$field] = $field;
    }

    foreach ($columns as $field => $column) {
      // first, make sure the field still exists.
      if (!isset($sanitized[$field])) {
        continue;
      }

      // If the field is the column, mark it so, or the column
      // it's set to is a column, that's ok
      if ($field == $column || $columns[$column] == $column && !empty($sanitized[$column])) {
        $sanitized[$field] = $column;
      }
      // Since we set the field to itself initially, ignoring
      // the condition is ok; the field will get its column
      // status back.
    }

    return $sanitized;
  }
}
