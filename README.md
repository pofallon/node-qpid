node-cproton
============

A Node.js native wrapper around the AMQP 1.0 Qpid Proton C API

Currently expects you to already have the qpid-proton library installed on your (Linux) system:
http://qpid.apache.org/proton/

## What does it do so far?
Not much yet.  It can connect to Azure ServiceBus via AMQP and send a message.

If you send more than a couple of messages at a time it gets cranky... :-)
