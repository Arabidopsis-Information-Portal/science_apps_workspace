<h2>Science App Logs <small><?php print $clear_logs_link ?></small></h2>

<?php foreach ($logs as $log): ?>
<small class="text-muted"><?php print date('c', $log['timestamp']) ?></small>
<pre><?php print $log['message'] ?></pre>
<?php endforeach; ?>
