#include <iostream>
#include <fstream>
#include <algorithm>
#include "PlanetWars.h"
#include "PlanetScore.h"
using namespace std;

PlanetWars* pw;

void DoTurn(int turn);
void try_attack_planets(int turn);
int ships_needed_to_keep_planet(Planet p);
vector<int> compute_ships_available_per_turn_with_attacks(vector<int> ships_per_turn, Planet p);
vector<int> compute_ships_available_per_turn_before_attacks(Planet p);
void try_to_attack_from_planet(int my_planet, int available_ships_for_attack, int turn);
vector<PlanetScore> compute_planets_list_sorted_by_score(int turn, int my_planet);
void decide_where_to_attack(int my_planet, int ships_for_attack, vector<PlanetScore> ps_score);
bool is_enough_fleets_attacking_planet(vector<Fleet> fleets, Planet p);
bool is_stupid_to_go_to_planet(Planet p, int turn);
bool is_neutral_planet_with_many_fleets(Planet p);
double compute_score(int growth_rate, int distance, int num_ships, bool is_enemy);


int main(int argc, char *argv[]) {
  std::string current_line;
  std::string map_data;
  int turn = 0;
  //   ofstream myfile;
  //   myfile.open("output");
  //   myfile.close();
  while (true) {
    int c = std::cin.get();
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
  try_attack_planets(turn);
}

void try_attack_planets(int turn) {
  int strongest_planet;
  int strongest_planet_ships;
  vector<Planet> my_planets = pw->MyPlanets();
  for (int i = 0; i < my_planets.size(); ++i) {
    int ships_needed_to_keep = ships_needed_to_keep_planet(my_planets[i]);
    try_to_attack_from_planet(my_planets[i].PlanetID(), ships_needed_to_keep, turn);
  }
}

int ships_needed_to_keep_planet(Planet p) {
  vector<int> ships_per_turn = compute_ships_available_per_turn_before_attacks(p);
  ships_per_turn = compute_ships_available_per_turn_with_attacks(ships_per_turn, p);
  int minimum_ships_in_planet_for_next_turns = 99999;
  for(int i = 1; i < 50; i++) {
    if(ships_per_turn[i] < minimum_ships_in_planet_for_next_turns)
      minimum_ships_in_planet_for_next_turns = ships_per_turn[i];
  }
  if(minimum_ships_in_planet_for_next_turns < 0) return 0;
  else return minimum_ships_in_planet_for_next_turns;
}

vector<int> compute_ships_available_per_turn_before_attacks(Planet p) {
  vector<int> ships_available_per_turn(50);
  ships_available_per_turn[0] = p.NumShips();
  for(int i = 1; i < 50; i++) {
    ships_available_per_turn[i] = ships_available_per_turn[i-1] + p.NumShips();
  }
  return ships_available_per_turn;
}

vector<int> compute_ships_available_per_turn_with_attacks(vector<int> ships_per_turn, Planet p) {
  vector<Fleet> enemy_fleets = pw->EnemyFleets();
  for(vector<Fleet>::iterator it = enemy_fleets.begin(); it < enemy_fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID())
      ships_per_turn[it->TurnsRemaining()] -= it->NumShips();
  }
  for(int i = 1; i < 50; i++) {
    ships_per_turn[i] -= ships_per_turn[i-1];
  }
  return ships_per_turn;
}

void try_to_attack_from_planet(int my_planet, int available_ships_for_attack, int turn) {
  vector<PlanetScore> planets_score = compute_planets_list_sorted_by_score(turn, my_planet);
  decide_where_to_attack(my_planet, available_ships_for_attack, planets_score);
}

vector<PlanetScore> compute_planets_list_sorted_by_score(int turn, int my_planet) {
  vector<Planet> not_my_planets = pw->NotMyPlanets();
  vector<PlanetScore> not_my_planets_score;
  for(vector<Planet>::const_iterator it = not_my_planets.begin(); it < not_my_planets.end(); ++it) {
    if(!is_stupid_to_go_to_planet(*it, turn)) {
      double score = compute_score(it->GrowthRate(), pw->Distance(it->PlanetID(), my_planet),
				   it->NumShips(), it->Owner() == 2);
      PlanetScore* ps = new PlanetScore(*it, score);
      not_my_planets_score.push_back(*ps);
    }
  }
  sort(not_my_planets_score.begin(), not_my_planets_score.end());
  return not_my_planets_score;
}

void decide_where_to_attack(int my_planet, int ships_for_attack, vector<PlanetScore> planets_score) {
  vector<Fleet> fleets = pw->Fleets();
  for(vector<PlanetScore>::iterator it = planets_score.begin(); it < planets_score.end(); ++it) {
    Planet p = *(it->GetPlanet());
    if(!is_enough_fleets_attacking_planet(fleets, p)) {
      if(p.GrowthRate() == 5 && ships_for_attack - 5 > p.NumShips()) {
	pw->IssueOrder(my_planet, p.PlanetID(), ships_for_attack - 5);
	ships_for_attack = 5;
      }
      else if(ships_for_attack > p.NumShips()*2 + 5) {
	pw->IssueOrder(my_planet, p.PlanetID(), p.NumShips()*2);
	ships_for_attack -= p.NumShips()*2;
      }
      else break;
    }
  }
}

bool is_enough_fleets_attacking_planet(vector<Fleet> fleets, Planet p) {
  int fleet_sum = 0;
  for(vector<Fleet>::const_iterator it = fleets.begin(); it < fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 1)
      fleet_sum += it->NumShips();
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 2)
      fleet_sum -= it->NumShips();
  }
  return fleet_sum > p.NumShips() + 5;
}

bool is_stupid_to_go_to_planet(Planet p, int turn) {
  return is_neutral_planet_with_many_fleets(p);
}

bool is_neutral_planet_with_many_fleets(Planet p) {
  return p.Owner() == 0 && p.NumShips() > 30;
}

double compute_score(int growth_rate, int distance, int num_ships, bool is_enemy) {
  if(num_ships == 0) num_ships = 1;
  if(distance == 0) distance = 1;
  double growth_rate_score = 60 * growth_rate;
  double num_ships_score = 5000 / num_ships / 2;
  double distance_score = 1000 / distance;
  double enemy_bonus = 0;
  /*  ofstream myfile;
  myfile.open("output",ios::app);
  myfile << growth_rate_score + num_ships_score + distance_score << " = " <<
    growth_rate << "("<<growth_rate_score<<")   + " << num_ships << " (" <<
    num_ships_score << ")   + " << distance << " (" << distance_score << ")\n";
    myfile.close();*/
  return growth_rate_score + num_ships_score + distance_score + enemy_bonus;
}
