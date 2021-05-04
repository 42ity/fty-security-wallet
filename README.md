# fty-security-wallet

fty-security-wallet solve those problems :

* To store common configuration payload (let’s call it Wallet Document) including sensitive data
* To exchange sensitive data in a secure way
* To be notified when configuration are updated
* Have a different level of access (ACL)
* SNMPv3, SNMPv1 are some of the potential kind of configuration  we would support

## Naming
* Document: A payload of the security wallet. It has a public and private part (secret)
* Portfolio: Pool of documents.
* Usage: Purpose for which a document can be use.
* Producer: Client which can create, update and delete Documents. This client can only read the public part.
* Consumer: Client which can read documents (public and private part) but which cannot modify any document.


## How to build

To build , run:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=usr -DBUILD_TESTING=On ..
make
sudo make install
```

## How to run

To run fty-security-wallet project:

* from within the source tree, run:

```bash
./build/agent/fty-security-wallet
```

For the other options available, refer to the manual page of fty-security-wallet

* from an installed base, using systemd, run:

```bash
systemctl start fty-security-wallet
```

## Architecture

### Overview

fty-security-wallet is composed of 1 agent and 1 client library :

Server side:
* SecurityWalletServer registers to malamute broker as security-wallet agent
  and handles mailbox requests

Client side:
* ConsumerAccessor registers to malamute broker using a client id (agent name) and then
  can execute Consumer requests to the SecurityWalletServer

* ProducerAccessor registers to malamute broker using a client id (agent name) and then
  can execute Producer requests to the SecurityWalletServer

* SecurityWalletServer need a database file for the data (database.json) and a
  configuration file (configuration.json)

* C++ Namespace for this project is "secw"

### Data structure organization

The Security wallet is use to store "Document".
A document has:
* 1 header (id, type, name, list of usage and list of tag)
* 1 public part
* 1 private part (secret)

A document has a list of "Usage"
A document has a list of "Tag" which are just a way to organize document

Each document are stored in a "Portfolio".

The SecurityWalletServer has a list of portfolio available.

### Access control
In progress: Will be done using the notion of "Usage" of document

### Available Api

Api is available for producer and consumer. See secw_producer_accessor.h and secw_consumer_accessor.h


### Published Document modification

To be Defined

