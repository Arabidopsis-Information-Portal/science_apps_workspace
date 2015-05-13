<?php

class Workspace {

  public $wid;

  public $uid;

  public $name;

  public $config;

  public $status;

  public $created;

  public $changed;

  function __construct($wid = 0, $uid = 0, $name = '', $config = FALSE, $status = 0, $created = 0, $changed = 0) {
    $this->wid = $wid;
    $this->uid = $uid;
    $this->name = $name;
    $this->config = $config;
    $this->status = $status;
    $this->created = $created;
    $this->changed = $changed;
  }

  public function isDefault() {
    return $this->status == 1;
  }

  public function getConfigObject() {
    if ($this->config) {
      return json_decode($this->config);
    }
  }

  public function setConfigObject($config_object) {
    $this->config = json_encode($config_object);
  }

  public function emptyConfig() {
    $this->config = '{"rows":[{"columns":[{"type":"app","app":null}]}]}';
  }

  public function save() {
    if ($this->wid) {
      // update
      db_update('science_apps_workspace_user_config')
        ->fields(array(
          'name' => $this->name,
          'config' => $this->config,
          'status' => $this->status,
          'changed' => REQUEST_TIME
        ))
        ->condition('wid', $this->wid)
        ->execute();
    } else {
      // insert
      $wid = db_insert('science_apps_workspace_user_config')
        ->fields(array(
          'uid' => $this->uid,
          'name' => $this->name,
          'config' => $this->config,
          'status' => $this->status,
          'created' => REQUEST_TIME,
          'changed' => REQUEST_TIME
        ))
        ->execute();
      $this->wid = $wid;
    }
    return $this;
  }

  public function delete() {}

  public static function get($wid) {
    $query = db_select('science_apps_workspace_user_config', 'ws');
    $query->fields('ws');
    $query->condition('wid', $wid);
    $query->range(0, 1);

    $result = $query->execute()->fetch();
    if ($result) {
      return new Workspace($result->wid, $result->uid, $result->name, $result->config, $result->status, $result->created, $result->changed);
    }
  }

  public static function getList($user) {
    $query = db_select('science_apps_workspace_user_config', 'ws');
    $query->fields('ws');
    $query->condition('uid', $user->uid);
    $query->orderBy('status', 'DESC');
    $query->orderBy('changed', 'DESC');

    $list = array();
    foreach ($query->execute()->fetchAllAssoc('wid') as $wid => $workspace) {
      $list[] = new Workspace($wid, $workspace->uid, $workspace->name, $workspace->config, $workspace->status, $workspace->created, $workspace->changed);
    }
    return $list;
  }

  public static function getDefault($user) {
    $query = db_select('science_apps_workspace_user_config', 'ws');
    $query->fields('ws');
    $query->condition('uid', $user->uid);
    $query->orderBy('status', 'DESC');
    $query->orderBy('changed', 'DESC');
    $query->range(0, 1);

    $result = $query->execute()->fetch();
    if ($result) {
      return new Workspace($result->wid, $result->uid, $result->name, $result->config, $result->status, $result->created, $result->changed);
    }
  }
}
