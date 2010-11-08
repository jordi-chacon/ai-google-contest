#include <iostream>
#include <fstream>
#include "PlanetWars.h"
#include "Attack.h"
#include "Defense.h"
using namespace std;

PlanetWars* pw;
GameState game_state;
Attack a;
Defense d;

void DoTurn(int turn);

int main(int argc, char *argv[]) {
  string current_line;
  string map_data;
  int turn = 0;
  //  ofstream myfile;myfile.open("output");myfile.close();
  while (true) {
    int c = cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
	turn++;
        PlanetWars planet_wars(map_data);
	pw = &planet_wars;
        map_data = "";
        DoTurn(turn);
	pw->FinishTurn();
      } else {
        map_data += current_line;
      }
      current_line = "";
    }
  }
  return 0;
}

void DoTurn(int turn) {
  game_state.init(pw);
  d.defense(pw, &game_state);
  a.attack(pw, &game_state, turn);
  game_state.clear();
}
