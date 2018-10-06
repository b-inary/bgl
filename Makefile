
.PHONY: all test doc clean

all:
	@mkdir -p build
	@cd build; cmake .. -GNinja && ninja

test: all
	@bin/run_tests

doc: src/
	@doxygen Doxyfile

clean:
	rm -rf bin build docs playground/bin
