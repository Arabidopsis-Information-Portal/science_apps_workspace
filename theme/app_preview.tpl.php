<div class="<?php echo implode(' ', $app_preview_classes) ?>">
  <div class="thumbnail">
    <div class="text-center"><i class="fa fa-cube fa-5x"></i></div>
    <div class="caption app-description">
      <h3>
        <?php echo $node->title ?><br>
        <small><?php
          if (array_key_exists('und', $node->science_app_version)) {
            echo $node->science_app_version['und'][0]['safe_value'];
          } else {
            echo 'latest';
          }
        ?></small>
      </h3>

      <?php if (isset($node->body['und'])) { ?>
      <div class="description" title="Click for more">
        <?php echo $node->body['und'][0]['safe_value']; ?>
      </div>
      <?php } ?>

    </div>
    <div class="caption">
      <?php echo $link; ?>
    </div>
  </div>
</div>
