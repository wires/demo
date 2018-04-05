CXXFLAGS=-Oz -m32 -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-math-errno -ffast-math -fno-unroll-loops -fmerge-all-constants -fno-ident -ffast-math -Wl,-z,norelno -Wl,--hash-style=gnu -Wl,--build-id=none -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-unroll-loops -fno-profile-use

.PHONY: demo
demo: main.cc
	clang++ ${CXXFLAGS} main.cc -o demo
	strip -S \
	  --strip-unneeded \
	  --remove-section=.note.gnu.gold-version \
	  --remove-section=.comment \
	  demo
	du -b demo | awk '{ print  (65536 - $$1 )} $$1 > 65536 { exit 1 }'

.PHONY: clean
clean:
	rm demo
