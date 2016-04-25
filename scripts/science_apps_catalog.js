(function($) {
    $('.science-app-preview .app-description').on('click', function() {
        $(this).addClass('reveal-description');
    });
    $('.science-app-preview').on('mouseleave', function() {
        var _self = $(this);
        setTimeout(function() {
            $('.app-description', _self).removeClass('reveal-description')
        }, 250);
    });
    // $('.app-preview').on('click', function() {
    //     var nid = this.dataset['appNid'];
    //     $('.app-preview-model.app-preview-' + nid).modal({
    //        
    //     });
    // });
})(jQuery);