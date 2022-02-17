---
layout: default
---

# Modes of operation

# File Overview

## `startup.toml`

## `config.toml`

# Connecting to a server

## Authorization

Upon initial startup, the module will generate several files that it uses to 
sign server requests. The server application verify these digital signatures to 
prove to a server that the module, and not some other party, is requesting
information. That said, the server needs to know about the public key.

To authorize, the module provides a JSON Web Token with an RS256 signature. After
an SD card is inserted, the public key is copied in PEM format to it, into a 
file named `public.pem`. This file may be uploaded to the server application, or
otherwise used to verify the signature provided in those HTTP requests.

The corresponding private key is saved onboard the device.

# Application configuration

## Display / Layout

The `config.toml` file contains the complete description of what to 

### Included fonts

### Custom fonts

## Message boxes