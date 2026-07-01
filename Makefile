BUILD_DIR := build
PRESET ?= debug

.PHONY: configure build run test clean

configure:
	cmake --preset $(PRESET)

build:
	cmake --build --preset $(PRESET)

run: build
	./$(BUILD_DIR)/$(PRESET)/bin/inkapp

test: build
	ctest --preset $(PRESET)

clean:
	cmake --build --preset $(PRESET) --target clean