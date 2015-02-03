<?php

function science_apps_workspace_edit_form($form, &$form_state, $workspace = null) {
  global $user;

  $apps_catalog = science_apps_workspace_apps_catalog($user->uid);

  if (! isset($form_state['workspace_config'])) {
    if (! $workspace) {
      $workspace = new Workspace();
      $workspace->emptyConfig();
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

function _science_apps_workspace_process_workspace_form($form_config_values) {
  // build config object from form submission
  $config = array('rows' => array());
  foreach($form_config_values['rows'] as $row) {
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
  return $config;
}

function science_apps_workspace_edit_form_submit(&$form, &$form_state) {
  global $user;
  $wid = $form_state['values']['wid'];
  $uid = $user->uid;
  $name = $form_state['values']['name'];

  $config = _science_apps_workspace_process_workspace_form($form_state['values']['config']);

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

function science_apps_workspace_admin_default_workspace($form, &$form_state) {
  $workspace = science_apps_workspace_default_workspace();
  $form = science_apps_workspace_edit_form($form, $form_state, $workspace);
  return $form;
}

function science_apps_workspace_admin_default_workspace_submit(&$form, &$form_state) {
  variable_set('science_apps_workspace_default_name', $form_state['values']['name']);
  $config = _science_apps_workspace_process_workspace_form($form_state['values']['config']);
  variable_set('science_apps_workspace_default_config', json_encode($config));
  drupal_set_message(t('The Default Workspace Configuration has been saved!'));
}
