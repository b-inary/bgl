
.PHONY: test debug examples target target-debug doc clean

test:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release && ninja run_tests
	@bin/run_tests

debug:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug && ninja run_tests

examples:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release && ninja examples

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
