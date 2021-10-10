#include "FreeTypeHelper.h"
#include "SDLhelper.h"
#include "vanilla.h"

#include "simulation.h"

glm::vec4 Sim::rect;
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

void SimWindow::update(float x, float y, float z, const glm::vec4& scale)
{
    switch (KeyManager::getJustPressed())
    {
    case SDLK_n:
        input = input == NODES ? NONE : NODES;
        break;
    case SDLK_ESCAPE:
        input = NONE;
        break;
    case SDLK_SPACE:
        Sim::paused = !Sim::paused;
        break;
    case SDLK_RIGHT:
        Sim::speed = std::min(2*Sim::speed,maxSpeed);
        break;
    case SDLK_LEFT:
        Sim::speed = std::max(minSpeed,Sim::speed/2);
        break;
    default:
        break;
    }
    switch (input)
    {
    case NODES:
        if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && pointInVec(rect,{x,y}))
        {
            Sim::addNode(*(new Node({x,y})));
        }
        break;
    }
    if (Sim::paused)
    {
        std::string message = "";
        switch (input)
        {
        case Input::NONE:
            message = "";
            break;
        case Input::NODES:
            message = "ADDING NODES,PRESS ESC TO STOP";
            break;
        default:
            message = "ERROR: INPUTMANAGER INVALID INPUT STATE";
            break;
        }
        Font::tnr.requestWrite({message,{rect.x + rect.z/2 ,rect.y  + rect.a*.01,-1,.65},0,{1,0,0,1},z,CENTER});
    }
    Window::update(x,y,z,scale);
}

void UIWindow::update(float mouseX, float mouseY, float z,const glm::vec4& state)
{

    requestWrite(Font::tnr,{"Speed: " + convert(Sim::speed) + "x", {rect.x + rect.z/4, rect.y + rect.a*.1,-1,.5}},z);
    std::string message = "";
    if (!Sim::paused)
    {
        switch (Sim::state)
        {
        case Sim::State::START:
            message = "Starting...";
            break;
        case Sim::State::ANTS:
            message = "Updating Ants";
            break;
        case Sim::State::EVAPORATE:
            message = "Evaporating all pheromones";
            break;
        case Sim::State::PHEROMONES:
            message = "Updating pheromones based on ant trails";
            break;
        case Sim::State::RESET:
            message = "Resetting ants state";
            break;
        default:
            message = "ERROR: Invalid Simulation state";
            break;

        }
    }

    requestWrite(Font::tnr,{Sim::paused ? "PAUSED" : "RUNNING",{rect.x + rect.z/2,rect.y + rect.a*.1,-1,1}},z);
    requestWrite(Font::tnr,{message,{rect.x + rect.z/2,rect.y + rect.a*.25,-1,.5}},z);

    if (!Sim::paused && Sim::input.optWindow->solved)
    {
        Font::tnr.requestWrite({"Fastest Solution: " + convert(Sim::input.optWindow->solutionLength),
                                {rect.x + rect.z*.9, rect.y + rect.a*.6, -1, .5},0,{0,0,0,1},z,RIGHT});
       Font::tnr.requestWrite({"Fastest ACO Solution: " + convert(Sim::input.optWindow->ACOLength),
                                {rect.x + rect.z*.9, rect.y + rect.a*.7, -1, .5},0,{0,0,0,1},z,RIGHT});
        Font::tnr.requestWrite({"ACO % longer than solution: " + convert((Sim::input.optWindow->ACOLength/Sim::input.optWindow->solutionLength - 1)*10000/10000),
                                {rect.x + rect.z*.9, rect.y + rect.a*.8, -1, .5},0,{0,0,0,1},z,RIGHT});
    }

    Window::update(mouseX,mouseY,z,state);

}

void InputManager::doInput()
{

}

void InputManager::render()
{

    inputWindow->updateTop(1);
    simWindow->updateTop(0);
    optWindow->updateTop(0);
}

float OptWindow::solve(Node* cur, std::unordered_set<Node*>& visited, std::vector<Node*>& path, float pathLength, std::vector<Node*>& bestPath, float bestLength)
{
    int size = Sim::nodes.size();
    if (visited.size() < size)
    {
        for (int i = 0; i < size; ++i)
        {
            Node* node = Sim::nodes[i].get();
            if (visited.find(node) == visited.end())
            {
                visited.insert(node);
                path.push_back(node);
                bestLength = solve(node,visited,path,pathLength + pointDistance(node->center,cur->center),bestPath, bestLength); //update bestLength
                visited.erase(visited.find(node));
                path.pop_back();
            }
        }
    }
    else
    {
        if (bestLength > pathLength || bestLength == 0) //bestLength is 0 when it's uninitialized
        {
            bestLength = pathLength;
            bestPath = path;
        }
    }
    return bestLength;
}

void OptWindow::solve()
{
    if (Sim::nodes.size() > 0)
    {
        std::unordered_set<Node*> visited;
        visited.insert(Sim::nodes[0].get());

        std::vector<Node*> path;
        path.reserve(Sim::nodes.size());
        path.push_back(Sim::nodes[0].get());

        solution.reserve(Sim::nodes.size());

        solutionLength = solve(Sim::nodes[0].get(),visited,path,0,solution,0);

        solved = true;
    }
}

void OptWindow::update(float x, float y,float z, const glm::vec4& scale)
{

    glm::vec2 offset ={rect.x,0}; //offset to render everything

    if (solution.size() != Sim::nodes.size())
    {
        solved = false;
    }
    if (!Sim::paused && !solved)
    {
        solve();
    }
    else if (solved)
    {
        int solutionSize = solution.size() - 1;
        for (int i = 0; i < solutionSize; ++i)
        {
            PolyRender::requestLine(glm::vec4(Sim::nodes[i]->center + offset, Sim::nodes[i + 1]->center + offset),{0,1,0,.6},.1,20);
        }
    }
    int size = Sim::nodes.size();
    int count = 0;
    int i = 0;
    std::unordered_set<Node*> visited;
    ACOLength = 0;
    while (count < size)
    {
        Sim::nodes[i]->render(offset);
        visited.insert(Sim::nodes[i].get());
        float max = 0;
        int index = i;
        for (int j = 0; j < size; ++j) //find the next node in our best path
        {
            if (j != i && visited.find(Sim::nodes[j].get()) == visited.end())
            {
                Connection* con = &Sim::connections[{Sim::nodes[i],Sim::nodes[j]}];
                if (con->getTotalPheromone() >= max)
                    {
                        index = j;
                        max = con->getTotalPheromone();
                    }
            }
        }
        PolyRender::requestLine(glm::vec4(Sim::nodes[i]->center + offset,Sim::nodes[index]->center + offset),{0,0,1,1},.75,7);
        ACOLength += pointDistance(Sim::nodes[i]->center,Sim::nodes[index]->center);
        i = index;
        count ++;
    }
    Window::update(x,y,z,scale);
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
   // std::cout << connections.size () << "\n";
    for (auto it = connections.begin(); it != conEnd; )
    {
        PolyRender::requestLine(glm::vec4(it->first.first.lock().get()->center,it->first.second.lock().get()->center),{1,0,0,it->second.pheromone*100},.5);
        ++it;
    }
    if (state == ANTS && currentAnt != ants.end() && currentAnt->get())
    {
        currentAnt->get()->render();
        currentAnt->get()->renderPath();
    }


    //PolyRender::requestRect({rect.x + rect.z, rect.y, RenderProgram::getScreenDimen().x - rect.z, rect.a},{.75,.75,.75,1},true,0,0);

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
