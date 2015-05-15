<h2>
  <?php print $node->title ?>
  <small><?php echo $app_link ?></small>
</h2>

<div class="admin-panel">
  <h3>App Description</h3>

  <div class="description">
    A brief description of the app, how to use it, and what it does.
  </div>

  <p><?php echo $description ?></p>
</div>

<div class="admin-panel">
  <h3>Data Sources</h3>

  <div class="description">
    Data source(s) consumed by this app, e.g., Araport Community Data APIs,
    user-provided data input, or third-party data APIs.
  </div>

  <p><?php echo $data_sources ?></p>
</div>


<div class="admin-panel">
  <h3>Technical Contact</h3>

  <div class="description">
    Who to contact with questions, comments during the review process.
  </div>

  <p><?php echo $tech_contact ?></p>
</div>
