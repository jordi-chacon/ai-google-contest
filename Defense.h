#ifndef DEFENSE_H_
#define DEFENSE_H_

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "PlanetWars.h"
#include "PlanetScore.h"
#include "GameState.h"

using namespace std;

class Defense {
 public:
  void defense(PlanetWars* pw, GameState* game_state);

 private:
  void try_to_send_ships_to_unsafe_planet(Planet unsafe_planet);

  PlanetWars* pw;
  GameState* game_state;
};

#endif
