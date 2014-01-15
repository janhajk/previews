<?php
// $Id:

/**
 * @file
 * Admin settings for Previews module.
 */


/*
 * ToDo
 * Button to Reset all Previews
 *  TRUNCATE TABLE `previews`
 *  Delete all Files in Previews Folder
 */

/**
 * Declares administrative settings for the module.
 */
function previews_admin_settings() {
  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  
  $form['general']['previews_collapsed'] = array(
    '#type' => 'radios', 
    '#title' => t('Preview is collapsed?'), 
    '#default_value' => variable_get('previews_collapsed', 0), 
    '#options' => array(t('no'), t('yes')),
  );
  $form['general']['previews_lightbox'] = array(
    '#type' => 'select',
    '#title' => t('Lightbox-Type'),
    '#default_value' => variable_get('previews_lightbox',''),
    '#options' => array(
      ''         => t('none'),
      'thickbox' => t('Thickbox'),
      'colorbox' => t('Colorbox'),
  ));

  $form['pdf'] = array(
    '#type' => 'fieldset',
    '#title' => t('PDF2Image'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Set the settings for PDF to Image Conversion')
  );

  $form['pdf']['previews_dpi'] = array(
    '#type'          => 'textfield',
    '#title'         => t('DPI Resolution of Image'),
    '#default_value' => variable_get('previews_dpi', '300'),
    '#size'          => 4,
    '#maxlength'     => 4,
    '#description'   => t('Give the Resolution of the Images in dpi, exp. 300 for 300dpi'),
  );
  $form['pdf']['previews_maxsize'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Maximum of PDF Size being converted in MB'),
    '#default_value' => variable_get('previews_maxsize', '1'),
    '#size'          => 4,
    '#maxlength'     => 4,
    '#description'   => t(''),
  );
  $form['pdf']['previews_color'] = array(
  '#type' => 'select', 
  '#title' => t('Conversion Colors'), 
  '#default_value' => variable_get('previews_color','Gray'),
  '#options' => array(
    'Gray' => t('Grayscale'), 
    'RGB' => t('Color (takes up more space)'), 
  ));
  $form['pdf']['previews_filetype'] = array(
  '#type' => 'select', 
  '#title' => t('Filetype to convert previews to'), 
  '#default_value' => variable_get('previews_filetype','png'),
  '#options' => array(
    'jpg' => t('jpg'), 
    'png' => t('png'), 
  ));
  return system_settings_form($form);
}

function previews_CacheFlush() {
  $sql = "DELETE FROM {cache} WHERE cid LIKE 'previews_thumbs%'";
  db_query($sql);
  drupal_set_message('Previews Cache Flushed!','warning');
  drupal_goto('<front>');
}

function previews_RebuildCache() {
  db_query("DELETE FROM {cache} WHERE cid LIKE 'previews%'");
  $nodes = array();
  $results = db_query("SELECT * FROM {node}");
  while ($r = db_fetch_object($results)) {
    $nodes[] = $r;
  }
  foreach ($nodes as $node) {
    $node = node_load($node->nid);
    _previews_getcontent($node,'',false);
  }
  drupal_set_message('Previews Cache Rebuilt!','warning');
  drupal_goto('<front>');
}


function previews_RebuildCacheNode($nid) {
  $vid = db_fetch_object(db_query("SELECT vid FROM {node_revisions} WHERE nid = '%d' ORDER BY vid DESC LIMIT 0,1",$nid))->vid;
  db_query("DELETE FROM {cache} WHERE cid LIKE 'previews_thumbs_0_%d_%d'",$nid,$vid);
  db_query("DELETE FROM {cache} WHERE cid LIKE 'previews_thumbs_1_%d_%d'",$nid,$vid);
  $nodes = array();
  $results = db_query("SELECT * FROM {node} WHERE nid = '%d'",$nid);
  while ($r = db_fetch_object($results)) {
    $nodes[] = $r;
  }
  foreach ($nodes as $node) {
    $node = node_load($node->nid);
    _previews_getcontent($node,'',false);
  }
  drupal_set_message(t('Previews Cache Rebuilt for Node %nid',array('%nid'=>$nid)),'warning');
  drupal_goto('/node/'.$nid);
}