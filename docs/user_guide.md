---
layout: default
---

# First Steps

Steps are outlined below, but need more description.
Actually, it might be better to find some ways to shorten this a bit.

## Connected Mode

1. Set up hardware
2. Create a server account
3. Flash the firmware
4. Power on with an SD card, and wait a minute, to generate authorization keys
5. Power off and copy `public.pem` and `key_uuid.toml` from the SD card
6. Set up your device on the server, using the files above
7. Set up a `config.toml` on the server
8. Create a `startup.toml` on the SD card
9. Put the SD card back in and power on again

## Standalone Mode

1. Set up hardware
2. Flash the firwmare
3. Format your SD card
4. Set up a `startup.toml`, `config.toml`, and any other files you may need on the SD card
5. Put your SD card in the module and power it on

# Modes of operation

# File Overview

## `startup.toml`

A file named `startup.toml` is used to configure the module's boot-time behavior. As such, it needs to be loaded
from an SD card - it is loaded and used before the module potentially connects to the internet.

### Example

Here's an example of a complete `startup.toml`:

```toml
mode = "online"

[[wifi]]
ssid = "network"
password = "test_password"
[[wifi]]
ssid = "network2"
password = "test_password2"

[server]
host = "hambones.app"
config_dir = "/dev_api/config/"
auth = true
```

### Description

The `startup.toml` file has several important fields.

#### Mode

First, the `mode` key determines whether the module will try to connect to the Internet to update its own configuration
or not. 
* If `mode` is `"online"`, the module will attempt to connect to the Internet and refresh its configuration from a 
  remote server.
* If `mode` is `"standalone"`, the module will not connect to the Internet.
* TODO: if `mode` is `"static"`, the module will connect to the Internet, but will not update its configuration
  automatically.

#### Wifi

The table under the `wifi` key, which may be repeated, (see the double bracket) provides WiFi credentials. The module
will attempt to connect to each of the networks in sequence, if its mode is `"online"` or `"static"`. 
* The `ssid` sub-field is required.
* If the network is not secure, the `password` sub-field can be omitted.

#### Server

The `server` key specifies the server the device should fetch its `config.toml`, if its mode is `"online"`. The module 
will issue a GET request to the server specified. If it provides an HTTP 200 response, the response content will be
saved on the module.

* The `host` is the internet address or fully-qualified domain name. It may include a port number.
* `config_dir` provides the subdirectory to fetch the configuration from.
* If `auth` is `true`, then the header for the request will include a JSON Web Token using the module's 
RSA key.

If the request does not complete or the server responds with an error, the module falls back to the previous configuration.

The `server` table has no effect if the module's mode is `"standalone"` or `"static"`.

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