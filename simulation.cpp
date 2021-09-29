#include "FreeTypeHelper.h"
#include "SDLhelper.h"

#include "simulation.h"

bool Sim::paused = true;
float Sim::alpha = 1;
float Sim::beta = 2;
float Sim::evaporation = .5;
std::vector<std::shared_ptr<Node>> Sim::nodes;
std::unordered_map<NodePair, Connection, std::hash<NodePair>> Sim::connections;
Sim::AntStorage Sim::ants;

Sim::State Sim::state = START;
InputManager Sim::input;
Sim::AntStorage::iterator Sim::currentAnt;



void InputManager::doInput()
{
    switch (KeyManager::getJustPressed())
    {
    case SDLK_n:
        input = input == NODES ? NONE : NODES;
        break;
    case SDLK_c:
        input = input == CONNECT ? NONE : CONNECT;
        break;
    case SDLK_ESCAPE:
        input = NONE;
        break;
    case SDLK_SPACE:
        Sim::paused=false;
    }
    switch (input)
    {
    case NODES:
        if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
        {
            Sim::addNode(*(new Node(pairtoVec(MouseManager::getMousePos()))));
        }
        break;
    case CONNECT:
        if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
        {
            auto temp = Sim::getNode(pairtoVec(MouseManager::getMousePos()));
            if (temp.lock().get())
            {
                node1 = temp;
            }
        }
        else if (MouseManager::getJustReleased() ==SDL_BUTTON_LEFT)
        {
            auto node2 = Sim::getNode(pairtoVec(MouseManager::getMousePos()));
            if (node2.lock().get())
            {
                Sim::addConnection(node1,node2);
            }
            node1.reset();
        }
        else if (MouseManager::isPressed(SDL_BUTTON_LEFT) && node1.lock().get())
        {
            PolyRender::requestLine(glm::vec4(node1.lock().get()->center,pairtoVec(MouseManager::getMousePos())),{1,0,0,1},1);
        }
        break;
    }
}


NodePtr Sim::addNode(Node& node)
{
    std::shared_ptr<Node> ptr = std::shared_ptr<Node>(&node);
    nodes.push_back(ptr);
    return ptr;
}

NodePtr Sim::getNode(const glm::vec2& pos)
{
    int size = nodes.size();
    for (int i = 0; i <size;++i)
    {
        if (pointDistance(nodes[i]->center,pos) <= Node::radius)
        {
            return nodes[i];
        }
    }
    return NodePtr();
}

void Sim::addConnection(NodePtr& ptr1, NodePtr& ptr2)
{
    if (ptr1.lock().get() && ptr2.lock().get())
    {
        connections[{ptr1,ptr2}] = {ptr1,ptr2,pointDistance(ptr1.lock().get()->center,ptr2.lock().get()->center)};
        ptr1.lock().get()->neighbors.push_back(ptr2);
        ptr2.lock().get()->neighbors.push_back(ptr1);
    }
}

void Sim::setup()
{
    auto end = connections.end();
    for (auto it = connections.begin();it != end;++it)
    {
        it->second.pheromone = 1;
        it->second.updateChance();
    }
}

void Sim::render()
{
    auto nodesEnd = nodes.end();
    for (auto it = nodes.begin(); it != nodesEnd;)
    {
        if (Node* node = it->get())
        {
            node->render();
            ++it;
        }
        else
        {
            it = nodes.erase(it);
        }
    }

    auto conEnd = connections.end();
    for (auto it = connections.begin(); it != conEnd; )
    {
        PolyRender::requestLine(glm::vec4(it->first.first.lock().get()->center,it->first.second.lock().get()->center),{1,1,it->second.pheromone,1},1);
        ++it;
    }
    int size = ants.size();
    for (int i = 0; i < size; ++i)
    {
        ants[i]->render();
    }
}

void Sim::step()
{
    std::string message = "PAUSED";
    if (!paused)
    {
        switch (state)
        {
        case START:
            message = "START (you shouldn't be seeing this lol)";
            currentAnt = ants.begin();
            setup();
            state = ANTS;
            //no break statement, just move straight to ANTS state
        case ANTS:
            message = "Update Ants";
            while (currentAnt != ants.end() && !currentAnt->get()) //if ant is null, delete. Keep deleting until we reach a valid ant. Should never happen but you never know
            {
                currentAnt = ants.erase(currentAnt);
            }
            if (currentAnt != ants.end())
            {
                currentAnt->get()->step();
                if (currentAnt->get()->path.size() == nodes.size())
                {
                    ++currentAnt;
                }
            }
            else
            {
                state = EVAPORATE;
            }
            break;
        case EVAPORATE:
            {
                message = "Evaporate all pheromones";
                auto end= connections.end();
                for (auto it = connections.begin(); it != end; ++it)
                {
                    if (it->first.first.lock().get() && it->first.second.lock().get())
                    {
                        it->second.pheromone = (1-evaporation)*it->second.pheromone;
                    }
                    else
                    {
                        it = connections.erase(it);
                    }
                }
                state = PHEROMONES;
                break;
            }
        case PHEROMONES:
            {
                message = "Update pheromones based on ant trails";
                if (currentAnt == ants.end())
                {
                    currentAnt = ants.begin();
                }
                currentAnt->get()->spray();
                ++currentAnt;
                if (currentAnt == ants.end())
                {
                    state = START;
                }
                break;
            }
        default:
            throw std::logic_error("Invalid State in Sim::step\n");
        }
    }
    render();
    input.doInput();
    Font::tnr.requestWrite({message,{300,0,-1,1}});
    Font::tnr.requestWrite({input.getMessage(),{300,100,RenderProgram::getScreenDimen().x - 300,100}});
}

