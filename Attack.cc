#include "Attack.h"


void Attack::DoAttack(PlanetWars* p, GameState* gs, int turn) {
  pw = p;
  game_state = gs;
  vector<int> my_frontier_planets = ComputeMyFrontierPlanets();
  SendFleetsToFrontierPlanets(my_frontier_planets);
  for (int i = 0; i < my_frontier_planets.size(); ++i) {
    PlanetState* ps = game_state->GetPlanetState(my_frontier_planets[i]);
    if(ps->GetAvailableShips() > 0)
      TryToAttackFromPlanet(my_frontier_planets[i], ps->GetAvailableShips(), turn);
  }
}

vector<int> Attack::ComputeMyFrontierPlanets() {
  vector<int> my_frontier_planets;
  vector<Planet> my_planets = pw->MyPlanets();
  vector<Planet> enemy_planets = pw->EnemyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    if(IsFrontier(*it, &my_planets, &enemy_planets))
       my_frontier_planets.push_back(it->PlanetID());
  }
  return my_frontier_planets;
}

bool Attack::IsFrontier(Planet p, vector<Planet>* my_planets, vector<Planet>* enemy_planets) {
  int min_distance_to_enemy = 1000;
  int closest_enemy_planet = 1;
  for(vector<Planet>::iterator it2 = enemy_planets->begin(); it2 < enemy_planets->end(); ++it2) {
    int aux_distance = pw->Distance(p.PlanetID(), it2->PlanetID());
    if(aux_distance < min_distance_to_enemy) {
      min_distance_to_enemy = aux_distance;
      closest_enemy_planet = it2->PlanetID();
    }
  }
  bool is_frontier = true;
  for(vector<Planet>::iterator it2 = my_planets->begin(); it2 < my_planets->end(); ++it2) {
    if(pw->Distance(it2->PlanetID(), closest_enemy_planet) < min_distance_to_enemy) {
      is_frontier = false;
      break;
    }
  }
  return is_frontier;
}

void Attack::SendFleetsToFrontierPlanets(vector<int> frontier_planets) {
  vector<Planet> my_planets = pw->MyPlanets();
  for(vector<Planet>::iterator it = my_planets.begin(); it < my_planets.end(); ++it) {
    int min_distance_to_frontier = 1000;
    int closest_frontier = 1;
    for(vector<int>::iterator it2 = frontier_planets.begin(); it2 < frontier_planets.end(); ++it2) {
      int aux_distance = pw->Distance(it->PlanetID(), *it2);
      if(aux_distance < min_distance_to_frontier) {
	min_distance_to_frontier = aux_distance;
	closest_frontier = *it2;
      }
    }
    if(closest_frontier != it->PlanetID()) {
      int available_ships = game_state->GetAvailableShips(it->PlanetID());
      pw->IssueOrder(it->PlanetID(), closest_frontier, available_ships);
    }
  }
}

void Attack::TryToAttackFromPlanet(int my_planet, int available_ships_for_attack, int turn) {
  vector<PlanetScore> planets_score = ComputePlanetsListSortedByScore(turn, my_planet);
  DecideWhereToAttack(my_planet, available_ships_for_attack, planets_score);
}

vector<PlanetScore> Attack::ComputePlanetsListSortedByScore(int turn, int my_planet) {
  vector<Planet> not_my_planets = pw->NotMyPlanets();
  vector<PlanetScore> not_my_planets_score;
  for(vector<Planet>::const_iterator it = not_my_planets.begin(); it < not_my_planets.end(); ++it) {
    if(!IsStupidToGoToPlanet(*it, turn)) {
      double score = ComputeScore(it->GrowthRate(), pw->Distance(it->PlanetID(), my_planet),
				   it->NumShips(), it->Owner() == 2);
      PlanetScore* ps = new PlanetScore(*it, score);
      not_my_planets_score.push_back(*ps);
    }
  }
  sort(not_my_planets_score.begin(), not_my_planets_score.end());
  return not_my_planets_score;
}

void Attack::DecideWhereToAttack(int my_planet, int ships_for_attack, vector<PlanetScore> planets_score) {
  vector<Fleet> fleets = pw->Fleets();
  for(vector<PlanetScore>::iterator it = planets_score.begin(); it < planets_score.end(); ++it) {
    Planet p = *(it->GetPlanet());
    if(ships_for_attack > p.NumShips() + 1) {
      if(p.Owner() == 0 && !IsEnoughFleetsAttackingPlanet(fleets, p)) {
	pw->IssueOrder(my_planet, p.PlanetID(), p.NumShips() + 1);
	ships_for_attack -= p.NumShips() + 1;
      }
      else if(p.Owner() > 1) {
	pw->IssueOrder(my_planet, p.PlanetID(), ships_for_attack - 1);
	break;
      }
    }
  }
}

bool Attack::IsEnoughFleetsAttackingPlanet(vector<Fleet> fleets, Planet p) {
  int fleet_sum = 0;
  for(vector<Fleet>::const_iterator it = fleets.begin(); it < fleets.end(); ++it) {
    if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 1)
      fleet_sum += it->NumShips();
    //if(it->DestinationPlanet() == p.PlanetID() && it->Owner() == 2)
    //fleet_sum -= it->NumShips();
  }
  return fleet_sum > p.NumShips();
}

bool Attack::IsStupidToGoToPlanet(Planet p, int turn) {
  return IsNeutralPlanetWithManyFleets(p) || IsReallyGoodNeutralPlanetCloseToEnemy(p);
}

bool Attack::IsNeutralPlanetWithManyFleets(Planet p) {
  return (p.Owner() == 0 && p.NumShips() > 30 && p.GrowthRate() == 1) ||
    (p.Owner() == 0 && p.NumShips() > 50 && p.GrowthRate() == 2);
}

bool Attack::IsReallyGoodNeutralPlanetCloseToEnemy(Planet p) {
  if(p.GrowthRate() < 3) return false;
  vector<Planet> enemy_planets = pw->EnemyPlanets();
  for(vector<Planet>::iterator it = enemy_planets.begin(); it < enemy_planets.end(); ++it) {
    if(pw->Distance(p.PlanetID(), it->PlanetID()) < 5) return true;
  }
  return false;
}

double Attack::ComputeScore(int growth_rate, int distance, int num_ships, bool is_enemy) {
  if(num_ships == 0) num_ships = 1;
  if(distance == 0) distance = 1;
  double growth_rate_score = 60 * growth_rate;
  double num_ships_score = 1000 / num_ships / 2;
  double distance_score;
  if(distance > 10) distance_score = -100000 * distance;
  else distance_score = 800 / distance;
  double enemy_bonus = 0;
  /* ofstream myfile; myfile.open("output",ios::app);
  myfile << growth_rate_score + num_ships_score + distance_score << " = " <<
    growth_rate << "("<<growth_rate_score<<")   + " << num_ships << " (" <<
    num_ships_score << ")   + " << distance << " (" << distance_score << ")\n";
    myfile.close();*/
  return growth_rate_score + num_ships_score + distance_score + enemy_bonus;
}
