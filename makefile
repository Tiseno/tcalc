
default:
	g++ main.cpp -DDEBUG -std=c++2a -Wall -o calc_d.out -lreadline
	./calc_d.out

release:
	g++ main.cpp -std=c++2a -Wall -o calc.out -lreadline
	./calc.out

