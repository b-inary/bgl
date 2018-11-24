#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  bgl_app app("File read bench");
  BGL_PARSE(app, argc, argv);

  CONSOLE_TIMER;
  CONSOLE_LOG("bench start");

  for (auto [g, p] : app.graph_iterator<graph>()) {
    CONSOLE_LOG("graph loaded: {}\n  # of nodes: {}\n  # of edges: {}",
                p, commify(g.num_nodes()), commify(g.num_edges()));
  }
}
