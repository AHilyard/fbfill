# fbfill

A simple utility to efficiently fill a linux framebuffer device with any color.
Great for embedded use-cases.

## Getting Started
**fbfill** has no external dependencies, and can be built with g++.   A Qt project file is included for convenience, even though the utility does not use any Qt libraries.

### Example build command
`g++ -o fbfill main.cpp`

## Usage
`fbfill [-r value] [-g value] [-b value] [-a value] [-f device]`

All flags are optional.  If not specified, color defaults to black (0, 0, 0, 0) and the device defaults to /dev/fb0.