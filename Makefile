all:
	#g++-12 -c src/debug.cpp -o bin/debug.o -Iinclude/
	g++ -c src/opcode.cpp -o bin/opcode.o -Iinclude/
	#g++ -c src/value.cpp -o bin/value.o -Iinclude/
	g++ -c src/vm.cpp -o bin/vm.o -Iinclude/
	g++ -c src/kreczmar.cpp -o bin/kreczmar.o -Iinclude/
	g++ -c main.cpp -o bin/main.o -Iinclude/
	cd bin && g++ main.o opcode.o vm.o kreczmar.o -o ./glert

build-examples : build-v2.1.0
build-v2.1.0 :
	g++ ./utils/calc.cpp -o ./bin/calc-writer -Iinclude

run-examples : run-v2.1.0
run-v2.1.0 :
	./bin/glert --debug true --file ./tests/calc.gle