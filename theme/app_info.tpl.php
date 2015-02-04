<h2>Science App Information</h2>

<p class="lead">
  Metadata for <b><?php print $node->title; ?></b>.
</p>

<div class="table-responsive">
  <table class="table">
    <thead>
      <th>Author</th>
      <th>Repository</th>
      <th>Version</th>
      <th>Public</th>
      <th>Last Updated</th>
    </thead>
    <tbody>
      <tr>
        <td><?php print $author->name ?></td>
        <td><a target="_blank" rel="nofollow" href="<?php print $repo ?>"><?php print $repo ?></a></td>
        <td><?php print $version ?></td>
        <td><?php print $published ? '<i class="fa fa-check-square-o"></i> Yes' : '<i class="fa fa-square-o"></i> No' ?></td>
        <td><?php print date('d-M-Y', $node->changed) ?></td>
      </tr>
    </tbody>
  </table>
</div>
