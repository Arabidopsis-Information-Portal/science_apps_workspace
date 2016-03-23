<?php

function science_apps_workspace_drush_command() {

  $items['check-ready-state'] = array(
    'description' => 'Audits apps in the workspace to ensure that they are fully configured.',
    'aliases' => array('sciapps-check', 'saw-chk'),
  );

  $items['list-unprocessed'] = array(
    'description' => 'List science apps that have not yet been processed.',
    'aliases' => array('sciapps-un', 'saw-un'),
    'options' => array(
      'limit' => 'Limit the length of the list',
    )
  );

  $items['apps-process'] = array(
    'description' => 'Process science apps that have not yet been processed.',
    'aliases' => array('sciapps-apps-proc', 'saw-apps-proc'),
    'options' => array(
      'nid' => 'The NID of a single app to process.',
      'limit' => 'Limit the number of apps to process at a time. Defaults to 10. Ignored if --nid passed',
    )
  );
  return $items;
}

function drush_science_apps_workspace_check_ready_state() {
  $records = db_select('science_apps_workspace_app_config', 'app')
    ->fields('app', array('nid'))
    ->execute();

  while ($r = $records->fetchAssoc()) {
    $nid = $r['nid'];
    //if (_science_apps_workspace_app_repo_ready($nid) && _science_apps_workspace_app_dependencies_ready($nid)) {
    if (_science_apps_workspace_app_repo_ready($nid) && _science_apps_workspace_app_check_dependencies_ready($nid)) {
      print "App nid=$nid READY\n";
      science_apps_workspace_set_app_config_ready($nid, SCIENCE_APP_STATE_READY);
    } else {
      science_apps_workspace_set_app_config_ready($nid, SCIENCE_APP_STATE_NOT_READY);
      print "App nid=$nid NOT READY\n";
    }
  }
}

function drush_science_apps_workspace_list_unprocessed() {
  // not ready apps
  $query = db_select('science_apps_workspace_app_config', 'app')->fields('app', array('nid'));
  if ($limit = drush_get_option('limit', FALSE)) {
    $query->range(0, $limit);
  }
  $records = $query->condition('ready', SCIENCE_APP_STATE_NOT_READY)->execute()->fetchAll();
  if ($records) {
    print "The following Science Apps are NOT READY:\n\n";
    print "NID\t\tTitle\n======\t\t=========================\n";
    foreach ($records as $r) {
      $node = node_load($r->nid);
      $title = substr($node->title, 0, 25);
      print "$r->nid\t\t$title\n";
    }
    print "\n";
  }

  // error state apps
  $query = db_select('science_apps_workspace_app_config', 'app')->fields('app', array('nid'));
  if ($limit = drush_get_option('limit', FALSE)) {
    $query->range(0, $limit);
  }
  $records = $query->condition('ready', SCIENCE_APP_STATE_ERROR)->execute()->fetchAll();
  if ($records) {
    print "The following Science Apps are ERROR STATE:\n\n";
    print "NID\t\tTitle\n======\t\t=========================\n";
    foreach ($records as $r) {
      $node = node_load($r->nid);
      $title = substr($node->title, 0, 25);
      print "$r->nid\t\t$title\n";
    }
  }


  // delete state apps
  $query = db_select('science_apps_workspace_app_config', 'app')->fields('app', array('nid'));
  if ($limit = drush_get_option('limit', FALSE)) {
    $query->range(0, $limit);
  }
  $records = $query->condition('ready', SCIENCE_APP_STATE_DELETED)->execute()->fetchAll();
  if ($records) {
    print "The following Science Apps are DELETE STATE:\n\n";
    print "NID\t\tTitle\n======\t\t=========================\n";
    foreach ($records as $r) {
      $node = node_load($r->nid);
      $title = substr($node->title, 0, 25);
      print "$r->nid\t\t$title\n";
    }
  }
}

function drush_science_apps_workspace_apps_process() {
  $app_nids = array();

  if ($nid = drush_get_option('nid', FALSE)) {
    $app_nids[] = $nid;
  } else {
    $limit = drush_get_option('limit', 10);
    $records = db_select('science_apps_workspace_app_config', 'app')
      ->fields('app', array('nid'))
      ->condition('ready', '0')
      ->range(0, $limit)
      ->execute()
      ->fetchAll();
    foreach ($records as $r) {
      $app_nids[] = $r->nid;
    }
    $del_records = db_select('science_apps_workspace_app_config', 'app')
      ->fields('app', array('nid'))
      ->condition('ready', '3')
      ->range(0, $limit)
      ->execute()
      ->fetchAll();
    foreach ($del_records as $r){
        $del_nids[] = $r->nid;
    }
  }

  if (count($app_nids)) {
    foreach ($app_nids as $nid) {
      print "Processing Science App nid=$nid...\n";
      _science_apps_workspace_prepare_app($nid);
    }
  } elseif (count($del_nids)){
    foreach ($del_nids as $nid){
        print "Deleting Science App nid=$nid...\n";
        _science_apps_workspace_app_clean_repo($nid);
        db_delete('science_apps_workspace_app_config')
          ->condition('nid', $node->nid)
          ->execute();
    }
  }else {
    print "No apps to process!\n";
  }
}
