all: router

networking.o: networking.cpp
	g++ -std=gnu++17 -Wall -Wextra -o networking.o -c networking.cpp

common.o: common.cpp
	g++ -std=gnu++17 -Wall -Wextra -o common.o -c common.cpp

main.o: main.cpp
	g++ -std=gnu++17 -Wall -Wextra -o main.o -c main.cpp

router: common.o main.o networking.o
	g++ -std=gnu++17 -Wall -Wextra common.o main.o networking.o -o router

clean:
	rm -f common.o main.o networking.o

distclean:
	rm -f common.o main.o networking.o router