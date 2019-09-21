CXX=/usr/bin/env clang++

.PHONY: test compile-bench clean

bin/unit_test: unit_test.cpp
	mkdir -p bin
	$(CXX) -std=c++17 -g -o bin/unit_test unit_test.cpp

test: bin/unit_test
	bin/unit_test

compile-bench: clean
	mkdir -p bin
	sh -c 'time $(CXX) -std=c++17 -o /dev/null compile_bench_inheritance.cpp'
	sh -c 'time $(CXX) -std=c++17 -o /dev/null compile_bench_traits.cpp'

clean:
	rm -rf bin

