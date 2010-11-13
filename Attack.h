#ifndef ATTACK_H_
#define ATTACK_H_

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "PlanetWars.h"
#include "PlanetScore.h"
#include "GameState.h"

using namespace std;

class Attack {
 public:
  void DoAttack(PlanetWars* pw, GameState* game_state, int turn);

 private:
  vector<int> ComputeMyFrontierPlanets();
  bool IsFrontier(Planet p, vector<Planet>* my_planets, vector<Planet>* enemy_planets);
  void SendFleetsToFrontierPlanets(vector<int> frontier_planets);
  void TryToAttackFromPlanet(int my_planet, int available_ships_for_attack, int turn);
  vector<PlanetScore> ComputePlanetsListSortedByScore(int turn, int my_planet);
  void DecideWhereToAttack(int my_planet, int ships_for_attack, vector<PlanetScore> ps_score);
  bool IsEnoughFleetsAttackingPlanet(vector<Fleet> fleets, Planet p);
  bool IsStupidToGoToPlanet(Planet p, int turn);
  bool IsNeutralPlanetWithManyFleets(Planet p);
  bool IsReallyGoodNeutralPlanetCloseToEnemy(Planet p);
  double ComputeScore(int growth_rate, int distance, int num_ships, bool is_enemy);

  PlanetWars* pw;
  GameState* game_state;
};

#endif
