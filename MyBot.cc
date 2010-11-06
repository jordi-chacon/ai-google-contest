#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include "PlanetWars.h"
#include "PlanetScore.h"
#include "PlanetState.h"
using namespace std;

PlanetWars* pw;
map<int, PlanetState*> planets_state;

void DoTurn(int turn);

// INITIALIZE
void initialise();
void initialise_ships_available_per_turn_and_planet();
void initialise_my_planets();
vector<int> ships_available_in_planet_per_turn(Planet p);
vector<int> compute_ships_available_per_turn_based_on_growth_rate(Planet p);
vector<int> compute_ships_available_per_turn_with_moving_fleet(vector<int> ships_per_turn, Planet p);
void initialise_my_unsafe_planets();
void check_and_maybe_set_as_my_unsafe_planet(Planet p);
void initialise_available_ships_in_my_planets();
int available_ships_in_my_planet(Planet p);

// DEFENSE
void defense();
void try_to_send_ships_to_unsafe_planet(Planet unsafe_planet);

// ATTACK
void attack(int turn);
void try_to_attack_from_planet(int my_planet, int available_ships_for_attack, int turn);
vector<PlanetScore> compute_planets_list_sorted_by_score(int turn, int my_planet);
void decide_where_to_attack(int my_planet, int ships_for_attack, vector<PlanetScore> ps_score);
bool is_enough_fleets_attacking_planet(vector<Fleet> fleets, Planet p);
bool is_stupid_to_go_to_planet(Planet p, int turn);
bool is_neutral_planet_with_many_fleets(Planet p);
double compute_score(int growth_rate, int distance, int num_ships, bool is_enemy);

// UTILS
int min(int a, int b);
void clear_maps();
void print_planets_state();
void print_planet_state(PlanetState* ps);

///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  std::string current_line;
  std::string map_data;
  int turn = 0;
  //  ofstream myfile;myfile.open("output");myfile.close();
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
  initialise();
  defense();
  attack(turn);
  clear_maps();
}

/////////////////////////////////////////////////////////////////////////////////////////
// I N I T I A L I Z E
/////////////////////////////////////////////////////////////////////////////////////////
void initialise() {
  initialise_ships_available_per_turn_and_planet();
  initialise_my_planets();
  initialise_my_unsafe_planets();
  initialise_available_ships_in_my_planets();
}

void initialise_ships_available_per_turn_and_planet() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = new PlanetState();
    planets_state[it->PlanetID()] = ps;
    ps->SetAvailableShipsPerTurn(ships_available_in_planet_per_turn(*it));
  }
}

vector<int> ships_available_in_planet_per_turn(Planet p) {
  vector<int> ships_per_turn = compute_ships_available_per_turn_based_on_growth_rate(p);
  return compute_ships_available_per_turn_with_moving_fleet(ships_per_turn, p);
}

vector<int> compute_ships_available_per_turn_based_on_growth_rate(Planet p) {
  vector<int> ships_available_per_turn(50);
  ships_available_per_turn[0] = p.NumShips();
  for(int i = 1; i < 50; i++) {
    ships_available_per_turn[i] = ships_available_per_turn[i-1] + p.GrowthRate();
  }
  return ships_available_per_turn;
}

vector<int> compute_ships_available_per_turn_with_moving_fleet(vector<int> ships_per_turn, Planet p) {
  vector<Fleet> enemy_fleets = pw->EnemyFleets();
  for(vector<Fleet>::iterator it = enemy_fleets.begin(); it < enemy_fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID())
      for(int i = it->TurnsRemaining(); i < 50; i++) {
	 ships_per_turn[i] -= it->NumShips();
      }
  }
  /*vector<Fleet> my_fleets = pw->MyFleets();
  for(vector<Fleet>::iterator it = my_fleets.begin(); it < my_fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID())
      for(int i = it->TurnsRemaining(); i < 50; i++) {
	ships_per_turn[i] += it->NumShips();
      }
  }*/
  return ships_per_turn;
}

void initialise_my_planets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = planets_state.find(it->PlanetID())->second;
    ps->SetPlanet(new Planet(*it));
  }
}

void initialise_my_unsafe_planets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    check_and_maybe_set_as_my_unsafe_planet(*it);
  }
}

void check_and_maybe_set_as_my_unsafe_planet(Planet p) {
  PlanetState* ps = planets_state.find(p.PlanetID())->second;
  vector<int>* ships_per_turn = ps->GetAvailableShipsPerTurn();
  for(int turn = 0; turn < 50; turn++) {
    if(ships_per_turn->at(turn) < 1) {
      ps->SetUnsafePlanet(true);
      ps->SetUnsafeInTurn(turn);
      break;
    }
  }
}

void initialise_available_ships_in_my_planets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = planets_state.find(it->PlanetID())->second;
    ps->SetAvailableShips(available_ships_in_my_planet(*it));
  }
}

int available_ships_in_my_planet(Planet p) {
  PlanetState* ps = planets_state.find(p.PlanetID())->second;
  vector<int>* ships_per_turn = ps->GetAvailableShipsPerTurn();
  int available_ships = 999999;
  for(int i = 1; i < 50; i++) {
    if(ships_per_turn->at(i) < available_ships)
      available_ships = ships_per_turn->at(i);
  }
  return available_ships;
}


/////////////////////////////////////////////////////////////////////////////////////////
// D E F E N S E
/////////////////////////////////////////////////////////////////////////////////////////
void defense() {
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    Planet p = *(it->second->GetPlanet());
    if(planets_state.find(p.PlanetID())->second->IsUnsafePlanet() && p.GrowthRate() >= 3) {
      try_to_send_ships_to_unsafe_planet(p);
    }
  }
}

void try_to_send_ships_to_unsafe_planet(Planet unsafe_planet) {
  int ships_needed = -1 * planets_state.find(unsafe_planet.PlanetID())->second->GetAvailableShips() + 1;
  if(ships_needed <= 0) {
    // Planets that will be lost in this same turn, we can't do anything about it :(
    return;
  }
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    PlanetState* ps = it->second;
    int available_ships_now = min(ps->GetPlanet()->NumShips(), ps->GetAvailableShips());
    if(available_ships_now > 5) {
      int ships_to_send = min(ships_needed, available_ships_now - 5);
      ships_needed -= ships_to_send;
      ps->SetAvailableShips(ps->GetAvailableShips() - ships_to_send);
      pw->IssueOrder(it->first, unsafe_planet.PlanetID(), ships_to_send);
      if(ships_needed <= 0) break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
// A T T A C K
/////////////////////////////////////////////////////////////////////////////////////////
void attack(int turn) {
  int strongest_planet;
  int strongest_planet_ships;
  vector<Planet> my_planets = pw->MyPlanets();
  for (int i = 0; i < my_planets.size(); ++i) {
    PlanetState* ps = planets_state.find(my_planets[i].PlanetID())->second;
    if(ps->GetAvailableShips() > 0)
      try_to_attack_from_planet(my_planets[i].PlanetID(), ps->GetAvailableShips(), turn);
  }
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
  /*  ofstream myfile; myfile.open("output",ios::app);
  myfile << growth_rate_score + num_ships_score + distance_score << " = " <<
    growth_rate << "("<<growth_rate_score<<")   + " << num_ships << " (" <<
    num_ships_score << ")   + " << distance << " (" << distance_score << ")\n";
    myfile.close();*/
  return growth_rate_score + num_ships_score + distance_score + enemy_bonus;
}


/////////////////////////////////////////////////////////////////////////////////////////
// U T I L S
/////////////////////////////////////////////////////////////////////////////////////////
int min(int a, int b) {
  if(a > b) return b;
  else return a;
}

void clear_maps(){
  planets_state.clear();
}

void print_planets_state() {
  ofstream myfile; myfile.open("output",ios::app);
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    myfile << "PLANET: " << it->first <<"\n";
    print_planet_state(it->second);
  }
  myfile << "\n-----------------------------------\n";
  myfile.close();
}

void print_planet_state(PlanetState* ps) {
  ofstream myfile; myfile.open("output",ios::app);
  Planet p = *(ps->GetPlanet());
  myfile << "is_unsafe: " << ps->IsUnsafePlanet() << "\n";
  myfile << "is_unsafe_turn: " << ps->GetUnsafeInTurn() << "\n";
  myfile << "available_ships: " << ps->GetAvailableShips() << "\n";
  myfile << "num_ships: " << p.NumShips() << "\n";
  vector<int>* available_ships_per_turn = ps->GetAvailableShipsPerTurn();
  for(int i = 0; i < available_ships_per_turn->size(); ++i)
    myfile << available_ships_per_turn->at(i) << " ";
  myfile << "------\n";
  myfile.close();
}
