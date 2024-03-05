#include <random>
#include <cmath>
#include "Node.cpp"

class connectionGene {
public:
    Node* fromNode;
    Node* toNode;
    float weight;
    bool enabled = true;
    int innovationNo;//each connection is given a innovation number to compare genomes

    //--------------------------------------------------------------------------------------------------------------------------------------------------
    
    //constructor
    connectionGene(Node* from, Node* to, float w, int inno) : fromNode(from), toNode(to), weight(w), innovationNo(inno) {}

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //changes the weight
    void mutateWeight() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);
        std::normal_distribution<> d(0, 1);

        float rand2 = dis(gen);
        if (rand2 < 0.1) {//10% of the time completely change the weight
            weight = dis(gen) * 2 - 1;
        }
        else {//otherwise slightly change it
            weight += d(gen) / 50;
            //keep weight between bounds
            if (weight > 1) {
                weight = 1;
            }
            if (weight < -1) {
                weight = -1;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //returns a copy of this connectionGene
    connectionGene* clone(Node from, Node to) {
        connectionGene* clone = new connectionGene(from, to, weight, innovationNo);
        clone->enabled = this->enabled;
        return clone;
    }
};


