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
            $build = science_app_view($app_node, 'embed');
            $output .= drupal_render($build->content['app']);
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
    return array(
        array(
            '#markup' => t('Science App Information'),
            '#prefix' => '<h2>',
            '#suffix' => '</h2>',
        ),
        array(
            '#markup' => t('Metadata for !title', array('!title' => "<b>{$node->title}</b>")),
            '#prefix' => '<p class="lead">',
            '#suffix' => '</p>',
        ),
        array(
            '#theme' => 'table',
            '#prefix' => '<div class="table-responsive">',
            '#suffix' => '</div>',
            '#header' => array(t('App Name'), t('Repository'), t('Version'), t('Public'), t('Last Updated')),
            '#rows' => array(_science_apps_workspace_app_info_tr($node),),
        )
    );
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

function science_apps_workspace_catalog() {
    $query = new EntityFieldQuery();
    $apps = $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'science_app')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->propertyOrderBy('title', 'ASC')
        ->execute();
    $apps = node_load_multiple(array_keys($apps['node']));
    $previews = array();
    foreach ($apps as $app) {
        if (user_access('use science apps')) {
            $use_link_href = url("node/{$app->nid}");
            $use_link_text = t('Use this app');
        } else {
            $use_link_href = url('user/login', array('destination' => "node/{$app->nid}"));
            $use_link_text = t('Login to use this app');
        }
        $link = user_access('use science apps') ?
            l(t('Use this app'), "node/{$app->nid}", array('attributes' => array('class' => array('btn', 'btn-xs', 'btn-primary')))) :
            l(t('Login to use this app'), "user/login", array('attributes' => array('class' => array('btn', 'btn-xs', 'btn-primary')), 'query' => array('destination' => "node/{$app->nid}")));
        $app_dir = _science_apps_workspace_apps_repo($app->nid);

        $app_config_defaults = array(
            'tags' => array(),
            'icon' => '',
        );
        if (is_file($app_dir . DIRECTORY_SEPARATOR . 'araport-app.json')) {
            $app_config = json_decode(file_get_contents($app_dir . DIRECTORY_SEPARATOR . 'araport-app.json'));
            $app_config = (object) array_merge($app_config_defaults, (array) $app_config);

        } else {
            $app_config = (object) $app_config_defaults;
        }

        $screenshot_dir = implode(DIRECTORY_SEPARATOR, array($app_dir, 'app', 'screenshots'));
        if (is_dir($screenshot_dir)) {
            $screenshot_dir = drupal_realpath($screenshot_dir);
            $screenshot_files = glob("$screenshot_dir/*.{jpg,jpeg,png,gif}", GLOB_BRACE);
            $app_screenshots = substr_replace($screenshot_files, "/app-embed/$app->nid/app/screenshots", 0, strlen($screenshot_dir));
        } else {
            $app_screenshots = false;
        }
        
        $previews[] = array(
            '#theme' => 'science_app_preview',
            '#node' => $app,
            '#app_config' => $app_config,
            '#app_screenshots' => $app_screenshots,
            '#author' => user_load($app->uid),
            '#link' => $link,
            '#use_link_href' => $use_link_href,
            '#use_link_text' => $use_link_text,
            '#description' => $app->body,
            '#app_preview_classes' => array('science-app-preview'),
        );
    }


    $script = '/scripts/science_apps_catalog.js';
    drupal_add_js(drupal_get_path('module', 'science_apps_workspace'). $script, array('scope' => 'footer'));

    return array(
        '#prefix' => '<div class="science-apps-catalog">',
        '#suffix' => '</div>',
        'children' => $previews,
    );
}

function science_apps_workspace_science_app_publish($node) {
    $output = array();
    if ($node->science_app_config->published == SCIENCE_APP_PUBLISHED_YES) {
        $output[] = array(
            '#markup' => '<h2>This application is published.</h2><p>This application has already been published and is available to all Araport users.</p>',
            '#prefix' => '<div class="jumbotron">',
            '#suffix' => '</div>',
        );
    } else {
        $publication_request = db_select('science_apps_workspace_app_publication_request', 'pr')
            ->fields('pr')
            ->condition('nid', $node->nid)
            ->execute()
            ->fetch();

        if ($publication_request) {
            if ($publication_request->status == SCIENCE_APP_REQUEST_SUBMITTED) {
                $output[] = array(
                    '#theme' => 'science_app_publication_request_submitted',
                    '#message' => t('Publication for this application was requested on @date, and is pending review from the Araport team.', array('@date' => strftime('%b %e, %Y', $publication_request->changed))),
                    '#request' => $publication_request,
                );
            } else {
                $output[] = array(
                    '#theme' => 'science_app_publication_request_returned',
                    '#message' => t('You have requested publication for this application, but the review team is requesting more information. Please see the reviewer\'s comments below.'),
                    '#request' => $publication_request,
                );
                $output[] = array(
                    '#markup' => '<p class="alert alert-info">'. t('Please make the requested changes and resubmit your request below.') .'</p>'
                );

                module_load_include('inc', 'science_apps_workspace', 'science_apps_workspace.forms');
                $output[] = drupal_get_form('science_apps_workspace_science_app_publish_form', $node, $publication_request);
            }
        } else { // $node->science_app_config->published == SCIENCE_APP_PUBLISHED_NO
            $output[] = array(
                '#markup'=> t('Request to Publish Application'),
                '#prefix' => '<h2>',
                '#suffix' => '</h2>',
            );
            $output[] = array(
                '#markup' => t('If you are ready to publish this app, complete the form below. A member of the Araport team will contact you with further details about publishing your app.'),
                '#prefix' => '<p>',
                '#suffix' => '</p>',
            );

            module_load_include('inc', 'science_apps_workspace', 'science_apps_workspace.forms');
            $output[] = drupal_get_form('science_apps_workspace_science_app_publish_form', $node);
        }
    }
    return $output;
}
