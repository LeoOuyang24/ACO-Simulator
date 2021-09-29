#include "ants.h"
#include "simulation.h"

Ant::Ant(NodePtr& ptr) : curNode(ptr)
{

}

void Ant::render()
{
    if (Node* node = curNode.lock().get())
    {
        PolyRender::requestNGon(20,node->center,10,{0,0,0,1},0,true,1,true);
    }
}
void Ant::step()
{
    if (Node* node = curNode.lock().get())
    {
        int size = node->neighbors.size();
        std::vector<std::pair<NodePtr,float>> valid; //valid neighbors to move to
        float totalProb = 0;
        for (int i = 0; i < size; ++i)
        {
            if (node->neighbors[i].lock().get())
            {
                if (visited.count(node->neighbors[i]) == 0 && node->neighbors[i].lock().get() != node )
                {
                    NodePair pear = std::make_pair(curNode,node->neighbors[i]);
                    totalProb += Sim::connections[pear].chance;
                    valid.push_back({node->neighbors[i],Sim::connections[pear].chance});
                }
            }
        }
        float gen = rand()%100/100.0f;
        float sum = 0;
        size = valid.size();
        for (int i = 0; i< size; ++i) //randomly choose a node
        {
            if (valid[i].second/totalProb + sum > gen) //calculate the chance for each node and see if our randomly generated number is within range
            {
                lastArc = {curNode,valid[i].first};
                pathLength += Sim::connections[{curNode,valid[i].first}].length;
                curNode = valid[i].first;
                path.push(curNode);
                break;
            }
            sum += valid[i].second;
        }

    }
}

void Ant::solve()
{
    while (path.size() != Sim::nodes.size())
    {
        step();
    }
}

void Ant::spray()
{
    if (path.size() > 1)
    {
        NodePtr cur= path.top();
        path.pop();
        while (path.size() > 0)
        {
            Sim::connections[{cur,path.top()}].pheromone += 1/(pathLength);
            path.pop();
        }
    }
}
