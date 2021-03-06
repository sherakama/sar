<?php
/**
 * @file
 * Drush plugin to globally search & replace data in ALL text fields.
 *
 * Eg: replace /sites/foo.com/files/ with /sites/bar.org/files/
 */

/**
 * Implements hook_drush_command().
 */
function sar_drush_command() {

  // Just for text fields.
  $items['search-and-replace'] = array(
    'aliases' => array('sar'),
    'callback' => 'drush_sar_replace',
    'description' => dt('Replace strings in text fields in all content.'),
    'options' => array(
      'blocks'   => dt('Only perform the search and replace on blocks.'),
      'bundles'  => dt('Only perform the search and replace on these named bundles.'),
      'fields'   => dt('Only perform the search and replace on these named fields.'),
      'dry-run'  => dt('Only show which changes would be made, do not modify any data.'),
      'show-ids' => dt('Show IDs of the entities that would be changed (Forces dry-run to ON).'),
    ),
    'arguments' => array(
      'search'  => dt('Existing text.'),
      'replace' => dt('Replacement text.'),
    ),
    'examples' => array(
      'simple'    => dt('drush sar devel.example.com www.example.com'),
      'blocks'    => dt('drush sar --blocks devel.example.com www.example.com'),
      'bundles'   => dt('drush sar --bundles=page,article devel.example.com www.example.com'),
      'fields'    => dt('drush sar --fields=field_name,field_other devel.example.com www.example.com'),
      'complex'   => dt('drush sar --bundles=article --fields=field_name,field_other devel.example.com www.example.com'),
      'dry run'   => dt('drush sar --dry-run devel.example.com www.example.com'),
      'show ids'  => dt('drush sar --show-ids devel.example.com www.example.com'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // For JS Injector rules.
  $items['search-and-replace-js-injector'] = array(
    'aliases' => array('sarjs'),
    'callback' => 'drush_sar_replace_js_injector',
    'description' => dt('Replace strings in all JS Injector rules.'),
    'options' => array(
      'regex' => dt('Use regular expression when searching for a match'),
    ),
    'arguments' => array(
      'search' => dt('Existing text.'),
      'replace' => dt('Replacement text.'),
    ),
    'examples' => array(
      'simple' => dt('drush sarjs devel.example.com www.example.com'),
      'regex' => dt('drush sarjs devel.example.com www.example.com --regex=^https://'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // For links.
  $items['search-and-replace-link'] = array(
    'aliases' => array('sarl'),
    'callback' => 'drush_sar_replace_link',
    'description' => dt('Replace strings in link fields in all content.'),
    'options' => array(
      'fields'   => dt('Only perform the search and replace on these named fields.'),
      'regex'    => dt('Use regular expression when searching for a match'),
    ),
    'arguments' => array(
      'search'  => dt('Existing text.'),
      'replace' => dt('Replacement text.'),
    ),
    'examples' => array(
      'simple'    => dt('drush sarl devel.example.com www.example.com'),
      'fields'    => dt('drush sarl --fields=field_name,field_other devel.example.com www.example.com'),
      'regex'    => dt('drush sarl devel.example.com www.example.com --regex=^https://'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // For menu items.
  $items['search-and-replace-menu'] = array(
    'aliases' => array('sarm'),
    'callback' => 'drush_sar_replace_menu',
    'description' => dt('Replace strings in menu items.'),
    'options' => array(
      'menus'   => dt('Only perform the search and replace on these menus.'),
      'regex'   => dt('Use regular expression when searching for a match'),
    ),
    'arguments' => array(
      'search'  => dt('Existing text.'),
      'replace' => dt('Replacement text.'),
    ),
    'examples' => array(
      'simple'    => dt('drush sarm devel.example.com www.example.com'),
      'menus'    => dt('drush sarm --menus=main_menu,navigation devel.example.com www.example.com'),
      'regex'    => dt('drush sarm devel.example.com www.example.com --regex=^https://'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // For Views.
  $items['search-and-replace-views'] = array(
    'aliases' => array('sarv'),
    'callback' => 'drush_sar_replace_views',
    'description' => dt('Replace strings in all Views. The search and replace strings support regex as used in preg_replace().'),
    'arguments' => array(
      'search' => dt('Existing text.'),
      'replace' => dt('Replacement text.'),
    ),
    'options' => array(
      'regex' => dt('Use regular expression when searching for a match'),
    ),
    'examples' => array(
      'simple' => dt('drush sarv devel.example.com www.example.com'),
      'regex' => dt('drush sarv --regex "([a-z0-9]).example.com" "\$1.example.org"'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // For Contexts.
  $items['search-and-replace-context'] = array(
    'aliases' => array('sarc'),
    'callback' => 'drush_sar_replace_context',
    'description' => dt('Replace strings in all contexts. The search and replace strings support regex as used in preg_replace().'),
    'arguments' => array(
      'search' => dt('Existing text.'),
      'replace' => dt('Replacement text.'),
    ),
    'options' => array(
      'regex' => dt('Use regular expression when searching for a match'),
    ),
    'examples' => array(
      'simple' => dt('drush sarc this-old-path my-new-url'),
      'regex' => dt('drush sarc --regex "([a-z0-9]).example.com" "\$1.example.org"'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}

/**
 * Implements hook_hook_drush_command_validate().
 */
function drush_sar_replace_validate() {
  $args = drush_get_arguments();
  if (count($args) < 3) {
    drush_set_error('NO_TEXT', dt('You must specify text to search & replace.'));
  }
}

/**
 * Command callback.
 *
 * This callback checks the version of Drupal it's operating on and runs an
 * appropriate function to do the actual work. If an unsupported version is
 * detected, it will exit.
 */
function drush_sar_replace() {

  // Parse our command line options.
  $options = drush_sar_parse_options();

  // If there's a problem yo I'll solve it...
  if (!empty($options['error'])) {
    foreach ($options['error'] as $error) {
      drush_log($error, 'error');
    }
    return 0;
  }

  drush_print(dt('Going to @operate a search & replace on all text fields in all bundles and all custom blocks on @site',
    array(
      '@site' => variable_get('site_name', 'Drupal'),
      '@operate' => empty($options['dryrun']) ? 'perform' : 'fake'
    )
  ));
  drush_print(dt('  !search => !replace', array('!search' => $options['search'], '!replace' => $options['replace'])));
  drush_print(dt(' '));

  if (empty($options['dryrun'])) {
    $continue = drush_confirm(dt('There is no undo. Have you just created a database backup?'));
    if (empty($continue)) {
      drush_log(dt('Aborting'), 'warning');
      return 0;
    }

    $continue = drush_confirm(dt('There really is no undo. Are you sure this is what you want to do?'));
    if (empty($continue)) {
      drush_log(dt('Aborting'), 'warning');
      return 0;
    }
  }
  else {
    drush_log(dt('Doing a dry run'), 'warning');
  }

  switch (drush_drupal_major_version()) {
    case 7:
      return _drush_sar_replace_d7($options);

    case 6:
      return _drush_sar_replace_d6($options);

    default:
      drush_log(dt('This version of Drupal is not supported.'), 'error');
      return 0;
  }
}

/**
 * Command callback function for drush sarj.
 *
 * Search and replace all strings in JS Injector rules.
 */
function drush_sar_replace_js_injector() {
  // Parse our command line options.
  $options = drush_sar_parse_options();

  // If there's a problem yo I'll solve it...
  if (!empty($options['error'])) {
    foreach ($options['error'] as $error) {
      drush_log($error, 'error');
    }
    return 0;
  }

  return _drush_sar_replace_js_injector($options);

}

/**
 * Command callback.
 *
 * This callback checks the version of Drupal it's operating on and runs an
 * appropriate function to do the actual work. If an unsupported version is
 * detected, it will exit.
 */
function drush_sar_replace_link() {

  // Parse our command line options.
  $options = drush_sar_parse_options();

  // If there's a problem yo I'll solve it...
  if (!empty($options['error'])) {
    foreach ($options['error'] as $error) {
      drush_log($error, 'error');
    }
    return 0;
  }

  drush_print(dt('Going to @operate a search & replace on all link fields in all bundles and all custom blocks on @site', array('@site' => variable_get('site_name', 'Drupal'), '@operate' => empty($options['dryrun']) ? 'perform' : 'fake')));
  drush_print(dt('  !search => !replace', array('!search' => $options['search'], '!replace' => $options['replace'])));
  drush_print(dt(' '));

  return _drush_sar_replace_links($options);

}

/**
 * Command callback function for drush sarm.
 *
 * Search and replace all strings in menu items (title & link)
 */
function drush_sar_replace_menu() {
  // Parse our command line options.
  $options = drush_sar_parse_options();

  // If there's a problem yo I'll solve it...
  if (!empty($options['error'])) {
    foreach ($options['error'] as $error) {
      drush_log($error, 'error');
    }
    return 0;
  }

  return _drush_sar_replace_menus($options);

}

/**
 * Command callback function for drush sarv.
 *
 * Search and replace all strings in Views.
 */
function drush_sar_replace_views() {
  // Parse our command line options.
  $options = drush_sar_parse_options();

  // If there's a problem yo I'll solve it...
  if (!empty($options['error'])) {
    foreach ($options['error'] as $error) {
      drush_log($error, 'error');
    }
    return 0;
  }

  return _drush_sar_replace_views($options);

}

/**
 * Command callback function for drush sarc.
 *
 * Search and replace all strings in Contexts.
 */
function drush_sar_replace_context() {
  // Parse our command line options.
  $options = drush_sar_parse_options();

  // If there's a problem yo I'll solve it...
  if (!empty($options['error'])) {
    foreach ($options['error'] as $error) {
      drush_log($error, 'error');
    }
    return 0;
  }

  return _drush_sar_replace_context($options);

}

/**
 * Parse CLI args and options and return as an array.
 */
function drush_sar_parse_options() {
  $error = array();

  // Check if there are specific bundles (or node types in D6) to process.
  $bundles = drush_get_option('bundles');
  if (!empty($bundles)) {
    $bundles = array_filter(explode(',', $bundles));
  }

  // Check if there are specific menus to process.
  $menus = drush_get_option('menus');
  if (!empty($menus)) {
    $menus = array_filter(explode(',', $menus));
  }

  // Check if there are specific fields to process.
  $fields = drush_get_option('fields');
  if (!empty($fields)) {
    $fields = array_filter(explode(',', $fields));
  }

  // Check if we're processing blocks.
  $blocks = drush_get_option('blocks', 0);

  // Doing a dry run?
  $dryrun = drush_get_option('dry-run', 0);

  // Use regex?
  $regex = drush_get_option('regex', 0);

  // Showing IDs?
  $showid = drush_get_option('show-ids', 0);
  if ($showid) {
    $dryrun = 1;
  }

  // The other CLI args.
  $args = drush_get_arguments();
  if (count($args) < 2) {
    $error[] = dt('You need to specify both a string to search and one to replace.');
  }

  // Check some sanities. Refuse to do bundles/fields and blocks.
  if (!empty($blocks) && (!empty($bundles) || !empty($fields))) {
    $error[] = dt('Cannot process blocks when bundles or fields are specified.');
  }

  if (empty($args[1])) {
    $error[] = dt('Must provide a search value.');
  }

  $replace = $args[2];
  if (empty($args[2])) {
    $replace = "";
  }

  // Return an array of options.
  return array(
    'search'  => $args[1],
    'replace' => $replace,
    'bundles' => $bundles,
    'fields'  => $fields,
    'blocks'  => $blocks,
    'dryrun'  => $dryrun,
    'showid'  => $showid,
    'error' => $error,
    'menus' => $menus,
    'regex' => $regex,
  );
}

/**
 * Private function to do a DB REPLACE query on the `js_injector_rule` table.
 *
 */
function _drush_sar_replace_js_injector($options) {
  if (!db_table_exists('js_injector_rule')) {
    drush_log("The 'js_injector_rule' table does not exist. Took no action.", "warning");
    return;
  }
  // views_display.
  $query = db_update("js_injector_rule")
  ->expression("js", "REPLACE(js, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
  if ($options['regex']) {
    $query->condition("js", $options['regex'], "REGEXP");
  }
  $result_data = $query->execute();

  drush_log("Replaced \"" . $options['search'] . "\"  with \"" . $options['replace'] . "\" in all JS Injector rules.", "ok");
}

/**
 * [_drush_sar_replace_links description]
 *
 * @param  [type] $options
 * [description]
 *
 * @return mixed
 * [description]
 */
function _drush_sar_replace_links($options) {

  // Grab all defined fields, then cycle through them and run update queries.
  $fields = field_info_fields();

  // Only linkn fields.
  $link_fields = array_filter($fields, "is_link_field");

  // If fields are explicit then filter out to just those.
  if (!empty($options['fields'])) {
    $link_fields = array_filter($link_fields, "is_field_option");
  }

  if (empty($link_fields)) {
    drush_log("No fields to process", "error");
    return;
  }

  foreach ($link_fields as $field_info) {
    $field_name = $field_info["field_name"];
    $columns = array_keys($field_info['columns']);
    $data_table = _field_sql_storage_tablename($field_info);
    $revision_table = _field_sql_storage_revision_tablename($field_info);

    foreach ($columns as $column_key) {
      $column = $field_name . "_" . $column_key;

      // THE MEAT -
      // -----------------------------------------------------------------------
      $query = db_update($data_table)
      ->expression($column, "REPLACE(${column}, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
      if ($options['regex']) {
        $query->condition($column, $options['regex'], "REGEXP");
      }
      $result_data = $query->execute();

      $query = db_update($revision_table)
      ->expression($column, "REPLACE(${column}, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
      if ($options['regex']) {
        $query->condition($column, $options['regex'], "REGEXP");
      }
      $result_revision = $query->execute();
      // -----------------------------------------------------------------------
    }
  }
}

/**
 * [_drush_sar_replace_menu description]
 * @param  array $options [description]
 * @return [type]          [description]
 */
function _drush_sar_replace_menus($options) {

  $menus = menu_load_all();
  $menu_names = array_keys($menus);
  if (isset($options['menus'])) {
    $menu_names = array_intersect($menu_names, $options['menus']);
  }

  // No valid menus found.
  if (empty($menu_names)) {
    drush_set_error('NO_MENUS', dt('No menus found by the provided options.'));
    return;
  }

  foreach ($menu_names as $menu_name) {
    // Link Path.
    $query = db_update("menu_links")
      ->condition("menu_name", $menu_name)
      ->expression("link_path", "REPLACE(link_path, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
    if ($options['regex']) {
      $query->condition("link_path", $options['regex'], "REGEXP");
    }
    $result_data = $query->execute();

    // Router Path.
    $query = db_update("menu_links")
      ->condition("menu_name", $menu_name)
      ->expression("router_path", "REPLACE(router_path, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
    if ($options['regex']) {
      $query->condition("router_path", $options['regex'], "REGEXP");
    }

    $result_data = $query->execute();

    // Link Title.
    $query = db_update("menu_links")
      ->condition("menu_name", $menu_name)
      ->expression("link_title", "REPLACE(link_title, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
    if ($options['regex']) {
      $query->condition("link_title", $options['regex'], "REGEXP");
    }
    $result_data = $query->execute();
  }

  echo "Replaced \"" . $options['search'] . "\"  with \"" . $options['replace'] . "\" in: " . implode($menu_names, ", ");
}

/**
 * Private function to do a DB REPLACE query on the `views_display` table.
 *
 */
function _drush_sar_replace_views($options) {
  if (!db_table_exists('views_display')) {
    drush_log("The 'views_display' table does not exist. Took no action.", "warning");
    return;
  }
  $changed = FALSE;
  // Get our views_display.
  $result = db_query('SELECT vid,id,display_options FROM {views_display} vd');
  foreach ($result as $record) {
    // Unserialize the `display_options` column to get it into an array.
    $display_options = unserialize($record->display_options);
    // Track the original.
    $display_options_orig = $display_options;
    // Format our search and replace strings for preg_replace(). Use preg_quote() to escape forward slashes like sites.stanford.edu/foo.
    $search = $options['search'];
    $replace = $options['replace'];

    if ($options['regex']) {
      $search = preg_quote($options['search'], "/");
      $search = '/' . $search . '/';

      if (isset($display_options['footer']['area']['content'])) {
        $display_options['footer']['area']['content'] = preg_replace($search, $replace, $display_options['footer']['area']['content']);
      }
      if (isset($display_options['header']['area']['content'])) {
        $display_options['header']['area']['content'] = preg_replace($search, $replace, $display_options['header']['area']['content']);
      }
      if (isset($display_options['footer']['area_text_custom'])) {
        $display_options['footer']['area_text_custom']['content'] = preg_replace($search, $replace, $display_options['footer']['area_text_custom']['content']);
      }
      if (isset($display_options['header']['area_text_custom']['content'])) {
        $display_options['header']['area_text_custom']['content'] = preg_replace($search, $replace, $display_options['header']['area_text_custom']['content']);
      }

      if (isset($display_options['path'])) {
        $display_options['path'] = preg_replace($search, $replace, $display_options['path']);
      }
    }
    else {
      if (isset($display_options['footer']['area']['content'])) {
        $display_options['footer']['area']['content'] = str_replace($search, $replace, $display_options['footer']['area']['content']);
      }
      if (isset($display_options['header']['area']['content'])) {
        $display_options['header']['area']['content'] = str_replace($search, $replace, $display_options['header']['area']['content']);
      }
      if (isset($display_options['footer']['area_text_custom']['content'])) {
        $display_options['footer']['area_text_custom']['content'] = str_replace($search, $replace, $display_options['footer']['area_text_custom']['content']);
      }
      if (isset($display_options['header']['area_text_custom']['content'])) {
        $display_options['header']['area_text_custom']['content'] = str_replace($search, $replace, $display_options['header']['area_text_custom']['content']);
      }

      if (isset($display_options['path'])) {
        $display_options['path'] = str_replace($search, $replace, $display_options['path']);
      }
    }

    // Only run the DB query if we have changed the options.
    if ($display_options !== $display_options_orig) {
      $display_options = serialize($display_options);
      // The primary keys for the views_display table are vid, id (e.g., 13,default).
      $vid = $record->vid;
      $id = $record->id;
      // "UPDATE views_display SET display_options = $display_options WHERE vid = $vid and id = $id".
      $num_updated = db_update('views_display')
        ->fields(array('display_options' => $display_options))
        ->condition('vid', $vid)
        ->condition('id', $id)
        ->execute();
      $message = t("The display options for vid=@vid and id=@id have been updated.", array('@vid' => $vid, '@id' => $id));
      drush_log($message, "ok");
      $changed = TRUE;
    }
  }
  if ($changed == TRUE) {
    drush_log("Replaced \"" . $options['search'] . "\"  with \"" . $options['replace'] . "\" in applicable Views.", "ok");
  }
  else {
    drush_log("sarv could not find the string \"" . $options['search'] . "\" to replace with \"" . $options['replace'] . "\".", "error");
  }
}

/**
 * Private function to do a DB REPLACE query on the `context` table.
 */
function _drush_sar_replace_context($options) {

  if (!db_table_exists('context')) {
    drush_log("The 'context' table does not exist. Took no action.", "warning");
    return;
  }

  $contexts = context_load();
  foreach ($contexts as $context) {

    if ($options['regex']) {
      _drush_sar_replace_context_regex($options, $context);
    }
    else {
      _drush_sar_replace_context_string($options, $context);
    }
  }

}

/**
 * [_drush_sar_replace_context_regex description]
 * @param  [type] $options [description]
 * @param  [type] $context [description]
 * @return [type]          [description]
 */
function _drush_sar_replace_context_regex($options, $context) {
  $search = '/' . $options['search'] . '/';
  $replace = $options['replace'];

  $context->name = preg_replace($search, $replace, $context->name);
  $context->description = preg_replace($search, $replace, $context->description);
  $context->tag = preg_replace($search, $replace, $context->tag);
  $conditions = array();
  foreach ($context->conditions as $k => $condition) {
    if (isset($condition['values'])) {
      foreach ($condition['values'] as $con_k => $con_v) {
        unset($context->conditions[$k][$con_k]);
        $new_k = preg_replace($search, $replace, $con_k);
        $new_v = preg_replace($search, $replace, $con_v);
        $conditions[$k]['values'][$new_k] = $new_v;
      }
    }
    else {
      $conditions[$k] = $condition;
    }
  }

  // Re set the conditions back.
  $context->conditions = $conditions;

  // Save the changes.
  $is_good = context_save($context);

  // If the machine name changed delete old one.
  if ($original_name !== $context->name && $is_good) {
    $context = context_load($original_name);
    context_delete($context);
  }

}

/**
 * [_drush_sar_replace_context_string description]
 * @param  [type] $options [description]
 * @param  [type] $context [description]
 * @return [type]          [description]
 */
function _drush_sar_replace_context_string($options, $context) {
  $search = $options['search'];
  $replace = $options['replace'];
  $original_name = $context->name;

  $context->name = str_replace($search, $replace, $context->name);
  $context->description = str_replace($search, $replace, $context->description);
  $context->tag = str_replace($search, $replace, $context->tag);
  $conditions = array();
  foreach ($context->conditions as $k => $condition) {
    if (isset($condition['values'])) {
      foreach ($condition['values'] as $con_k => $con_v) {
        unset($context->conditions[$k][$con_k]);
        $new_k = str_replace($search, $replace, $con_k);
        $new_v = str_replace($search, $replace, $con_v);
        $conditions[$k]['values'][$new_k] = $new_v;
      }
    }
    else {
      $conditions[$k] = $condition;
    }
  }

  // Re set the conditions back.
  $context->conditions = $conditions;

  // Save the changes.
  $is_good = context_save($context);

  // If the machine name changed delete old one.
  if ($original_name !== $context->name && $is_good) {
    $context = context_load($original_name);
    context_delete($context);
  }

}



/**
 * [_is_link_field description]
 * @param  [type]  $var [description]
 * @return boolean      [description]
 */
function is_link_field($var) {
  return ($var['module'] === "link");
}

/**
 * [is_field_option description]
 * @param  [type]  $var [description]
 * @return boolean      [description]
 */
function is_field_option($var) {
  $fields = drush_get_option('fields');
  if (!empty($fields)) {
    $fields = array_filter(explode(',', $fields));
  }

  return in_array($var['field_name'], $fields);
}

/**
 * Does the search and replace for Drupal 7 sites.
 */
function _drush_sar_replace_d7($options) {
  if (empty($options['blocks'])) {

    // Grab all defined fields, then cycle through them and run update queries.
    $fields = field_info_fields();
    foreach ($fields as $field) {

      // Skip fields that aren't provided by the text module.
      if ($field['module'] != 'text') {
        continue;
      }

      // Check if we've specified fields. If so, only process those.
      if (!empty($options['fields']) && !in_array($field['field_name'], $options['fields'])) {
        continue;
      }

      $bundles = array();
      foreach ($field['bundles'] as $entity => $bundle) {
        $bundles += array_values($bundle);
      }

      // If we specified bundles, only process for these bundles. If this field
      // does not exist on the bundles specified, just skip to the next field.
      if (!empty($options['bundles'])) {
        $bundles = array_intersect($options['bundles'], $bundles);
      }

      if (empty($bundles)) {
        continue;
      }

      // At this stage, $bundles contains either the full list of bundles for
      // this field or the subset we need to operate on, so the update query
      // condition is always OK.

      // Use (private, naughty) API calls to grab the table and column names.
      $data_table = _field_sql_storage_tablename($field);
      $revision_table = _field_sql_storage_revision_tablename($field);
      $column = _field_sql_storage_columnname($field['field_name'], 'value');
      if ($field['type'] == 'text_with_summary') {
        $summary_column = _field_sql_storage_columnname($field['field_name'], 'summary');
      }

      drush_log(dt('Processing field @name (@bundles)', array('@name' => $field['field_name'], '@bundles' => implode(', ', $bundles))), 'ok');

      // Update data in FIELD_LOAD_CURRENT.
      if (empty($options['dryrun'])) {
        $query = db_update($data_table)
          ->expression($column, "REPLACE(${column}, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
        // Replace values in summary columns as well.
        if ($field['type'] == 'text_with_summary') {
          $query->expression($summary_column, "REPLACE(${summary_column}, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
        }
        $query->condition('bundle', $bundles, 'IN');
        $updated = $query->execute();
      }
      else {
        $or = db_or();
        $or->condition($column, '%' . db_like($options['search']) . '%', 'LIKE');
        if ($field['type'] == 'text_with_summary') {
          $or->condition($summary_column, '%' . db_like($options['search']) . '%', 'LIKE');
        }
        if (empty($options['showid'])) {
          $updated = db_select($data_table, 'd')->fields('d', array('entity_id'))->condition($or)->execute()->rowCount();
        }
        else {
          $updated = db_select($data_table, 'd')->fields('d', array('entity_type', 'entity_id'))->condition($or)->execute();
        }
      }

      if (!empty($options['showid'])) {
        foreach ($updated as $row) {
          drush_print(dt('@entity/@id', array('@entity' => $row->entity_type, '@id' => $row->entity_id)));
        }
      }
      else {
        drush_log(dt('  Updated @bundle.', array('@bundle' => format_plural($updated, '1 bundle', '@count bundles'))), 'ok');
      }

      // Update data in FIELD_LOAD_REVISION.
      if (empty($options['dryrun'])) {
        $query = db_update($revision_table)
          ->expression($column, "REPLACE(${column}, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
        // Replace values in summary columns as well.
        if ($field['type'] == 'text_with_summary') {
          $query->expression($summary_column, "REPLACE(${summary_column}, :search, :replace)", array(':search' => $options['search'], ':replace' => $options['replace']));
        }
        $query->condition('bundle', $bundles, 'IN');
        $updated = $query->execute();
      }
      else if (empty($options['showid'])) { // Only run showid on current revisions.
        $or = db_or();
        $or->condition($column, '%' . db_like($options['search']) . '%', 'LIKE');
        if ($field['type'] == 'text_with_summary') {
          $or->condition($summary_column, '%' . db_like($options['search']) . '%', 'LIKE');
        }
        $updated = db_select($revision_table, 'r')->fields('r', array('entity_id'))->condition($or)->execute()->rowCount();
      }

      drush_log(dt('  Updated @revision.', array('@revision' => format_plural($updated, '1 bundle revision', '@count bundle revisions'))), 'ok');
    }
  }

  // We process blocks if no fields were specified.
  if (empty($options['fields'])) {
    // Do the blocks too.
    if (empty($options['dryrun'])) {
      $updated = db_update('block_custom')
        ->expression('body', "REPLACE(body, :old, :new)", array(':old' => $options['search'], ':new' => $options['replace']))
        ->execute();
    }
    else {
      if (empty($options['showid'])) {
        $updated = db_select('block_custom', 'b')->fields('b', array('bid'))->condition('body', '%' . db_like($options['search']) . '%' , 'LIKE')->execute()->rowCount();
      }
      else {
        $updated = db_select('block_custom', 'b')->fields('b', array('bid'))->condition('body', '%' . db_like($options['search']) . '%' , 'LIKE')->execute();
      }
    }

    if (!empty($options['showid'])) {
      foreach ($updated as $row) {
        drush_print(dt('block/@bid', array('@bid' => $row->bid)));
      }
    }
    else {
      drush_log(dt('  Updated @block.', array('@block' => format_plural($updated, '1 custom block', '@count custom blocks'))), 'ok');
    }
  }

  return 1;
}

/**
 * Does the search and replace for Drupal 6 sites.
 */
function _drush_sar_replace_d6($options) {
  // Update all node revisions
  drush_log(dt('Processing node revisions'), 'ok');
  if (empty($options['dryrun'])) {
    $query = "UPDATE {node_revisions} r INNER JOIN {node n} ON (r.nid = n.nid) SET r.body = REPLACE(r.body, '%s', '%s'), r.teaser = REPLACE(r.teaser, '%s', '%s')";
    $args  = array($options['search'], $options['replace'], $options['search'], $options['replace']);
  }
  else {
    $query = "SELECT COUNT(r.vid) FROM {node_revisions} r INNER JOIN {node n} ON (r.nid = n.nid) WHERE r.body LIKE '%%%s%%' OR r.teaser LIKE '%%%s%%'";
    $args  = array($options['search'], $options['search']);
  }

  // If we've limited the node types...
  if (!empty($options['bundles'])) {
    $query .= " AND n.type in (" . db_placeholders($options['bundles'], 'varchar') .")";
    $args   = array_merge($args, $options['bundles']);
  }

  $result = db_query($query, $args);
  if (empty($options['dryrun'])) {
    $count = db_affected_rows();
  }
  else {
    $count = db_result($result);
  }

  drush_log(dt('  Updated @revision.', array('@revision' => format_plural($count, '1 node revision', '@count node revisions'))), 'ok');

  if (empty($options['fields']) && empty($options['bundles'])) {
    // Do the blocks too.
    drush_log(dt('Processing custom blocks'), 'ok');
    if (empty($options['dryrun'])) {
      db_query("UPDATE {boxes} SET body = REPLACE(body, '%s', '%s')", $options['search'], $options['replace']);
      $count = db_affected_rows();
    }
    else {
      $count = db_result(db_query("SELECT COUNT(bid) FROM {boxes} WHERE body LIKE '%%%s%%'", $options['search']));
    }
    drush_log(dt('  Updated @block.', array('@block' => format_plural($count, '1 custom block', '@count custom bocks'))), 'ok');
  }

  // Check if CCK is present and perform the replace on text fields (assuming we didn't just want blocks).
  if (module_exists('content') && empty($options['blocks'])) {
    $types = content_types();

    foreach ($types as $type) {
      // If we specified bundles and the current type doesn't match, just skip.
      if (!empty($options['bundles']) && !in_array($type['type'], $options['bundles'])) {
        continue;
      }

      foreach ($type['fields'] as $field) {
        if ($field['module'] != 'text' || !$field['db_storage']) {
          continue;
        }

        // Check if we've specified fields. If so, only process those.
        if (!empty($options['fields']) && !in_array($field['field_name'], $options['fields'])) {
          continue;
        }

        $info = content_database_info($field);
        $revision_table = $info['table'];
        $column = isset($info['columns']['value']['column']) ? $info['columns']['value']['column'] : NULL;

        if (!empty($column)) {
          // Update data in the field revision table.
          drush_log(dt('Processing field @name in @type', array('@name' => $field['field_name'], '@type' => $type['name'])), 'ok');

          if (empty($options['dryrun'])) {
            $query = "UPDATE {${revision_table}} r";
            if (!empty($options['bundles'])) {
              // If we need to the node type...
              $query .= " INNER JOIN {node} n ON (r.nid = n.nid)";
            }
            $query .= " SET r.${column} = REPLACE(r.${column}, '%s', '%s')";
            $args = array($options['search'], $options['replace']);
          }
          else {
            $query = "SELECT COUNT(r.vid) FROM {${revision_table}} r";
            if (!empty($options['bundles'])) {
              // If we need to the node type...
              $query .= " INNER JOIN {node} n ON (r.nid = n.nid)";
            }
            $args = array();
          }

          if (!empty($options['bundles'])) {
            // Limit the current query to the current node type, which is $type['type'].
            $query .= " WHERE n.type = '%s'";
            $args[] = $type['type'];
          }

          if (empty($options['dryrun'])) {
            db_query($query, $args);
            $count = db_affected_rows();
          }
          else {
            $count = db_result(db_query($query, $args));
          }
          drush_log(dt('  Updated @revision.', array('@revision' => format_plural($count, '1 field revision', '@count field revisions'))), 'ok');
        }
      }
    }
  }

  return 1;
}
