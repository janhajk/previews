<?php

/**
 * Get's the $nid from a File of a CCK File-Field $fid
 */
function _previews_fid2nid($fid) {
	// Get all the CCK-File-Fields
	$fields = array();
	$sql = "SELECT field_name FROM {content_node_field} WHERE type LIKE 'filefield';";
	$result = db_query($sql);
	while ($r = db_fetch_array($result)) {
		$fields[] = $r['field_name'];
	}
	$found = false;
	
	foreach ($fields as $field) {
		// Checks if the table exists
		$sql = "SHOW TABLES LIKE 'content_".$field."'";
		$result = db_query($sql);
		$i = false;
		while ($r = db_fetch_array($result)) { $i = true; }
		// If it does, see if fid is in this table
		if ($i) {
			$sql = "SELECT content_".$field.".nid FROM files 
        LEFT JOIN content_".$field."
        ON (files.fid = content_".$field.".".$field."_fid)
        WHERE ".$field."_fid = '%d'";
			$result = db_query($sql, $fid);
			while ($r = db_fetch_array($result)) {
				$nid = $r['nid'];
			}
			if (count($nid)) {break;} // if fid is in table, stop while and return nid
		}
	}
	return $nid;
	
}

/**
 * This function returns all cck Filefields of a specific Nodetype
 * @param string $nodeType node type as in node-Table
 * @return array array with all the FileFields-Names
 */
function _previews_getFilefields($nodeType) {
   $filefields = array();
   $sql = "SELECT type_name, field_name FROM content_node_field_instance
       WHERE widget_module LIKE 'filefield'
       AND type_name LIKE '%s'";
   $result = db_query($sql, $nodeType);
   while ($r = db_fetch_array($result)) {
   	 $filefields[] = $r['field_name'];
   }
   return $filefields;
}




/**
 * gets all Images that are available for an fid from the raw-Folder
 */
function _previews_getFilesFromFid($fid, $folder) {
	$previews = array();
	$path = file_directory_path().'/previews/'.$folder;
  $previews = file_scan_directory($path,'^'.$fid.'[\.-]');
  asort($previews);
  return $previews;
}

/*
 * Checks if data URI is supported
 */
function _previews_dataUriSupport() {
  return !preg_match('/(?i)msie [1-7]/',$_SERVER['HTTP_USER_AGENT']);
}

function _previews_flattenArray($arr) {
  $output = array();
  array_walk_recursive($arr, create_function('$val, $key, $obj', 'array_push($obj, $val);'), &$output);
  return $output;
}


/**
 * Untersucht, ob Server bereit ist für PDF Rendering
 *
 * schaut zuerst, ob Ghostscript (gs) noch läuft. Ist dies der Fall
 * wird true zurückgegeben
 * Als nächstes wird geschaut, ob die CPU-Last grösser als 60% ist. Ist dies
 * der Fall wird ebenfalls true zurückgegeben
 * ansonsten false
 * @return boolean true, wenn Server ausgelastet/bzw bereits am Konvertieren ist,
 *                 false, wenn server bereit ist
 */
function _previews_isStillRunning() {
  // Check if other Ghostscript-conversions are in progress
  // abort if this is the case to avoid memory crashes
  if(preg_match('|gs\s\-sDEVICE|',shell_exec('ps -aux'))) {
  	return true;
  }
  // Check also if CPU is running over 40%
  else {
    $cpu = _previews_getCpuLoad();
    if ($cpu['us'] > 40){
      return true;
    }
    else {
      return false;
    }
  }
}


function _previews_getCpuLoad() {
  $output = array();
  exec("top -b -n 1 -d 1 | grep ^Cpu", $top);
  preg_match('|Cpu\(s\):(.*)|',$top[0],$results);
  $cpu = explode(',',$results[1]);
  foreach ($cpu as $key=>$c) {
    $pair = explode('%',$c);
    $output[trim($pair[1])] = (float)trim($pair[0]);

  }
  return $output ;
}


/**
 * Creates Previews Folder Tree if non existent
 */
function _previews_CreateFolderTree() {
  $folders = array(
      '',   // Creates Previews Folder itself
      '/raw',
      '/thumbnails',
      '/screen',
      '/medium',
      '/big',
  );
  
  $previewsPath = file_directory_path().'/previews';
  
  foreach ($folders as $folder) {
    if (!is_dir($previewsPath.$folder)) {
      if (!mkdir($previewsPath.$folder)) {
        drupal_set_message(t('Das System konnte den Ordner \'%folder\' nicht erstellen!', array('%folder' => $folder)));
      }
    }
  }
}
  