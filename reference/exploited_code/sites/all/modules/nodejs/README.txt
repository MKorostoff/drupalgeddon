Node.js integration
===================

This module adds Node.js integration to Drupal.

Setup
=====

1. Install Node.js.

1a. Install from a package manager:

https://github.com/joyent/node/wiki/Installing-Node.js-via-package-manager

1b. Build from source on Ubuntu (and probably Debian):

NOTE: replace the 'x' in v0.8.x with the current latest stable version.

  sudo apt-get install build-essential git curl openssl libssl-dev
  mkdir -p ~/local/src
  cd ~/local/src
  git clone --branch v0.8.x git://github.com/joyent/node.git
  cd node
  ./configure
  make
  sudo make install

2. Install required Node.js modules with the Node Package Manager (NPM).

OPTIONAL: install node-gyp, so that the ws package is faster.

  npm install -g node-gyp

Make sure you are in the nodejs module directory - NPM needs the package.json
file that comes with the nodejs module to install the right modules.

  cd path/to/your/nodejs/module/directory
  npm install

3. Create a 'nodejs.config.js' file in your nodejs module directory.

Read the 'nodejs.config.js.example' file. A basic configuration can be as simple as:

settings = {
  host: '*',
  serviceKey: 'CHANGE ME',
  backend: {
    port: 80,
    host: 'CHANGE ME TO THE HOSTNAME OF YOUR DRUPAL SITE',
    scheme: 'http',
    basePath: '',
    messagePath: '/nodejs/message'
  },
  debug: true
};

Set debug to false when you are happy with your setup.

4. Run the node server with the command: node server.js

As long as you have 'debug: true' in your configuration file, you'll see lots of helpful messages.

