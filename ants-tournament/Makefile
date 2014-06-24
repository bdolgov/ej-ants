all: tournament

config.o: config.cpp tournament.hpp
	g++ -o config.o -c config.cpp -std=c++11 -ggdb

main.o: main.cpp tournament.hpp
	g++ -o main.o -c main.cpp -std=c++11 -ggdb

tournament: main.o config.o
	g++ -o tournament main.o config.o -ggdb

clean:
	rm -f *.o tournament
