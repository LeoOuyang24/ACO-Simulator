#include "ants.h"
#include "simulation.h"

Ant::Ant(const NodePtr& ptr) : startingNode(ptr) //we don't want to visit our starting node as visited because we have to go back to it at some point
{
    setNode(ptr);
}

void Ant::setNode(const NodePtr& ptr)
{

    auto arc = Sim::connections.find({ptr,curNode});
    if ( arc != Sim::connections.end())
        {
            pathLength += arc->second.length;
        }
    path.push_back({ptr,nullptr});
    curNode = ptr;
    //path.push_back(ptr);
}

void Ant::render()
{
    if (Node* node = curNode.lock().get())
    {
        PolyRender::requestNGon(20,node->center,10,{0,0,0,1},0,true,2,true);
    }
}

void Ant::renderPath()
{
    if (Node* node = curNode.lock().get())
    {
        LinkedNode* cur = path.start;
    //        std::cout << path.path.size() << "\n";
        while (cur != path.end && cur != nullptr)
        {
            PolyRender::requestLine(glm::vec4(cur->current.lock().get()->center,cur->next->current.lock().get()->center),{0,0,1,.1},.75,5);
            cur = cur->next;
        }
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
                if (!path.contains(node->neighbors[i]) )
                {
                    //if we have yet to visit this node
                    //add it to the potential list and take not of the chance to visit it
                    NodePair pear = std::make_pair(curNode,node->neighbors[i]);
                    totalProb += Sim::connections[pear].chance;
                    valid.push_back({node->neighbors[i],Sim::connections[pear].chance});
                   // std::cout << Sim::connections << "\n";
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
                //lastArc = {curNode,valid[i].first};
               // pathLength += Sim::connections[{curNode,valid[i].first}].length;
                setNode(valid[i].first);
               // path.push(curNode);
                break;
            }
            sum += valid[i].second/totalProb;
        }
        if (size == 0)
        {
            if (path.path.size() != Sim::nodes.size())
            {
                std::cout <<"Ant is stuck!\n";
            }
        }

    }
}

void Ant::solve()
{
    while (path.path.size() != Sim::nodes.size())
    {
        step();
    }
}

void Ant::spray()
{
    if (path.path.size() > 1)
    {

        //NodePtr cur= path[path.size() - 1];
        LinkedNode* cur = path.start;
        while (cur->next != nullptr)
        {
            Sim::connections[{cur->current,cur->next->current}].pheromone += 1.0f/(pathLength);
            cur= cur->next;
        }
    }
}

void Ant::reset()
{
    //lastArc.first.reset();
    //lastArc.second.reset();
    pathLength = 0;
    path.clear();
    setNode(startingNode);
}

bool Ant::done()
{
    return path.path.size() >= Sim::nodes.size();
}
