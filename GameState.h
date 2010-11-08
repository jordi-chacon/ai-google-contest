#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "PlanetState.h"

using namespace std;

class GameState {
 public:
  void init(PlanetWars* p);
  void clear();
  PlanetState* get_planet_state(int planet_id);
  std::vector<Planet*> get_unsafe_planets();
  std::vector<Planet*> get_safe_planets();
  void print_game_state();
  void print_planet_state(int planet_id);
  void print_planet_state(PlanetState* ps);

 private:
  void init_ships_available_per_turn_and_planet();
  void init_my_planets();
  std::vector<int> ships_available_in_planet_per_turn(Planet p);
  std::vector<int> compute_ships_available_per_turn_with_moving_fleet(Planet p);
  std::vector<int> compute_ships_available_per_turn_based_on_growth_rate(std::vector<int> ships_per_turn,
									 Planet p);
  void init_my_unsafe_planets();
  void check_and_maybe_set_as_my_unsafe_planet(Planet p);
  void init_available_ships_in_my_planets();
  int available_ships_in_my_planet(Planet p);

  std::map<int, PlanetState*> planets_state;
  PlanetWars* pw;
};

#endif
