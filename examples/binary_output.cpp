#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  bgl_app app("output binary formatted graph file");
  BGL_PARSE(app, argc, argv);

  for (auto [g, p] : app.graph_iterator<graph>()) {
    path bgl_path = p;
    bgl_path.replace_extension(".bgl");
    EXPECT_MSG(!path::exists(bgl_path), "overwrite {}", bgl_path);

    CONSOLE_LOG("read graph: {}\n  # of nodes: {}\n  # of edges: {}",
                p, commify(g.num_nodes()), commify(g.num_edges()));

    write_graph_binary(bgl_path, g);
  }
}
