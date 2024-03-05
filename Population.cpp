#include <vector>
#include <iostream>
#include <cmath> // For floor function
#include <algorithm> // For sort function
#include "connectionHistory.cpp"
#include "Player.cpp"
#include "Species.cpp"

class Population {
    std::vector<Player> pop;
    Player bestPlayer;
    int bestScore = 0;
    int gen;
    std::vector<connectionHistory> innovationHistory; // Assuming connectionHistory is defined elsewhere
    std::vector<Player> genPlayers;
    std::vector<Species> species;
    bool massExtinctionEvent = false;
    bool newStage = false;

public:
    Population(int size) {
        for (int i = 0; i < size; i++) {
            pop.push_back(Player());
            pop[i].brain.generateNetwork();
            pop[i].brain.mutate(innovationHistory);
        }
    }

    void updateAlive() {
        for (size_t i = 0; i < pop.size(); i++) {
            if (!pop[i].dead) {
                pop[i].look();
                pop[i].think();
                pop[i].update();
                if (!showNothing && (!showBest || i == 0)) {
                    pop[i].show();
                }
            }
        }
    }

    bool done() {
        for (size_t i = 0; i < pop.size(); i++) {
            if (!pop[i].dead) {
                return false;
            }
        }
        return true;
    }

    void setBestPlayer() {
        Player tempBest = species[0].players[0];
        tempBest.gen = gen;

        if (tempBest.score > bestScore) {
            genPlayers.push_back(tempBest.cloneForReplay());
            std::cout << "old best: " << bestScore << "\n";
            std::cout << "new best: " << tempBest.score << "\n";
            bestScore = tempBest.score;
            bestPlayer = tempBest.cloneForReplay();
        }
    }

    void naturalSelection() {
        speciate();
        calculateFitness();
        sortSpecies();
        if (massExtinctionEvent) {
            massExtinction();
            massExtinctionEvent = false;
        }
        cullSpecies();
        setBestPlayer();
        killStaleSpecies();
        killBadSpecies();
        std::cout << "generation " << gen << " Number of mutations " << innovationHistory.size() << " species: " << species.size() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
        float averageSum = getAvgFitnessSum();
        std::vector<Player> children;
        std::cout << "Species:\n";
        for (size_t j = 0; j < species.size(); j++) {
            std::cout << "best unadjusted fitness: " << species[j].bestFitness << "\n";
            for (size_t i = 0; i < species[j].players.size(); i++) {
                std::cout << "player " << i << " fitness: " << species[j].players[i].fitness << " score " << species[j].players[i].score << ' ';
            }
            std::cout << "\n";
            children.push_back(species[j].champ.cloneForReplay());
            int NoOfChildren = std::floor(species[j].averageFitness / averageSum * pop.size()) - 1;
            for (int i = 0; i < NoOfChildren; i++) {
                children.push_back(species[j].giveMeBaby(innovationHistory));
            }
        }
        while (children.size() < pop.size()) {
            children.push_back(species[0].giveMeBaby(innovationHistory));
        }
        pop.clear();
        pop = children;
        gen += 1;
        for (size_t i = 0; i < pop.size(); i++) {
            pop[i].brain.generateNetwork();
        }
    }

    void speciate() {
        for (Species& s : species) {
            s.players.clear();
        }
        for (size_t i = 0; i < pop.size(); i++) {
            bool speciesFound = false;
            for (Species& s : species) {
                if (s.sameSpecies(pop[i].brain)) {
                    s.addToSpecies(pop[i]);
                    speciesFound = true;
                    break;
                }
            }
            if (!speciesFound) {
                species.push_back(Species(pop[i]));
            }
        }
    }

    void calculateFitness() {
        for (size_t i = 1; i < pop.size(); i++) {
            pop[i].calculateFitness();
        }
    }

    void sortSpecies() {
        for (Species& s : species) {
            s.sortSpecies();
        }

        std::vector<Species> temp;
        while (!species.empty()) {
            auto maxIt = std::max_element(species.begin(), species.end(), [](const Species& a, const Species& b) {
                return a.bestFitness < b.bestFitness;
                });
            temp.push_back(*maxIt);
            species.erase(maxIt);
        }
        species = temp;
    }

    void killStaleSpecies() {
        for (size_t i = 2; i < species.size(); i++) {
            if (species[i].staleness >= 15) {
                species.erase(species.begin() + i);
                i--;
            }
        }
    }

    void killBadSpecies() {
        float averageSum = getAvgFitnessSum();
        for (size_t i = 1; i < species.size(); i++) {
            if (species[i].averageFitness / averageSum * pop.size() < 1) {
                species.erase(species.begin() + i);
                i--;
            }
        }
    }

    float getAvgFitnessSum() {
        float averageSum = 0;
        for (const Species& s : species) {
            averageSum += s.averageFitness;
        }
        return averageSum;
    }

    void cullSpecies() {
        for (Species& s : species) {
            s.cull();
            s.fitnessSharing();
            s.setAverage();
        }
    }

    void massExtinction() {
        for (size_t i = 5; i < species.size(); i++) {
            species.erase(species.begin() + i);
            i--;
        }
    }
};


