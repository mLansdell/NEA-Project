#include <vector>
#include <cmath>
#include <algorithm>

class Node {
    int number;
    float inputSum = 0;
    float outputValue = 0;
    std::vector<connectionGene> outputConnections;
    int layer = 0;

public:
    Node(int no) : number(no) {}

    void engage() {
        if (layer != 0) {
            outputValue = sigmoid(inputSum);
        }
        for (auto& connection : outputConnections) {
            if (connection.enabled) {
                connection.toNode->inputSum += connection.weight * outputValue;
            }
        }
    }

    float stepFunction(float x) {
        return x < 0 ? 0 : 1;
    }

    float sigmoid(float x) {
        return 1 / (1 + std::exp(-4.9 * x));
    }

    bool isConnectedTo(Node* node) {
        if (node->layer == layer) {
            return false;
        }

        if (node->layer < layer) {
            for (auto& connection : node->outputConnections) {
                if (connection.toNode == this) {
                    return true;
                }
            }
        }
        else {
            for (auto& connection : outputConnections) {
                if (connection.toNode == node) {
                    return true;
                }
            }
        }
        return false;
    }

    Node* clone() {
        Node* clone = new Node(number);
        clone->layer = layer;
        return clone;
    }
};

class connectionGene {
public:
    Node* toNode;
    float weight;
    bool enabled = true;

    connectionGene(Node* toNode, float weight, bool enabled = true) : toNode(toNode), weight(weight), enabled(enabled) {}
};


