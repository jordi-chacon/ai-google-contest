#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "PlanetState.h"

using namespace std;

class GameState {
 public:
  void Init(PlanetWars* p);
  void Clear();
  PlanetState* GetPlanetState(int planet_id);
  std::vector<Planet*> GetUnsafePlanets();
  std::vector<Planet*> GetUnsafePlanetsWithGrowthRate(int gr);
  std::vector<Planet*> GetSafePlanets();
  std::vector<Planet*> GetSafePlanetsSortedByDistanceToPlanet(int planet_id);
  bool AreSafePlanetsWithAvailableShips();
  int GetNeededShipsToSafeUnsafePlanet(int planet_id);
  int GetAvailableShips(int planet_id);
  int GetPlanetLostInTurn(int planet_id);
  void DecreaseAvailableShips(int planet_id, int ships_sent);
  void PrintGameState();
  void PrintPlanetState(int planet_id);
  void PrintPlanetState(PlanetState* ps);

 private:
  void InitShipsAvailablePerTurnAndPlanet();
  void InitMyPlanets();
  std::vector<int> ShipsAvailableInPlanetPerTurn(Planet p);
  std::vector<int> ComputeShipsAvailablePerTurnWithMovingFleet(Planet p);
  std::vector<int> ComputeShipsAvailablePerTurnBasedOnGrowthRate(std::vector<int> ships_per_turn,
									 Planet p);
  void InitMyUnsafePlanets();
  void CheckAndMaybeSetAsMyUnsafePlanet(Planet p);
  void InitAvailableShipsInMyPlanets();
  int AvailableShipsInMyPlanet(Planet p);

  std::map<int, PlanetState*> planets_state;
  PlanetWars* pw;
};

#endif
