#CXXFLAGS=-Oz -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-math-errno -ffast-math -fno-unroll-loops -fmerge-all-constants -fno-ident -ffast-math  -Wl,--hash-style=gnu -Wl,--build-id=none -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-unroll-loops -fno-profile-use -o demo
#CXX=clang++

CXXFLAGS=-s FULL_ES3=1 -std=c++11 -s USE_GLFW=3 -o public/demo.html
CXX=emcc

.PHONY: demo
demo: main.cc
	$(CXX) `pkg-config --cflags --libs glfw3 gl` ${CXXFLAGS} main.cc
	#strip -S demo
	#du -b demo | awk '{ print  (65536 - $$1 )} $$1 > 65536 { exit 1 }'

.PHONY: clean
clean:
	rm demo
