#pragma once
#include "bgl/graph/visitor.hpp"
#include <deque>

/// [destructive] order by SlashBurn
namespace bgl {
template <typename GraphType>
node_t order_by_slashburn(GraphType &g, double r = 0.01) {
  node_t k = std::ceil(r * g.num_nodes());

  // hub selection function
  auto hub_selection = fn(degree_vector) {
    node_t n = degree_vector.size();
    std::vector<node_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), fn(v, w) { return degree_vector[v] > degree_vector[w]; });

    // random tie-break
    if (degree_vector[order[k - 1]] == degree_vector[order[k]]) {
      auto [lb, ub] = std::equal_range(order.begin(), order.end(), degree_vector[order[k - 1]],
                                       fn(v, w) { return degree_vector[v] > degree_vector[w]; });
      std::shuffle(lb, ub, bgl_random);
    }

    order.resize(k);
    return order;
  };

  GraphType gu = g.clone().make_undirected();
  std::deque<node_t> order_head, order_tail;

  std::vector<node_t> orig_id(g.num_nodes());
  std::iota(orig_id.begin(), orig_id.end(), 0);

  while (true) {
    node_t n = gu.num_nodes();

    // generate degree vector
    std::vector<node_t> degree_vector(n);
    for (node_t v : gu.nodes()) {
      degree_vector[v] = gu.outdegree(v);
    }

    // hub selection
    std::vector<node_t> hubset = hub_selection(degree_vector);
    std::vector<bool> is_hub(n);
    for (node_t v : hubset) {
      is_hub[v] = true;
      order_tail.push_front(orig_id[v]);
    }

    // node permutation
    std::vector<node_t> perm;
    for (node_t v : gu.nodes()) {
      if (!is_hub[v]) {
        orig_id[perm.size()] = orig_id[v];
        perm.push_back(v);
      }
    }
    perm.insert(perm.end(), hubset.begin(), hubset.end());
    gu.permute_nodes(perm);

    // compute label for hub ordering
    std::vector<node_t> label(n);
    for (int i : irange(k)) {
      node_t v = n - k + i;
      for (node_t w : gu.neighbors(v)) {
        label[w] = i;
      }
    }

    // remove hubs
    gu.resize(n - k);

    int current_ccid = 0;
    std::vector<int> ccid(n - k, -1);
    std::vector<node_t> ccsize;
    visitor_by_distance<GraphType> visitor(gu);

    // compoute CCs
    for (node_t v : gu.nodes()) {
      if (ccid[v] == -1) {
        node_t current_size = 0;
        visitor.visit(v, fn(w, d[[maybe_unused]]) {
          if (ccid[w] != -1) return false;
          ccid[w] = current_ccid;
          ++current_size;
          return true;
        });
        ++current_ccid;
        ccsize.push_back(current_size);
      }
    }

    std::vector<node_t> maxhub(current_ccid);
    for (node_t v : gu.nodes()) {
      maxhub[ccid[v]] = std::max(label[v], maxhub[ccid[v]]);
    }

    // detect largest CC
    int lccid = std::max_element(ccsize.begin(), ccsize.end()) - ccsize.begin();
    std::vector<node_t> lcc_nodes, spoke_nodes;
    for (node_t v : gu.nodes()) {
      (ccid[v] == lccid ? lcc_nodes : spoke_nodes).push_back(v);
    }

    // order spoke nodes
    std::sort(spoke_nodes.begin(), spoke_nodes.end(), fn(v, w) {
      return maxhub[ccid[v]] < maxhub[ccid[w]] ||
             (maxhub[ccid[v]] == maxhub[ccid[w]] &&
              (ccsize[ccid[v]] < ccsize[ccid[w]] ||
               (ccsize[ccid[v]] == ccsize[ccid[w]] &&
                (gu.outdegree(v) < gu.outdegree(w) ||
                 (gu.outdegree(v) == gu.outdegree(w) && v < w)))));
    });
    for (node_t v : spoke_nodes) {
      order_head.push_back(orig_id[v]);
    }

    // node permutation
    std::vector<node_t> perm2;
    perm2.insert(perm2.end(), lcc_nodes.begin(), lcc_nodes.end());
    perm2.insert(perm2.end(), spoke_nodes.begin(), spoke_nodes.end());
    for (int i : irange(lcc_nodes.size())) {
      orig_id[i] = orig_id[lcc_nodes[i]];
    }
    gu.permute_nodes(perm2);

    // remove spokes
    gu.resize(lcc_nodes.size());

    // termination
    if (lcc_nodes.size() < k) {
      for (node_t v : gu.nodes()) {
        order_head.push_back(orig_id[v]);
      }
      break;
    }
  }

  // final ordering/permutation
  std::vector<node_t> order;
  order.insert(order.end(), order_head.begin(), order_head.end());
  order.insert(order.end(), order_tail.begin(), order_tail.end());
  g.permute_nodes(order);

  return order_head.size();
}
}  // namespace bgl
