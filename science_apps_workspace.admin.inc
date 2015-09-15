<?php

function science_apps_workspace_science_app_publish_admin($request_status = SCIENCE_APP_REQUEST_SUBMITTED) {
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
        t('Technical contact'),
        t('Requested'),
        t('Action'),
      ),
      '#rows' => $rows,
      '#empty' => t('There are no app publication requests in this status.'),
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

  $form['actions'] = array(
    'save' => array(
      '#type' => 'submit',
      '#value' => t('Save for Later'),
    ),

    'return' => array(
      '#type' => 'submit',
      '#value' => t('Return to User'),
    ),

    'accept' => array(
      '#type' => 'submit',
      '#value' => t('Accept Publication Request'),
    ),

    'reject' => array(
      '#type' => 'submit',
      '#value' => t('Reject Publication Request'),
    ),
  );

  return $form;
}

function science_apps_workspace_science_app_publish_admin_review_form_submit($form, &$form_state) {

  $app_request = $form_state['values']['app_request'];
  $node = $form_state['values']['node'];
  $op = $form_state['clicked_button']['#value'];

  watchdog('science_apps_workspace', print_r($app_request, 1), array(), WATCHDOG_DEBUG);

  if ($op == $form_state['values']['return']) {
    $app_request->status = SCIENCE_APP_REQUEST_RETURNED;
  } else if ($op == $form_state['values']['accept']) {
    $app_request->status = SCIENCE_APP_REQUEST_COMPLETED;
    $node->status = 1;
    $node->science_app_config->published = SCIENCE_APP_PUBLISHED_YES;
    $node->science_app_config->no_update = TRUE;
  } else if ($op == $form_state['values']['reject']) {
    $app_request->status = SCIENCE_APP_REQUEST_COMPLETED;
    $node->status = 0;
    $node->science_app_config->published = SCIENCE_APP_PUBLISHED_NO;
    $node->science_app_config->no_update = TRUE;
  }

  // Update the review requeset
  $app_request->publication_notes = $form_state['values']['publication_notes'];
  db_update('science_apps_workspace_app_publication_request')
    ->fields(array(
      'publication_notes' => $app_request->publication_notes,
      'status' => $app_request->status,
    ))
    ->condition('rid', $app_request->rid)
    ->execute();

  // Update the config
  db_update('science_apps_workspace_app_config')
    ->fields(array('published' => $node->science_app_config->published))
    ->condition('nid', $node->nid)
    ->execute();

  // Update the node
  node_save($node);

  drupal_set_message('Application Review Saved!');
  drupal_goto('admin/config/araport/publication-requests');
}
