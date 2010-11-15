#include "GameState.h"


void GameState::Init(PlanetWars* p) {
  pw = p;
  InitShipsAvailablePerTurnAndPlanet();
  InitPlanets();
  InitMyUnsafePlanets();
  InitNeutralPlanets();
  InitAvailableShipsInPlanets();
}

void GameState::Clear() {
  planets_state.clear();
  neutral_planets_state.clear();
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

vector<Planet*> GameState::GetMySafePlanets() {
  vector<Planet*> safe_planets;
  for(map<int, PlanetState*>::iterator it = planets_state.begin(); it != planets_state.end(); ++it) {
    if(!it->second->IsUnsafePlanet() && it->second->GetPlanet()->Owner() == 1)
      safe_planets.push_back(it->second->GetPlanet());
  }
  return safe_planets;
}

std::vector<Planet*> GameState::GetMySafePlanetsSortedByDistanceToPlanet(int planet_id) {
  vector<Planet*> safe_planets = GetMySafePlanets();
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

bool GameState::AreMySafePlanetsWithAvailableShips() {
  vector<Planet*> safe_planets = GetMySafePlanets();
  for(vector<Planet*>::iterator it = safe_planets.begin(); it != safe_planets.end(); ++it) {
    PlanetState* ps = GetPlanetState((*it)->PlanetID());
    if(ps->GetAvailableShips() > 1) return true;
  }
  return false;
}

int GameState::GetNeededShipsToSafeUnsafePlanet(int planet_id) {
  return -1 * GetPlanetState(planet_id)->GetNeededShipsToSafe();
}

int GameState::GetAvailableShips(int planet_id) {
  return GetPlanetState(planet_id)->GetAvailableShips();
}

vector<int>* GameState::GetAvailableShipsPerTurn(int planet_id) {
  return GetPlanetState(planet_id)->GetAvailableShipsPerTurn();
}

int GameState::GetPlanetLostInTurn(int planet_id) {
  return GetPlanetState(planet_id)->GetUnsafeInTurn();
}

int GameState::GetNeededShipsToTakeNeutralPlanet(int planet_id, int turn) {
  vector<TurnState> state_per_turn = neutral_planets_state[planet_id];
  int needed = -1;
  if(state_per_turn[turn].owner != 1)
    needed = state_per_turn[turn].num_ships;
  else {
    for(int i = turn + 1; i < 50; i++) {
      if(state_per_turn[i].owner != 1) {
	needed = state_per_turn[i].num_ships;
	break;
      }
    }
  }
  return needed;
}

void GameState::SetAvailableShipsPerTurn(int planet_id, vector<int> v) {
  GetPlanetState(planet_id)->SetAvailableShipsPerTurn(v);
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
  vector<Planet> planets = pw->Planets();
  for(vector<Planet>::iterator it = planets.begin(); it < planets.end(); ++it) {
    if(it->Owner() != 0) {
      PlanetState* ps = new PlanetState();
      planets_state[it->PlanetID()] = ps;
      ps->SetAvailableShipsPerTurn(ShipsAvailableInPlanetPerTurn(*it));
    }
  }
}

vector<int> GameState::ShipsAvailableInPlanetPerTurn(Planet p) {
  vector<int> ships_per_turn = ComputeShipsAvailablePerTurnWithMovingFleet(p);
  return ComputeShipsAvailablePerTurnBasedOnGrowthRate(ships_per_turn, p);
}

vector<int> GameState::ComputeShipsAvailablePerTurnWithMovingFleet(Planet p) {
  vector<int> ships_per_turn(50, p.NumShips());
  vector<Fleet> fleets = pw->Fleets();
  for(vector<Fleet>::iterator it = fleets.begin(); it < fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID()) {
      if(p.Owner() == 1 && it->Owner() == 1 || p.Owner() > 1 && it->Owner() > 1)
	for(int i = it->TurnsRemaining(); i < 50; i++) {
	  ships_per_turn[i] += it->NumShips();
	}
      else
	for(int i = it->TurnsRemaining(); i < 50; i++) {
	  ships_per_turn[i] -= it->NumShips();
	}
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

void GameState::InitPlanets() {
  vector<Planet> planets = pw->Planets();
  for(vector<Planet>::iterator it = planets.begin(); it < planets.end(); ++it) {
    if(it->Owner() > 0 ) {
      PlanetState* ps = planets_state.find(it->PlanetID())->second;
      ps->SetPlanet(new Planet(*it));
    }
  }
}

void GameState::InitMyUnsafePlanets() {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    CheckAndMaybeSetAsMyUnsafePlanet(*it);
  }
}

void GameState::InitNeutralPlanets() {
  vector<Planet> neutral_planets = pw->NeutralPlanets();
  for(vector<Planet>::iterator it = neutral_planets.begin(); it < neutral_planets.end(); ++it) {
    vector<TurnState> state_per_turn;
    neutral_planets_state[it->PlanetID()] = ComputeTurnStateNeutralPlanet(*it);
  }
}

vector<TurnState> GameState::ComputeTurnStateNeutralPlanet(Planet p) {
  vector<TurnState> state_per_turn;
  int planet_id = p.PlanetID();
  vector<Fleet> fleets = pw->Fleets();
  vector<Fleet> fleets_attacking_planet;
  for(vector<Fleet>::iterator it = fleets.begin(); it < fleets.end(); ++it) {
    if(it->DestinationPlanet() == planet_id)
      fleets_attacking_planet.push_back(*it);
  }
  TurnState first_ts;
  first_ts.owner = 0;
  first_ts.num_ships = p.NumShips();
  state_per_turn.push_back(first_ts);
  for(int turn = 1; turn < 50; turn++) {
    vector<Fleet> fleets_arriving_now;
    for(vector<Fleet>::iterator it = fleets_attacking_planet.begin();
	it < fleets_attacking_planet.end(); ++it) {
      if(it->TurnsRemaining() == turn)
	fleets_arriving_now.push_back(*it);
    }
    state_per_turn.push_back(ComputeNextTurnState(p, state_per_turn[turn-1], fleets_arriving_now));
  }
  return state_per_turn;
}

TurnState GameState::ComputeNextTurnState(Planet p, TurnState prev, vector<Fleet> arriving_fleets) {
  TurnState ts;
  ts.owner = prev.owner;
  ts.num_ships = prev.num_ships;
  if(ts.owner != 0) ts.num_ships += p.GrowthRate();
  pair<int,int> fleets_ships = ComputeAmountArrivingFleetsPerPlayer(arriving_fleets);
  if(ts.owner == 1) {
    ts.num_ships += fleets_ships.first - fleets_ships.second;
    ts = MaybeChangeNeutralPlanetOwner(ts, 2);
  }
  else if(ts.owner > 1) {
    ts.num_ships += fleets_ships.second - fleets_ships.first;
    ts = MaybeChangeNeutralPlanetOwner(ts, 1);
  }
  else {
    ts = ComputeTurnStateWhenNeutralOwner(ts, fleets_ships);
  }
  return ts;
}

pair<int,int> GameState::ComputeAmountArrivingFleetsPerPlayer(vector<Fleet> arriving_fleets) {
  pair<int,int> fleets_ships(0,0);
  for(vector<Fleet>::iterator it = arriving_fleets.begin(); it < arriving_fleets.end(); ++it) {
    if(it->Owner() == 1) fleets_ships.first += it->NumShips();
    else fleets_ships.second += it->NumShips();
  }
  return fleets_ships;
}

TurnState GameState::ComputeTurnStateWhenNeutralOwner(TurnState ts, pair<int,int> fleets_ships) {
  while(ts.num_ships >= 0) {
    if(fleets_ships.first > 0 && fleets_ships.second > 0) {
      ts.num_ships -= 2;
      fleets_ships.first--;
      fleets_ships.second--;
    }
    else break;
  }
  if(ts.num_ships < 0) {
    if(fleets_ships.first > fleets_ships.second) {
      ts.num_ships = fleets_ships.first - fleets_ships.second;
      ts.owner = 1;
    }
    else {
      ts.num_ships = fleets_ships.second - fleets_ships.first;
      ts.owner = 2;
    }
  }
  else if(fleets_ships.first > 0) {
    ts.num_ships -= fleets_ships.first;
    ts = MaybeChangeNeutralPlanetOwner(ts, 1);
  }
  else {
    ts.num_ships -= fleets_ships.second;
    ts = MaybeChangeNeutralPlanetOwner(ts, 2);
  }
  return ts;
}

TurnState GameState::MaybeChangeNeutralPlanetOwner(TurnState ts, int owner) {
  if(ts.num_ships < 0) {
    ts.num_ships *= -1;
    ts.owner = owner;
  }
  return ts;
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

void GameState::InitAvailableShipsInPlanets() {
  for(map<int, PlanetState*>::iterator it = planets_state.begin();
      it != planets_state.end(); ++it) {
    PlanetState* ps = it->second;
    ps->SetAvailableShips(AvailableShipsInPlanet(ps));
  }
}

int GameState::AvailableShipsInPlanet(PlanetState* ps) {
  vector<int>* ships_per_turn = ps->GetAvailableShipsPerTurn();
  int available_ships = 999999;
  for(int i = 0; i < 50; i++) {
    if(ships_per_turn->at(i) < available_ships)
      available_ships = ships_per_turn->at(i);
  }
  return available_ships - 1;
}

