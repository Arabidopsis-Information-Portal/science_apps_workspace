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
    $row = array(
      l($app->title, "node/$nid"),
      l($pr->technical_contact_name, "mailto:$pr->technical_contact_mail", array('absolute' => TRUE)),
      strftime('%b %e, %Y', $pr->created),
      l(t('Review'), "admin/config/araport/publication-requests/{$pr->rid}", array('attributes' => array('class' => array('btn', 'btn-primary')))),
    );
    if ($request_status == SCIENCE_APP_REQUEST_COMPLETED) {
      $row[] = $app->status ? '<i class="fa fa-check-square-o"></i> Published' : '<i class="fa fa-square-o"></i> Not published';
    }
    $rows[] = $row;
  }

  $output = array(
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

  if ($request_status == SCIENCE_APP_REQUEST_COMPLETED) {
    $output[1]['#header'][] = t('Published');
  }

  return $output;
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
      '#value' => t('Accept and Publish'),
    ),

    'reject' => array(
      '#type' => 'submit',
      '#value' => t('Close without Publishing'),
      '#attributes' => array('style'=>'float:right')
    ),
  );

  $operations = array(
    '<b>' . $form['actions']['save']['#value'] . '</b>: Defer action on this request until later.',
    '<b>' . $form['actions']['return']['#value'] . '</b>: Return the request to the user with comments, presumably action items necessary for publication.',
    '<b>' . $form['actions']['accept']['#value'] . '</b>: Accept and immediately publish the app.',
    '<b>' . $form['actions']['reject']['#value'] . '</b>: Close the request without publishing the app. If the user responds the comments the request will be put back in the queue.',
  );

  $form['action_info'] = array(
    '#type' => 'markup',
    '#markup' => 'You can perform the following operations on this request: ' . theme('item_list', array('items' => $operations)),
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
  );

  return $form;
}

function science_apps_workspace_science_app_publish_admin_review_form_submit($form, &$form_state) {

  $app_request = $form_state['values']['app_request'];
  $node = $form_state['values']['node'];
  $op = $form_state['clicked_button']['#value'];

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

  // notify submitter, technical contact
  $to = array();
  $author = user_load($node->uid);
  $to[] = $author->mail;
  $to[] = $app_request->technical_contact_mail;
  drupal_mail('science_apps_workspace', 'publication_request_notify_user', implode(',', $to), 'und', array('nid' => $node->nid, 'rid' => $app_request->rid));

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
