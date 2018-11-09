#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  bgl_app app("file read bench");
  BGL_PARSE(app, argc, argv);

  CONSOLE_TIMER;
  CONSOLE_LOG("bench start");

  for (auto [g, p] : app.graph_iterator<graph>()) {
    CONSOLE_LOG("read graph: {}\n  # of nodes: {}\n  # of edges: {}",
                p, g.num_nodes(), g.num_edges());
  }
}
