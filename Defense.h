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
  void DoDefense(PlanetWars* pw, GameState* game_state);

 private:
  vector<Planet*> TryToSendHelpBasic(vector<Planet*> unsafe_planets);
  void SendHelp(int unsafe_planet_id, vector<pair<int, int> > help);

  PlanetWars* pw;
  GameState* game_state;
};

#endif
