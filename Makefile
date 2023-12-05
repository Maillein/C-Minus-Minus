build:
	[ -d build ] || cmake -S . -B build
	cmake --build build

test:
	./test/test.sh

clean:
	-@rm -rf build

.PHONY: build test clean
