<header>
  <nav class="navbar navbar-default">
    <div class="container-fluid">
      <a class="navbar-brand" href="#"><?php print $workspace_name ?></a>
      <?php print $edit_workspace_link ?>
      <div class="navbar-right">
        <div class="btn-group">
          <button type="button" class="btn btn-default navbar-btn dropdown-toggle" data-toggle="dropdown">
            <i class="fa fa-cog"></i><span class="sr-only"><?php print t('Settings') ?></span>
          </button>
          <?php print $workspace_settings_links ?>
        </div>

        <div class="btn-group">
          <button type="button" class="btn btn-default navbar-btn dropdown-toggle" data-toggle="dropdown">
            <i class="fa fa-bars"></i><span class="sr-only"><?php print t('Change Workspace') ?></span>
          </button>
          <?php print $workspace_switcher_links ?>
        </div>
      </div>
    </div>
  </nav>
</header>
