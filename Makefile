CC=g++

all: MyBot

clean:
	rm -rf *.o MyBot

MyBot: MyBot.o PlanetWars.o GameState.o Attack.o Defense.o PlanetScore.o PlanetState.o
