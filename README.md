# fty-security-wallet

fty-security-wallet solve those problems :

* To store common configuration payload (let’s call it Wallet Document) including sensitive data
* To exchange sensitive data in a secure way
* To be notified when configuration are updated
* Have a different level of access (ACL)
* SNMPv3, SNMPv1 are some of the potential kind of configuration  we would support


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

fty-security-wallet is composed of 1 actor :

* SecurityWalletServer registers to malamute broker as security-wallet agent 
  and handles mailbox requests 

### Mailbox requests

It is possible to request the agent security-wallet for:

* CREATE : create (a list of) new documents in the default portfolio security wallet
* DELETE : remove an existing document
* UPDATE_WITHOUT_SECRET : update secw_doc_name, secw_doc_tag and secw_doc_public. The secret is not touched.
* UPDATE_SECRET : update all except secw_doc_type and secw_doc_id
* GET_WITHOUT_SECRET : return public part of Document 
* GET_WITH_SECRET : return public and private part of Document

The USER peer  sends the following messages using MAILBOX SEND to
"security-wallet" peer :

* REQUEST __COMMAND__/'msg-correlation-id'

where
* subject should be REQUEST
* '/' indicates a multipart string message
* __COMMAND__ available are CREATE, DELETE, UPDATE_WITHOUT_SECRET
* 'msg-correlation-id' MUST be unique identifier of the message

```bash
bmsg request security-wallet REQUEST GET_DUMMY_WITH_SECRET 1234 default 
bmsg request security-wallet REQUEST GET_DUMMY_WITHOUT_SECRET 1234 default
``` 

### Published Document modification

Document update/creation/deletion are published on the 'SECURITY_WALLET' stream.

To be Defined

Example of alert message:

```bash
stream=SECURITY_WALLET
sender=security-wallet
subject=<TBD>
TBD
```
