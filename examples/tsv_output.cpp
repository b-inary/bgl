#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  bgl_app app("Output tsv formatted graph file");
  BGL_PARSE(app, argc, argv);

  for (auto [g, p] : app.graph_iterator<graph>()) {
    path out_path = p;
    out_path.replace_extension(".tsv");
    EXPECT_MSG(!path::exists(out_path), "overwrite {}", out_path);

    CONSOLE_LOG("graph loaded: {}\n  # of nodes: {}\n  # of edges: {}",
                p, commify(g.num_nodes()), commify(g.num_edges()));

    write_graph_tsv(out_path, g, false);
  }
}
