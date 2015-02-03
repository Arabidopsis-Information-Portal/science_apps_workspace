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
      $output .= science_apps_workspace_render_missing_app();
    }
  } else {
    $output .= science_apps_workspace_render_placeholder();
  }
  $output .= '</div>';
  return $output;
}

function science_apps_workspace_render_missing_app() {
  return '<div class="alert alert-warning"><h4>This app has been removed.</h4><p>This application is no longer available on the portal. Please update your workspace configuration.</p></div>';
}

function science_apps_workspace_render_placeholder() {
  global $user;
  $placeholder = '<div class="app-placeholder">';
  if ($user->uid) {
    $placeholder .= '<h1>Choose an app!</h1><p>Click the Workspace Settings button <button disabled type="button" class="btn btn-default"><i class="fa fa-cog"></i></button> at the top of the page to configure your workspace.</p>';
  } else {
    $placeholder .= '<h1>Log in to customize your workspace!</h1><p>Araport users can create customized workspaces tailored to their workflows. <a href="/user/login">Log in</a> or <a href="/user/register">create an account</a> now!</p>';
  }
  $placeholder .= '</div>';
  return $placeholder;
}

function science_apps_workspace_page_header($workspace) {
  $output = '<header><nav class="navbar navbar-default"><div class="container-fluid"><a class="navbar-brand" href="#">' . $workspace->name . '</a>';

  /* edit current workspace */
  if ($workspace->wid && is_numeric($workspace->wid) && user_access('manage workspaces')) {
    $url = $workspace->wid ? 'workspace/edit/' . $workspace->wid : 'workspace/new';
    $output .= l('<i class="fa fa-th"></i> ' . t('Edit workspace'), $url, array('html' => TRUE, 'attributes' => array('class' => 'btn btn-default navbar-btn')));
  }

  $output .= '<div class="navbar-right">';

  /* workspace management */
  $output .= '<div class="btn-group">';
  $output .= '<button type="button" class="btn btn-default navbar-btn dropdown-toggle" data-toggle="dropdown"><i class="fa fa-cog"></i><span class="sr-only">'.t('Settings').'</span></button>';
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
  $output .= theme('item_list', array('items' => $items, 'attributes' => array('class' => 'dropdown-menu', 'role' => 'menu')));
  $output .= '</div> ';

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

    $output .= '<div class="btn-group">';
    $output .= '<button type="button" class="btn btn-default navbar-btn dropdown-toggle" data-toggle="dropdown"><i class="fa fa-bars"></i><span class="sr-only">'.t('Change Workspace').'</span></button>';
    $output .= theme('item_list', array('items' => $items, 'attributes' => array('class' => 'dropdown-menu', 'role' => 'menu')));
    $output .= '</div> ';
  }

  /* close up navbar-right */
  $output .= '</div>';

  /* close up header*/
  $output .= '</div></nav></header>';

  return $output;
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
