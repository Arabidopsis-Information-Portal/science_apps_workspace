<?php

function science_apps_workspace_apps_admin_view() {
  $header = array(
    array('data' => t('Title'), 'type' => 'property', 'specifier' => 'title'),
    array('data' => t('Version'), 'type' => 'field', 'specifier' => array('field' => 'science_app_version', 'column' => 'value')),
    t('State'),
    t('Status'),
    t('Owner'),
    t('Actions'),
  );

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node');
  $query->entityCondition('bundle', 'science_app');
  $query->tableSort($header);
  $apps = $query->execute();
  $apps = node_load_multiple(array_keys($apps['node']));

  $rows = array();

  foreach ($apps as $app) {
    $owner = user_load($app->uid);

    if ($app->science_app_config->ready == SCIENCE_APP_STATE_NOT_READY) {
      $app_ready_state = '&#8635; '.t('Preparing');
    } else if ($app->science_app_config->ready == SCIENCE_APP_STATE_READY) {
      $app_ready_state = '&#10003; '.t('Ready');
    } else { // $app->science_app_config->ready == SCIENCE_APP_STATE_ERROR
      $app_ready_state = '&#10007; '.t('Error');
    }

    if ($app->science_app_config->published == SCIENCE_APP_PUBLISHED_YES) {
      $app_published_status = '&#10003; '.t('Published');
    } else if ($app->science_app_config->published == SCIENCE_APP_PUBLISHED_NO) {
      $app_published_status =  t('Not published');
    } else { // $app->science_app_config->published == SCIENCE_APP_PUBLISHED_REQUESTED
      $app_published_status =  t('Requested');
    }

    $rows[] = array(
      l($app->title, "node/{$app->nid}"),
      count($app->science_app_version) ? $app->science_app_version['und'][0]['safe_value'] : '',
      $app_ready_state,
      $app_published_status,
      l($owner->name, "user/{$owner->uid}"),
      $app->science_app_config->published ?
        l(t('Unpublish'), "admin/config/araport/apps/{$app->nid}/unpublish") :
        l(t('Publish'), "admin/config/araport/apps/{$app->nid}/publish"),
    );
  }

  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );
}

function science_apps_workspace_apps_admin_confirm_publish($form, &$form_state, $app_nid) {
  $app = node_load($app_nid);
  $form['#app'] = $app;
  $form['nid'] = array('#type' => 'value', '#value' => $app_nid);
  return confirm_form(
    $form,
    t('Are you sure you want to publish the app %app_title', array('%app_title' => $app->title)),
    'node/' . $app->nid,
    t('Once published, this app will be listed in the Apps Catalog and available to all users of the site.'),
    t('Publish'),
    t('Cancel'),
    'science_app_confirm_publish'
  );
}

function science_apps_workspace_apps_admin_confirm_publish_submit($form, &$form_state) {
  global $user;

  $app = $form['#app'];
  $app->status = 1;
  $app->science_app_config->published = SCIENCE_APP_PUBLISHED_YES;
  $app->science_app_config->no_update = TRUE;

  // Update the config
  db_update('science_apps_workspace_app_config')
    ->fields(array('published' => $app->science_app_config->published))
    ->condition('nid', $app->nid)
    ->execute();

  // Update the app
  node_save($app);

  // log the event
  watchdog('science_apps_workspace', 'Science App %app_title (nid=@app_nid) PUBLISHED via admin by %user', array(
    '%app_title' => $app->title,
    '@app_nid' => $app->nid,
    '%user' => $user->name));

  drupal_set_message(t('The Science App %app_title has been published!', array('%app_title' => $app->title)));

  $form_state['redirect'] = 'admin/config/araport/apps';
}

function science_apps_workspace_apps_admin_confirm_unpublish($form, &$form_state, $app_nid) {
  $app = node_load($app_nid);
  $form['#app'] = $app;
  $form['nid'] = array('#type' => 'value', '#value' => $app_nid);
  return confirm_form(
    $form,
    t('Are you sure you want to unpublish the app %app_title', array('%app_title' => $app->title)),
    'node/' . $app->nid,
    t('Once published, this app will NO LONGER be listed in the Apps Catalog and will NOT be available to all users of the site.'),
    t('Unpublish'),
    t('Cancel'),
    'science_app_confirm_unpublish'
  );
}

function science_apps_workspace_apps_admin_confirm_unpublish_submit($form, &$form_state) {
  global $user;

  $app = $form['#app'];
  $app->status = 0;
  $app->science_app_config->published = SCIENCE_APP_PUBLISHED_NO;
  $app->science_app_config->no_update = TRUE;

  // Update the config
  db_update('science_apps_workspace_app_config')
    ->fields(array('published' => $app->science_app_config->published))
    ->condition('nid', $app->nid)
    ->execute();

  // Update the app
  node_save($app);

  // log the event
  watchdog('science_apps_workspace', 'Science App %app_title (nid=@app_nid) UNPUBLISHED via admin by %user', array(
    '%app_title' => $app->title,
    '@app_nid' => $app->nid,
    '%user' => $user->name));

  drupal_set_message(t('The Science App %app_title has been unpublished.', array('%app_title' => $app->title)));

  $form_state['redirect'] = 'admin/config/araport/apps';
}

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
    '#title' => t('Publication Notes'),
    '#description' => t('Use this space to communicate to the user any updates, changes, or requirements that need to be met in order to publish this application. This comments will be sent to the application authors.'),
    '#required' => FALSE,
    '#default_value' => $app_request->publication_notes,
  );

  $form['reviewer_comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Review Team Internal Notes'),
    '#description' => t('Please provide any administrative notes about this application review here. These will NOT be displayed to the application authors.'),
    '#required' => TRUE,
    '#default_value' => $app_request->reviewer_comments,
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
    $notification_type = 'publication_request_return_to_user';
    $app_request->status = SCIENCE_APP_REQUEST_RETURNED;
  } else if ($op == $form_state['values']['accept']) {
    $notification_type = 'publication_request_notify_published';
    $app_request->status = SCIENCE_APP_REQUEST_COMPLETED;
    $node->status = 1;
    $node->science_app_config->published = SCIENCE_APP_PUBLISHED_YES;
    $node->science_app_config->no_update = TRUE;
  } else if ($op == $form_state['values']['reject']) {
    $notification_type = 'publication_request_return_to_user';
    $app_request->status = SCIENCE_APP_REQUEST_COMPLETED;
    $node->status = 0;
    $node->science_app_config->published = SCIENCE_APP_PUBLISHED_NO;
    $node->science_app_config->no_update = TRUE;
  } else {
    // SAVE FOR LATER
    $notification_type = FALSE;
  }

  if ($notification_type) {
    // notify submitter, technical contact
    $to = $app_request->technical_contact_mail;
    $author = user_load($node->uid);
    $cc = $author->mail;
    drupal_mail('science_apps_workspace', $notification_type, $to, 'und',
                array(
                  'nid' => $node->nid, 'rid' => $app_request->rid,
                  'headers' => array('Cc' => $cc)
                ));
  }

  // Update the review requeset
  $app_request->publication_notes = $form_state['values']['publication_notes'];
  $app_request->reviewer_comments = $form_state['values']['reviewer_comments'];
  db_update('science_apps_workspace_app_publication_request')
    ->fields(array(
      'publication_notes' => $app_request->publication_notes,
      'reviewer_comments' => $app_request->reviewer_comments,
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
