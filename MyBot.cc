#include <iostream>
#include <fstream>
#include <algorithm>
#include "PlanetWars.h"
#include "PlanetScore.h"
using namespace std;

PlanetWars* pw;

double compute_score(int growth_rate, int num_ships, int distance, bool is_enemy) {
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

bool is_enough_fleets_going_to_planet(vector<Fleet> fleets, Planet p) {
  int fleet_sum = 0;
  for(vector<Fleet>::const_iterator it = fleets.begin(); it < fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 1)
      fleet_sum += it->NumShips();
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 2)
      fleet_sum -= it->NumShips();
  }
  return fleet_sum > p.NumShips() + 5;
}

bool is_neutral_planet_with_many_fleets(Planet p) {
  return p.Owner() == 0 && p.NumShips() > 30;
}

bool is_stupid_to_go_to_planet(Planet p, int turn) {
  return is_neutral_planet_with_many_fleets(p);
}

void SendShips(int my_planet, int my_planet_ships, int turn) {
  // (1) Find most interesting planets to send fleets
  vector<Planet> not_my_planets = pw->NotMyPlanets();
  vector<PlanetScore> not_my_planets_score;
  for(vector<Planet>::const_iterator it = not_my_planets.begin();
      it < not_my_planets.end(); ++it) {
    if(!is_stupid_to_go_to_planet(*it, turn)) {
      int num_ships = it->NumShips();
      int growth_rate = it->GrowthRate();
      int distance = pw->Distance(it->PlanetID(), my_planet);
      double score = compute_score(growth_rate, num_ships, distance,
				   it->Owner() == 2);
      PlanetScore* ps = new PlanetScore(*it, score);
      not_my_planets_score.push_back(*ps);
    }
  }
  sort(not_my_planets_score.begin(), not_my_planets_score.end());

  // (2) Send ships from my planet to the weakest planets that I don't own
  // and that I haven't seen fleets to it
  vector<Fleet> fleets = pw->Fleets();
  for(vector<PlanetScore>::iterator it = not_my_planets_score.begin();
      it < not_my_planets_score.end(); ++it) {
    Planet p = *(it->GetPlanet());
    if(!is_enough_fleets_going_to_planet(fleets, p)) {
      if(p.GrowthRate() == 5 && my_planet_ships - 5 > p.NumShips()) {
	pw->IssueOrder(my_planet, p.PlanetID(), my_planet_ships - 5);
	my_planet_ships = 5;
      }
      else if(my_planet_ships > p.NumShips()*2 + 5) {
	pw->IssueOrder(my_planet, p.PlanetID(), p.NumShips()*2);
	my_planet_ships -= p.NumShips()*2;
      }
      else break;
    }
  }
}

int ships_needed_to_defend_planet(Planet p) {
  vector<Fleet> enemy_fleets = pw->EnemyFleets();
  vector<int> ships_available_per_turn(50);
  ships_available_per_turn[0] = p.NumShips();
  for(int i = 1; i < 50; i++) {
    ships_available_per_turn[i] = ships_available_per_turn[i-1] + p.NumShips();
  }
  for(vector<Fleet>::iterator it = enemy_fleets.begin();
      it < enemy_fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID())
      ships_available_per_turn[it->TurnsRemaining()] -= it->NumShips();
  }
  for(int i = 1; i < 50; i++) {
    ships_available_per_turn[i] -= ships_available_per_turn[i-1];
  }
  int minimum_ships_in_planet_for_next_turns = 99999;
  for(int i = 1; i < 50; i++) {
    if(ships_available_per_turn[i] < minimum_ships_in_planet_for_next_turns)
      minimum_ships_in_planet_for_next_turns = ships_available_per_turn[i];
  }
  if(minimum_ships_in_planet_for_next_turns < 0) return 0;
  else return minimum_ships_in_planet_for_next_turns;
}

void DoTurn(int turn) {
  // Per each of my planets try to send ships to other planets
  int strongest_planet;
  int strongest_planet_ships;
  vector<Planet> my_planets = pw->MyPlanets();
  for (int i = 0; i < my_planets.size(); ++i) {
    int ships_needed_to_defend = ships_needed_to_defend_planet(my_planets[i]);
    SendShips(my_planets[i].PlanetID(), ships_needed_to_defend, turn);
  }
}


// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
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
