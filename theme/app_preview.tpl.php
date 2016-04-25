<div class="<?php echo implode(' ', $app_preview_classes) ?>">
    <div class="thumbnail">
        <?php if ($app_config->icon) { ?>
            <img src="/app-embed/<?php print $node->nid ?>/<?php print $app_config->icon ?>" style="height:60px;">
        <?php } else { ?>
            <div class="text-center"><i class="fa fa-cube" style="height:60px;font-size:60px;"></i></div>
        <?php } ?>
        <div class="caption app-description">
            <h5>
                <small><?php echo $author->name; ?>/</small><br>
                <?php echo $node->title ?><br>
            </h5>
            <h6>
                <?php if (array_key_exists('und', $node->science_app_version)) {
                    echo $node->science_app_version['und'][0]['safe_value'];
                } else {
                    echo 'latest';
                } ?>
            </h6>

            <?php if ($app_config->tags) { ?>
                <i class="fa fa-tag"></i>
                <?php print implode(', ', $app_config->tags); ?>
            <?php } ?>

            <?php if (isset($node->body['und'])) { ?>
                <div class="description" title="Click for more">
                    <?php echo $node->body['und'][0]['safe_value']; ?>
                </div>
            <?php } ?>

        </div>
        <div class="caption text-center">
            <a href="#preview:<?php print $node->nid ?>" class="btn btn-xs btn-default app-preview"
               data-toggle="modal" data-target="#app-preview-<?php print $node->nid ?>">
                More information
            </a>
            <a href="<?php print $use_link_href ?>" class="btn btn-xs btn-primary"><?php print $use_link_text ?></a>
        </div>
    </div>
    <div class="app-preview-modal modal fade" id="app-preview-<?php print $node->nid ?>" tab-index="-1" role="dialog" aria-labelledby="app-preview-<?php print $node->nid ?>-title">
        <div class="modal-dialog modal-lg">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
                    <h4 class="modal-title" id="app-preview-<?php print $node->nid ?>-title">
                        <?php print $node->title ?>
                        <small><?php print $node->science_app_version ? $node->science_app_version['und'][0]['safe_value'] : 'latest' ?></small>
                    </h4>
                </div>
                <div class="modal-body">
                    <p><b>Author:</b> <?php print $author->name ?></p>
                    <p>
                        <b>Repository:</b>
                        <a href="<?php print $node->science_app_repository['und'][0]['value'] ?>" target="_blank" rel="nofollow">
                            <?php print $node->science_app_repository['und'][0]['value'] ?>
                        </a>
                    </p>

                    <?php if ($app_config->tags) { ?>
                        <p><i class="fa fa-tag"></i> <?php print implode(', ', $app_config->tags); ?>
                        </p>
                    <?php } ?>

                    <?php if (isset($node->body['und'])) { ?>
                        <b>Description:</b>
                        <?php echo $node->body['und'][0]['safe_value']; ?>
                    <?php } ?>

                    <?php if ($app_screenshots) { ?>
                        <div class="screenshots-scroller">
                            <?php foreach($app_screenshots as $i => $screenshot) { ?>
                                <?php $idx = $i + 1; ?>
                                <div class="screenshot">
                                    <img src="<?php print $screenshot ?>"
                                         title="<?php print "Screenshot $idx" ?>"
                                         alt="<?php print "$node->title Screenshot $idx" ?>"
                                         class="">
                                    <p class="text-center">
                                        <span class="label label-primary"><?php print "$node->title Screenshot $idx" ?></span>
                                    </p>
                                </div>
                            <?php } ?>
                        </div>
                    <?php } else { ?>
                        <div class="jumbotron text-center" style="background-color:#ddd;">
                            <p class="text-muted">No screenshots</p>
                            <p><i class="fa fa-5x fa-desktop text-muted"></i></p>
                        </div>
                    <?php } ?>

                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
                    <a href="<?php print $use_link_href ?>" class="btn btn-primary"><?php print $use_link_text ?></a>
                </div>
            </div>
        </div>
    </div>
</div>
