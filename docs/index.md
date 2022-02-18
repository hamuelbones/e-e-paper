---
layout: default
---

# Battery-powered ePaper Display

This site/project documents a hackable internet-connected e-Paper display (or, e-e-Paper). 

The electronics hardware utilizes an ESP32-C3 module to work with Waveshare 24-pin displays, and allows attaching
a lithium-ion battery which can be charged over USB.

The firmware uses configuration loaded from an SD card and/or retrieved from the Internet to determine what to
show on the display.

In and of itself, this module is not a finished product; however, it may be a good starting point for your
own project, and could be adapted to many different use cases.

## Examples

Coming soon!

## Features

* Outputs onto 24-pin Waveshare e-Paper display modules (currently black/white only)
* Runs on a lithium battery and charges over USB
* Is highly configurable by editing `toml` files
* Can connect to the Internet over WiFi, or standalone using a microSD card
* Can be updated over USB and, in the future, over the air 

## Apps

The display can or can be configured to show the following things:

* Message Box: Rotate through a list of messages, either statically or retrieved from a server.
* TODO: Weather forecasts
* TODO: A simple clock
* TODO: A calendar display app

The project status is largely in flux and currently in a proof-of-concept state. Lots may change! The plan is that I will
have kits available for purchase, and allow for you to utilize the same server I use (for a small monthly cost) to host
configuration and apps that require web-facing parts.

That said, you are free to have the board fabricated on your own, host the server software yourself, or not use any
server at all.

## Where to go from here

If you'd like an introduction to using the device and how it can be used, take a look
the [user guide](user_guide.html).

If you're interested in the hardware, check the [hardware reference](hardware.html).

For more information on building or modifying the firmware, see the [build guide](build.html).

