
PROJECT = sample-project

.PHONY: build debug test clean

build-type ?= Release

build:
	@cd ../..; $(MAKE) target=$(PROJECT) build-type=$(build-type)

debug: build-type = debug
debug: build

test: build
	@bin/run_tests

clean:
	rm -rf bin
