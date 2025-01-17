<?php

/**
 * @file
 * Theme and preprocess functions.
 */

/**
 * Theme function for header row of XLSX file using PhpSpreadsheet.
 */
function theme_views_data_export_xlsx_header(&$vars) {
  // TODO: Should this theme views_data_export_xlsx_header be declared in hook_theme()?
  _views_data_export_header_shared_preprocess($vars);
  $rows = [$vars['header']];
  $vars['view']->style_plugin->appendRows($rows);
}

/**
 * Theme function for data rows XLSX file using PhpSpreadsheet.
 */
function theme_views_data_export_xlsx_body(&$vars) {
  _views_data_export_body_shared_preprocess($vars);
  $rows = $vars['themed_rows'];
  $vars['view']->style_plugin->appendRows($rows);

  if (isset($vars['options']['color'])) {
    $vars['view']->style_plugin->applyColorOptions($vars['options']['color']);
  }
}
