
.PHONY: all debug test target target-debug doc clean

all:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release && ninja

debug:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug && ninja

test: all
	@bin/run_tests

target:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release && ninja ${TARGET}

target-debug:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug && ninja ${TARGET}

doc: include/bgl/
	@doxygen Doxyfile

clean:
	rm -rf bin build docs
