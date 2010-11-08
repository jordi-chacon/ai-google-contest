#include "GameState.h"


void GameState::init(PlanetWars* p) {
  pw = p;
  init_ships_available_per_turn_and_planet();
  init_my_planets();
  init_my_unsafe_planets();
  init_available_ships_in_my_planets();
}

void GameState::clear() {
  planets_state.clear();
}

PlanetState* GameState::get_planet_state(int planet_id) {
  return planets_state.find(planet_id)->second;
}

vector<Planet*> GameState::get_unsafe_planets() {
  vector<Planet*> unsafe_planets;
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(it->second->IsUnsafePlanet()) unsafe_planets.push_back(it->second->GetPlanet());
  }
  return unsafe_planets;
}

vector<Planet*> GameState::get_safe_planets() {
  vector<Planet*> safe_planets;
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(!it->second->IsUnsafePlanet()) safe_planets.push_back(it->second->GetPlanet());
  }
  return safe_planets;
}

void GameState::print_game_state() {
  ofstream myfile; myfile.open("output",ios::app);
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    myfile << "PLANET: " << it->first <<"\n";
    print_planet_state(it->second);
  }
  myfile << "\n-----------------------------------\n";
  myfile.close();
}

void GameState::print_planet_state(int id) {
  print_planet_state(planets_state[id]);
}

void GameState::print_planet_state(PlanetState* ps) {
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


////////  P R I V A T E //////////////////////////////////////////////////////////////

void GameState::init_ships_available_per_turn_and_planet() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = new PlanetState();
    planets_state[it->PlanetID()] = ps;
    ps->SetAvailableShipsPerTurn(ships_available_in_planet_per_turn(*it));
  }
}

vector<int> GameState::ships_available_in_planet_per_turn(Planet p) {
  vector<int> ships_per_turn = compute_ships_available_per_turn_with_moving_fleet(p);
  return compute_ships_available_per_turn_based_on_growth_rate(ships_per_turn, p);
}

vector<int> GameState::compute_ships_available_per_turn_with_moving_fleet(Planet p) {
  vector<int> ships_per_turn(50, p.NumShips());
  vector<Fleet> enemy_fleets = pw->EnemyFleets();
  for(vector<Fleet>::iterator it = enemy_fleets.begin(); it < enemy_fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID())
      for(int i = it->TurnsRemaining(); i < 50; i++) {
	ships_per_turn[i] -= it->NumShips();
      }
  }
  vector<Fleet> my_fleets = pw->MyFleets();
  for(vector<Fleet>::iterator it = my_fleets.begin(); it < my_fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID())
      for(int i = it->TurnsRemaining(); i < 50; i++) {
	ships_per_turn[i] += it->NumShips();
      }
  }
  return ships_per_turn;
}

vector<int> GameState::compute_ships_available_per_turn_based_on_growth_rate(vector<int> ships_per_turn,
									     Planet p) {
  for(int i = 1; i < 50; i++) {
    if(ships_per_turn[i-1] > 0) ships_per_turn[i] += p.GrowthRate();
    else if(ships_per_turn[i-1] < 0) ships_per_turn[i] -= p.GrowthRate();;
  }
  return ships_per_turn;
}

void GameState::init_my_planets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = planets_state.find(it->PlanetID())->second;
    ps->SetPlanet(new Planet(*it));
  }
}

void GameState::init_my_unsafe_planets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    check_and_maybe_set_as_my_unsafe_planet(*it);
  }
}

void GameState::check_and_maybe_set_as_my_unsafe_planet(Planet p) {
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

void GameState::init_available_ships_in_my_planets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = planets_state.find(it->PlanetID())->second;
    ps->SetAvailableShips(available_ships_in_my_planet(*it));
  }
}

int GameState::available_ships_in_my_planet(Planet p) {
  PlanetState* ps = planets_state.find(p.PlanetID())->second;
  vector<int>* ships_per_turn = ps->GetAvailableShipsPerTurn();
  int available_ships = 999999;
  for(int i = 0; i < 50; i++) {
    if(ships_per_turn->at(i) < available_ships)
      available_ships = ships_per_turn->at(i);
  }
  return available_ships;
}

