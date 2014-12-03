#!/bin/sh
#
# To set you file permissions in a secure manner, change path/to/drupal/docroot
# to your actual Drupal docroot and your_user_name to your actual unix username.
# www-data
#
# @see https://www.drupal.org/node/244924

#########################################
#             SETTINGS                  #
#########################################
#this should be the folder where index.php lives
drupal_root="/Users/matt/Sites/exploitable.local/docroot"

#These two settings are probable safe to leave unchanged.
#See: finding your apache username http://serverfault.com/questions/125865/finding-out-what-user-apache-is-running-as
admin_account="root"
apache_username="www-data"
#########################################
#             END SETTINGS              #
#########################################

#Forbid apache from writing to your Drupal application files
cd ${drupal_root}
chown -R ${admin_account}:${apache_username} .
find . -type d -exec chmod u=rwx,g=rx,o= '{}' \;
find . -type f -exec chmod u=rw,g=r,o= '{}' \;

# Set proper permissions on the /sites/*/files directories.  These directories
# need to be writable by apache but not writable by anyone else.
cd /${drupal_root}/sites
find . -type d -name files -exec chmod ug=rwx,o= '{}' \;
find . -type f -name settings.php -exec chmod 440 '{}' \;
for d in ./*/files
do
   find $d -type d -exec chmod ug=rwx,o= '{}' \;
   find $d -type f -exec chmod ug=rw,o= '{}' \;
done
