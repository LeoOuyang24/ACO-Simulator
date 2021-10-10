#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED

#include "SDLhelper.h"
#include "glInterface.h"

#include "ants.h"

struct SimWindow : public Window
{
    enum Input //things the user can do to edit the simulation
    {
        NONE,
        NODES //add node
    };
    constexpr static int maxSpeed = 256; //max and min speed the Simulation can move at
    constexpr static int minSpeed = 1;
    Input input= NONE;

    SimWindow(const glm::vec4& rect) : Window(rect,nullptr,{1,1,1,1},0) //window dimensions aren't known until run time, so we have to pass that in as a parameter
    {

    }
    void update(float x,float y, float z, const glm::vec4& scale);
};

struct UIWindow : public Window
{
    UIWindow(const glm::vec4& rect) : Window(rect,nullptr,{.5,.5,.5,1},1)
    {

    }
    void update(float x, float y, float z, const glm::vec4& scale);
};

struct OptWindow : public Window
{
    constexpr static int solThick = 20; //thickness to draw the best solution
    constexpr static int ACOThick = 15; //thickness to draw the best ACO solution
    constexpr static int lastThick = 5; //thickness to draw the last solution
    bool solved = false; //whether or not we've solved TSP
    float solutionLength = 0, ACOLength = 0, lastLength = 0; //actual best solution length, best solution calculated by ACO thus far, and length of the last solution calculated respectively
    std::vector<Node*> solution;
    std::vector<Node*> ACOSolution;
    float solve(Node* cur, std::unordered_set<Node*>& visited, std::vector<Node*>& path, float pathLength, std::vector<Node*>& bestPath, float bestLength);
    void solve(); //TSP Brute force
    OptWindow(const glm::vec4& rect) : Window(rect,nullptr,{.75,.75,.75,1},0)
    {

    }
    void update(float x, float y, float z, const glm::vec4& scale);
};

struct InputManager
{
    std::unique_ptr<UIWindow> inputWindow;
    std::unique_ptr<SimWindow> simWindow;
    std::unique_ptr<OptWindow> optWindow; //windows for UI, simulation, and optimal solution
    void doInput();
    void init()
    {
        glm::vec2 screenDimen = RenderProgram::getScreenDimen();
        glm::vec4 inputRect = {0,0,screenDimen.x,screenDimen.y*.2}; //inputWindow rect
        glm::vec4 simRect = {0,inputRect.y + inputRect.a,screenDimen.x/2, screenDimen.y - inputRect.y - inputRect.a}; //simWindow rect
        inputWindow.reset((new UIWindow(inputRect)));
        simWindow.reset((new SimWindow(simRect)));
        optWindow.reset((new OptWindow({simRect.x + simRect.z,simRect.y,screenDimen.x - simRect.z,simRect.a})));
    }

    void render();
};

struct Sim
{
    enum State //state we are in the simulation/algorithm
    {
        START, //beginning of simulation
        ANTS, //update ants
        EVAPORATE, //evaporate all pheromones
        PHEROMONES, //update pheromones
        RESET //reset all ant states and positions
    };
    static glm::vec4 rect; //the window rect where the user can interact with the simulation.
    static DeltaTime pause;
    static bool paused;
    static float iterations;
    static float alpha; //variables that allow us to adjust our heuristic
    static float beta;
    static float evaporation; //pheromone evaporation from 0 to 1
    static int speed; //movements per second. <= 0 to pause after every movement
    static std::vector<std::shared_ptr<Node>> nodes;
    static std::unordered_map<NodePair, Connection> connections;
    typedef std::vector<std::shared_ptr<Ant>> AntStorage;
    static AntStorage ants;

    static State state;
    static AntStorage::iterator currentAnt; //represents the current ant to either step or update pheromones

    static InputManager input;

    static NodePtr addNode(Node& node);
    static NodePtr getNode(const glm::vec2& pos); //gets the node at that position. Returns an empty ptr otherwise
    static Connection& addConnection(const NodePtr& ptr1, const NodePtr& ptr2); //adds a connection between the given nodes and returns a reference. USeful for setup()

    static float greedy(); //solve TSP with greedy algorithm
    static void setup();
    static void render();
    static void step();
    static void update();
    static void reset();
    static void fullReset(); //deletes all nodes and ants


};

#endif // SIMULATION_H_INCLUDED
