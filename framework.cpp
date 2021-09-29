#include "framework.h"
#include "simulation.h"

size_t std::hash<NodePtr>::operator () (const NodePtr& ptr) const
{
    return std::hash<Node*>{}(ptr.lock().get());
}

bool std::equal_to<NodePtr>::operator ()(const NodePtr& a, const NodePtr& b) const
{
    return a.lock().get() == b.lock().get();
}

bool operator==(const NodePtr& a,const NodePtr& b)
{
    return std::equal_to<NodePtr>()(a,b);
}

size_t std::hash<NodePair>::operator ()(const NodePair& pear) const
{
    size_t hash1 = std::hash<NodePtr>()(pear.first);
    return hash1 ^ std::hash<NodePtr>()(pear.second);
    //not too worried about xor being a poor hash combiner because we want it to be symmetric and
    //we should never have a case where both nodePtrs are the same since nodes can't have an edge to themselves

}

Node* Connection::getNode1()
{
    return node1.lock().get();
}
Node* Connection::getNode2()
{
    return node2.lock().get();
}
void Connection::updateChance()
{
    //http://www.scholarpedia.org/article/Ant_colony_optimization#ConstructAntSolutions
    //the heuristic in our case is 1/length, so it's inversely proportional to how long the distance is
    chance = pow(pheromone,Sim::alpha)*pow(1/length,Sim::beta);
}
