neoPixelColorCycle
==================

Pulse colors on a NeoPixel Ring via an Arduino.


Purpose
=======

One of those "just for fun" projects, I saw [Mr. BlinkySkull](http://maniacallabs.com/2014/08/05/blinkyskull/) on Maniacal Labs, and knew I had to make one for myself.  This is the arduino code which drives the NeoPixel Ring, making the project come together.


Prerequisites
=============

The Adafruit NeoPixel library is required for this project.

https://github.com/adafruit/Adafruit_NeoPixel


Hardware
========

For the arduino, I used a generic [Arduino Pro Mini compatible microcontroller](http://www.ebay.com/itm/200957063666?_trksid=p2060778.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT), controlling a [NeoPixel Ring](https://www.adafruit.com/products/1463).  A motion sensor, switch, button, and related componants are also used.  See the [schematic](_schematic.png) or [fritzing](_fritzing.png) files for more information.


Installation
============

Open the file in your arduino dev environment, modify any of the settings you want in the define section (especially pin numbers), and compile/upload to your board.


Further Information
===================

Pictures and description can be found at [mtmosier.com](http://mtmosier.com/80-arduino/75-neopixel-skull).


Copyright Information
=====================

All code contained is licensed as GPLv3.

Copyright 2014, Michael T. Mosier (mtmosier at gmail dot com).

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.
