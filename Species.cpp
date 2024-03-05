#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "Player.cpp"
#include "Genome.cpp"

class Species {
public:
    std::vector<Player> players;
    float bestFitness = 0;
    Player champ;
    float averageFitness = 0;
    int staleness = 0;
    Genome rep;

    float excessCoeff = 1;
    float weightDiffCoeff = 0.5;
    float compatibilityThreshold = 3;

    Species() {}

    Species(Player p) {
        players.push_back(p);

        bestFitness = p.fitness;
        rep = p.brain.clone();
        champ = p.cloneForReplay();
    }

    bool sameSpecies(Genome g) {
        float compatibility;
        float excessAndDisjoint = getExcessDisjoint(g, rep);
        float averageWeightDiff = averageWeightDiff(g, rep);
        float largeGenomeNormaliser = g.genes.size() - 20;
        if (largeGenomeNormaliser < 1) {
            largeGenomeNormaliser = 1;
        }
        compatibility = (excessCoeff * excessAndDisjoint / largeGenomeNormaliser) + (weightDiffCoeff * averageWeightDiff);
        return (compatibilityThreshold > compatibility);
    }

    void addToSpecies(Player p) {
        players.push_back(p);
    }

    float getExcessDisjoint(Genome brain1, Genome brain2) {
        float matching = 0.0;
        for (size_t i = 0; i < brain1.genes.size(); i++) {
            for (size_t j = 0; j < brain2.genes.size(); j++) {
                if (brain1.genes[i].innovationNo == brain2.genes[j].innovationNo) {
                    matching++;
                    break;
                }
            }
        }
        return (brain1.genes.size() + brain2.genes.size() - 2 * (matching));
    }

    float averageWeightDiff(Genome brain1, Genome brain2) {
        if (brain1.genes.empty() || brain2.genes.empty()) {
            return 0;
        }
        float matching = 0;
        float totalDiff = 0;
        for (size_t i = 0; i < brain1.genes.size(); i++) {
            for (size_t j = 0; j < brain2.genes.size(); j++) {
                if (brain1.genes[i].innovationNo == brain2.genes[j].innovationNo) {
                    matching++;
                    totalDiff += std::abs(brain1.genes[i].weight - brain2.genes[j].weight);
                    break;
                }
            }
        }
        if (matching == 0) {
            return 100;
        }
        return totalDiff / matching;
    }

    void sortSpecies() {
        std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
            return a.fitness > b.fitness;
            });

        if (players.empty()) {
            std::cout << "fucking";
            staleness = 200;
            return;
        }

        if (players[0].fitness > bestFitness) {
            staleness = 0;
            bestFitness = players[0].fitness;
            rep = players[0].brain.clone();
            champ = players[0].cloneForReplay();
        }
        else {
            staleness++;
        }
    }

    void setAverage() {
        float sum = 0;
        for (auto& player : players) {
            sum += player.fitness;
        }
        averageFitness = sum / players.size();
    }

    Player giveMeBaby(std::vector<connectionHistory> innovationHistory) {
        Player baby;
        if (random(1) < 0.25) {
            baby = selectPlayer().clone();
        }
        else {
            Player parent1 = selectPlayer();
            Player parent2 = selectPlayer();

            if (parent1.fitness < parent2.fitness) {
                baby = parent2.crossover(parent1);
            }
            else {
                baby = parent1.crossover(parent2);
            }
        }
        baby.brain.mutate(innovationHistory);
        return baby;
    }

    Player selectPlayer() {
        float fitnessSum = 0;
        for (auto& player : players) {
            fitnessSum += player.fitness;
        }
        float rand = random(fitnessSum);
        float runningSum = 0;
        for (auto& player : players) {
            runningSum += player.fitness;
            if (runningSum > rand) {
                return player;
            }
        }

        return players[0];
    }

    void cull() {
        if (players.size() > 2) {
            players.erase(players.begin() + players.size() / 2, players.end());
        }
    }

    void fitnessSharing() {
        for (auto& player : players) {
            player.fitness /= players.size();
        }
    }
};


