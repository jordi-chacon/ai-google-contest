#include "GameState.h"


void GameState::Init(PlanetWars* p) {
  pw = p;
  InitShipsAvailablePerTurnAndPlanet();
  InitMyPlanets();
  InitMyUnsafePlanets();
  InitAvailableShipsInMyPlanets();
}

void GameState::Clear() {
  planets_state.clear();
}

PlanetState* GameState::GetPlanetState(int planet_id) {
  return planets_state.find(planet_id)->second;
}

vector<Planet*> GameState::GetUnsafePlanets() {
  vector<Planet*> unsafe_planets;
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(it->second->IsUnsafePlanet()) unsafe_planets.push_back(it->second->GetPlanet());
  }
  return unsafe_planets;
}

vector<Planet*> GameState::GetUnsafePlanetsWithGrowthRate(int gr) {
  vector<Planet*> unsafe_planets;
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(it->second->IsUnsafePlanet() && it->second->GetPlanet()->GrowthRate() == gr)
      unsafe_planets.push_back(it->second->GetPlanet());
  }
  return unsafe_planets;
}

vector<Planet*> GameState::GetSafePlanets() {
  vector<Planet*> safe_planets;
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(!it->second->IsUnsafePlanet()) safe_planets.push_back(it->second->GetPlanet());
  }
  return safe_planets;

void defense() {
  vector<Planet*> unsafe = game_state.GetUnsafePlanets();
  for(vector<Planet*>::iterator it = unsafe.begin(); it < unsafe.end(); ++it) {
    if((*it)->GrowthRate() >= 3)
      try_to_send_ships_to_unsafe_planet(**it);
  }
}

void try_to_send_ships_to_unsafe_planet(Planet unsafe_planet) {
  int ships_needed = game_state.GetAvailableShips(unsafe_planet.PlanetID());
  if(ships_needed <= 0) {
    // Planets that will be lost in this same turn, we can't do anything about it :(
    return;
  }
  vector<Planet*> safe = game_state.GetSafePlanets();
  for(vector<Planet*>::iterator it = safe.begin(); it < safe.end(); ++it) {
    PlanetState* ps = game_state.GetPlanetState((*it)->PlanetID());
    int available_ships_now = min(ps->GetPlanet()->NumShips(), ps->GetAvailableShips());
    if(available_ships_now > 5) {
      int ships_to_send = min(ships_needed, available_ships_now - 5);
      ships_needed -= ships_to_send;
      ps->SetAvailableShips(ps->GetAvailableShips() - ships_to_send);
      pw->IssueOrder((*it)->PlanetID(), unsafe_planet.PlanetID(), ships_to_send);
      if(ships_needed <= 0) break;
    }
  }
}

}

std::vector<Planet*> GameState::GetSafePlanetsSortedByDistanceToPlanet(int planet_id) {
  vector<Planet*> safe_planets = GetSafePlanets();
  int key, i;
  for(int j = 1; j < safe_planets.size(); j++) {
    Planet* p = safe_planets[j];
    int distance = pw->Distance(safe_planets[j]->PlanetID(), planet_id);
    i = j-1;
    while(i >= 0 && pw->Distance(safe_planets[i]->PlanetID(), planet_id) > distance) {
      safe_planets[i+1] = safe_planets[i];
      i--;
    }
    safe_planets[i+1] = p;
  }
  return safe_planets;
}

bool GameState::AreSafePlanetsWithAvailableShips() {
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(!it->second->IsUnsafePlanet() && it->second->GetAvailableShips() > 1) return true;
  }
  return false;
}

int GameState::GetNeededShipsToSafeUnsafePlanet(int planet_id) {
  return -1 * GetPlanetState(planet_id)->GetNeededShipsToSafe();
}

int GameState::GetAvailableShips(int planet_id) {
  return GetPlanetState(planet_id)->GetAvailableShips();
}

int GameState::GetPlanetLostInTurn(int planet_id) {
  return GetPlanetState(planet_id)->GetUnsafeInTurn();
}

void GameState::DecreaseAvailableShips(int planet_id, int ships_sent) {
  PlanetState* ps = GetPlanetState(planet_id);
  ps->SetAvailableShips(ps->GetAvailableShips() - ships_sent);
}

void GameState::PrintGameState() {
  ofstream myfile; myfile.open("output",ios::app);
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    PrintPlanetState(it->second);
  }
  myfile << "\n-----------------------------------\n";
  myfile.close();
}

void GameState::PrintPlanetState(int id) {
  PrintPlanetState(planets_state[id]);
}

void GameState::PrintPlanetState(PlanetState* ps) {
  ofstream myfile; myfile.open("output",ios::app);
  Planet p = *(ps->GetPlanet());
  myfile << "PLANET: " << ps->GetPlanet()->PlanetID() <<"\n";
  myfile << "is_unsafe: " << ps->IsUnsafePlanet() << "\n";
  myfile << "is_unsafe_turn: " << ps->GetUnsafeInTurn() << "\n";
  myfile << "available_ships: " << ps->GetAvailableShips() << "\n";
  myfile << "num_ships: " << p.NumShips() << "\n";
  vector<int>* available_ships_per_turn = ps->GetAvailableShipsPerTurn();
  for(int i = 0; i < available_ships_per_turn->size(); ++i)
    myfile << available_ships_per_turn->at(i) << " ";
  myfile << "\n------\n";
  myfile.close();
}


////////  P R I V A T E //////////////////////////////////////////////////////////////

void GameState::InitShipsAvailablePerTurnAndPlanet() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = new PlanetState();
    planets_state[it->PlanetID()] = ps;
    ps->SetAvailableShipsPerTurn(ShipsAvailableInPlanetPerTurn(*it));
  }
}

vector<int> GameState::ShipsAvailableInPlanetPerTurn(Planet p) {
  vector<int> ships_per_turn = ComputeShipsAvailablePerTurnWithMovingFleet(p);
  return ComputeShipsAvailablePerTurnBasedOnGrowthRate(ships_per_turn, p);
}

vector<int> GameState::ComputeShipsAvailablePerTurnWithMovingFleet(Planet p) {
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

vector<int> GameState::ComputeShipsAvailablePerTurnBasedOnGrowthRate(vector<int> ships_per_turn,
									     Planet p) {
  int turns_same_owner = 0;
  for(int i = 1; i < 50; i++) {
    if((ships_per_turn[i-1] > 0 && ships_per_turn[i-1] < 0) ||
       (ships_per_turn[i-1] < 0 && ships_per_turn[i-1] > 0)) turns_same_owner = 0;
    turns_same_owner++;
    if(ships_per_turn[i-1] > 0) ships_per_turn[i] += p.GrowthRate() * turns_same_owner;
    else if(ships_per_turn[i-1] < 0) ships_per_turn[i] -= p.GrowthRate() * turns_same_owner;
  }
  return ships_per_turn;
}

void GameState::InitMyPlanets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = planets_state.find(it->PlanetID())->second;
    ps->SetPlanet(new Planet(*it));
  }
}

void GameState::InitMyUnsafePlanets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    CheckAndMaybeSetAsMyUnsafePlanet(*it);
  }
}

void GameState::CheckAndMaybeSetAsMyUnsafePlanet(Planet p) {
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

void GameState::InitAvailableShipsInMyPlanets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    PlanetState* ps = planets_state.find(it->PlanetID())->second;
    ps->SetAvailableShips(AvailableShipsInMyPlanet(*it));
  }
}

int GameState::AvailableShipsInMyPlanet(Planet p) {
  PlanetState* ps = planets_state.find(p.PlanetID())->second;
  vector<int>* ships_per_turn = ps->GetAvailableShipsPerTurn();
  int available_ships = 999999;
  for(int i = 0; i < 50; i++) {
    if(ships_per_turn->at(i) < available_ships)
      available_ships = ships_per_turn->at(i);
  }
  return available_ships - 1;
}

