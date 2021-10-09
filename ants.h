#ifndef ANTS_H_INCLUDED
#define ANTS_H_INCLUDED

#include <memory>
#include <unordered_set>
#include <vector>
#include <functional>

#include "framework.h"

struct LinkedNode //basically a linked pointer node that keeps track of the next node in our path.
{
    const NodePtr current;
    LinkedNode* next = nullptr;
};

template<>
struct std::hash<LinkedNode>
{
    size_t operator()(const LinkedNode& node) const
    {
        return std::hash<NodePtr>()(node.current);
    }
};

template <>
struct std::equal_to<LinkedNode>
{
    bool operator()(const LinkedNode& node1, const LinkedNode& node2) const
    {
        return std::equal_to<NodePtr>()(node1.current,node2.current);
    }
};

/*bool operator==(const LinkedNode& a,const LinkedNode& b)
{
    return std::equal_to<LinkedNode>()(a,b);
}*/

struct LinkedSet  //fetch visited points in usually O(1) while also keeping track of our path
{
    LinkedNode* start = nullptr, *end = nullptr;
    std::unordered_set<LinkedNode> path;
    void push_back(const LinkedNode& node)
    {
        path.insert(LinkedNode(node));
        if (path.size()== 1)
        {
            end = const_cast<LinkedNode*>(&*(path.find(node)));
            start = end;
        }
        else
        {
            end->next = const_cast<LinkedNode*>(&*(path.find(node)));//our set copies the parameter so we have to get a pointer to
            end = end->next ;
        }


    }
    void clear()
    {
        path.clear();
        start = nullptr;
        end = nullptr;
    }
    bool contains(const NodePtr& ptr)
    {
        return path.find({ptr}) != path.end();
    }
    void map(bool (*func)(LinkedNode* node)) //given a function that does something with nodes and returns true to continue looping, apply the function to each node
    {
        LinkedNode* cur = start;
        while (cur != nullptr)
        {
            if (func(cur))
            {
                cur = cur->next;
            }
            else
            {
                break;
            }
        }
    }
};

struct Ant
{
    NodePtr curNode, startingNode;
   // NodePair lastArc; //last arc the ant walked on
   LinkedSet path;
    float pathLength = 0;
    Ant(const NodePtr& node);
    void setNode(const NodePtr& node); //set curNode to node and add it to our list of visited nodes
    void render();
    void renderPath();
    void step();
    void solve(); //run step until we have visited all nodes
    void spray(); //apply pheromone. path.size is 0 at the end
    void reset(); //basically resets all starting variables except for startingNode
    bool done();
};

#endif // ANTS_H_INCLUDED
