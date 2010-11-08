#include "Attack.h"


void Attack::attack(PlanetWars* p, GameState* gs, int turn) {
  pw = p;
  game_state = gs;
  int strongest_planet;
  int strongest_planet_ships;
  vector<Planet> my_planets = pw->MyPlanets();
  for (int i = 0; i < my_planets.size(); ++i) {
    PlanetState* ps = game_state->get_planet_state(my_planets[i].PlanetID());
    if(ps->GetAvailableShips() > 0)
      try_to_attack_from_planet(my_planets[i].PlanetID(), ps->GetAvailableShips(), turn);
  }
}

void Attack::try_to_attack_from_planet(int my_planet, int available_ships_for_attack, int turn) {
  vector<PlanetScore> planets_score = compute_planets_list_sorted_by_score(turn, my_planet);
  decide_where_to_attack(my_planet, available_ships_for_attack, planets_score);
}

vector<PlanetScore> Attack::compute_planets_list_sorted_by_score(int turn, int my_planet) {
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

void Attack::decide_where_to_attack(int my_planet, int ships_for_attack, vector<PlanetScore> planets_score) {
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

bool Attack::is_enough_fleets_attacking_planet(vector<Fleet> fleets, Planet p) {
  int fleet_sum = 0;
  for(vector<Fleet>::const_iterator it = fleets.begin(); it < fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 1)
      fleet_sum += it->NumShips();
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 2)
      fleet_sum -= it->NumShips();
  }
  return fleet_sum > p.NumShips() + 5;
}

bool Attack::is_stupid_to_go_to_planet(Planet p, int turn) {
  return is_neutral_planet_with_many_fleets(p);
}

bool Attack::is_neutral_planet_with_many_fleets(Planet p) {
  return p.Owner() == 0 && p.NumShips() > 30;
}

double Attack::compute_score(int growth_rate, int distance, int num_ships, bool is_enemy) {
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
