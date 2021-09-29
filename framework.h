#ifndef FRAMEWORK_H_INCLUDED
#define FRAMEWORK_H_INCLUDED

#include <memory>

#include "render.h"

class Node;
typedef std::weak_ptr<Node> NodePtr;

template<>
struct std::hash<NodePtr>
{
    size_t operator () (const NodePtr& ptr) const;
};

template<>
struct std::equal_to<NodePtr>
{
    bool operator()(const NodePtr& a, const NodePtr& b) const;
};

bool operator==(const NodePtr& a, const NodePtr& b);

typedef std::pair<NodePtr,NodePtr> NodePair;

template<>
struct std::hash<NodePair>
{
    size_t operator()(const NodePair& pear) const;
};

struct Connection
{
    NodePtr node1, node2;
    float length = 1;
    float pheromone = 0;
    float chance = 0; //chance of taking this connection
    Node* getNode1();
    Node* getNode2();
    void updateChance();
};

struct Node
{
  static constexpr float radius = 10;
  glm::vec2 center;
  std::vector<NodePtr> neighbors;
  Node(const glm::vec2& pos) : center(pos)
  {

  }
  virtual void render()
  {
      PolyRender::requestCircle({1,0,0,1},center,radius,true,0);
  }
  /*virtual void renderConnections()
  {
        int size = neighbors.size();
        for (int i = 0; i < size; ++i)
        {
            if (Node* node = neighbors[i]->getNode())
            {
                PolyRender::requestLine(glm::vec4(rect.x + rect.z/2, rect.y + rect.a/2, node->rect.x + node->rect.z/2, node->rect.y + node->rect.a/2),
                                        {1,1,neighbors[i]->pheromone/100,1},
                                        1,RenderCamera::currentCamera);
            }
        }
  }*/
};


#endif
