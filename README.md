# Drupalgeddon examples

[![youtube video](http://i.imgur.com/cEAL6dU.png)](http://www.youtube.com/watch?v=-ZSu7usB-F8)

The files included in this repository are a reverse engineered version of the files which were used against my site during the [Drupalgeddon](https://www.drupal.org/SA-CORE-2014-005) attacks.  Read more at [http://mattkorostoff.com/article/I-survived-drupalgeddon-how-hackers-took-over-my-site](http://mattkorostoff.com/article/I-survived-drupalgeddon-how-hackers-took-over-my-site)

# Explaination

## Attack files
* `exploit.php`: the main attack file.  This injects some SQL and installs 2 backdoors.
* `deobfuscate.php`: for reference only.  A deobfuscated version of the file which is uploaded on line 60 of exploit.php
* `execute-php.php`: must be used in conjunction with exploit.php.  Once exploit.php has finished running, `execute-php.php` can be used to run any php on the exploited server.
* `inject-sql.php`: a stand alone client for SQL injection.  This file may be used on its own without running any other file.
* `uploader.php`: for reference only.  a local version of the file which is fetched on line 175 of exploit.php

## Defense files
* `secure_file_permissions.sh`: A shell script which will set your file permissions to safe values
* `varnish_config.vcl`: an example VCL file which will drop all post traffic in Varnish
* `htaccess.txt`: works with the above VCL file.  creates a list of IP which are permitted to access apache directly without going through varnish.

## Reference files
* `exploited_code`: an exact copy of my code base, after it was attacked.
* `exploited_db.sql.zip`: an exact copy of my database, after it was attacked (passwords removed)
