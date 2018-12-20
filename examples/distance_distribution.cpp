#include "bgl/bgl.hpp"
using namespace bgl;

int main(int argc, char **argv) {
  bgl_app app("Compute approximate distance distribution using HyperBall");
  int log2k = 10;
  app.add_option("-b,--log2k", log2k, "Control number of registers of HyperLogLog (default: 10)");
  BGL_PARSE(app, argc, argv);

  for (auto [g, p] : app.graph_iterator<graph>()) {
    CONSOLE_TIMER;
    CONSOLE_LOG("graph loaded: {}\n  # of nodes: {}\n  # of edges: {}", p, commify(g.num_nodes()),
                commify(g.num_edges()));

    std::vector<std::atomic<double>> distance_distribution(101);
    hyperball(g, log2k, fn(v[[maybe_unused]], d, c) {
      double cur = distance_distribution[d];
      while (!distance_distribution[d].compare_exchange_weak(cur, cur + c)) continue;
    });

    fmt::print("distance distribution:\n{}\n", distance_distribution);
  }
}
