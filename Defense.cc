#include "Defense.h"

void Defense::defense(PlanetWars* p, GameState* gs) {
  pw = p;
  game_state = gs;
  vector<Planet*> unsafe_planets = game_state->get_unsafe_planets();
  for(vector<Planet*>::iterator it = unsafe_planets.begin(); it < unsafe_planets.end(); ++it) {
    if((*it)->GrowthRate() >= 3) try_to_send_ships_to_unsafe_planet(**it);
  }
}

void Defense::try_to_send_ships_to_unsafe_planet(Planet unsafe_planet) {
  int ships_needed = -1 * game_state->get_planet_state(unsafe_planet.PlanetID())->GetAvailableShips() + 1;
  if(ships_needed <= 0) {
    // Planets that will be lost in this same turn, we can't do anything about it :(
    return;
  }
  vector<Planet*> safe_planets = game_state->get_safe_planets();
  for(vector<Planet*>::iterator it = safe_planets.begin(); it < safe_planets.end(); ++it) {
    PlanetState* ps = game_state->get_planet_state((*it)->PlanetID());
    int available_ships_now = min((*it)->NumShips(), ps->GetAvailableShips());
    if(available_ships_now > 5) {
      int ships_to_send = min(ships_needed, available_ships_now - 5);
      ships_needed -= ships_to_send;
      ps->SetAvailableShips(ps->GetAvailableShips() - ships_to_send);
      pw->IssueOrder((*it)->PlanetID(), unsafe_planet.PlanetID(), ships_to_send);
      if(ships_needed <= 0) break;
    }
  }
}
