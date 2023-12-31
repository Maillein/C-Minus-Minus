build:
	[ -d build ] || cmake -S . -B build
	cmake --build build

test:
	./test/test.sh

clean:
	-@rm -rf build
	-@rm AST.dot AST.png

ast: AST.png
AST.png: AST.dot
	dot -Tpng $< -o AST.png

.PHONY: build test clean
