# An example varnish configuration file which will refuse all POST traffic.
# When used in conjunction with the htaccess file in this same directory, this
# will ensure that 1) no one can POST to Varnish and 2) only a specific group of
# users can POST to apache.
#
# This VCL file doesn't really stand on it's own.  For a more complete tutorial
# on Varnish for Drupal see https://www.digitalocean.com/community/tutorials/how-to-configure-varnish-for-drupal-with-apache-on-debian-and-ubuntu

/**
 * The location of the apache web server.
 */
backend default {
    .host = "127.0.0.1";
    .port = "8000";
    .max_connections = 250;
    .connect_timeout = 300s;
    .first_byte_timeout = 300s;
    .between_bytes_timeout = 300s;
}

/**
 * Refuse all post traffic.
 */
sub vcl_recv {
    if (req.request == "POST") {
        error 404 "No";
    }
}
