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

To build fty-security-wallet project run:

```bash
./autogen.sh
./configure
make
make check # to run self-test
```

## How to run

To run fty-security-wallet project:

* from within the source tree, run:

```bash
./src/fty-security-wallet
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

### Mailbox protocol
To be Redefined

Protocol between clients and server is define as follow.

Request from client:
* Message subject "REQUEST"
* Frame 1: Correlation id
* Frame 2: Command
* frame 3-n : Parameters for the command

Normal Reply
* Message subject "REPLY"
* Frame 1: Correlation id
* Frame 2: Data in Json format or "OK" if no data where needed

Error Reply
* Message subject "REPLY"
* Frame 1: Correlation id
* Frame 2: "ERROR"
* Frame 3: Error description in Json if the error belong to SecwException

#### Available Command
To be Redefined

The library is using this command. Please avoid to use mailbox directly.
Prefer to use the library and the class ClientAccess
It is possible to request the agent security-wallet for:

* GET_PORTFOLIO_LIST: Return list of portfolio on the server
* GET_LIST_WITH_SECRET: Return the list of document from the server without private part
* GET_LIST_WITHOUT_SECRET: Return the list of document which can be access by the client from the server with private part
* GET_EDITABLE_TAGS: Return list of tag which can be use to edit document
* GET_READABLE_TAGS: Return list of tag which can be use to read private data
* GET_WITHOUT_SECRET: Get a document without private data
* GET_WITH_SECRET: Get a document with private data

```bash
bmsg request security-wallet REQUEST 1234 GET_LIST_WITH_SECRET  default 
bmsg request security-wallet REQUEST 1234 GET_LIST_WITHOUT_SECRET  default
``` 

### Published Document modification

To be Defined

Document update/creation/deletion are published on the 'SECURITY_WALLET' stream.

To be Defined

Example of alert message:

```bash
stream=SECURITY_WALLET
sender=security-wallet
subject=<TBD>
TBD
```
