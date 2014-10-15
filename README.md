node-qpid
============

A Node.js native wrapper around Apache Qpid, specifically the AMQP 1.0 Proton C API.

## Usage

You can use it to exchange messages with AMQP 1.0 brokers:

```javascript
// Sending
var Messenger = require('qpid').proton.Messenger;
var m = new Messenger();

m.send({address: address, body: message}, function(err) {
  if (err) {
    console.log("Error sending message: " + err.message);
  }
});

```

```javascript
// Receiving
var Messenger = require('qpid').proton.Messenger;
var m = new Messenger();

m.on('subscribed', function(url) {
  console.log("Subscribed to " + url);
});

m.on('message', function(message) {
  console.log(message.body);
});

m.subscribe(address).receive();
```

The module has been preliminarily tested against:
* [Windows Azure Service Bus](http://www.windowsazure.com/en-us/develop/net/how-to-guides/service-bus-amqp-overview/)
* [ActiveMQ 5.8 snapshot builds](https://repository.apache.org/content/repositories/snapshots/org/apache/activemq/apache-activemq/5.8-SNAPSHOT/) w/ AMQP 1.0 support
* RabbitMQ with the [AMQP 1.0 Plugin](https://github.com/rabbitmq/rabbitmq-amqp1.0)

## Installation

This module expects that you have the following installed on your system:
* [qpid-proton library](http://qpid.apache.org/proton/) version 0.7
* nodejs-dev
* necessary build tools

Building qpid-proton is fairly involved, so to ease that burden, grabbing and installing 
the binaries from debian sid is a faster (and probably less error prone) approach.  There
is a PPA for qpid/released, however the libraries there are not compiled against OpenSSL 
and SSL support is needed for connecting to Service Bus using AMQP.

For Ubuntu 14.04, you can set up all the necessary requirements as follows:

```bash
# wget http://ftp.us.debian.org/debian/pool/main/q/qpid-proton/libqpid-proton2-dev_0.7-1_amd64.deb
# wget http://ftp.us.debian.org/debian/pool/main/q/qpid-proton/libqpid-proton2_0.7-1_amd64.deb
# sudo dpkg -i libqpid-proton2-dev_0.7-1_amd64.deb libqpid-proton2_0.7-1_amd64.deb
# sudo apt-get install build-essential git
# sudo apt-get install nodejs-legacy nodejs-dev nodejs npm
```

Until this module is committed back to main branch and published as an npm module, to build and install it, do the following:

```bash
# mkdir tmp
# cd tmp
# git clone https://github.com/jmspring/node-qpid.git
# cd node-qpid
# sudo npm install -g .
```

At this point, you are ready to use the module.

## Issues

Grabbing binaries directly from debian sid for Ubuntu is a little silly, but for now it works.

It's still rough around the edges and not ready for prime time, but pull requests are welcomed!

## Acknowledgements

[node_sqlite3](https://github.com/developmentseed/node-sqlite3) has been an invaluable resource while writing this native module!
