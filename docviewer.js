/**
 * geplante Objekt Version
 */
var pViewer = function() {
 // ...
};




$(document).ready(function() {
  previews_init();
  pv_setBrowserExtra();
});

/**
 * Initalisiert den Viewer
 */
var previews_init = function() {
    $('body').prepend('<img id="previews_docviewer_img_preloader_medium" style="display:none;" />');
    $('body').prepend('<img id="previews_docviewer_img_preloader_next"   style="display:none;" />');
    $('.previews_thumb span').css('cursor','pointer').click(function(){
      previews_docviewer_startOverlay( $(this).attr('preview') );
      // close docviewer on Escape-Key
      $(document).unbind('keydown').keydown(function(e){
        if (e.keyCode == 27) {
          previews_docviewer_close();
        }
      });
    });  
};

/**
 * Schliesst den Viewer
 * - Scrollbars wieder anzeigen
 * - Overlays und Viewer-Container entfernen
 * - keydown Ereignisse entfernen
 * - Lupe/Powerzoom entfernen
 */
var previews_docviewer_close = function() {
  $(document).unbind('keydown');
  $('#previews_docviewer_img').addimagezoom(false);
  $('#previews_docviewer_container').remove();
  $('#previews_docviewer_overlay').remove();
  $('html, body').css('overflow', 'auto');
}

/**
 * Viewer öffnen
 * macht
 *
 */
var previews_docviewer_startOverlay = function(preview) {
  var attr        = preview.split('-');
  var fid         = parseInt(attr[0],10);
  var pid         = parseInt(attr[1],10);
  
  var filespath   = Drupal.settings.previews.images.filespath;
  var oFile       = (Drupal.settings.previews.thumbs.data==undefined)?Drupal.settings.previews.thumbs[fid][pid]:Drupal.settings.previews.thumbs.data[fid][pid];
  var imgWidth    = oFile.imagesize.width;
  var imgHeight   = oFile.imagesize.height;

  var screen      = filespath + 'screen/' + fid.toString() + '-' + gPid(pid) + '.' + 'jpg';
  var medium      = filespath + 'medium/' + fid.toString() + '-' + gPid(pid) + '.' + 'jpg';
  //var big         = filespath + 'big/'    + fid.toString() + '-' + gPid(pid) + '.' + (oFile.parent.type==='pdf'?'png':'jpg');

  previews_createLightbox();

  //position it correctly after downloading
	$('#previews_docviewer_img').unbind().removeAttr('src').attr('src',screen);
  var w     = $(window).width();
  var h     = $(window).height();
  var ratio = imgWidth/imgHeight;

  // Bild auf Bildschirmgrösse Verkleinern
  if (imgWidth > w) {
    imgWidth  = w;
    imgHeight = w / ratio;
  }
  if (imgHeight > h) {
    imgHeight = h;
    imgWidth  = h * ratio;
  }
  $('#previews_docviewer_container').css({
      'width' : imgWidth,
      'height': imgHeight
  });
  $('#previews_docviewer_img').attr({
    'width':  imgWidth-20,
    'height': imgHeight-40
  });
  pv_center(imgWidth, imgHeight);
  $('#previews_docviewer_container').animate({opacity:1}, 400); // show

  $('#previews_docviewer_img_preloader_medium').unbind().removeAttr('src').attr('src',medium).load(function() {
    $('#previews_docviewer_img').removeAttr('src').attr('src',medium);
    // Check if next Thumbnail exists and preload if so
    if (oFile.next_thumb_id) {
      var next = filespath + 'screen/' + fid.toString() + '-' + gPid(pid+1) + '.' + 'jpg';
      //$('#previews_docviewer_img_preloader_next').attr({src:next});
    }
  });

  // Bei Click auf Bild nächstes Bild anzeigen
  $('#previews_docviewer_img').unbind('click').click(function(){
    oFile.next_thumb_id?previews_docviewer_startOverlay(oFile.next_thumb_id):previews_docviewer_close();
  });

  $('#previews_images_mag').unbind('click').click(function () {
    if ($('.magnifyarea').length) {
      $('#previews_docviewer_img').addimagezoom(false);
    }
    else {
      $('#previews_docviewer_img').addimagezoom({
        zoomrange: [2, 8],
        magnifiersize: [400,400],
        magnifierpos: 'right',
        cursorshade: true,
        cursorshadecolor: 'pink',
        cursorshadeopacity: 0.3,
        cursorshadeborder: '1px solid red'
      });
    }
  });
  $('#previews_images_source').attr('href',oFile.parent.url);
  $('#previews_images_source').html(shortenString(oFile.parent.name));
};

/**
 * erstellt HTML für Lightbox
 */
var previews_createLightbox = function() {
  $('#previews_docviewer_img').unbind().removeAttr('src');
  $('#previews_docviewer_img_preloader_medium').unbind().removeAttr('src');
  
  if (! $('#previews_docviewer_overlay').length) {
    var close     = '<div id="previews_images_close"></div>';
    var mag       = '<div id="previews_images_mag"></div>';
    var source    = '<a id="previews_images_source">&nbsp;</a>';
    var toolbar   = '<div id="previews_docviewer_toolbar">Download: '+source+close+mag+'</div>';
    var container = '<div id="previews_docviewer_container"><img id="previews_docviewer_img" />'+toolbar+'</div>';
    var overlay   = '<div id="previews_docviewer_overlay"></div>';

    $('body, html').css('overflow', 'hidden');
    $('body').append(overlay+container);
    $('#previews_docviewer_overlay').css({opacity:0}).stop().show().stop().animate({opacity:0.6}, 400);
    $('#previews_docviewer_overlay, #previews_images_close').click(function(){
      previews_docviewer_close();
    });
  }
};


/**
 * Erstellt eine ID mit führenden Nullen
 */
var gPid = function(pid) {
  return ('000' + pid.toString()).substring(('000' + pid.toString()).length-3);
};

var pv_center = function(pvWidth,pcHeight) {
  $('#previews_docviewer_container').css({marginLeft: '-' + parseInt((pvWidth / 2),10) + 'px', width: pvWidth + 'px'});
  if (!($.browserextra.msie6)) { // take away IE6
    $('#previews_docviewer_container').css({marginTop: '-' + parseInt((pcHeight / 2),10) + 'px'});
  }
};

var pv_setBrowserExtra = function() {
  // Return if already set.
  if ($.browserextra) {
    return;
  }
  // Add iPhone, IE 6 and Mac Firefox browser detection.
  // msie6 fixes the fact that IE 7 now reports itself as MSIE 6.0 compatible
  var userAgent = navigator.userAgent.toLowerCase();
  $.browserextra = {
    iphone: /iphone/.test( userAgent ),
    msie6: /msie/.test( userAgent ) && !/opera/.test( userAgent ) && /msie 6\.0/.test( userAgent ) && !/msie 7\.0/.test( userAgent ) && !/msie 8\.0/.test( userAgent ),
    macfirefox: /mac/.test( userAgent ) && /firefox/.test( userAgent )
  };
};


var shortenString = function(s) {
  var len = 30;
  if (s.length > len) {
    return s.substr(0,len/2) + '...' + s.substr(-len/2);
  }
  else {
    return s;
  }
};





















/*Featured Image Zoomer (May 8th, 2010)
* This notice must stay intact for usage
* Author: Dynamic Drive at http://www.dynamicdrive.com/
* Visit http://www.dynamicdrive.com/ for full source code
*/

// Feb 21st, 2011: Script updated to v1.5, which now includes new feature by jscheuer1 (http://www.dynamicdrive.com/forums/member.php?u=2033) to show optional "magnifying lens" while over thumbnail image.
// March 1st, 2011: Script updated to v1.51. Minor improvements to inner workings of script.


jQuery('head').append('<style type="text/css">.featuredimagezoomerhidden {visibility: hidden!important;}</style>');

var featuredimagezoomer={
	magnifycursor: 'crosshair', //Value for CSS's 'cursor' attribute, added to original image

	/////NO NEED TO EDIT BEYOND HERE////////////////
	dsetting: { //default settings
			magnifierpos: 'right',
			magnifiersize:[200, 200],
			cursorshadecolor: '#fff',
			cursorshadeopacity: 0.3,
			cursorshadeborder: '1px solid black',
			cursorshade: false,
			leftoffset: 15, //offsets here are used (added to) the width of the magnifyarea when
			rightoffset: 10 //calculating space requirements and to position it visa vis any drop shadow
		},
	isie: (function(){/*@cc_on @*//*@if(@_jscript_version >= 5)return true;@end @*/return false;})(), //is this IE?

	showimage: function($, $tracker, $mag, showstatus){
		var specs=$tracker.data('specs'), d=specs.magpos, fiz=this;
		var coords=$tracker.data('specs').coords //get coords of tracker (from upper corner of document)
		specs.windimensions={w:$(window).width(), h:$(window).height()}; //remember window dimensions
		var magcoords={} //object to store coords magnifier DIV should move to
		magcoords.left = coords.left + (d === 'left'? -specs.magsize.w - specs.lo : $tracker.width() + specs.ro);
		//switch sides for magnifiers that don't have enough room to display on the right if there's room on the left:
		if(d!=='left' && magcoords.left + specs.magsize.w + specs.lo >= specs.windimensions.w && coords.left - specs.magsize.w >= specs.lo){
			magcoords.left = coords.left - specs.magsize.w - specs.lo;
		} else if(d==='left' && magcoords.left < specs.ro) { //if there's no room on the left, move to the right
			magcoords.left = coords.left + $tracker.width() + specs.ro;
		}
		$mag.css({left: magcoords.left, top:coords.top}).show(); //position magnifier DIV on page
		specs.$statusdiv.html('Current Zoom: '+specs.curpower+'<div style="font-size:80%">Use Mouse Wheel to Zoom In/Out</div>');
		if (showstatus) //show status DIV? (only when a range of zoom is defined)
			fiz.showstatusdiv(specs, 400, 2000);
	},

	hideimage: function($tracker, $mag, showstatus){
		var specs=$tracker.data('specs');
		$mag.hide();
		if (showstatus)
			this.hidestatusdiv(specs);
	},

	showstatusdiv: function(specs, fadedur, showdur){
		clearTimeout(specs.statustimer)
		specs.$statusdiv.fadeIn(fadedur) //show status div
		specs.statustimer=setTimeout(function(){featuredimagezoomer.hidestatusdiv(specs)}, showdur) //hide status div after delay
	},

	hidestatusdiv: function(specs){
		specs.$statusdiv.stop(true, true).hide()
	},

	getboundary: function(b, val, specs){ //function to set x and y boundaries magnified image can move to (moved outside moveimage for efficiency)
		if (b=="left"){
			var rb=-specs.imagesize.w*specs.curpower+specs.magsize.w
			return (val>0)? 0 : (val<rb)? rb : val
		}
		else{
			var tb=-specs.imagesize.h*specs.curpower+specs.magsize.h
			return (val>0)? 0 : (val<tb)? tb : val
		}
	},

	moveimage: function($tracker, $maginner, $cursorshade, e){
		var specs=$tracker.data('specs'), csw = Math.round(specs.magsize.w/specs.curpower), csh = Math.round(specs.magsize.h/specs.curpower),
		csb = specs.csborder, fiz = this, imgcoords=specs.coords, pagex=(e.pageX || specs.lastpagex), pagey=(e.pageY || specs.lastpagey),
		x=pagex-imgcoords.left, y=pagey-imgcoords.top;
		$cursorshade.css({ // keep shaded area sized and positioned proportionately to area being magnified
			visibility: '',
			width: csw,
			height: csh,
			top: Math.min(specs.imagesize.h-csh-csb, Math.max(0, y-(csb+csh)/2)) + imgcoords.top,
			left: Math.min(specs.imagesize.w-csw-csb, Math.max(0, x-(csb+csw)/2)) + imgcoords.left
		});
		var newx=-x*specs.curpower+specs.magsize.w/2 //calculate x coord to move enlarged image
		var newy=-y*specs.curpower+specs.magsize.h/2
		$maginner.css({left:fiz.getboundary('left', newx, specs), top:fiz.getboundary('top', newy, specs)})
		specs.$statusdiv.css({left:pagex-10, top:pagey+20})
		specs.lastpagex=pagex //cache last pagex value (either e.pageX or lastpagex), as FF1.5 returns undefined for e.pageX for "DOMMouseScroll" event
		specs.lastpagey=pagey
	},

	magnifyimage: function($tracker, e, zoomrange){
		var delta=e.detail? e.detail*(-120) : e.wheelDelta //delta returns +120 when wheel is scrolled up, -120 when scrolled down
		var specs=$tracker.data('specs')
		var magnifier=specs.magnifier, od=specs.imagesize, power=specs.curpower
		var newpower=(delta>0)? Math.min(power+1, zoomrange[1]) : Math.max(power-1, zoomrange[0]) //get new power
		var nd=[od.w*newpower, od.h*newpower] //calculate dimensions of new enlarged image within magnifier
		magnifier.$image.css({width:nd[0], height:nd[1]})
		specs.curpower=newpower //set current power to new power after magnification
		specs.$statusdiv.html('Current Zoom: '+specs.curpower)
		this.showstatusdiv(specs, 0, 500)
		$tracker.trigger('mousemove')
	},

	init: function($, $img, options){
    if (options === false) {
      $('.magnifyarea, .cursorshade, .zoomstatus, .zoomtracker').remove();
      return true;
    }
		var setting=$.extend({}, this.dsetting, options), w = $img.width(), h = $img.height(), o = $img.offset(),
		fiz = this, $tracker, $cursorshade, $statusdiv, $magnifier, lastpage = {pageX: 0, pageY: 0};
		setting.largeimage = setting.largeimage || $img.get(0).src;
		$magnifier=$('<div class="magnifyarea" style="z-index:101;position:absolute;width:'+setting.magnifiersize[0]+'px;height:'+setting.magnifiersize[1]+'px;left:-10000px;top:-10000px;visibility:hidden;overflow:hidden;border:1px solid black;" />')
			.append('<div style="position:relative;left:0;top:0;" />')
			.appendTo(document.body) //create magnifier container
		//following lines - create featured image zoomer divs, and absolutely positioned them for placement over the thumbnail and each other:
		if(setting.cursorshade){
			$cursorshade = $('<div class="cursorshade" style="visibility:hidden;position:absolute;left:0;top:0;z-index:101" />')
				.css({border: setting.cursorshadeborder, opacity: setting.cursorshadeopacity, backgroundColor: setting.cursorshadecolor})
				.appendTo(document.body);
		} else {
			$cursorshade = $('<div />'); //dummy shade div to satisfy $tracker.data('specs')
		}
		$statusdiv = $('<div class="zoomstatus preloadevt" style="position:absolute;visibility:hidden;left:0;top:0;z-index:101" />')
			.html('')
			.appendTo(document.body); //create DIV to show "loading" gif/ "Current Zoom" info
		$tracker = $('<div class="zoomtracker" style="z-index:101;cursor:progress;position:absolute;left:'+o.left+'px;top:'+o.top+'px;height:'+h+'px;width:'+w+'px;" />')
			.css({backgroundImage: (this.isie? 'url(cannotbe)' : 'none')})
			.appendTo(document.body);
		$(window).resize(function(){ //in case resizing the window repostions the image
				var o = $img.offset();
				$tracker.css({left: o.left, top: o.top});
			});

		function getspecs($maginner, $bigimage){ //get specs function
			var magsize={w:$magnifier.width(), h:$magnifier.height()}
			var imagesize={w:w, h:h}
			var power=(setting.zoomrange)? setting.zoomrange[0] : ($bigimage.width()/w).toFixed(5)
			$tracker.data('specs', {
				$statusdiv: $statusdiv,
				statustimer: null,
				magnifier: {$outer:$magnifier, $inner:$maginner, $image:$bigimage},
				magsize: magsize,
				magpos: setting.magnifierpos,
				imagesize: imagesize,
				curpower: power,
				coords: getcoords(),
				csborder: $cursorshade.outerWidth(),
				lo: setting.leftoffset,
				ro: setting.rightoffset
			})
		}

		function getcoords(){ //get coords of thumb image function
			var offset=$tracker.offset() //get image's tracker div's offset from document
			return {left:offset.left, top:offset.top}
		}

		$tracker.mouseover(function(e){
					$cursorshade.add($magnifier).add($statusdiv).removeClass('featuredimagezoomerhidden');
					$tracker.data('premouseout', false);
			}).mouseout(function(e){
					$cursorshade.add($magnifier).add($statusdiv.not('.preloadevt')).addClass('featuredimagezoomerhidden');
					$tracker.data('premouseout', true);
			}).mousemove(function(e){ //save tracker mouse position for initial magnifier appearance, if needed
				lastpage.pageX = e.pageX;
				lastpage.pageY = e.pageY;
			});

		$tracker.one('mouseover', function(e){
			var $maginner=$magnifier.find('div:eq(0)')
			var $bigimage=$('<img src="'+setting.largeimage+'"/>').appendTo($maginner)
			var showstatus=setting.zoomrange && setting.zoomrange[1]>setting.zoomrange[0]
			$img.css({opacity:0.1}) //"dim" image while large image is loading
			var imgcoords=getcoords()
			$statusdiv.css({left:imgcoords.left+w/2-$statusdiv.width()/2, top:imgcoords.top+h/2-$statusdiv.height()/2, visibility:'visible'})
			$bigimage.bind('loadevt', function(){ //magnified image ONLOAD event function (to be triggered later)
				$img.css({opacity:1}) //restore thumb image opacity
				$statusdiv.empty().css({border:'1px solid black', background:'#C0C0C0', padding:'4px', font:'bold 13px Arial', opacity:0.8}).hide().removeClass('preloadevt');
				if($tracker.data('premouseout')){
					$statusdiv.addClass('featuredimagezoomerhidden');
				}
				if (setting.zoomrange){ //if set large image to a specific power
					var nd=[w*setting.zoomrange[0], h*setting.zoomrange[0]] //calculate dimensions of new enlarged image
					$bigimage.css({width:nd[0], height:nd[1]})
				}
				getspecs($maginner, $bigimage) //remember various info about thumbnail and magnifier
				$magnifier.css({display:'none', visibility:'visible'})
				$tracker.mouseover(function(e){ //image onmouseover
					$tracker.data('specs').coords=getcoords() //refresh image coords (from upper left edge of document)
					fiz.showimage($, $tracker, $magnifier, showstatus)
				})
				$tracker.mousemove(function(e){ //image onmousemove
					fiz.moveimage($tracker, $maginner, $cursorshade, e)
				})
				if (!$tracker.data('premouseout')){
					fiz.showimage($, $tracker, $magnifier, showstatus);
					fiz.moveimage($tracker, $maginner, $cursorshade, lastpage);
				}
				$tracker.mouseout(function(e){ //image onmouseout
					fiz.hideimage($tracker, $magnifier, showstatus)
				}).css({cursor: fiz.magnifycursor});
				if (setting.zoomrange && setting.zoomrange[1]>setting.zoomrange[0]){ //if zoom range enabled
					$tracker.bind('DOMMouseScroll mousewheel', function(e){
						fiz.magnifyimage($tracker, e, setting.zoomrange);
						e.preventDefault();
					});
				}
			})	//end $bigimage onload
			if ($bigimage.get(0).complete){ //if image has already loaded (account for IE, Opera not firing onload event if so)
				$bigimage.trigger('loadevt')
			}
			else{
				$bigimage.bind('load', function(){$bigimage.trigger('loadevt')})
			}
		})
	},

	iname: (function(){var itag = jQuery('<img />'), iname = itag.get(0).tagName; itag.remove(); return iname;})()
};

jQuery.fn.addimagezoom=function(options){
	var $=jQuery;
	return this.each(function(){ //return jQuery obj
		if (this.tagName !== featuredimagezoomer.iname)
			return true; //skip to next matched element
		featuredimagezoomer.init($, $(this), options);
	});
};