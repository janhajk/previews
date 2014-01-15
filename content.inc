<?php

/**
 * returns all thumbnails for node
 * @param object $node
 * @param string $title Title for the Fieldset
 * @param boolean $useCache if set to false, cache is bypassed
 * @return formated html thumbnails in a fieldset
 */
function _previews_getcontent($node, $title=0, $useCache = true) {
  global $user;
  if (!$title) {
    $title = 'Datei Viewer <a href="/'.('previews/rebuildcache/'.$node->nid).'"><img src="/'.drupal_get_path('module', 'previews').'/images/update.png" width="16" height="16" /></a>';
  }

  // Drupal-Cache
  $cache = cache_get('previews_thumbs_'._previews_dataUriSupport().'_'.$node->nid.'_'.$node->vid);
  if ($cache && $useCache) {
    $js = cache_get('previews_thumbs_attributes_'.$node->nid.'_'.$node->vid);
    drupal_add_js(array('previews'=>array('thumbs'=>$js)), 'setting', 'header');
    return str_replace('%title%',$title,$cache->data);
  }

	$thumbs      = array();
  $thumbs_attr = array();
	$filefields  = _previews_getFilefields($node->type);
	foreach ($filefields as $filefield) {
		$files = $node->$filefield;
    if (isset($files[0]) && count($files[0])) {
		  foreach ($files as $file) {
        $previews = _previews_getFilesFromFid($file['fid'],'thumbnails');
		  	foreach ($previews as $filepath=>$preview) {
          list($fid, $pid) = explode('-',$preview->name);
          $thumbs_attr[$file['fid']][(int) $pid] = _previews_imgGetAttr($preview,$file);
          $thumbs[$file['fid']][]      = theme('previews_thumbnail',$filepath,$file['fid'],$file['filepath']);
		  	}
        // in Process icon if pdf has not been converted yet
        if (!count($previews) && $file['filemime'] == 'application/pdf') {
          $filepath = drupal_get_path('module', 'previews').'/images/ajax-loader.gif';
          $thumbs[$file['fid']][] = theme('previews_thumbnail',$filepath,$file['fid'],$file['filepath']);
        }
		  }
		}
	}
  if ($user->uid != 1) {
    cache_set('previews_thumbs_attributes_'.$node->nid.'_'.$node->vid, $thumbs_attr,'cache',(time()+60*60*12));
  }
  drupal_add_js(array('previews'=>array('thumbs'=>$thumbs_attr)), 'setting', 'header');
  $out = theme('previews_fieldset', $node, $thumbs);
  return str_replace('%title%',$title,$out);
}


/**
 * Updates the Preview table and adds the new Files
 * @return integer Count of new added Elements
 */
function _previews_updateTable() {
  // Insert new Files to Previews Table
  $fids = array();
  // Takes all the files from Files-Table that are pdf's and are not
  // present in previews-Table
  $sql = "".
      "SELECT {files}.fid, {files}.filepath ".
      "FROM {files} " .
  		"LEFT JOIN {previews} ".
      "ON ({files}.fid = {previews}.fid) " .
  		"WHERE ({files}.filemime = 'application/pdf' OR " .
        "{files}.filemime = 'image/jpeg' OR " .
        "{files}.filemime = 'image/png') AND " .
        "{previews}.fid IS NULL";
  //print '<pre>'.print_r(str_replace(array('{','}'),'',$sql),1).'</pre>';
  $result = db_query($sql);
  // Store new files in Array;
  while ($r = db_fetch_object($result)) {
//    print '<pre>'.print_r($r,1).'</pre>';
  	// Filter out Files from subdirecotries of files-Folder
  	if (strstr($r->filepath,'/dossier/')) {
    	$fids[] = $r->fid;
  	}
  }
  //print '<pre>'.print_r($fids,1).'</pre>';
  // Add new files to previews-Table
  if (count($fids)) {
	  foreach ($fids as $fid) {
	  	// TODO: Use drupal_write_record()
		$sql = "INSERT INTO {previews} (fid) VALUES ('%d')";
		db_query($sql, $fid);
	  }
  }
  return count($fids);
}

/**
 * Selects amount of Files to Convert
 * @return array (fid, filepath, timestamp) of Files
 */
function _previews_getFilesToConvert() {
  $fids = array();
  $max_day = 5;
  $max_night = 5;
  $hour = (int) date("H",time());
  $count = ($hour < 8 || $hour > 22)?$max_night:$max_day;
  $sql = "SELECT {previews}.fid, {files}.filepath, {files}.timestamp FROM
      {previews} LEFT JOIN {files} ON ({previews}.fid = {files}.fid)
      WHERE {previews}.preview = 0
      ORDER BY fid DESC LIMIT 0,%d";
  $results = db_query($sql,$count);
  while ($r = db_fetch_array($results)) {
    $fids[] = $r;
  }
  return $fids;
}


/**
 * returns the properties of the conversion
 * @param string $kind if set, returns specific setup key
 * @return array
 */
function _previews_getSetup($kind=0) {
	static $setup;
	if (count($setup) && $kind) return $setup[$kind];

  $setup = array(
		'type'		=> variable_get('previews_filetype','png'),
		'color'		=> variable_get('previews_color','Gray'),
		'dpi'		  => variable_get('previews_dpi', '300'),
		'maxsize' => variable_get('previews_maxsize', '1')*1000*1024,
    'lightbox'=> variable_get('previews_lightbox',''),
	);
  if ($kind) {
    return $setup[$kind];
  }
	return $setup;
}


/**
 * returns an array with the Javascript Attributes for the Preview File
 * @param object $file The File Object of the Thumbnail
 * @return array Array containing the Attributes of the Preview-Image 
 */
function _previews_imgGetAttr($file, $parentFile) {
  //drupal_set_message('<pre>'.print_r($file, 1).'</pre>');
  $attr = array();
  list($fid, $tid) = explode('-',$file->name);

  $attr['thumb_id']       = $file->name;
  $nextId                 = substr('000'.($tid+1),strlen('000'.($tid+1))-3,3);
  $attr['next_thumb_id']  = file_exists(preg_replace('|\-\d\d\d\.|','-'.$nextId.'.',$file->filename))?$fid.'-'.$nextId:0;
  $size                   = getimagesize(str_replace('/thumbnails/','/medium/',$file->filename));
  $attr['imagesize']      = array('width' => $size[0],'height' => $size[1]);
  $attr['parent']['type'] = pathinfo($parentFile['filepath'], PATHINFO_EXTENSION);
  $attr['parent']['name'] = pathinfo($parentFile['filepath'], PATHINFO_BASENAME);
  $attr['parent']['path'] = $parentFile['filepath'];
  $attr['parent']['url']  = file_create_url($parentFile['filepath']);
  
  return $attr;
}