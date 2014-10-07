<?php

function science_apps_workspace_page() {
  global $user;

  $output = '<div class="workspace">';
  $workspace = Workspace::getDefault($user);
  if (! $workspace) {
    $workspace = new Workspace();
  }
  if (isset($workspace)) {
    $config = json_decode($workspace->config);
    foreach ($config->rows as $row) {
      $output .= science_apps_workspace_render_row($row);
    }
  } else {
    $output .= '<div class="jumbotron">';
    $output .= '<h1>There aren\'t any apps in your workspace!</h1>';
    $output .= '<p>If you recently created one, please wait a few minutes while we prepare it. If not, why don\'t you add one now?</p></p>';
  }
  $output .= '</div>';
  return '<div class="science-apps-workspace">' . science_apps_workspace_page_header($workspace) . $output . '</div>';
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
    $output .= theme('science_app_display', array('node' => $app_node));
  } else {
    $output .= '<div class="app-placeholder"><h1>Choose an app!</h1><p>Click the Workspace Settings button <button disabled type="button" class="btn btn-default"><i class="fa fa-cog"></i></button> at the top of the page to configure your workspace.</p></div>';
  }
  $output .= '</div>';
  return $output;
}

function science_apps_workspace_page_header($workspace) {
  $output = '<header><nav class="navbar navbar-default"><div class="container-fluid"><a class="navbar-brand" href="#">' . $workspace->name . '</a>';
  if (user_access('create science_app content')) {
    $output .= l('<i class="fa fa-flask"></i> ' . t('Create New App'), 'node/add/science-app', array('html' => TRUE, 'attributes' => array('class' => 'btn btn-primary navbar-btn')));
  }
  if (user_access('use science apps')) {
    $output .= '<div class="btn-group navbar-right"><button type="button" class="btn btn-default navbar-btn dropdown-toggle" data-toggle="dropdown"><i class="fa fa-cog"></i><span class="sr-only">'.t('Settings').'</span></button><ul class="dropdown-menu" role="menu">';
    $output .= '<li>'. l(t('Edit this Workspace'), 'workspace/edit/' . $workspace->wid) . '</li>';
    $output .= '<li>'. l(t('Manage Workspaces'), 'workspace/configure') . '</li>';
    $output .= '</ul></div>';
  }
  $output .= '</div></nav></header>';

  return $output;
}

function science_apps_workspace_config($wid = null) {
  global $user;

  $output = '';

  if ($wid) {
    return $wid;
  } else {
    $workspaces = db_select('science_apps_workspace_user_config', 'ws')
      ->fields('ws')
      ->condition('ws.uid', $user->uid)
      ->execute()->fetchAllAssoc('wid');

    if (count($workspaces)) {
      $items = array();
      foreach ($workspaces as $wid => $ws) {
        $item = '<strong>' . $ws->name . '</strong>';
        $item .= ' <i class="fa fa-' . ($ws->status ? 'check-square-o' : 'square-o') . '"></i> Default workspace ';
        $item .= l(t('Edit this workspace'), 'workspace/edit/' . $wid, array('attributes' => array('class' => 'btn btn-default')));
        $items[] = $item;
      }

      $output .= theme('item_list', array('title' => t('Select a Workspace'), 'items' => $items));
    } else {
      $output .= '<div class="jumbotron"><h1>You haven\'t configured any workspaces yet!</h1></div>';
    }

    $output .= l(t('Create a New Workspace'), 'workspace/new', array('attributes' => array('class' => 'btn btn-lg btn-primary btn-block')));
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
  );

  $form['config'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('Workspace Setup'),
  );

  $form['config']['instruct'] = array(
    '#type' => 'markup',
    '#markup' => t('Each Workspace is a collection of applications that are displayed together. Applications are arranged in rows and columns. Each row contains columns, and each column can contain either more rows or an application. You choose the number of rows and columns and select the applications for your workspace.'),
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

  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Default Workspace'),
    '#description' => t('Check this if you would like this workspace to be your default'),
    '#default_value' => $workspace->status,
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

  $status = $form_state['values']['status'];

  if ($wid) {
    $workspace = Workspace::get($wid);
  } else {
    $workspace = new Workspace();
  }
  $workspace->uid = $uid;
  $workspace->name = $name;
  $workspace->status = $status;
  $workspace->setConfigObject($config);
  $ret = $workspace->save();

  drupal_set_message(t('The Workspace Configuration has been saved!'));
  $form_state['redirect'] = 'workspace';
}
