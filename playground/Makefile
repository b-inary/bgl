
.PHONY: build debug clean

build-type ?= Release

build:
	@cd ..; $(MAKE) target=playground build-type=$(build-type)

debug: build-type = Debug
debug: build

%: %_main.cpp
	@cd ..; $(MAKE) target=playground-$@ build-type=$(build-type)

clean:
	rm -rf bin
