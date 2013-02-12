node-qpid
============

A Node.js native wrapper around the Qpid API; specifically the AMPQ 1.0 Proton C Messenger API

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
* Windows Azure Service Bus
* ActiveMQ 5.8 nightly builds w/ AMQP 1.0 support
* RabbitMQ with the AMQP 1.0 Plugin

Peer-to-peer support is a work in progress

## Installation

It expects you to already have the qpid-proton library installed on your (Linux) system:
http://qpid.apache.org/proton/

## Issues

Plenty, I'm sure... :-)

## Acknowledgements

[node_sqlite3](https://github.com/developmentseed/node-sqlite3) has been an invaluable resource while learning to write a native module!
