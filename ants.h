#ifndef ANTS_H_INCLUDED
#define ANTS_H_INCLUDED

#include <memory>
#include <unordered_set>
#include <stack>
#include <functional>

#include "framework.h"


struct Ant
{
    NodePtr curNode;
    NodePair lastArc; //last arc the ant walked on
    float pathLength = 0;
    std::stack<NodePtr> path;
    std::unordered_set<NodePtr> visited;
    Ant(NodePtr& node);
    void render();
    void step();
    void solve(); //run step until we have visited all nodes
    void spray(); //apply pheromone. path.size is 0 at the end
};

#endif // ANTS_H_INCLUDED
