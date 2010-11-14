#include <algorithm>
#include "Defense.h"

void Defense::DoDefense(PlanetWars* p, GameState* gs) {
  pw = p;
  game_state = gs;
  for(int growth_rate = 5; growth_rate > 0; growth_rate--) {
    if(game_state->AreMySafePlanetsWithAvailableShips()) {
      vector<Planet*> unsafe_planets = game_state->GetUnsafePlanetsWithGrowthRate(growth_rate);
      vector<Planet*> still_unsafe_planets = TryToSendHelpBasic(unsafe_planets);
      //      TryToSendHelpComplex(still_unsafe_planets);
      }
  }
}

vector<Planet*> Defense::TryToSendHelpBasic(vector<Planet*> unsafe_planets) {
  for(vector<Planet*>::iterator it = unsafe_planets.begin(); it < unsafe_planets.end(); ++it) {
    int unsafe_planet_id = (*it)->PlanetID();
    vector<pair<int, int> > help;
    int ships_needed = game_state->GetNeededShipsToSafeUnsafePlanet(unsafe_planet_id);
    vector<Planet*> safe_planets = game_state->GetMySafePlanetsSortedByDistanceToPlanet(unsafe_planet_id);
    for(vector<Planet*>::iterator it2 = safe_planets.begin(); it2 < safe_planets.end(); ++it2) {
      int safe_planet_id = (*it2)->PlanetID();
      if(pw->Distance(unsafe_planet_id, safe_planet_id) <= game_state->GetPlanetLostInTurn(unsafe_planet_id)) {
	int to_send = min(ships_needed, game_state->GetAvailableShips(safe_planet_id));
	help.push_back(pair<int, int>(safe_planet_id, to_send));
	ships_needed -= to_send;
	if(ships_needed == 0) {
	  SendHelp(unsafe_planet_id, help);
	  unsafe_planets.erase(it);
	  break;
	}
      }
      else break;
    }
  }
  return unsafe_planets;
}

void Defense::TryToSendHelpComplex(vector<Planet*> unsafe_planets) {
  for(vector<Planet*>::iterator it = unsafe_planets.begin(); it < unsafe_planets.end(); ++it) {
    map<int, pair<int, int> > help = ComputeAllPossibleHelp(*it);
    SendHelpIfWillBeSaved(*it, help);
  }
}

map<int, pair<int, int> > Defense::ComputeAllPossibleHelp(Planet* p) {
  map<int, pair<int, int> > help;
  int accumulated_help = 0;
  vector<Planet*> safe_planets = game_state->GetMySafePlanetsSortedByDistanceToPlanet(p->PlanetID());
  for(vector<Planet*>::iterator it = safe_planets.begin(); it < safe_planets.end(); ++it) {
    int safe_planet_id = (*it)->PlanetID();
    int available_ships = game_state->GetAvailableShips(safe_planet_id);
    accumulated_help += available_ships;
    int turn = pw->Distance(safe_planet_id, p->PlanetID());
    help[turn] = pair<int,int>(safe_planet_id, available_ships);
  }
  return help;
}

void Defense::SendHelpIfWillBeSaved(Planet* p, map<int, pair<int, int> > all_possible_help) {
  vector<pair<int,int> > help;
  vector<int> available_ships_per_turn = vector<int>(50);
  CopyAvailableShipsPerTurnVector(available_ships_per_turn, p->PlanetID());
  for(int i = 0; i < 50; i++) {
    if(all_possible_help.find(i) != all_possible_help.end()) {
      help.push_back(all_possible_help[i]);
      for(int j = i; j < 50; j++) {
	bool mine_at_turn = false;
	if(available_ships_per_turn[j] > 0) mine_at_turn = true;
	available_ships_per_turn[j] += all_possible_help[i].second;
	if(available_ships_per_turn[j] > 0 && !mine_at_turn)
	  available_ships_per_turn[j] += p->GrowthRate() * 2;
      }
    }
    bool current_help_is_enough = true;
    for(int j = 0; j < 50; j++) {
      if(available_ships_per_turn[j] <= 0) {
	current_help_is_enough = false;
	break;
      }
    }
    if(current_help_is_enough) {
      game_state->SetAvailableShipsPerTurn(p->PlanetID(), available_ships_per_turn);
      SendHelp(p->PlanetID(), help);
    }
  }
}

void Defense::SendHelp(int unsafe_planet_id, vector<pair<int, int> > help) {
  for(vector<pair<int, int> >::iterator it = help.begin(); it < help.end(); ++it) {
    int sender_id = it->first;
    int ships_to_send = it->second;
    pw->IssueOrder(sender_id, unsafe_planet_id, ships_to_send);
    game_state->DecreaseAvailableShips(sender_id, ships_to_send);
  }
}

void Defense::CopyAvailableShipsPerTurnVector(vector<int> dst, int planet_id) {
  vector<int>* src = game_state->GetAvailableShipsPerTurn(planet_id);
  for(int i = 0; i < 50; i++) {
    dst[i] = src->at(i);
  }
}
