<div class="app-placeholder">
  <?php if ($anonymous): ?>
    <h1>Log in to customize your workspace!</h1>
    <p>
      Araport users can create customized workspaces tailored to their workflows.
      <a href="/user/login">Log in</a> or <a href="/user/register">create an account</a>
      now!
    </p>
  <?php else: ?>
    <h1>Choose an app!</h1>
    <p>
      Click the Workspace Settings button
      <button disabled="" type="button" class="btn btn-default"><i class="fa fa-cog"></i></button>
      at the top of the page to configure your workspace.
    </p>
  <?php endif; ?>
</div>
