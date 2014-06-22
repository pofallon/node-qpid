node-qpid
============

A Node.js native wrapper around Apache Qpid, specifically the AMQP 1.0 Proton C API

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

It expects you to already have version 0.3 of the [qpid-proton library](http://qpid.apache.org/proton/) installed on your (Linux) system.


## Issues

It's still rough around the edges and not ready for prime time, but pull requests are welcomed!

## Acknowledgements

[node_sqlite3](https://github.com/developmentseed/node-sqlite3) has been an invaluable resource while writing this native module!
