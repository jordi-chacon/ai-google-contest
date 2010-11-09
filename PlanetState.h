#ifndef PLANET_STATE_H_
#define PLANET_STATE_H_

#include "PlanetWars.h"

class PlanetState {
 public:
  PlanetState();

  void SetPlanet(Planet* p);
  void SetAvailableShipsPerTurn(std::vector<int> v);
  void SetUnsafePlanet(bool b);
  void SetUnsafeInTurn(int i);
  void SetAvailableShips(int i);
  Planet* GetPlanet();
  std::vector<int>* GetAvailableShipsPerTurn();
  bool IsUnsafePlanet();
  int GetUnsafeInTurn();
  int GetAvailableShips();
  int GetNeededShipsToSafe();

 private:
  Planet* planet;
  std::vector<int>* available_ships_per_turn;
  bool unsafe_planet;
  int unsafe_in_turn;
  int available_ships;
};

#endif
