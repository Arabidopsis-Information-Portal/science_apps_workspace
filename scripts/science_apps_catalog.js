/* globals jQuery, _ */
(function(window, $, _) {
  $('.science-app-preview .app-description').on('click', function() {
    $(this).addClass('reveal-description');
  });
  $('.science-app-preview').on('mouseleave', function() {
    var _self = $(this);
    setTimeout(function() {
      $('.app-description', _self).removeClass('reveal-description')
    }, 250);
  });

  var filterCatalog = function filterCatalog(e) {
    var term = $(this).val();
    var tokens = [];
    _.each(term.split(/\s*("[^"]+")\s*/), function(token) {
      if (token) {
        var tmp = /"([^"]+)"/.exec(token);
        if (tmp) {
          tokens.push(tmp[1]);
        } else {
          tokens = tokens.concat(token.split(' '));
        }
      }
    });
    tokens = _.chain(tokens).compact().unique().value();
    if (tokens.length > 0) {
      _.each(window.scienceApps, function(app) {
        var stringified = JSON.stringify(app);
        for (var i = 0; i < tokens.length; i++) {
          var token = tokens[i].trim();
          //attribute filter
          if (/\w+:/.test(token)) {
            var parts = token.split(':');
            var attr = parts[0];
            var val = parts[1];
            if (attr in app && new RegExp(val, 'i').test(app[attr])) {
              $('#science-app-'+app.id).show();
            } else {
              $('#science-app-'+app.id).hide();
            }
          }
          // general filter
          else {
            if (new RegExp(token, 'i').test(stringified)) {
              $('#science-app-'+app.id).show();
            } else {
              $('#science-app-'+app.id).hide();
            }
          }
        }
      });
    } else {
      $('.science-app-preview').show();
    }
  };
  filterCatalog = _.throttle(filterCatalog, 200);
  $('input[name=filter_apps]').on('input', filterCatalog);
})(window, jQuery, _);
