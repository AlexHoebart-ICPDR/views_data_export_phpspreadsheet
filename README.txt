Views Data Export PhpSpreadsheet
==========================

Description
-----------
This module extends Views Data Export module to allow XLSX files export.
Clone from https://www.drupal.org/project/views_data_export_phpexcel to
use a library that is a) maintained and b) compatible with PHP 7.2.
Add option select color foreach column

Requirements
------------
1. Views Data Export
2. PhpSpreadsheet <https://https://github.com/PHPOffice/PhpSpreadsheet/>
3. Libraries or Composer Manager

Installation if Using Libraries Module
--------------------------------------
1. Download the PhpSpreadsheet library and extract it inside your site's libraries folder:
- sites/all/libraries/PhpSpreadsheet
2. Run "composer install" in the libraries folder to install all dependencies.
3. Enable the module.

Installation if Using Composer Manager
--------------------------------------
1. Ensure that composer_manager is enabled.
2. Use drush to install the module: `drush en views_data_export_phpspreadsheet`
3. Check the site's Status Report for the "PhpSpreadsheet library" entry.  With
composer_manager enabled, drush should have automatically installed the
necessary libraries.  If not, run:
- `drush composer-json-rebuild`
- `drush composer-manager update phpoffice\phpspreadsheet`

Switch from Libraries to Composer Manager
-----------------------------------------
1. Enable composer_manager.
2. Delete the existing library files at sites/all/libraries/PhpSpreadsheet.
3. Rebuild the master composer.json file: `drush composer-json-rebuild`
4. Update/install: `drush composer-manager update phpoffice/phpspreadsheet`
5. Check the site's Status Report for the "PhpSpreadsheet library" entry.

Switching from views_data_export_phpexcel
-----------------------------------------------
This module is intended to run as a replacement for views_data_export_phpexcel
to the greatest extent possible.  Ideally, you will not need to rebuild your
views to use it. To make this possible, however, some function names have been
duplicated, which makes the modules incompatible.

1. Disable views_data_export_phpexcel.  If you have features that depend on it,
this will fail until you remove the dependency.
- Search for "views_data_export_phpexcel" in your feature .info files.
- If there is a dependency, remove or comment that line, and add
'features_exclude[dependencies][views_data_export_phpexcel] = views_data_export_phpexcel'
at the bottom.
- `drush cc all`
- Try disabling the module again.
2. Install and enable this module.

Usage
-----
Follow the instructions from README.txt of the Views Data Export module.
