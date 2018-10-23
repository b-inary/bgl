
PROJECT = sample-project

.PHONY: all debug test clean

all:
	@cd ../..; $(MAKE) target TARGET=$(PROJECT)

debug:
	@cd ../..; $(MAKE) target-debug TARGET=$(PROJECT)

test: all
	@bin/run_tests

clean:
	rm -rf bin
