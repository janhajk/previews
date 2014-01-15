<?php
/**
 * converts a pdf to images
 */
function _previews_convertPdf($file) {
    // checks if file exists at all
    if (!file_exists($file['filepath'])) return false;

    // get the user Setup and throw out files that are too big
    $setup = _previews_getSetup();
    //if (filesize($file['filepath']) > $setup['maxsize']) return false;

    // get numbers of pages and throw out those that are too big
    //
    // TODO: -1 means numpages could not be read
    // Those should be treated extra; for now they are not treated at all
    $numPages = _previews_getNumPagesPdf($file['filepath']);
    //if ($numPages > 100) return false;


    $path = file_directory_path().'/previews';

    // Create Type-Folder if it doesn't exist yet
    if (!is_dir($path.'/raw')) {
      if (!mkdir($path.'/raw')) drupal_set_message('konnte Ordner \''.$folder.'\' nicht erstellen!');
    }

    // Convert the PDF with Ghostscript into a set of png-images
    $command = "gs -sDEVICE=png16m -dNOPAUSE -dBATCH ".
     "-sOutputFile=".$path.'/raw/'.$file['fid']."-%03d.png".
     " -r".$setup['dpi']." -q ".$file['filepath'].'; chmod 777 '.$path.'/raw/'.$file['fid'].'*';
    $r = passthru($command);
    // Pre Table Update, so Files won't get converted twice if new cron-job comes in
    db_query("UPDATE {previews} SET preview = '".$file['timestamp']."', pages = '".$numPages."' WHERE fid = ".$file['fid']);
    //dsm($r);
    // Create Thumbnails and Screenviews
    $files = _previews_getFilesFromFid($file['fid'], 'raw');
    foreach ($files as $path=>$fileclass) { // basname: 1234-001.png
            $r = _previews_ImageScale($fileclass->basename, 'thumbnail');
            $r = _previews_ImageScale($fileclass->basename, 'screen');
            $r = _previews_ImageScale($fileclass->basename, 'medium');
            //$r = _previews_ImageScale($fileclass->basename, 'big');
            _previews_removeRaw($fileclass->basename);
    }
    return count($files);
}

/**
 * creates a screen-Viewing Image from a jpg or png
 * $file array(fid, path, timestamp)
 */
function _previews_convertImage($file) {
  // Maximum Pixels for width and height
  $max_side = 2500;

  $path = file_directory_path().'/previews';

  // Check if File exist
  if (!file_exists($file['filepath'])) return false;

	$image = new Imagick($file['filepath']);
  $image->setImageCompression(Imagick::COMPRESSION_JPEG);
  $image->setImageCompressionQuality(40);

  // Convert Image to a Maximum of $max_sidex$max_side for Screenviewing
    if ($image->getImageWidth() > $max_side || $image->getImageHeight() > $max_side) {
        $image->resizeImage($max_side,$max_side,Imagick::FILTER_QUADRATIC,1,true);
    }
  $image->setImageBackgroundColor('white');

	$image->writeImage($path.'/raw/'.$file['fid'].'.jpg');
  
	$image->clear();
	$image->destroy();
	return true;
}


/**
 * Scales an Image
 * there are 3 Formats that will be created
 * Thumbnail: Will be shown on node view on the bottom
 * Screen:    Opens in viewer after clicking on Thumbnail
 * big:       Is the image used for magnified area
 * string $filename ; exp. '1234-003.jpg'
 * $type screen/thumbnail/big
 */
function _previews_ImageScale($filename, $type) {
  include_once('resize-class.php');
  // Path to previews in filesystem
  $path = file_directory_path().'/previews';

  // File-Extension jpg or png
  $ext = strtolower(array_pop(explode('.',$filename)));
  
  $src = $path.'/raw/'.$filename;
  // Check if raw-File exist
  if (!file_exists($src) || !is_readable($src)) {echo 'Error: '.$src.'<br />'; return false;}

  switch ($type) {
    case 'screen':
      $max_side = 250;
      $q = 10;
      $folder   = 'screen';
      $new_ext  = 'jpg';
      break;
    case 'medium':
      $max_side = 2000;
      $q = 30;
      $folder   = 'medium';
      $new_ext  = 'jpg';
      break;
    case 'thumbnail':
      $max_side = 60;
      $q        = 40;
      $folder   = 'thumbnails';
      $new_ext  = 'jpg';
      break;
    case 'big':
      $max_side = 5000;
      $q        = 60;
      $folder   = 'big';
      $new_ext  = $ext;
      break;
  }

  $outputFileName = $path.'/'.$folder.'/'.str_replace('.'.$ext,'.'.$new_ext,$filename);
  list($width, $height, , ) = getimagesize($src);
  
  // Delete any old Files -> overwrite
  if (file_exists($outputFileName)) unlink ($outputFileName);
  
  $ratio = $width/$height;  
  $max_width  = ($width  > $height)?$max_side:$max_side*$ratio;
  $max_height = ($height > $width )?$max_side:$max_side/$ratio;
  
  $resizeObj = new resize($src);
  
  // Scale Images
  switch ($type) {
      case 'screen':
          if (!$width || $width > $max_side || $height > $max_side) {
            $resizeObj->resizeImage($max_width, $max_height, 0);
            $resizeObj -> saveImage($outputFileName, $q);
          }
          else copy($src, $outputFileName);
          break;
      case 'thumbnail':
          $resizeObj->resizeImage(60, 60, 'crop');
          $resizeObj->saveImage($outputFileName, $q);
          break;
      case 'big':
          if (!$width || $width > $max_side || $height > $max_side) {
            $resizeObj->resizeImage($max_width, $max_height, 0);
            $resizeObj -> saveImage($outputFileName, $q);
          }
          else copy($src, $outputFileName);
          break;
      case 'medium':
          if (!$width || $width > $max_side || $height > $max_side) {
            $resizeObj->resizeImage($max_width, $max_height, 0);
            $resizeObj -> saveImage($outputFileName, $q);
          }
          else copy($src, $outputFileName);
          break;
      default:          
  }
  // return Filename of the created scaled image
  return $outputFileName;
}


/**
 * returns the number of pages of a PDF-File
 * returns -1 if not found
 */
function _previews_getNumPagesPdf($filepath){
  $fp = @fopen($filepath,"r");
  $max=-1;
  while(!feof($fp)) {
    $line = fgets($fp,255);
    if (preg_match('/\/Count [0-9]+/', $line, $matches)){
      preg_match('/[0-9]+/',$matches[0], $matches2);
      if ($max<$matches2[0]) $max=$matches2[0];
    }
  }
  fclose($fp);
  return $max;
}


function _previews_removeRaw($filename) {
  $path = $path = file_directory_path().'/previews/raw/';
  return unlink($path.$filename);
}