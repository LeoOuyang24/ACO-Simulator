#include "FreeTypeHelper.h"
#include "SDLhelper.h"
#include "vanilla.h"

#include "simulation.h"

DeltaTime Sim::pause;
bool Sim::paused = true;
float Sim::alpha = 1;
float Sim::beta = 2;
float Sim::evaporation = .5;
int Sim::speed = 1;
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
    case SDLK_a:
        input = input == ANTS ? NONE: ANTS;
        break;
    case SDLK_ESCAPE:
        input = NONE;
        break;
    case SDLK_SPACE:
        Sim::paused=!Sim::paused;
        break;
    case SDLK_RIGHT:
        Sim::speed = std::min(2*Sim::speed,maxSpeed);
        break;
    case SDLK_LEFT:
        Sim::speed = std::max(minSpeed,Sim::speed/2);
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
    case ANTS:
        if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
        {
             Sim::ants.emplace_back((new Ant(Sim::getNode(pairtoVec(MouseManager::getMousePos())))));
        }
        break;
    }
}

void InputManager::render()
{
    if (Sim::paused)
    {
          Font::tnr.requestWrite({getMessage(),{RenderProgram::getScreenDimen().x/2,100,-1,.5}});
    }
    Font::tnr.requestWrite({"Speed: " + convert(Sim::speed) + "x", {RenderProgram::getScreenDimen().x/4, 0,-1,.5}});
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

Connection& Sim::addConnection(const NodePtr& ptr1, const NodePtr& ptr2)
{
    if (ptr1.lock().get() && ptr2.lock().get())
    {
        connections[{ptr1,ptr2}] = {ptr1,ptr2,pointDistance(ptr1.lock().get()->center,ptr2.lock().get()->center)};
        ptr1.lock().get()->neighbors.push_back(ptr2);
        ptr2.lock().get()->neighbors.push_back(ptr1);
        return connections[{ptr1,ptr2}];
    }
    throw std::logic_error("Sim::addConnection: One or more nodes were null!");
}

void Sim::setup()
{
    ants.clear();
    int size = Sim::nodes.size();
    for (int i = 0; i < size;++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (nodes[i].get() != nodes[j].get())
            {
                Connection* ptr = &addConnection(nodes[i],nodes[j]);
                ptr->basePheromone = 1;
                ptr->updateChance();
            }
        }
        ants.emplace_back(new Ant(nodes[i]));
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
        PolyRender::requestLine(glm::vec4(it->first.first.lock().get()->center,it->first.second.lock().get()->center),{1,0,0,it->second.pheromone*100},-1);
        ++it;
    }
    if (state == ANTS && currentAnt != ants.end() && currentAnt->get())
    {
        currentAnt->get()->render();
        currentAnt->get()->renderPath();
    }
    std::string message = "";
    if (!paused)
    {
        switch (state)
        {
        case START:
            message = "Starting...";
            break;
        case ANTS:
            message = "Updating Ants";
            break;
        case EVAPORATE:
            message = "Evaporating all pheromones";
            break;
        case PHEROMONES:
            message = "Updating pheromones based on ant trails";
            break;
        case RESET:
            message = "Resetting ants state";
            break;
        default:
            message = "ERROR: Invalid Simulation state";
            break;

        }
    }
    Font::tnr.requestWrite({paused ? "PAUSED" : "RUNNING",{RenderProgram::getScreenDimen().x/2,0,-1,1}});
    Font::tnr.requestWrite({message,{RenderProgram::getScreenDimen().x/2,50,-1,.5}});

    input.render();
}

void Sim::step()
{
    switch (state)
    {
    case START:
        setup();
        currentAnt = ants.begin();
        state = ANTS;
        break;
        //break so we render ants at starting position
    case ANTS:
        while (currentAnt != ants.end() && !currentAnt->get()) //if ant is null, delete. Keep deleting until we reach a valid ant. Should never happen but you never know
        {
            currentAnt = ants.erase(currentAnt);
        }
        /*for (auto it = connections.begin(); it != connections.end(); ++it)
        {
            std::cout << it->get() << "\n";
        }*/

        if (currentAnt != ants.end())
        {
           // std::cout << currentAnt - ants.begin() << "\n";
            if (currentAnt->get()->done())
            {
                ++currentAnt;
             //   std::cout << "DONE\n";
            }
            else
            {
                currentAnt->get()->step();
            }
        }
        else
        {
            state = EVAPORATE;
        }
        break;
    case EVAPORATE:
        {
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
            if (ants.size() > 0)
            {
                if (currentAnt == ants.end())
                {
                    currentAnt = ants.begin();
                }
                currentAnt->get()->spray();
                ++currentAnt;
                if (currentAnt == ants.end())
                {
                    state = RESET;
                }
            }
            break;
        }
    case RESET:
        {
            reset();
            currentAnt = ants.begin();
            state = ANTS;
            break;
        }
    default:
        throw std::logic_error("Invalid State in Sim::step\n");
    }
}

void Sim::update()
{
    if (!paused)
    {
        if (speed > 0 )
        {
            if ( (!pause.isSet() || pause.getTimePassed() >= 1000/speed))
            {
                step();
                pause.set();
            }
        }
        else //if speed is < 0, step and then immediately pause
        {
            step();
            paused =true;
        }
    }
    render();
    input.doInput();


}

void Sim::reset()
{
    int size = ants.size();
    for (int i = 0; i < size; ++i)
    {
        ants[i]->reset();
    }
    state = START;
}
