#include "extlib/CLI11.hpp"
#include "io.hpp"

#define BGL_PARSE(app, argc, argv) \
  try { \
    (app).name(bgl::path::relative((argv)[0]).string()); \
    (app).parse((argc), (argv)); \
  } catch (const CLI::ParseError &e) { \
    return (app).exit(e); \
  }

namespace bgl {
class bgl_app : public CLI::App {
private:
  template <typename GraphType>
  class cui_graph_iterator {
  public:
    cui_graph_iterator() {}
    cui_graph_iterator(const bgl_app &app)
      : folder_mode_{app.folder_mode_}
      , recursive_{app.recursive_}
      , rename_id_{app.rename_id_}
      , simplify_{app.simplify_}
      , undirected_{app.undirected_}
    {
      for (const std::string &p : app.paths_) {
        paths_.push_back(p);
      }
      if (folder_mode_) {
        iter_ = graph_folder_iterator<GraphType>(paths_[index_], recursive_, rename_id_);
      }
      ready();
    }

    std::pair<GraphType&, const path&> operator*() {
      if (folder_mode_) return *iter_;
      return {g_, paths_[index_]};
    }

    cui_graph_iterator &operator++() {
      if (folder_mode_) ++iter_; else ++index_;
      return ready();
    }

    cui_graph_iterator &begin() { return *this; }
    cui_graph_iterator end() const { return {}; }
    bool operator==(const cui_graph_iterator &rhs) const {
      return index_ == paths_.size() && rhs.index_ == rhs.paths_.size();
    }
    bool operator!=(const cui_graph_iterator &rhs) const {
      return !(*this == rhs);
    }

  private:
    std::size_t index_ = 0;
    std::vector<path> paths_;
    const bool folder_mode_ = false;
    const bool recursive_ = false;
    const bool rename_id_ = false;
    const bool simplify_ = false;
    const bool undirected_ = false;
    GraphType g_;
    graph_folder_iterator<GraphType> iter_;

    cui_graph_iterator &ready() {
      if (folder_mode_) {
        while (true) {
          if (iter_ != iter_.end()) break;
          if (++index_ >= paths_.size()) return *this;
          iter_ = graph_folder_iterator<GraphType>(paths_[index_], recursive_, rename_id_);
        }
      } else {
        if (index_ >= paths_.size()) return *this;
        if (paths_[index_].extension() == ".bgl") {
          g_ = read_graph_binary<GraphType>(paths_[index_]);
        } else {
          g_ = read_graph_tsv<GraphType>(paths_[index_], rename_id_);
        }
      }
      GraphType &g = folder_mode_ ? (*iter_).first : g_;
      if (simplify_) g.simplify();
      if (undirected_) g.make_undirected();
      return *this;
    }
  };

public:
  bgl_app(const std::string &desc = "") : CLI::App(desc) {
    get_formatter()->column_width(24);
    get_formatter()->label("REQUIRED", "");

    add_option("paths", paths_, "input path(s)")->required()->check(CLI::ExistingPath);
    add_flag("-d,--rename-id", rename_id_, "rename node IDs when input is tsv format");
    add_flag("-f,--folder", folder_mode_, "read all graphs in folder(s)");
    add_flag("-r,--recursive", recursive_, "read all graphs in folder(s) recursively");
    add_flag("-s,--simplify", simplify_, "simplify graph (remove self edges and multiple edges)");
    add_flag("-u,--undirected", undirected_, "make graph undirected");
  }

  void parse(int argc, const char *const *argv) {
    CLI::App::parse(argc, argv);
    if (recursive_) folder_mode_ = true;
  }

  template <typename GraphType>
  cui_graph_iterator<GraphType> graph_iterator() const {
    cui_graph_iterator<GraphType> iter = *this;
    if (iter == iter.end()) {
      std::cerr << get_name() << ": ";
      std::cerr << rang::style::bold << rang::fg::yellow;
      std::cerr << "warning: ";
      std::cerr << rang::style::reset << rang::fg::reset;
      std::cerr << "graph file does not exist in specified folder(s)\n";
    }
    return iter;
  }

  bool is_simple() const { return simplify_; }
  bool is_undirected() const { return undirected_; }

private:
  std::vector<std::string> paths_;
  bool folder_mode_;
  bool recursive_;
  bool rename_id_;
  bool simplify_;
  bool undirected_;
};
} // namespace bgl
