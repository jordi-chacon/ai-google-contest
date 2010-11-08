#ifndef ATTACK_H_
#define ATTACK_H_

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "PlanetWars.h"
#include "PlanetScore.h"
#include "GameState.h"

using namespace std;

class Attack {
 public:
  void attack(PlanetWars* pw, GameState* game_state, int turn);

 private:
  void try_to_attack_from_planet(int my_planet, int available_ships_for_attack, int turn);
  vector<PlanetScore> compute_planets_list_sorted_by_score(int turn, int my_planet);
  void decide_where_to_attack(int my_planet, int ships_for_attack, vector<PlanetScore> ps_score);
  bool is_enough_fleets_attacking_planet(vector<Fleet> fleets, Planet p);
  bool is_stupid_to_go_to_planet(Planet p, int turn);
  bool is_neutral_planet_with_many_fleets(Planet p);
  double compute_score(int growth_rate, int distance, int num_ships, bool is_enemy);

  PlanetWars* pw;
  GameState* game_state;
};

#endif
