<?php

/**
 * Themes a Fieldset with Thumbnails from Previews
 * @param object $node
 * @param array $thumbs Array containing Themed
 *    Thumbnails array(fid1=>array(thumb1,thumb2),fid2=>...) where
 *    thumb1 etc. is html Thumbnail
 * @return string themed Fieldset
 */
function theme_previews_fieldset($node,$thumbs) {
  global $user;
  //$thumbs = _previews_flattenArray($thumbs);
  $thumbs_html = '';
  foreach ($thumbs as $fid => $thumbgroup) {
    $thumbs_html .= theme('previews_thumbnail_group', $thumbgroup, $fid).'<div class="previews_thumb" style="height:60px;background:black;width:3px;">&nbsp;</div>';
  }
	if (count($thumbs)) {
		$fieldset = array(
		  '#title'       => '%title%',
		  '#collapsible' => variable_get('previews_collapsed', 0),
		  '#collapsed'   => variable_get('previews_collapsed', 0),
		  '#value'       => $thumbs_html,
      '#attributes'  => array('class'=>'previews_fieldset'),
		 );
    $out = theme('fieldset', $fieldset);
    if ($user->uid != 1) {
      cache_set('previews_thumbs_'._previews_dataUriSupport().'_'.$node->nid.'_'.$node->vid, $out,'cache',(time()+60*60*12));
    }
  }
  else {
    $out = '';
  }
  return $out;
}

function theme_previews_thumbnail_group($thumbs,$fid) {
  $value = implode('',$thumbs);
  return $value;
}

/**
 * Themes one Preview-Thumbnail
 *
 * @param string $thumbpath full path to the Image-thumb
 * @param integer $fid fid of the parent file
 * @param string $filepath full path to the parent File
 * @return string HTML of thumbnail
 */
function theme_previews_thumbnail($thumbpath,$fid,$filepath) {
  if (!strpos($thumbpath,'/images/ajax-loader.gif')) {
    $wxh = getimagesize(str_replace('/thumbnails/','/medium/',$thumbpath));
  }
  else {
    $wxh = array(0,0);
  }

  $img  = theme('image',
                _previews_dataUriSupport() ? theme('previews_imgURI', $thumbpath) : file_create_url($thumbpath),
                '',
                array_pop(explode('/',$filepath)),
                array(
                     'width' => '60px',
                     'height'=> '60px',

                    // 'id'    => 'previews_thumb_'.$fid,
                ),
                FALSE
              );
  if (_previews_dataUriSupport()) $img = str_replace('src="/','src="',$img);
  
  $attributes = array(
    'fid'    => $fid,
    'pid'    => (int) array_pop(explode('-', array_shift(explode('.',array_pop(explode('/',$thumbpath)))))),
    'source' => file_create_url($filepath),
    'width'  => $wxh[0],
    'height' => $wxh[1],
  );/*
  $out = '<span pid="'.$attributes['pid'].'" title="'.$attributes['title'].'" source="'.$attributes['source'].'" wxh="'.$attributes['wxh'].'">'.$img.'</span>';
  $attr = implode('|',$attributes);
  $out = '<span id="previews_thumb_'.$fid.$attributes['pid'].'" preview="'.$attr.'">'.$img.'</span>';
  */
  $out = '<span id="previews_thumb_'.$fid.$attributes['pid'].'" preview="'.$fid.'-'.$attributes['pid'].'">'.$img.'</span>';
  return '<div class="previews_thumb">'.$out.'</div>';
}


/**
 * Returns src for Image URI
 * @param <type> $path
 * @return string
 */
function theme_previews_imgURI($path) {
 return 'data:image/jpeg;base64,'.chunk_split(base64_encode(file_get_contents($path))).' ';
}
