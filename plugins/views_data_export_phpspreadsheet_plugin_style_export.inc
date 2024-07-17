<?php

/**
 * @file
 * PhpSpreadsheet style plugin.
 */

class views_data_export_phpspreadsheet_plugin_style_export extends views_data_export_plugin_style_export {

  /**
   * Store current row number in spreadsheet
   *
   * @var int
   */
  private $spreadsheet_current_row = 1;

  /**
   * The PhpSpreadsheet object, if it's initialized.
   *
   * @var \PhpOffice\PhpSpreadsheet\Spreadsheet
   */
  private $spreadsheet;

  /**
   * @inheritdoc
   */
  public function construct() {
    parent::construct();
    $this->loadLibrary();
  }

  /**
   * Loads the PhpSpreadsheet library.
   *
   * @return bool
   *   Returns TRUE if the PhpSpreadsheet library was successfully loaded,
   * FALSE otherwise.
   */
  public function loadLibrary() {
    // Load PhpSpreadsheet library if primary class is not already present.
    if (!class_exists('\PhpOffice\PhpSpreadsheet\Spreadsheet')) {
      $library = libraries_load('PhpSpreadsheet');
    }
    else {
      $library = ['loaded' => TRUE];
    }
    if (empty($library['loaded'])) {
      $message = t('The PhpSpreadsheet library could not be found! Please following the instructions from the README.txt to get the library.');
      watchdog('views_data_export_phpspreadsheet', $message, [], WATCHDOG_ERROR);
      backdrop_set_message($message, $type = 'error');
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Unloads the spreadsheet.
   */
  public function unload() {
    $this->spreadsheet->disconnectWorksheets();
    unset($this->spreadsheet);
  }

  /**
   * Create a new PhpSpreadsheet.
   */
  function createSpreadsheet() {
    $spreadsheet = new \PhpOffice\PhpSpreadsheet\Spreadsheet();
    $spreadsheet->setActiveSheetIndex(0);

    return $spreadsheet;
  }

  /**
   * Set the PhpSpreadsheet document to the output file path.
   */
  protected function saveSpreadsheetFile() {
    $file_path = backdrop_realpath($this->view->display_handler->outputfile_path());

    $writer = \PhpOffice\PhpSpreadsheet\IOFactory::createWriter($this->getSpreadsheet(), "Xlsx");
    $writer->save($file_path);

    // Writer has deleted and re-created the file, so fix permissions now.
    backdrop_chmod($file_path);
  }

  /**
   * Get the PhpSpreadsheet.
   */
  protected function getSpreadsheet() {
    if (!$this->spreadsheet) {
      if (empty($this->view->display_handler->batched_execution_state)) {
        $this->spreadsheet = $this->createSpreadsheet();
      }
      elseif (empty($this->view->display_handler->batched_execution_state->sandbox['spreadsheet_current_row'])) {
        // This is a batch and it has just started.
        $this->spreadsheet = $this->createSpreadsheet();
        $this->view->display_handler->batched_execution_state->sandbox['spreadsheet_current_row'] = TRUE;
        $this->saveSpreadsheetFile();
      }
      else {
        $reader = \PhpOffice\PhpSpreadsheet\IOFactory::createReader("Xlsx");
        $file_path = backdrop_realpath($this->view->display_handler->outputfile_path());
        if (file_exists($file_path)) {
          $this->spreadsheet = $reader->load($file_path);
          $this->spreadsheet->setActiveSheetIndex(0);
          $this->spreadsheet_current_row = $this->view->display_handler->batched_execution_state->sandbox['spreadsheet_current_row'];
          unlink($file_path);
        }
      }
    }

    return $this->spreadsheet;
  }

  /**
   * Append rows to the PhpSpreadsheet.
   */
  public function appendRows(array $rows) {
    $this->getSpreadsheet()
      ->getActiveSheet()
      ->fromArray($rows, NULL, 'A' . $this->spreadsheet_current_row);
    $this->spreadsheet_current_row += count($rows);
  }

  /**
   * Apply color options to spreadsheet.
   *
   * @param array $colors
   */
  public function applyColorOptions(array $colors) {
    $field_names = $this->display->handler->get_field_labels();

    foreach (array_keys($field_names) as $index => $field_name) {
      $col_name = \PhpOffice\PhpSpreadsheet\Cell\Coordinate::stringFromColumnIndex($index + 1);
      $col_coordinate = $col_name . '1:' . $col_name . ($this->spreadsheet_current_row - 1);
      if (!empty($colors[$field_name])) {
        $color_set = ltrim($colors[$field_name], '#');
        if (!empty($color_set) && !in_array($color_set, ['000', '000000'])) {
          $this->getSpreadsheet()->getActiveSheet()
            ->getStyle($col_coordinate)
            ->getFill()
            ->setFillType(\PhpOffice\PhpSpreadsheet\Style\Fill::FILL_SOLID)
            ->getStartColor()
            ->setARGB($color_set);
        }
      }
    }
  }

  /**
   * Implements parent::option_definition().
   */
  public function option_definition() {
    $options = parent::option_definition();

    return $options;
  }

  /**
   * Implements parent::options_form().
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = [
        '#markup' => '<div class="error messages">' . t('You need at least one field before you can configure your table settings') . '</div>',
      ];
      return;
    }

    // Note: views UI registers this theme handler on our behalf. Your module
    // will have to register your theme handlers if you do stuff like this.
    $form['color'] = [
      '#title' => t('Color column'),
      '#type' => 'fieldset',
      '#tree' => TRUE,
    ];

    if (!empty($this->options['columns'])) {
      $columns = $this->sanitize_columns($this->options['columns']);
    }
    else {
      $columns = $this->sanitize_columns([]);
    }

    // Create an array of allowed columns from the data we know.
    $field_names = $this->display->handler->get_field_labels();
    if (!empty($columns)) {
      foreach ($columns as $field => $column) {
        $option_default = (!empty($this->options['color'][$field])) ? $this->options['color'][$field] : '';
        $form['color'][$field] = [
          '#title' => $field_names[$field],
          '#type' => 'textfield',
          '#default_value' => $option_default,
          '#attributes' => ['class' => ['color']],
        ];
      }
    }
    backdrop_add_js(backdrop_get_path('module', 'views_data_export_phpspreadsheet') . "/js/color.js");
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
    $sanitized = [];
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

  /**
   * Render the display in this style.
   */
  public function render() {
    if ($this->uses_row_plugin() && empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }

    $this->render_header();
    $this->render_body();
    return $this->render_footer();
  }

  /**
   * @inheritdoc
   */
  public function render_header() {
    parent::render_header();

    if (!empty($this->view->display_handler->batched_execution_state)) {
      $this->view->display_handler->batched_execution_state->sandbox['spreadsheet_current_row'] = $this->spreadsheet_current_row;
      $this->saveSpreadsheetFile();
    }
  }

  /**
   * @inheritdoc
   */
  public function render_body() {
    parent::render_body();

    if (!empty($this->view->display_handler->batched_execution_state)) {
      $this->view->display_handler->batched_execution_state->sandbox['spreadsheet_current_row'] = $this->spreadsheet_current_row;
      $this->saveSpreadsheetFile();
    }
  }

  /**
   * @inheritdoc
   */
  public function render_footer() {
    parent::render_footer();

    if (!empty($this->view->display_handler->batched_execution_state)) {
      $this->saveSpreadsheetFile();
    }
    else {
      // Avoid corrupted file on some servers
      // see https://stackoverflow.com/questions/57730290/phpspreadsheet-corrupted-xlsx-file
      ob_end_clean();

      // Catch the output of the spreadsheet.
      ob_start();

      $writer = new \PhpOffice\PhpSpreadsheet\Writer\Xlsx($this->getSpreadsheet());
      if (!empty($writer)) {
        $writer->save('php://output');
      }

      // Clearing a Spreadsheet from memory.
      $this->unload();

      return ob_get_clean();
    }
  }

}
