<?php

/**
 * @file
 * Theme and preprocess functions.
 */

/**
 * Theme function for XLSX file using PhpSpreadsheet.
 */
function theme_views_data_export_xlsx_body(&$vars) {
  _views_data_export_header_shared_preprocess($vars);
  _views_data_export_body_shared_preprocess($vars);
  $complete_array = array_merge(array($vars['header']), $vars['themed_rows']);
  // Decode HTML entities.
  foreach ($complete_array as &$row) {
    foreach ($row as &$value) {
      $value = decode_entities($value);
    }
  }

  // Load PhpSpreadsheet library.
  $library = libraries_load('PhpSpreadsheet');

  // Create PhpSpreadsheet spreadsheet and add rows to it.
  $xlsx = new \PhpOffice\PhpSpreadsheet\Spreadsheet();
  $xlsx->setActiveSheetIndex(0);
  $xlsx->getActiveSheet()->fromArray($complete_array);
  foreach (array_keys($vars["header"]) as $index=>$field_name){
    $col_name = \PhpOffice\PhpSpreadsheet\Cell\Coordinate::stringFromColumnIndex($index+1);
    $col_coordinate = $col_name.'1:'.$col_name.count($complete_array);
    if(!empty($vars["options"]["color"][$field_name])){
      $color_set= ltrim($vars["options"]["color"][$field_name],'#');
      if(!empty($color_set) && !in_array($color_set,['000','000000'])){
        $xlsx->getActiveSheet()->getStyle($col_coordinate)->getFill()->setFillType(\PhpOffice\PhpSpreadsheet\Style\Fill::FILL_SOLID)->getStartColor()->setARGB($color_set);
      }
    }
  }

  $excel_type = 'excel2007';
  switch ($excel_type) {
    default:
    case 'excel2007':
      $writer = new \PhpOffice\PhpSpreadsheet\Writer\Xlsx($xlsx);
      break;
  }

  // Catch the output of the spreadsheet.
  ob_start();
  if(!empty($writer)){
    $writer->save('php://output');
    unset($writer);
  }
  $excelOutput = ob_get_clean();
  return $excelOutput;
}
