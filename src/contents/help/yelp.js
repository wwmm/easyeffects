
document.addEventListener('DOMContentLoaded', function() {
  var yelp_hash_highlight = function () {
    if (location.hash != '') {
      var sect = document.querySelector(location.hash);
      if (sect != null) {
        sect.classList.add('yelp-hash-highlight');
        window.setTimeout(function () {
          sect.classList.remove('yelp-hash-highlight');
        }, 500);
      }
    }
  }
  window.addEventListener('hashchange', yelp_hash_highlight, false);
  yelp_hash_highlight();
}, false);

var __yelp_generate_id_counter__ = 0;
function yelp_generate_id () {
  var ret = 'yelp--' + (++__yelp_generate_id_counter__).toString();
  if (document.getElementById(ret) != null)
    return yelp_generate_id();
  else
    return ret;
};
function yelp_ui_expander_init (expander) {
  var yelpdata = null;
  var innerdiv = null;
  var region = null;
  var contents = null;
  var title = null;
  var title_e = null;
  var title_c = null;
  var titlespan = null;
  var issect = false;
  for (var i = 0; i < expander.children.length; i++) {
    var child = expander.children[i];
    if (child.classList.contains('yelp-data-ui-expander')) {
      yelpdata = child;
    }
    else if (child.classList.contains('inner')) {
      innerdiv = child;
    }
  }
  if (innerdiv == null) {
    return;
  }
  for (var i = 0; i < innerdiv.children.length; i++) {
    var child = innerdiv.children[i];
    if (child.classList.contains('region')) {
      region = child;
    }
    else if (child.classList.contains('title')) {
      title = child;
    }
    else if (child.classList.contains('hgroup')) {
      title = child;
      issect = true;
    }
  }
  if (region == null || title == null) {
    return;
  }
  if (!region.hasAttribute('id')) {
    region.setAttribute('id', yelp_generate_id());
  }
  title.setAttribute('aria-controls', region.getAttribute('id'));

  if (yelpdata != null) {
    for (var i = 0; i < yelpdata.children.length; i++) {
      var child = yelpdata.children[i];
      if (child.classList.contains('yelp-title-expanded')) {
        title_e = child;
      }
      else if (child.classList.contains('yelp-title-collapsed')) {
        title_c = child;
      }
    }
  }
  titlespan = title.querySelector('span.title');
  if (titlespan == null) {
    return;
  }
  if (title_e == null) {
    var node = document.createElement('div');
    node.className = 'yelp-title-expanded';
    node.innerHTML = titlespan.innerHTML;
    yelpdata.appendChild(node);
    title_e = node;
  }
  if (title_c == null) {
    var node = document.createElement('div');
    node.className = 'yelp-title-collapsed';
    node.innerHTML = titlespan.innerHTML;
    yelpdata.appendChild(node);
    title_c = node;
  }

  var ui_expander_zoom_region = function (event) {
    if (yelpdata.getAttribute('data-yelp-expanded') != 'false') {
      ui_expander_toggle();
      event.preventDefault();
    }
  }
  if (expander.nodeName == 'section' || expander.nodeName == 'SECTION') {
    for (var i = 0; i < region.children.length; i++) {
      var child = region.children[i];
      if (child.classList.contains('contents')) {
        contents = child;
        break;
      }
    }
    contents.addEventListener('click', ui_expander_zoom_region, true);
  }
  else {
    region.addEventListener('click', ui_expander_zoom_region, true);
  }

  var ui_expander_toggle = function () {
    if (yelpdata.getAttribute('data-yelp-expanded') == 'false') {
      yelpdata.setAttribute('data-yelp-expanded', 'true');
      expander.classList.remove('ui-expander-e');
      expander.classList.add('ui-expander-c');
      region.setAttribute('aria-expanded', 'false');
      if (title_c != null)
        titlespan.innerHTML = title_c.innerHTML;
    }
    else {
      yelpdata.setAttribute('data-yelp-expanded', 'false');
      expander.classList.remove('ui-expander-c');
      expander.classList.add('ui-expander-e');
      region.setAttribute('aria-expanded', 'true');
      if (title_e != null)
        titlespan.innerHTML = title_e.innerHTML;
    }
  };
  expander.yelp_ui_expander_toggle = ui_expander_toggle;
  title.addEventListener('click', ui_expander_toggle, false);
  ui_expander_toggle();
}
document.addEventListener('DOMContentLoaded', function() {
  var matches = document.querySelectorAll('.ui-expander');
  for (var i = 0; i < matches.length; i++) {
    yelp_ui_expander_init(matches[i]);
  }
  var yelp_hash_ui_expand = function () {
    if (location.hash != '') {
      var sect = document.querySelector(location.hash);
      if (sect != null) {
        for (var cur = sect; cur instanceof Element; cur = cur.parentNode) {
          if (cur.classList.contains('ui-expander')) {
            if (cur.classList.contains('ui-expander-c')) {
              cur.yelp_ui_expander_toggle();
            }
          }
        }
        sect.scrollIntoView();
      }
    }
  };
  window.addEventListener('hashchange', yelp_hash_ui_expand, false);
  yelp_hash_ui_expand();
}, false);

yelp_color_text_light = 'rgb(75,75,73)';
yelp_color_gray_background = 'rgb(244,244,244)';
yelp_color_gray_border = '#c0bfbc';
function yelp_figure_init (figure) {
  var zoom = figure.querySelector('a.figure-zoom');

  var figure_resize = function () {
    var zoomed = zoom.classList.contains('figure-zoomed');
    var imgs = figure.querySelectorAll('img');
    for (var i = 0; i < imgs.length; i++) {
      var img = imgs[i];
      var mediaDiv = null;
      for (var cur = img; cur instanceof Element; cur = cur.parentNode) {
        if ((cur.nodeName == 'div' || cur.nodeName == 'DIV') &&
            cur.classList.contains('media')) {
          mediaDiv = cur;
          break;
        }
      }
      if (mediaDiv == null)
        continue;
      if (!img.hasAttribute('data-yelp-original-width')) {
        var iwidth = null;
        if (img.hasAttribute('width'))
          iwidth = parseInt(img.getAttribute('width'));
        else
          iwidth = img.width;
        img.setAttribute('data-yelp-original-width', iwidth);
        var iheight = null;
        if (img.hasAttribute('height'))
          iheight = parseInt(img.getAttribute('height'));
        else
          iheight = img.height * (iwidth / img.width);
        img.setAttribute('data-yelp-original-height', iheight);
      }
      var owidth = img.width;
      var oheight = img.height;
      img.width = parseInt(img.getAttribute('data-yelp-original-width'));
      img.height = parseInt(img.getAttribute('data-yelp-original-height'));
      var mediaw = mediaDiv.offsetWidth;
      img.width = owidth;
      img.height = oheight;
      if (parseInt(img.getAttribute('data-yelp-original-width')) <= mediaw) {
        img.width = parseInt(img.getAttribute('data-yelp-original-width'));
        img.height = parseInt(img.getAttribute('data-yelp-original-height'));
        zoom.style.display = 'none';
      }
      else if (zoomed) {
        img.width = parseInt(img.getAttribute('data-yelp-original-width'));
        img.height = parseInt(img.getAttribute('data-yelp-original-height'));
        zoom.style.display = 'block';
      }
      else {
        img.width = mediaw;
        img.height = (parseInt(img.getAttribute('data-yelp-original-height')) *
                      img.width /
                      parseInt(img.getAttribute('data-yelp-original-width')));
        zoom.style.display = 'block';
      }
    }
  }
  figure.yelp_figure_resize = figure_resize;
  figure.yelp_figure_resize();

  zoom.onclick = function (e) {
    var zoomed = zoom.classList.contains('figure-zoomed');
    if (zoomed)
      zoom.classList.remove('figure-zoomed');
    else
      zoom.classList.add('figure-zoomed');
    figure.yelp_figure_resize();
    return false;
  };
}
window.addEventListener('load', function() {
  var figures = document.querySelectorAll('div.figure');
  for (var i = 0; i < figures.length; i++) {
    if (figures[i].querySelector('img') != null)
      yelp_figure_init(figures[i]);
  }
  var timeout = null;
  var yelp_figures_resize = function () {
    if (timeout != null)
      return;
    timeout = window.setTimeout(function () {
      for (var i = 0; i < figures.length; i++) {
        if (figures[i].querySelector('img') != null)
          figures[i].yelp_figure_resize();
      }
      window.clearTimeout(timeout);
      timeout = null;
    }, 100);
  };
  window.addEventListener('resize', yelp_figures_resize, false);
}, false);
function yelp_media_init (media) {
  media.removeAttribute('controls');
  if (media.parentNode.classList.contains('links-tile-img')) {
    return;
  }

  media.addEventListener('click', function () {
    if (media.paused)
      media.play();
    else
      media.pause();
  }, false);

  var controls = null;
  for (var cur = media.nextSibling; cur instanceof Element; cur = cur.nextSibling) {
    if (cur.classList.contains('media-controls')) {
      controls = cur;
      break;
    }
  }
  if (controls == null) {
    media.setAttribute('controls', 'controls');
    return;
  }
  var playbutton = controls.querySelector('button.media-play');
  playbutton.addEventListener('click', function () {
    if (media.paused || media.ended)
      media.play();
    else
      media.pause();
  }, false);

  var mediachange = function () {
    if (media.ended)
      media.pause()
    if (media.paused) {
      playbutton.setAttribute('value', playbutton.getAttribute('data-play-label'));
      playbutton.classList.remove('media-play-playing');
    }
    else {
      playbutton.setAttribute('value', playbutton.getAttribute('data-pause-label'));
      playbutton.classList.add('media-play-playing');
    }
  }
  media.addEventListener('play', mediachange, false);
  media.addEventListener('pause', mediachange, false);
  media.addEventListener('ended', mediachange, false);

  var mediarange = controls.querySelector('input.media-range');
  mediarange.addEventListener('input', function () {
    var pct = this.value;
    if (pct < 0)
      pct = 0;
    if (pct > 100)
      pct = 100;
    media.currentTime = (pct / 100.0) * media.duration;
  }, false);
  var curspan = controls.querySelector('span.media-current');
  var durspan = controls.querySelector('span.media-duration');
  var durationUpdate = function () {
    if (!isNaN(media.duration)) {
      mins = parseInt(media.duration / 60);
      secs = parseInt(media.duration - (60 * mins));
      durspan.textContent = (mins + (secs < 10 ? ':0' : ':') + secs);
    }
  };
  media.addEventListener('durationchange', durationUpdate, false);

  var ttmlDiv = null;
  var ttmlNodes = null;
  for (var i = 0; i < media.parentNode.children.length; i++) {
    var child = media.parentNode.children[i];
    if (child.classList.contains('media-ttml'))
      ttmlDiv = child;
  }
  if (ttmlDiv != null) {
    ttmlNodes = ttmlDiv.querySelectorAll('.media-ttml-node');
  }

  var timeUpdate = function () {
    var pct = (media.currentTime / media.duration) * 100;
    mediarange.value = pct;
    var mins = parseInt(media.currentTime / 60);
    var secs = parseInt(media.currentTime - (60 * mins))
    curspan.textContent = (mins + (secs < 10 ? ':0' : ':') + secs);
    if (ttmlNodes != null) {
      for (var i = 0; i < ttmlNodes.length; i++) {
        var ttml = ttmlNodes[i];
        if (media.currentTime >= parseFloat(ttml.getAttribute('data-ttml-begin')) &&
            (!ttml.hasAttribute('data-ttml-end') ||
             media.currentTime < parseFloat(ttml.getAttribute('data-ttml-end')) )) {
          if (ttml.tagName == 'span' || ttml.tagName == 'SPAN')
            ttml.style.display = 'inline';
          else
            ttml.style.display = 'block';
        }
        else {
          ttml.style.display = 'none';
        }
      }
    }
  };
  media.addEventListener('timeupdate', timeUpdate, false);
};
document.addEventListener('DOMContentLoaded', function() {
  var matches = document.querySelectorAll('video, audio');
  for (var i = 0; i < matches.length; i++) {
    yelp_media_init(matches[i]);
  }
}, false);

document.addEventListener('DOMContentLoaded', function() {
  var tiles = document.querySelectorAll('div.links-tile');
  for (var i = 0; i < tiles.length; i++) {
    (function (tile) {
      if (!tile.parentNode.classList.contains('links-tiles') &&
          (tile.nextElementSibling &&
           tile.nextElementSibling.classList.contains('links-tile')) &&
          !(tile.previousElementSibling &&
            tile.previousElementSibling.classList.contains('links-tile'))) {
        var tilesdiv = document.createElement('div');
        tilesdiv.className = 'links-tiles';
        tile.parentNode.insertBefore(tilesdiv, tile);
        var cur = tile;
        while (cur && cur.classList.contains('links-tile')) {
          var curcur = cur;
          cur = cur.nextElementSibling;
          tilesdiv.appendChild(curcur);
        }
        for (j = 0; j < 2; j++) {
          var paddiv = document.createElement('div');
          paddiv.className = 'links-tile';
          tilesdiv.appendChild(paddiv);
        }
      }
    })(tiles[i]);
  }
});
document.addEventListener('DOMContentLoaded', function() {
  var overlays = document.querySelectorAll('a.ui-overlay');
  for (var i = 0; i < overlays.length; i++) {
    (function (ovlink) {
      var overlay = ovlink.parentNode.querySelector('div.ui-overlay');
      var ui_overlay_show = function (ev) {
        overlay.style.display = 'block';
        overlay.classList.add('ui-overlay-show');
        var screen = document.querySelector('div.ui-overlay-screen');
        if (screen == null) {
          screen = document.createElement('div');
          screen.className = 'ui-overlay-screen';
          document.body.appendChild(screen);
        }
        var inner = overlay.querySelector('div.inner');
        var close = inner.querySelector('a.ui-overlay-close');
        var media = inner.querySelectorAll('audio, video');

        var overlay_play_func = function () {
          for (var j = 0; j < media.length; j++) {
            media[j].play();
          }
        };
        var overlay_play_timeout = window.setTimeout(overlay_play_func, 1000);

        var ui_overlay_funcs = {};
        ui_overlay_funcs['hide'] = function () {
          overlay.style.display = 'none';
          document.body.removeChild(screen);
          document.removeEventListener('keydown', ui_overlay_funcs['keydown'], false);
          for (var j = 0; j < media.length; j++) {
            media[j].pause();
          }
          window.clearTimeout(overlay_play_timeout);
        };
        ui_overlay_funcs['hideclick'] = function (uiev) {
          ui_overlay_funcs['hide']();
          uiev.preventDefault();
        };
        ui_overlay_funcs['keydown'] = function (uiev) {
          if (uiev.keyCode == 27) {
            ui_overlay_funcs['hide']();
          }
        };
        screen.addEventListener('click', ui_overlay_funcs['hideclick'], false);
        close.addEventListener('click', ui_overlay_funcs['hideclick'], false);
        document.addEventListener('keydown', ui_overlay_funcs['keydown'], false);
        ev.preventDefault();
      };
      ovlink.addEventListener('click', ui_overlay_show, false);
    })(overlays[i]);
  }
});
