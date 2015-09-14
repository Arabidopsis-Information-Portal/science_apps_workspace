<?php

function science_apps_workspace_science_app_publish_admin($request_status = SCIENCE_APP_REQUEST_SUBMITTED) {
  error_log('status: '.$request_status);
  $query = db_select('science_apps_workspace_app_publication_request', 'pr');
  $publication_requests = $query->fields('pr')
                                ->condition('pr.status', $request_status, '=')
                                ->execute()
                                ->fetchAllAssoc('rid');
  $rows = array();
  foreach ($publication_requests as $pr) {
    $nid = $pr->nid;
    $app = node_load($nid);
    $rows[] = array(
      l($app->title, "node/$nid"),
      $pr->status,
      l($pr->technical_contact_name, "mailto:$pr->technical_contact_mail", array('absolute' => TRUE)),
      strftime('%b %e, %Y', $pr->created),
      l(t('Review'), "admin/config/araport/publication-requests/{$pr->rid}", array('attributes' => array('class' => array('btn', 'btn-primary')))),
    );
  }

  return array(
    array(
      '#theme' => 'html_tag',
      '#tag' => 'p',
      '#value' => t('Listed below are apps pending admin review for publication.')
    ),
    array(
      '#theme' => 'table',
      '#header' => array(
        t('Application name'),
        t('Status'),
        t('Technical contact'),
        t('Requested'),
        t('Action'),
      ),
      '#rows' => $rows,
      '#empty' => t('There are pending app publication requests.'),
    ),
  );
}

function science_apps_workspace_science_app_publish_admin_review($rid) {
  $query = db_select('science_apps_workspace_app_publication_request', 'pr');
  $req = $query->fields('pr')
              ->condition('rid', $rid)
              ->execute()
              ->fetch();
  $node = node_load($req->nid);
  return array(
    array(
      '#theme' => 'science_app_publication_request',
      '#description' => check_plain($req->description),
      '#data_sources' => check_plain($req->data_sources),
      '#tech_contact' => l(check_plain($req->technical_contact_name) . ' <' . check_plain($req->technical_contact_mail) . '>', "mailto:{$req->technical_contact_mail}", array('absolute' => TRUE)),
      '#app_link' => l(t('View this application'), "node/{$req->nid}"),
      '#node' => $node,
      '#request_data' => $req,
    ),

    drupal_get_form('science_apps_workspace_science_app_publish_admin_review_form', $req, $node),

    // array(
    //   '#theme' => 'html_tag',
    //   '#tag' => 'pre',
    //   '#value' => print_r($req, 1),
    // ),
  );
}

function science_apps_workspace_science_app_publish_admin_review_form($form, &$form_state, $app_request, $node) {
  $form = array();

  $form['app_request'] = array(
    '#type' => 'value',
    '#value' => $app_request,
  );

  $form['node'] = array(
    '#type' => 'value',
    '#value' => $node,
  );

  $form['publication_notes'] = array(
    '#type' => 'textarea',
    '#title' => t('Reviewer Notes'),
    '#description' => t('Please provide any administrative notes about this application review here. These will NOT be displayed to the application authors.'),
    '#required' => TRUE,
    '#default_value' => $app_request->publication_notes,
  );

  $form['published'] = array(
    '#type' => 'select',
    '#title' => t('Published Status'),
    '#options' => array(
      t('Not Published'),
      t('Published'),
      t('Requested'),
    ),
    '#required' => TRUE,
    '#default_value' => $node->science_app_config->published,
  );

  $form['actions'] = array(
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save Review'),
    ),
  );

  return $form;
}

function science_apps_workspace_science_app_publish_admin_review_form_submit($form, &$form_state) {
  $app_request = $form_state['values']['app_request'];
  $node = $form_state['values']['node'];

  // Update the config
  $node->science_app_config->published = $form_state['values']['published'];
  $node->science_app_config->no_update = TRUE;
  db_update('science_apps_workspace_app_config')
    ->fields(array('published' => $node->science_app_config->published))
    ->condition('nid', $node->nid)
    ->execute();

  // Update the node, if necessary
  if ($form_state['values']['published'] == SCIENCE_APP_PUBLISHED_YES) {
    $node->status = SCIENCE_APP_PUBLISHED_YES;
  } else {
    $node->status = 0;
  }
  node_save($node);

  // Update the review requeset
  $app_request->publication_notes = $form_state['values']['publication_notes'];
  $app_request->status = 1;
  db_update('science_apps_workspace_app_publication_request')
    ->fields(array(
      'publication_notes' => $app_request->publication_notes,
      'status' => $app_request->status,
    ))
    ->condition('rid', $app_request->rid)
    ->execute();

  drupal_set_message('Application Review Saved!');
  drupal_goto('admin/config/araport/publication-requests');
}
