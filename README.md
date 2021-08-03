JPEGENC
-------
Copyright (c) 2021 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>

What is it?
------------
An 'embedded-friendly' (aka Arduino) JPEG image encoding library<br>
<br>

Why did you write it?
---------------------
Starting in the late 80's I wrote my own imaging codecs for the existing standards (CCITT G3/G4 was the first). I soon added GIF, JPEG and not long after that, the PNG specification was ratified. All of this code was "clean room" - written just from the specification. I used my imaging library in many projects and products over the years and recently decided that some of my codecs could get a new lease on life as open source, embedded-friendly libraries for microcontrollers.<br>
<br>

What's special about it?<br>
------------------------
There are many open source libraries written for Linux that would be useful to run on embedded processors. What stops them from "just working" is that they depend on a file system, heap management and sometimes additional external dependencies. The purpose of this project is to provide a fast and convenient JPEG encoder which can run without an operating system or even a C-Runtime library. I've optimized the code for speed and memory usage so it should be faster than any other FOSS libraries available.<br>

Feature summary:<br>
----------------<br>
- Runs on any MCU with at least 4K of free RAM<br>
- No external dependencies (including malloc/free)<br>
- Encode an image MCU by MCU<br>
- Encode directly to your own buffer or a file with I/O callbacks you provide<br>
- Supported pixel types: grayscale, RGB565, RGB888 and ARGB8888 (alpha ignored)<br>
- Allows for optional color subsampling (4:4:4 or 4:2:0)<br>
- Supports 4 quality levels (LOW, MED, HIGH, BEST)
- Arduino-style C++ library class with simple API<br>
- Can by built as straight C as well<br>
<br>

How fast is it?<br>
---------------<br>
The examples folder contains a sketch to measure the performance of encoding a 1024x1024 image generated dynamically.<br>

Documentation:<br>
---------------<br>
Detailed information about the API is in the Wiki<br>
See the examples folder for easy starting points<br>
<br>
