L3D Cube Network Tests
======================

Tests of network connection quality under various circumstances for streaming to the [L3D Cube](http://l3dcube.com/).

# Building Locally

To compile a firmware binary using a local copy of the library:

1. Follow the instructions for compiling the [Spark firmware](https://github.com/spark/firmware#1-download-and-install-dependencies). Make sure you can successfuly compile the firmware before continuing.
2. Edit the FIRMWARE_DIR variable in the l3d-cube makefile to the path of the spark firmware repository on your machine.
3. Choose a test to compile or put your own code in firmware/tests.
4. Run `make bin/<name of test>.bin` to generate firmware for that test in the bin/ directory.
5. Flash the firmware using `spark flash` (the [Spark CLI tool](https://github.com/spark/spark-cli)) or dfu-util.
