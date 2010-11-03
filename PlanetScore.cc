#include "PlanetWars.h"
#include "PlanetScore.h"

PlanetScore::PlanetScore(Planet p, double s) {
  planet = new Planet(p);
  score = s;
}

bool PlanetScore::operator< (const PlanetScore& ps) const {
  return score > ps.score;
}

bool PlanetScore::operator> (const PlanetScore& ps) const {
  return score < ps.score;
}

bool PlanetScore::operator== (const PlanetScore& ps) const {
  return score == ps.score;
}

void PlanetScore::SetPlanet(Planet* p) {
  planet = p;
}

void PlanetScore::SetScore(double s) {
  score = s;
}

Planet* PlanetScore::GetPlanet() {
  return planet;
}

double PlanetScore::GetScore() {
  return score;
}

