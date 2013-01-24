node-qpid
============

A Node.js native wrapper around the Qpid API; specifically the AMPQ 1.0 Proton C Messenger API

## Usage

You can use it to send and receive very basic messages to/from Azure Service Bus:

```javascript
var Messenger = require('qpid').proton.Messenger;
var m = new Messenger().subscribe(url);

m.on('subscribed', function(url) {
  console.log("Successfully subscribed to URL: " + url);
  m.send("Hello world!", function() {
    console.log("Message sent.");
  });
});

m.on('message', function(msg) {
  console.log("Message received: " + msg.body);
  // Stop after receiving one message
  m.stop();
});

m.on('error', function(err) {
  console.log("Error: " + err.message);
});

m.receive();
```

## Installation

It expects you to already have the qpid-proton library installed on your (Linux) system:
http://qpid.apache.org/proton/

## Issues

Plenty, I'm sure... :-)

## Acknowledgements

[node_sqlite3](https://github.com/developmentseed/node-sqlite3) has been an invaluable resource while learning to write a native module!
