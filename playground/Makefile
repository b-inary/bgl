
.PHONY: all debug clean

all:
	@cd ..; $(MAKE) target TARGET=playground

debug:
	@cd ..; $(MAKE) target-debug TARGET=playground

%: %_main.cpp
	@cd ..; $(MAKE) target TARGET=playground-$@

clean:
	rm -rf bin
