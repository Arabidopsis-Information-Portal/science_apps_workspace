<?php

function science_apps_workspace_page($wid = 0) {
  global $user;

  if ($wid) {
    $workspace = Workspace::get($wid);
    if ($workspace) {
      if (! science_apps_workspace_workspace_access($workspace)) {
        return drupal_access_denied();
      }
    } else {
      return drupal_not_found();
    }
  } else {
    $workspace = Workspace::getDefault($user);
  }

  if (! $workspace) {
    if ($user->uid) {
      drupal_set_message(t('You are currently using the Default workspace. !create_link', array('!create_link' => l(t('Customize your own workspace.'), 'workspace/new', array('query' => array('destination' => 'workspace'))))), 'info');
    } else {
      drupal_set_message(t('!login_link to customize your Workspace.', array('!login_link' => l(t('Log in'), 'user/login', array('query' => array('destination' => 'workspace'))))), 'info');
    }
    $workspace = new Workspace();
  }

  $config = json_decode($workspace->config);
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
    // TODO: do render
    $app_node = node_load($app);
    if ($app_node) {
      $output .= theme('science_app_display', array('node' => $app_node));
    } else {
      $output .= '<div class="alert alert-warning"><h4>This app has been removed.</h4><p>This application is no longer available on the portal. Please update your workspace configuration.</p></div>';
    }
  } else {
    $output .= '<div class="app-placeholder"><h1>Choose an app!</h1><p>Click the Workspace Settings button <button disabled type="button" class="btn btn-default"><i class="fa fa-cog"></i></button> at the top of the page to configure your workspace.</p></div>';
  }
  $output .= '</div>';
  return $output;
}

function science_apps_workspace_page_header($workspace) {
  $name = empty($workspace->name) ? variable_get('science_apps_workspace_default_name', t('Default')) : $workspace->name;
  $output = '<header><nav class="navbar navbar-default"><div class="container-fluid"><a class="navbar-brand" href="#">' . $name . '</a>';

  /* edit current workspace */
  if (user_access('manage workspaces')) {
    $url = $workspace->wid ? 'workspace/edit/' . $workspace->wid : 'workspace/new';
    $output .= l('<i class="fa fa-th"></i> ' . t('Edit workspace'), $url, array('html' => TRUE, 'attributes' => array('class' => 'btn btn-default navbar-btn')));
  }

  $output .= '<div class="navbar-right">';

  /* workspace management */
  $output .= '<div class="btn-group">';
  $output .= '<button type="button" class="btn btn-default navbar-btn dropdown-toggle" data-toggle="dropdown"><i class="fa fa-cog"></i><span class="sr-only">'.t('Settings').'</span></button>';
  $items = array();
  if (user_access('manage workspaces')) {
    if ($workspace->wid) {
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
    foreach ($workspaces as $ws) {
      if ($ws->wid == $workspace->wid) {
        $items[] = array(
          'data' => l($ws->name, 'workspace/' . $ws->wid),
          'class' => array('active')
        );
      } else {
        $items[] = l($ws->name, 'workspace/' . $ws->wid);
      }
    }

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

function science_apps_workspace_edit_form($form, &$form_state, $workspace = null) {
  global $user;

  $apps_catalog = science_apps_workspace_apps_catalog($user->uid);

  if (! isset($form_state['workspace_config'])) {
    if (! $workspace) {
      $workspace = new Workspace();
    }
    $form_state['workspace_config'] = $workspace->getConfigObject();
  }
  $config = $form_state['workspace_config'];

  $form['wid'] = array(
    '#type' => 'value',
    '#value' => $workspace->wid
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Workspace Name'),
    '#required' => TRUE,
    '#default_value' => $workspace->name,
    '#attributes' => array(
      'placeholder' => t('My Workspace'),
    ),
  );

  $form['config'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('Workspace Setup'),
  );

  $form['config']['instruct'] = array(
    '#type' => 'markup',
    '#markup' => t('Each Workspace is a collection of applications that are displayed together. Applications are arranged in rows and columns. Each row contains columns, and each column can contain an application. You choose the number of rows and columns and select the applications for your workspace.'),
    '#prefix' => '<div class="alert alert-info">',
    '#suffix' => '</div>',
  );

  $form['config']['rows'] = array(
    '#prefix' => '<div id="ws-rows" class="ws-rows">',
    '#suffix' => '</div>',
  );

  // foreach row...
  foreach ($config->rows as $row_i => $row) {

    $form['config']['rows'][$row_i] = array(
      '#prefix' => '<div class="ws-row">',
      '#suffix' => '</div>',
    );

    $form['config']['rows'][$row_i]['header'] = array(
      '#markup' => '<h4>Row ' . ($row_i + 1) . '</h4>'
    );

    $form['config']['rows'][$row_i]['add_column'] = array(
      '#type' => 'submit',
      '#name' => 'ws_row_'.$row_i.'_cols_add',
      '#value' => t('Add Column'),
      '#attributes' => array(
        'class' => array('btn-xs'),
        'title' => count($row->columns) == 4 ? t('At this time there is a maximum of four (4) columns') : ''
      ),
      '#limit_validation_errors' => array(),
      '#submit' => array('science_apps_workspace_add_column_submit'),
      '#ajax' => array(
        'callback' => 'science_apps_workspace_ajax_column_callback',
        'wrapper' => 'ws-row-'.$row_i.'-cols',
      ),
    );

    $form['config']['rows'][$row_i]['columns'] = array(
      '#prefix' => '<div class="ws-cols row" id="ws-row-'.$row_i.'-cols">',
      '#suffix' => '</div>',
    );

    // foreach column...
    foreach ($row->columns as $col_i => $col) {

      $columnClass = 'col-md-' . 12 / count($row->columns);

      $form['config']['rows'][$row_i]['columns'][$col_i] = array(
        '#prefix' => '<div class="ws-col '.$columnClass.'">',
        '#suffix' => '</div>',
      );

      $form['config']['rows'][$row_i]['columns'][$col_i]['type'] = array(
        '#type' => 'value',
        '#value' => 'app',
      );

      // TODO: for now, just support columns of type app

      // $form['config']['rows'][$row_i]['columns'][$col_i]['type'] = array(
      //   '#type' => 'select',
      //   '#title' => t('Column Type'),
      //   '#options' => array('rows' => t('Rows'), 'app' => t('App')),
      //   '#empty_option' => t('Choose column type'),
      //   '#default_value' => $col->type,
      // );

      // if ($col->type == 'row') {
      //   // foreach row...
      //
      // } else { // $col->type == 'app'
        $form['config']['rows'][$row_i]['columns'][$col_i]['app'] = array(
          '#type' => 'select',
          '#title' => t('Select Application'),
          '#options' => $apps_catalog,
          '#empty_option' => t('Choose application'),
          '#default_value' => $col->app,
        );
      // }

      $form['config']['rows'][$row_i]['columns'][$col_i]['remove'] = array(
        '#type' => 'submit',
        '#name' => 'ws_row_'.$row_i.'_cols_'.$col_i.'_rem',
        '#value' => t('Remove Column'),
        '#attributes' => array('class' => array('btn-xs')),
        '#limit_validation_errors' => array(),
        '#submit' => array('science_apps_workspace_remove_column_submit'),
        '#ajax' => array(
          'callback' => 'science_apps_workspace_ajax_column_callback',
          'wrapper' => 'ws-row-'.$row_i.'-cols',
        ),
      );

    } // end foreach column

    $form['config']['rows'][$row_i]['remove'] = array(
      '#type' => 'submit',
      '#name' => 'ws_row_'.$row_i.'_rem',
      '#value' => t('Remove Row'),
      '#attributes' => array('class' => array('btn-xs')),
      '#limit_validation_errors' => array(),
      '#submit' => array('science_apps_workspace_remove_row_submit'),
      '#ajax' => array(
        'callback' => 'science_apps_workspace_ajax_row_callback',
        'wrapper' => 'ws-rows',
      ),
    );

  } // end foreach $config->rows

  $form['config']['add_row'] = array(
    '#type' => 'submit',
    '#name' => 'ws_row_add',
    '#value' => t('Add Row'),
    '#attributes' => array('class' => array('btn-xs')),
    '#limit_validation_errors' => array(),
    '#submit' => array('science_apps_workspace_add_row_submit'),
    '#ajax' => array(
      'callback' => 'science_apps_workspace_ajax_row_callback',
      'wrapper' => 'ws-rows',
    ),
  );

  $form['actions'] = array(
    array(
      '#type' => 'submit',
      '#value' => t('Save Workspace'),
    ),
  );

  return $form;
}

function science_apps_workspace_add_row_submit($form, &$form_state) {
  $config = $form_state['workspace_config'];
  $new_row = new stdClass();
  $new_row->columns = array();
  $col = new stdClass();
  $col->type = 'app';
  $col->app = '';
  $new_row->columns[] = $col;
  $config->rows[] = $new_row;
  $form_state['rebuild'] = TRUE;
}

function science_apps_workspace_remove_row_submit($form, &$form_state) {
  $row_i = $form_state['clicked_button']['#parents'][2];
  $config = $form_state['workspace_config'];
  unset($config->rows[$row_i]);
  $form_state['rebuild'] = TRUE;
}

function science_apps_workspace_ajax_row_callback($form, &$form_state) {
  return $form['config']['rows'];
}

function science_apps_workspace_add_column_submit($form, &$form_state) {
  $row_i = $form_state['clicked_button']['#parents'][2];
  $config = $form_state['workspace_config'];
  $col = new stdClass();
  $col->type = 'app';
  $col->app = '';
  $config->rows[$row_i]->columns[] = $col;
  $form_state['rebuild'] = TRUE;
}

function science_apps_workspace_remove_column_submit($form, &$form_state) {
  $row_i = $form_state['clicked_button']['#parents'][2];
  $col_i = $form_state['clicked_button']['#parents'][4];
  $config = $form_state['workspace_config'];
  unset($config->rows[$row_i]->columns[$col_i]);
  $form_state['rebuild'] = TRUE;
}

function science_apps_workspace_ajax_column_callback($form, &$form_state) {
  $row_i = $form_state['clicked_button']['#parents'][2];
  return $form['config']['rows'][$row_i]['columns'];
}

function science_apps_workspace_edit_form_submit(&$form, &$form_state) {
  global $user;
  $wid = $form_state['values']['wid'];
  $uid = $user->uid;
  $name = $form_state['values']['name'];

  // build config object from form submission
  $config = array('rows' => array());
  foreach($form_state['values']['config']['rows'] as $row) {
    $columns = array();
    foreach ($row['columns'] as $column) {
      // TODO: deal with different column types [app, rows]
      $columns[] = array(
        'type' => $column['type'],
        'app' => $column['app']
      );
    }
    $config['rows'][] = array('columns' => $columns);
  }

  if ($wid) {
    $workspace = Workspace::get($wid);
  } else {
    $workspace = new Workspace();
  }
  $workspace->uid = $uid;
  $workspace->name = $name;
  $workspace->setConfigObject($config);
  $ret = $workspace->save();

  drupal_set_message(t('The Workspace Configuration has been saved!'));
  $form_state['redirect'] = 'workspace/' . $ret->wid;
}
