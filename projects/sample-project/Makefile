
.PHONY: all debug test clean

all:
	@cd ../..; $(MAKE) target TARGET=sample-project

debug:
	@cd ../..; $(MAKE) target-debug TARGET=sample-project

test: all
	@bin/run_tests

clean:
	rm -rf bin
