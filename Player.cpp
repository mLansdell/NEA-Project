#include <vector>
#include <algorithm> // For std::max_element
#include "Genome.cpp"

class Player {
    float fitness;
    Genome brain;
    std::vector<float> vision = std::vector<float>(8);
    std::vector<float> decision = std::vector<float>(4);
    float unadjustedFitness;
    int lifespan = 0;
    int bestScore = 0;
    bool dead;
    int score;
    int gen = 0;
    int genomeInputs = 13;
    int genomeOutputs = 4;

public:
    Player() : brain(genomeInputs, genomeOutputs) {}

    void show() {}

    void move() {}

    void update() {}

    void look() {}

    void think() {
        float max = 0;
        int maxIndex = 0;

        decision = brain.feedForward(vision);
        for (size_t i = 0; i < decision.size(); i++) {
            if (decision[i] > max) {
                max = decision[i];
                maxIndex = i;
            }
        }
    }

    Player clone() {
        Player clone;
        clone.brain = brain.clone();
        clone.fitness = fitness;
        clone.brain.generateNetwork();
        clone.gen = gen;
        clone.bestScore = score;
        return clone;
    }

    Player cloneForReplay() {
        Player clone;
        clone.brain = brain.clone();
        clone.fitness = fitness;
        clone.brain.generateNetwork();
        clone.gen = gen;
        clone.bestScore = score;

        return clone;
    }

    void calculateFitness() {}

    Player crossover(Player parent2) {
        Player child;
        child.brain = brain.crossover(parent2.brain);
        child.brain.generateNetwork();
        return child;
    }
};


