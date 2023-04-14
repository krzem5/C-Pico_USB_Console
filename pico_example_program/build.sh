#!/bin/bash
if [ ! -d "build" ]; then
	mkdir build
	cd build
	cmake ..
	cd ..
fi
cd build
make -j16&&[[ -d "$PICO_DRIVE_PATH" ]]&&cp example_program.uf2 "$PICO_DRIVE_PATH/example_program.uf2"
cd ..
