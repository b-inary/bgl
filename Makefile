
.PHONY: all debug test target doc clean

all:
	@mkdir -p build
	@cd build; cmake .. -GNinja && ninja

debug:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug && ninja

test: all
	@bin/run_tests

target:
	@mkdir -p build
	@cd build; cmake .. -GNinja && ninja ${TARGET}

doc: src/
	@doxygen Doxyfile

clean:
	rm -rf bin build docs
