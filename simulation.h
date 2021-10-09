#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED

#include "SDLhelper.h"

#include "ants.h"

struct InputManager
{
    enum Input //things the user can do to edit the simulation
    {
        NONE,
        NODES, //add node
        CONNECT, //add connection
        ANTS //add ant
    };
    constexpr static int maxSpeed = 256; //max and min speed the Simulation can move at
    constexpr static int minSpeed = 1;
    bool mouseDown = false; //used to calculate drag with the mouse
    Input input= NONE;
    NodePtr node1;
    void doInput();
    std::string getMessage()
    {
        switch (input)
        {
        case NONE:
            return "";
        case NODES:
            return "ADDING NODES,PRESS ESC TO STOP";
        case CONNECT:
            return "ADDING CONNECTIONS, PRESS ESC TO STOP";
        case ANTS:
            return "ADDING ANTS, PRESS ESC TO STOP";
        default:
            return "ERROR: INPUTMANAGER INVALID INPUT STATE";
        }
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

    static DeltaTime pause;
    static bool paused;
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

    static void setup();
    static void render();
    static void step();
    static void update();
    static void reset();


};

#endif // SIMULATION_H_INCLUDED
