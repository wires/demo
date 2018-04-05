demo: main.cc
	clang++ -Oz main.cc -o demo
	strip demo
	du -b demo | awk '$$1 > 4096 { exit 1 }'

.PHONY: clean
clean:
	rm demo
