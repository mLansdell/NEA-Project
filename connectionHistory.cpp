#include <vector>
#include <algorithm>
#include "Node.cpp"
#include "Genome.cpp"

class connectionHistory {
    int fromNode;
    int toNode;
    int innovationNumber;
    std::vector<int> innovationNumbers;//the innovation Numbers from the connections of the genome which first had this mutation
                                       //this represents the genome and allows us to test if another genoeme is the same
                                       //this is before this connection was added

    //--------------------------------------------------------------------------------------------------------------------------------------------------

    //constructer
public:
    connectionHistory(int from, int to, int inno, const std::vector<int>& innovationNos)
        : fromNode(from), toNode(to), innovationNumber(inno), innovationNumbers(innovationNos) {}

    //returns whether the genome matches the original genome and the connection is between the same nodes
    bool matches(const Genome& genome, const Node& from, const Node& to) {
        if (genome.genes.size() == innovationNumbers.size()) {//if the number of connections are different then the genoemes aren't the same
            if (from.number == fromNode && to.number == toNode) {
                //next check if all the innovation numbers match from the genome
                for (const auto& gene : genome.genes) {
                    if (std::find(innovationNumbers.begin(), innovationNumbers.end(), gene.innovationNo) == innovationNumbers.end()) {
                        return false;
                    }
                }
                //if reached this far then the innovationNumbers match the genes innovation numbers and the connection is between the same nodes
                //so it does match
                return true;
            }
        }
        return false;
    }
};


