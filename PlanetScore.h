#ifndef PLANET_SCORE_H_
#define PLANET_SCORE_H_

#include "PlanetWars.h"

class PlanetScore {
 public:
  PlanetScore(Planet p, double s);
  bool operator> (const PlanetScore& ps) const;
  bool operator< (const PlanetScore& ps) const;
  bool operator== (const PlanetScore& ps) const;

  void SetPlanet(Planet* p);
  void SetScore(double s);
  Planet* GetPlanet();
  double GetScore();
 private:
  Planet* planet;
  double score;
};

#endif
