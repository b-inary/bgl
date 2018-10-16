
.PHONY: all test clean

all:
	@cd ../..; $(MAKE) target TARGET=sample-project

test: all
	@bin/run_tests

clean:
	rm -rf bin
