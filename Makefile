
.PHONY: build test run_tests doc clean

build-type ?= Release
target ?= examples

build:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=$(build-type) && ninja $(target)

test: target = run_tests
test: build run_tests

run_tests:
	@bin/run_tests

doc: include/bgl/
	@doxygen Doxyfile

clean:
	rm -rf bin build docs
