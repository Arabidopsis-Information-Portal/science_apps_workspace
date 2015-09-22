<h2>Publication Pending</h2>

<p class="lead">
  <?php echo $message ?>
</p>

<h4>Your submission:</h4>
<dl>
  <dt>App Description</dt>
  <dd><?php echo $request->description ?></dd>

  <dt>Data Sources</dt>
  <dd><?php echo $request->data_sources ?></dd>

  <dt>Technical Contact</dt>
  <dd>
    <a href="mailto:<?php echo $request->technical_contact_mail ?>"><?php echo $request->technical_contact_name ?></a>
  </dd>
</dl>