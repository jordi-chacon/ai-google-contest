#include <iostream>
#include <fstream>
#include <algorithm>
#include "PlanetWars.h"
#include "PlanetScore.h"
using namespace std;

void SendShips(const PlanetWars& pw, int my_planet, int my_planet_ships) {
  //  ofstream myfile;
  // myfile.open("output", ios::app);

  // (1) Find most interesting planets to send fleets
  std::vector<Planet> neutral_planets = pw.NotMyPlanets();
  std::vector<PlanetScore> neutral_planets_score;
  for(std::vector<Planet>::const_iterator it = neutral_planets.begin();
      it < neutral_planets.end(); ++it) {
    int num_ships = it->NumShips();
    int growth_rate = it->GrowthRate();
    int distance = pw.Distance(it->PlanetID(), my_planet);
    double score = 40*growth_rate + 5000/num_ships/2 + 1000/distance;
    //    myfile << score << " = " << growth_rate << "(" << 40*growth_rate<<") + "<<
    //  num_ships<<"("<<5000/num_ships/2<<") + "<<distance<<"("<<1000/distance<<")\n";
    PlanetScore* ps = new PlanetScore(*it, score);
    neutral_planets_score.push_back(*ps);
  }
  sort(neutral_planets_score.begin(), neutral_planets_score.end());

  // (2) Send ships from my planet to the weakest planets that I don't own
  for(std::vector<PlanetScore>::iterator it = neutral_planets_score.begin();
      it < neutral_planets_score.end(); ++it) {
    Planet p = *(it->GetPlanet());
    if(my_planet_ships > p.NumShips() + 5) {
      pw.IssueOrder(my_planet, p.PlanetID(), p.NumShips()+1);
      my_planet_ships -= p.NumShips();
    }
    else return;
  }
}

void DoTurn(const PlanetWars& pw) {
  // Per each of my planets try to send ships to other planets
  int strongest_planet = -1;
  double source_score = -999999.0;
  int strongest_planet_ships = 0;
  std::vector<Planet> my_planets = pw.MyPlanets();
  for (int i = 0; i < my_planets.size(); ++i) {
    SendShips(pw, my_planets[i].PlanetID(), my_planets[i].NumShips());
  }
}


// This is just the main game loop that takes care of communicating with the
// game engine for you. You don't have to understand or change the code below.
int main(int argc, char *argv[]) {
  std::string current_line;
  std::string map_data;
  while (true) {
    int c = std::cin.get();
    current_line += (char)c;
    if (c == '\n') {
      if (current_line.length() >= 2 && current_line.substr(0, 2) == "go") {
        PlanetWars pw(map_data);
        map_data = "";
        DoTurn(pw);
	pw.FinishTurn();
      } else {
        map_data += current_line;
      }
      current_line = "";
    }
  }
  return 0;
}
