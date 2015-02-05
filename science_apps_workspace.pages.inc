<?php

function science_apps_workspace_page($wid = 0) {
  global $user;

  if (! $user->uid) {
    // anonymous users cannot use science apps
    $message = t('You need to be !logged_in to access Science Apps.', array('!logged_in' => l(t('logged in'), 'user/login', array('query' => array('destination' => 'workspace')))));
    return '<div class="jumbotron"><p>'.$message.'</p></div>';
  }

  if ($wid) {
    if ($wid == 'default') {
      $workspace = science_apps_workspace_default_workspace();
    } else {
      $workspace = Workspace::get($wid);
      if ($workspace) {
        if (! science_apps_workspace_workspace_access($workspace)) {
          return drupal_access_denied();
        }
      } else {
        return drupal_not_found();
      }
    }
  } else {
    $workspace = Workspace::getDefault($user);
  }

  if (! $workspace) {
    drupal_set_message(t('You are currently using the Default workspace. !create_link', array('!create_link' => l(t('Customize your own workspace.'), 'workspace/new', array('query' => array('destination' => 'workspace'))))), 'info');
    $workspace = science_apps_workspace_default_workspace();
  }

  $config = $workspace->getConfigObject();

  $rows = array();
  foreach ($config->rows as $row) {
    $rows[] = science_apps_workspace_render_row($row);
  }

  return '<div class="science-apps-workspace">'
    . science_apps_workspace_page_header($workspace)
    . '<div class="workspace">'
    . implode('', $rows)
    . '</div></div>';
}

function science_apps_workspace_render_row($row) {
  $output = '<div class="row">';

  // for now, this will conform to bootstrap grid. later make it more flexible
  $columnClass = 'col-md-' . 12 / count($row->columns);

  foreach ($row->columns as $column) {
    $output .= science_apps_workspace_render_column($column, $columnClass);
  }

  $output .= '</div>';
  return $output;
}

function science_apps_workspace_render_column($column, $columnClass) {
  $output = '<div class="'. $columnClass .'">';

  if ($column->type == 'rows') {
    foreach ($column->rows as $row) {
      $output .= science_apps_workspace_render_row($row);
    }
  } else { // $column->type == 'app'
    $output .= science_apps_workspace_render_app($column->app);
  }

  $output .= '</div>';
  return $output;
}

function science_apps_workspace_render_app($app) {
  $output = '<div class="app">';
  if ($app) {
    $app_node = node_load($app);
    if ($app_node) {
      $output .= theme('science_app_display', array('node' => $app_node));
    } else {
      $output .= theme('science_app_removed', array('node' => $app_node));
    }
  } else {
    $output .= theme('science_app_placeholder', array('anonymous' => user_is_anonymous()));
  }
  $output .= '</div>';
  return $output;
}

function science_apps_workspace_page_header($workspace) {
  $workspace_name = $workspace->name;

  if ($workspace->wid && is_numeric($workspace->wid) && user_access('manage workspaces')) {
    $url = $workspace->wid ? 'workspace/edit/' . $workspace->wid : 'workspace/new';
    $edit_workspace_link = l('<i class="fa fa-th"></i> ' . t('Edit workspace'), $url, array('html' => TRUE, 'attributes' => array('class' => 'btn btn-default navbar-btn')));
  }

  $items = array();
  if (user_access('manage workspaces')) {
    if ($workspace->wid && is_numeric($workspace->wid)) {
      $items[] = l(t('Edit this Workspace'), 'workspace/edit/' . $workspace->wid);
    }
    $items[] = l(t('Manage Workspaces'), 'workspace/configure');
  }
  if (user_access('create science_app content')) {
    $items[] = array('data' => '', 'class' => array('divider'));
    $items[] = l(t('Create New App'), 'node/add/science-app');
  }
  $workspace_settings_links = theme('item_list', array('items' => $items, 'attributes' => array('class' => 'dropdown-menu', 'role' => 'menu')));

  /* workspace switcher */
  global $user;
  $workspaces = Workspace::getList($user);
  if (count($workspaces)) {
    $items = array();
    $items[] = array('class' => array('dropdown-header'), 'data' => 'My Workspaces');
    foreach ($workspaces as $ws) {
      $name = $ws->isDefault() ? $ws->name . ' (default)' : $ws->name;
      $items[] = array(
        'data' => l($name, 'workspace/' . $ws->wid),
        'class' => array($workspace->wid == $ws->wid ? 'active' : '')
      );
    }
    $items[] = array('class' => array('dropdown-header'), 'data' => 'Site Workspaces');
    $items[] = array(
      'data' => l(t('Default Workspace'), 'workspace/default'),
      'class' => array($workspace->wid == 'default' ? 'active' : '')
    );
    $workspace_switcher_links = theme('item_list', array('items' => $items, 'attributes' => array('class' => 'dropdown-menu', 'role' => 'menu')));
  }

  return theme('science_apps_workspace_header', compact('workspace_name', 'edit_workspace_link', 'workspace_settings_links', 'workspace_switcher_links'));
}

function science_apps_workspace_config($wid = null) {
  global $user;

  $output = '';

  if ($wid) {
    return $wid;
  } else {
    $workspaces = Workspace::getList($user);

    if (count($workspaces)) {
      $items = array();
      foreach ($workspaces as $ws) {
        $item = '<h4>' . $ws->name . ($ws->status ? ' <small><i class="fa fa-check-square-o"></i> Default Workspace</small>' : '' ) . '</h4>';

        $app_nids = array();
        $config = $ws->getConfigObject();
        foreach ($config->rows as $row) {
          foreach ($row->columns as $col) {
            $app_nids[] = $col->app;
          }
        }
        $app_titles = array_map(function($node) { return l($node->title, 'node/'.$node->nid); }, node_load_multiple($app_nids));
        $apps_list = implode(', ', $app_titles);
        $item .= '<p><b>Apps:</b> '.$apps_list.'</p>';

        $item .= '<p>';
        $item .= l(t('Edit this workspace'), 'workspace/edit/' . $ws->wid, array('attributes' => array('class' => 'btn btn-default')));
        $item .= ' ';
        $item .= l(t('Make this workspace default'), 'workspace/make-default/' . $ws->wid, array('attributes' => array('class' => 'btn btn-default')));
        $item .= '</p>';
        $items[] = array(
          'data' => $item,
          'class' => array('list-group-item'),
        );
      }

      $output .= theme('item_list', array('title' => t('Your Workspaces'), 'items' => $items, 'attributes' => array('class' => 'list-group')));
    } else {
      $output .= '<div class="jumbotron"><h1>You haven\'t configured any workspaces yet!</h1></div>';
    }

    $output .= l(t('Create a New Workspace'), 'workspace/new', array('attributes' => array('class' => 'btn btn-lg btn-primary')));
    $output .= ' ';
    $output .= l(t('Done'), 'workspace', array('attributes' => array('class' => 'btn btn-lg btn-default')));
  }

  return $output;
}

function science_apps_workspace_science_app_info($node) {
  $author = user_load($node->uid);
  $repo = $node->science_app_repository['und'][0]['value'];
  $version = $node->science_app_version ? $node->science_app_version['und'][0]['value'] : 'master';
  $published = $node->status;
  return theme('science_app_information', compact('node', 'author', 'repo', 'version', 'published'));
}

function science_apps_workspace_science_app_info_logs($node) {
  $result_set = db_select('science_apps_workspace_app_log', 'log')
  ->fields('log')
  ->condition('log.nid', $node->nid)
  ->execute();

  $logs = array();
  while ($record = $result_set->fetchAssoc()) {
    $vars = unserialize($record['variables']);
    $logs[] = array(
      'timestamp' => $record['timestamp'],
      'message' => t($record['message'], $vars),
    );
  }

  $clear_logs_link = l(t('Clear App Logs'), "node/$node->nid/science-app/logs/clear", array('attributes' => array('class' => 'btn btn-danger')));

  return theme('science_app_logs', compact('node', 'logs', 'clear_logs_link'));
}

function science_apps_workspace_science_app_info_logs_clear($node) {
  db_delete('science_apps_workspace_app_log')
  ->condition('nid', $node->nid)
  ->execute();
  drupal_set_message('App Logs Cleared!');
  drupal_goto("node/$node->nid/science-app");
}
