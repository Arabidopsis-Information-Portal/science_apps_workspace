<?php

function science_apps_workspace_page() {
  global $user;

  $output = '';
  if ($user->uid) {
    $apps = science_apps_workspace_get_user_apps($user->uid);

    if ($apps) {
      // foreach ($apps as $app_nid => $app) {
      //   $app_dir = 'private://' . implode(DIRECTORY_SEPARATOR, array('science_apps_repository', $app_nid));
      //   // app.html
      //   $output = file_get_contents(implode(DIRECTORY_SEPARATOR, array($app_dir, 'app', 'app.html')));
      //   // app.css
      //   drupal_add_css(file_get_contents(implode(DIRECTORY_SEPARATOR, array($app_dir, 'app', 'styles', 'app.css'))), array('type' => 'inline', 'scope' => 'header');
      //   // app.js
      //   drupal_add_js(file_get_contents(implode(DIRECTORY_SEPARATOR, array($app_dir, 'app', 'scripts', 'app.js'))), array('type' => 'inline', 'scope' => 'footer');
      // }
    } else {
      $output .= '<div class="jumbotron">';
      $output .= '<h1>There aren\'t any apps in your workspace!</h1>';
      $output .= '<p>If you recently created one, please wait a few minutes while we prepare it. If not, why don\'t you add one now?</p><p><a href="/node/add/science-app?destination=/workspace" class="btn btn-primary btn-lg">Create an App</a></p>';
    }
  }

  return $output;
}
