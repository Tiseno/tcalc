debug:
	g++ main.cpp -DDEBUG -std=c++2a -Wall -o tcalc_d.out -lreadline
	./tcalc_d.out

release:
	g++ main.cpp -std=c++2a -Wall -o tcalc.out -lreadline
