
.PHONY: test debug target target-debug doc clean

test:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release && ninja
	@bin/run_tests

debug:
	@mkdir -p build
	@cd build; cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug && ninja

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
