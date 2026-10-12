##
## img-settings — developer shortcuts around CMake
## Requires Qt 6.9+ (with Qt Shader Tools) and CMake. libjxl is optional.
##   macOS: brew install qt cmake jpeg-xl pkgconf
##

BUILD      := build
CMAKE_ARGS ?=

.PHONY: all configure build test run clean

all: build

## Configure the CMake build directory (Release)
configure:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Release $(CMAKE_ARGS)

## Build the app and the unit tests
build: configure
	cmake --build $(BUILD) --parallel

## Run the engine unit tests
test: build
	ctest --test-dir $(BUILD) --output-on-failure

## Build and launch the app (macOS)
run: build
	open $(BUILD)/img-settings.app

clean:
	rm -rf $(BUILD) dist
