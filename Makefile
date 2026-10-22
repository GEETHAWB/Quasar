.PHONY: configure build run clean format format-check

configure:
	cmake --preset ninja-debug

build: configure
	cmake --build --preset build-ninja-debug

run: build
	clear
	./build/ninja-debug/Quasar

clean:
	cmake --build build/ninja-debug --target clean

format:
	cmake --build build/ninja-debug --target format

format-check:
	cmake --build build/ninja-debug --target format-check