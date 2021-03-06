#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "PlanetState.h"

using namespace std;

struct TurnState {
  int owner;
  int num_ships;
};


class GameState {
 public:
  void Init(PlanetWars* p);
  void Clear();
  PlanetState* GetPlanetState(int planet_id);
  std::vector<Planet*> GetUnsafePlanets();
  std::vector<Planet*> GetUnsafePlanetsWithGrowthRate(int gr);
  std::vector<Planet*> GetMySafePlanets();
  std::vector<Planet*> GetMySafePlanetsSortedByDistanceToPlanet(int planet_id);
  bool AreMySafePlanetsWithAvailableShips();
  int GetNeededShipsToSafeUnsafePlanet(int planet_id);
  int GetAvailableShips(int planet_id);
  vector<int>* GetAvailableShipsPerTurn(int planet_id);
  int GetPlanetLostInTurn(int planet_id);
  int GetNeededShipsToTakeNeutralPlanet(int planet_id, int turn);
  void SetAvailableShipsPerTurn(int planet_id, vector<int> v);
  void DecreaseAvailableShips(int planet_id, int ships_sent);
  void PrintGameState();
  void PrintPlanetState(int planet_id);
  void PrintPlanetState(PlanetState* ps);

 private:
  void InitShipsAvailablePerTurnAndPlanet();
  void InitPlanets();
  std::vector<int> ShipsAvailableInPlanetPerTurn(Planet p);
  std::vector<int> ComputeShipsAvailablePerTurnWithMovingFleet(Planet p);
  std::vector<int> ComputeShipsAvailablePerTurnBasedOnGrowthRate(std::vector<int> ships_per_turn,
									 Planet p);
  void InitMyUnsafePlanets();
  void InitNeutralPlanets();
  std::vector<TurnState> ComputeTurnStateNeutralPlanet(Planet p);
  TurnState ComputeNextTurnState(Planet p, TurnState prev, vector<Fleet> arriving_fleets);
  TurnState MaybeChangeNeutralPlanetOwner(TurnState ts, int owner);
  pair<int,int> ComputeAmountArrivingFleetsPerPlayer(vector<Fleet> arriving_fleets);
  TurnState ComputeTurnStateWhenNeutralOwner(TurnState ts, pair<int,int> fleets_ships);
  TurnState MaybeChangeNeutralPlanetOwner(TurnState ts, Fleet f);
  void CheckAndMaybeSetAsMyUnsafePlanet(Planet p);
  void InitAvailableShipsInPlanets();
  int AvailableShipsInPlanet(PlanetState* ps);

  std::map<int, PlanetState*> planets_state;
  PlanetWars* pw;
  std::map<int, std::vector<TurnState> > neutral_planets_state;
};

#endif
