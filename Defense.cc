#include <algorithm>
#include "Defense.h"

void Defense::DoDefense(PlanetWars* p, GameState* gs) {
  pw = p;
  game_state = gs;
  for(int growth_rate = 5; growth_rate > 0; growth_rate--) {
    if(game_state->AreSafePlanetsWithAvailableShips()) {
      vector<Planet*> unsafe_planets = game_state->GetUnsafePlanetsWithGrowthRate(growth_rate);
      vector<Planet*> still_unsafe_planets = TryToSendHelpBasic(unsafe_planets);
    }
  }
}

vector<Planet*> Defense::TryToSendHelpBasic(vector<Planet*> unsafe_planets) {
  for(vector<Planet*>::iterator it = unsafe_planets.begin(); it < unsafe_planets.end(); ++it) {
    int unsafe_planet_id = (*it)->PlanetID();
    vector<pair<int, int> > help;
    int ships_needed = game_state->GetNeededShipsToSafeUnsafePlanet(unsafe_planet_id);
    vector<Planet*> safe_planets = game_state->GetSafePlanetsSortedByDistanceToPlanet(unsafe_planet_id);
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

void Defense::SendHelp(int unsafe_planet_id, vector<pair<int, int> > help) {
  for(vector<pair<int, int> >::iterator it = help.begin(); it < help.end(); ++it) {
    int sender_id = it->first;
    int ships_to_send = it->second;
    pw->IssueOrder(sender_id, unsafe_planet_id, ships_to_send);
    game_state->DecreaseAvailableShips(sender_id, ships_to_send);
  }
}
