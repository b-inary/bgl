
.PHONY: all test target doc clean

all:
	@mkdir -p build
	@cd build; cmake .. -GNinja && ninja

test: all
	@bin/run_tests

target:
	@mkdir -p build
	@cd build; cmake .. -GNinja && ninja ${TARGET}

doc: src/
	@doxygen Doxyfile

clean:
	rm -rf bin build docs
