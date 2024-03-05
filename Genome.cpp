#include <vector>
#include "connectionGene.cpp"
#include "Node.cpp"

class Genome {
public:
    std::vector<connectionGene> genes;//a list of connections between nodes which represent the NN
    std::vector<Node*> nodes;//list of nodes
    int inputs;
    int outputs;
    int layers = 2;
    int nextNode = 0;
    int biasNode;
    std::vector<Node*> network;//a list of the nodes in the order that they need to be considered in the NN

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    Genome(int in, int out) {
        //set input number and output number
        inputs = in;
        outputs = out;

        //create input nodes
        for (int i = 0; i < inputs; i++) {
            nodes.push_back(new Node(i));
            nextNode++;
            nodes[i]->layer = 0;
        }

        //create output nodes
        for (int i = 0; i < outputs; i++) {
            nodes.push_back(new Node(i + inputs));
            nodes[i + inputs]->layer = 1;
            nextNode++;
        }
        nodes.push_back(new Node(nextNode));
        biasNode = nextNode;//bias node
        nextNode++;
        nodes[biasNode]->layer = 0;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------
    
    //returns the node with a matching number
    //sometimes the nodes will not be in order
    Node* getNode(int nodeNumber) {
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i]->number == nodeNumber) {
                return nodes[i];
            }
        }
        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //adds the conenctions going out of a node to that node so that it can acess the next node during feeding forward
    void connectNodes() {
        for (int i = 0; i < nodes.size(); i++) {//clear the connections
            nodes[i]->outputConnections.clear();
        }
        for (int i = 0; i < genes.size(); i++) {//for each connectionGene 
            genes[i]->fromNode->outputConnections.push_back(genes[i]);//add it to node
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //feeding in input values into the NN and returning output array
    std::vector<float> feedForward(std::vector<float> inputValues) {
        //set the outputs of the input nodes
        for (int i = 0; i < inputs; i++) {
            nodes[i]->outputValue = inputValues[i];
        } 
        nodes[biasNode]->outputValue = 1;//output of bias is 1
        for (int i = 0; i < network.size(); i++) {//for each node in the network engage it(see node class for what this does)
            network[i]->engage();
        }


        //the outputs are nodes[inputs] to nodes [inputs+outputs-1]
        std::vector<float> outs(outputs);
        for (int i = 0; i < outputs; i++) {
            outs[i] = nodes[inputs + i]->outputValue;
        }
        
        for (int i = 0; i < nodes.size(); i++) {//reset all the nodes for the next feed forward
            nodes[i]->inputSum = 0;
        }
        
        return outs;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //sets up the NN as a list of nodes in order to be engaged 
    void generateNetwork() {
        connectNodes();
        network.clear();
        //for each layer add the node in that layer, since layers cannot connect to themselves there is no need to order the nodes within a layer

        for (int l = 0; l < layers; l++) {//for each layer
            for (int i = 0; i < nodes.size(); i++) {//for each node
                if (nodes[i]->layer == l) {//if that node is in that layer
                    network.push_back(nodes[i]);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //mutate the NN by adding a new node
    //it does this by picking a random connection and disabling it then 2 new connections are added 
    //1 between the input node of the disabled connection and the new node
    //and the other between the new node and the output of the disabled connection

    void addNode(std::vector<connectionHistory*> innovationHistory) {
        //pick a random connection to create a node between
        if (genes.size() == 0) {
            addConnection(innovationHistory);
            return;
        }
        
        int randomConnection = floor(random(genes.size()));
        while (genes[randomConnection]->fromNode == nodes[biasNode] && genes.size() != 1) {//dont disconnect bias
            randomConnection = floor(random(genes.size()));
        }
        
        genes[randomConnection]->enabled = false;//disable it
        int newNodeNo = nextNode;
        nodes.push_back(new Node(newNodeNo));
        nextNode++;

        //add a new connection to the new node with a weight of 1
        int connectionInnovationNumber = getInnovationNumber(innovationHistory, genes[randomConnection]->fromNode, getNode(newNodeNo));
        genes.push_back(new connectionGene(genes[randomConnection]->fromNode, getNode(newNodeNo), 1, connectionInnovationNumber));
        connectionInnovationNumber = getInnovationNumber(innovationHistory, getNode(newNodeNo), genes[randomConnection]->toNode);

        //add a new connection from the new node with a weight the same as the disabled connection
        genes.push_back(new connectionGene(getNode(newNodeNo), genes[randomConnection]->toNode, genes[randomConnection]->weight, connectionInnovationNumber));
        getNode(newNodeNo)->layer = genes[randomConnection]->fromNode->layer + 1;
        connectionInnovationNumber = getInnovationNumber(innovationHistory, nodes[biasNode], getNode(newNodeNo));

        //connect the bias to the new node with a weight of 0 
        genes.push_back(new connectionGene(nodes[biasNode], getNode(newNodeNo), 0, connectionInnovationNumber));

        //if the layer of the new node is equal to the layer of the output node of the old connection then a new layer needs to be created
        //more accurately the layer numbers of all layers equal to or greater than this new node need to be incrimented
        if (getNode(newNodeNo)->layer == genes[randomConnection]->toNode->layer) {
            for (int i = 0; i < nodes.size() - 1; i++) {//don't include this newest node
                if (nodes[i]->layer >= getNode(newNodeNo)->layer) {
                    nodes[i]->layer++;
                }
            }
            layers++;
        }
        connectNodes();
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //adds a connection between 2 nodes which aren't currently connected
    void addConnection(std::vector<connectionHistory*> innovationHistory) {
        if (fullyConnected()) {//cannot add a connection to a fully connected network
            println("connection failed");
            return;
        }

        //get random nodes
        int randomNode1 = floor(random(nodes.size()));
        int randomNode2 = floor(random(nodes.size()));
        while (randomConnectionNodesAreShit(randomNode1, randomNode2)) {//while the random nodes are no good
            //get new ones
            randomNode1 = floor(random(nodes.size()));
            randomNode2 = floor(random(nodes.size()));
        }
        int temp;
        if (nodes[randomNode1]->layer > nodes[randomNode2]->layer) {//if the first random node is after the second then switch
            temp = randomNode2;
            randomNode2 = randomNode1;
            randomNode1 = temp;
        }

        //get the innovation number of the connection
        //this will be a new number if no identical genome has mutated in the same way 
        int connectionInnovationNumber = getInnovationNumber(innovationHistory, nodes[randomNode1], nodes[randomNode2]);
        //add the connection with a random array
        genes.push_back(new connectionGene(nodes[randomNode1], nodes[randomNode2], random(-1, 1), connectionInnovationNumber));//changed this so if error here
        connectNodes();
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    bool randomConnectionNodesAreShit(int r1, int r2) {
        if (nodes[r1]->layer == nodes[r2]->layer) return true;// if the nodes are in the same layer 
        if (nodes[r1]->isConnectedTo(nodes[r2])) return true;//if the nodes are already connected
        return false;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //returns the innovation number for the new mutation
    //if this mutation has never been seen before then it will be given a new unique innovation number
    //if this mutation matches a previous mutation then it will be given the same innovation number as the previous one
    int getInnovationNumber(std::vector<connectionHistory*> innovationHistory, Node* from, Node* to) {
        bool isNew = true;
        int connectionInnovationNumber = nextConnectionNo;
        for (int i = 0; i < innovationHistory.size(); i++) {//for each previous mutation
            if (innovationHistory[i]->matches(this, from, to)) {//if match found
                isNew = false;
                connectionInnovationNumber = innovationHistory[i]->innovationNumber;//set the innovation number as the innovation number of the match
                break;
            }
        }
        if (isNew) {//if the mutation is new then create an arrayList of integers representing the current state of the genome
            std::vector<int> innoNumbers;
            for (int i = 0; i < genes.size(); i++) {//set the innovation numbers
                innoNumbers.push_back(genes[i]->innovationNo);
            }

            //then add this mutation to the innovationHistory 
            innovationHistory.push_back(new connectionHistory(from->number, to->number, connectionInnovationNumber, innoNumbers));
            nextConnectionNo++;
        }
        return connectionInnovationNumber;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    bool fullyConnected() {
        int maxConnections = 0;
        std::vector<int> nodesInLayers(layers);

        for (int i = 0; i < nodes.size(); i++) {
            nodesInLayers[nodes[i]->layer] += 1;
        }

        for (int i = 0; i < layers - 1; i++) {
            int nodesInFront = 0;
            for (int j = i + 1; j < layers; j++) {
                nodesInFront += nodesInLayers[j];
            }
            maxConnections += nodesInLayers[i] * nodesInFront;
        }
        if (maxConnections == genes.size()) {
            return true;
        }
        return false;
    }

    void mutate(std::vector<connectionHistory*> innovationHistory) {
        if (genes.size() == 0) {
            addConnection(innovationHistory);
        }
        float rand1 = random(1);
        if (rand1 < 0.8) {
            for (int i = 0; i < genes.size(); i++) {
                genes[i]->mutateWeight();
            }
        }

        float rand2 = random(1);
        if (rand2 < 0.08) {
            addConnection(innovationHistory);
        }

        float rand3 = random(1);
        if (rand3 < 0.02) {
            addNode(innovationHistory);
        }
    }

    Genome* crossover(Genome* parent2) {
        Genome* child = new Genome(inputs, outputs, true);
        child->genes.clear();
        child->nodes.clear();
        child->layers = layers;
        child->nextNode = nextNode;
        child->biasNode = biasNode;
        std::vector<connectionGene*> childGenes;
        std::vector<bool> isEnabled;

        for (int i = 0; i < genes.size(); i++) {
            bool setEnabled = true;
            int parent2gene = matchingGene(parent2, genes[i]->innovationNo);
            if (parent2gene != -1) {
                if (!genes[i]->enabled || !parent2->genes[parent2gene]->enabled) {
                    if (random(1) < 0.75) {
                        setEnabled = false;
                    }
                }
                float rand = random(1);
                if (rand < 0.5) {
                    childGenes.push_back(genes[i]);
                }
                else {
                    childGenes.push_back(parent2->genes[parent2gene]);
                }
            }
            else {
                childGenes.push_back(genes[i]);
                setEnabled = genes[i]->enabled;
            }
            isEnabled.push_back(setEnabled);
        }

        for (int i = 0; i < nodes.size(); i++) {
            child->nodes.push_back(nodes[i]->clone());
        }

        for (int i = 0; i < childGenes.size(); i++) {
            child->genes.push_back(childGenes[i]->clone(child->getNode(childGenes[i]->fromNode->number), child->getNode(childGenes[i]->toNode->number)));
            child->genes[i]->enabled = isEnabled[i];
        }
        child->connectNodes();
        return child;
    }

    Genome(int in, int out, bool crossover) {
        inputs = in;
        outputs = out;
    }

    int matchingGene(Genome* parent2, int innovationNumber) {
        for (int i = 0; i < parent2->genes.size(); i++) {
            if (parent2->genes[i]->innovationNo == innovationNumber) {
                return i;
            }
        }
        return -1;
    }

    void printGenome() {
        println("Print genome  layers:", layers);
        println("bias node: " + biasNode);
        println("nodes");
        for (int i = 0; i < nodes.size(); i++) {
            print(nodes[i]->number + ",");
        }
        println("Genes");
        for (int i = 0; i < genes.size(); i++) {
            println("gene " + genes[i]->innovationNo, "From node " + genes[i]->fromNode->number, "To node " + genes[i]->toNode->number,
                "is enabled " + genes[i]->enabled, "from layer " + genes[i]->fromNode->layer, "to layer " + genes[i]->toNode->layer, "weight: " + genes[i]->weight);
        }
        println();
    }

    Genome* clone() {
        Genome* clone = new Genome(inputs, outputs, true);
        for (int i = 0; i < nodes.size(); i++) {
            clone->nodes.push_back(nodes[i]->clone());
        }

        for (int i = 0; i < genes.size(); i++) {
            clone->genes.push_back(genes[i]->clone(clone->getNode(genes[i]->fromNode->number), clone->getNode(genes[i]->toNode->number)));
        }
        clone->layers = layers;
        clone->nextNode = nextNode;
        clone->biasNode = biasNode;
        clone->connectNodes();
        return clone;
    }

    void drawGenome(int startX, int startY, int w, int h) {
        std::vector<std::vector<Node*>> allNodes;
        std::vector<PVector> nodePoses;
        std::vector<int> nodeNumbers;

        for (int i = 0; i < layers; i++) {
            std::vector<Node*> temp;
            for (int j = 0; j < nodes.size(); j++) {
                if (nodes[j]->layer == i) {
                    temp.push_back(nodes[j]);
                }
            }
            allNodes.push_back(temp);
        }

        for (int i = 0; i < layers; i++) {
            fill(255, 0, 0);
            float x = startX + ((i + 1) * w) / (layers + 1.0);
            for (int j = 0; j < allNodes[i].size(); j++) {
                float y = startY + (((float)(j + 1.0) * h) / (allNodes[i].size() + 1.0));
                nodePoses.push_back(PVector(x, y));
                nodeNumbers.push_back(allNodes[i][j]->number);
            }
        }

        stroke(0);
        strokeWeight(2);
        for (int i = 0; i < genes.size(); i++) {
            if (genes[i]->enabled) {
                stroke(0);
            }
            else {
                stroke(100);
            }
            PVector from;
            PVector to;
            from = nodePoses[nodeNumbers.indexOf(genes[i]->fromNode->number)];
            to = nodePoses[nodeNumbers.indexOf(genes[i]->toNode->number)];
            if (genes[i]->weight > 0) {
                stroke(255, 0, 0);
            }
            else {
                stroke(0, 0, 255);
            }
            strokeWeight(map(abs(genes[i]->weight), 0, 1, 0, 5));
            line(from.x, from.y, to.x, to.y);
        }

        for (int i = 0; i < nodePoses.size(); i++) {
            fill(255);
            stroke(0);
            strokeWeight(1);
            ellipse(nodePoses[i].x, nodePoses[i].y, 20, 20);
            textSize(10);
            fill(0);
            textAlign(CENTER, CENTER);
            text(nodeNumbers[i], nodePoses[i].x, nodePoses[i].y);
        }
    }
};


