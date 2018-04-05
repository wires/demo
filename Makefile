CXXFLAGS=-Oz -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-unroll-loops

.PHONY: demo
demo: main.cc
	clang++ ${CXXFLAGS} main.cc -o demo
	strip demo
	du -b demo | awk '{ print } $$1 > 4096 { exit 1 }'

.PHONY: clean
clean:
	rm demo
