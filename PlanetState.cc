#include <iostream>
#include <fstream>
#include "PlanetWars.h"
#include "PlanetState.h"

PlanetState::PlanetState() {
  unsafe_planet = false;
  unsafe_in_turn = -1;
}


void PlanetState::SetPlanet(Planet* p) {
  planet = p;
}

void PlanetState::SetAvailableShipsPerTurn(std::vector<int> v) {
  available_ships_per_turn = new std::vector<int>(v);
}

void PlanetState::SetUnsafePlanet(bool b) {
  unsafe_planet = b;;
}

void PlanetState::SetUnsafeInTurn(int i ) {
  unsafe_in_turn = i;
}

void PlanetState::SetAvailableShips(int i) {
  available_ships = i;
}

Planet* PlanetState::GetPlanet() {
  return planet;
}

std::vector<int>* PlanetState::GetAvailableShipsPerTurn() {
  return available_ships_per_turn;
}

bool PlanetState::IsUnsafePlanet() {
  return unsafe_planet;
}

int PlanetState::GetUnsafeInTurn() {
  return unsafe_in_turn;
}

int PlanetState::GetAvailableShips() {
  return available_ships;
}

