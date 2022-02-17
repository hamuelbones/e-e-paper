# ESP-32 battery-powered ePaper Display

This repository hosts hardware and firmware files for an e-paper display board. Main features include:

* Internet connectivity and main processing using an esp32c3 module
* Output to a 28-pin Waveshare e-Paper display module (currently black/white only)
* Ability to run on a lithium battery and charging over USB
* High degree of configurability using toml files
* Can run standalone just getting data from an SD card or online
* Customization via editing `.toml` configuration files on the SD card/server
* Flashing over USB and, in the future, over-the-air updates

The display has or will be able to fetch and display the following things:

* Message Box: fetches a list of messages from a server and rotates through displaying them
* TODO: Weather forecasts
* TODO: A simple clock
* TODO: A calendar display app

The project status is largely in flux and currently in a proof-of-concept state. Lots may change!

If running the display in online mode, connecting it to a server application will allow you to create
dynamic message boxes, and update configuration files remotely. Note the display is fully functional
even when running offline; changes just need to be made to files on the SD card.

TODO: Add link to server repo

It is possible to clone the repository and run an instance of it yourself. Soon, it will be possible
to sign up for a (paid) account as an alternative.

# Building

## Dependencies

The project is built using CMake.


You'll need these if building the simulator target:

* pkg-config
* GTK+3
* OpenSSL

The hardware target needs 

Aside from these, the firmware depends on the following projects.

* tomlc99
* monobit / hoard of bitfonts
* freeRTOS and the POSIX port

## Native/Simulator Target

## Hardware Target

# Hardware files

For reference, the output schematics and fabrication files (gerbers) from the initial schematics 
are provided. 

That said, there are a litany of small and large changes that will be fixed with a new design:

* The pinout of the connector and display are in reverse order, so the board folds in the
  wrong/unintended direction in an enclosure.
* The LDO layout was not done to allow for the variant of the LDO with an enable pin
* The power/charging circuitry can probably be re-done to have lower quiescent power draw and
  be easier to hand-solder.
* Proper battery terminals should be added. On v1 the battery connection is done to test points.
* It would be nice to have some sort of mounting holes in the board.
* One footprint is of an improper size (0603 when 0805 was intended)
* Footprints in general can probably be adjusted for easier hand soldering.
* It would be nice to also support the larger display connector and parallel interfaces,
  since larger displays drop support for 1-wire SPI.
* The SD card holder placement can be adjusted to be more flush to the edge of the board.

With the new design, the project files will be better structured to be repository-friendly,
and KiCad project files will be added