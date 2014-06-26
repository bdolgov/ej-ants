all: tournament

config.o: config.cpp tournament.hpp
	g++ -o config.o -c config.cpp -std=c++11 -ggdb

main.o: main.cpp tournament.hpp
	g++ -o main.o -c main.cpp -std=c++11 -ggdb

renderer.o: renderer.cpp tournament.hpp
	g++ -o renderer.o -c renderer.cpp -std=c++11 -ggdb

tournament: main.o config.o renderer.o
	g++ -o tournament main.o config.o renderer.o -ggdb -pthread

clean:
	rm -f *.o tournament
