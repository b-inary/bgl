#include "io.hpp"
#include "../extlib/CLI11.hpp"

#define BGL_PARSE(app, argc, argv)                       \
  try {                                                  \
    (app).name(bgl::path::relative((argv)[0]).string()); \
    (app).parse((argc), (argv));                         \
  } catch (const CLI::ParseError &e) {                   \
    return (app).exit(e);                                \
  }

namespace bgl {
class bgl_app : public CLI::App {
private:
  template <typename GraphType>
  class cui_graph_iterator {
  public:
    cui_graph_iterator() {}
    cui_graph_iterator(const bgl_app &app) : app_{&app} {
      for (const std::string &p : app_->paths_) {
        paths_.push_back(p);
      }
      if (app_->folder_mode_) {
        iter_ = graph_folder_iterator<GraphType>(paths_[index_], app_->recursive_);
      }
      ready();
    }

    // prohibit copying
    cui_graph_iterator(const cui_graph_iterator &) = delete;
    cui_graph_iterator(cui_graph_iterator &&) = default;
    cui_graph_iterator &operator=(const cui_graph_iterator &) = delete;
    cui_graph_iterator &operator=(cui_graph_iterator &&) = default;

    std::pair<GraphType &, const path &> operator*() {
      if (app_->folder_mode_) return *iter_;
      return {g_, paths_[index_]};
    }

    cui_graph_iterator &operator++() {
      if (app_->folder_mode_) {
        ++iter_;
      } else {
        ++index_;
      }
      return ready();
    }

    cui_graph_iterator &&begin() { return std::move(*this); }
    cui_graph_iterator end() const { return {}; }
    bool operator==(const cui_graph_iterator &rhs) const {
      return index_ == paths_.size() && rhs.index_ == rhs.paths_.size();
    }
    bool operator!=(const cui_graph_iterator &rhs) const { return !(*this == rhs); }

  private:
    const bgl_app *app_ = nullptr;
    std::size_t index_ = 0;
    std::vector<path> paths_;
    GraphType g_;
    graph_folder_iterator<GraphType> iter_;

    cui_graph_iterator &ready() {
      if (app_->folder_mode_) {
        while (true) {
          if (iter_ != iter_.end()) break;
          if (++index_ >= paths_.size()) return *this;
          iter_ = graph_folder_iterator<GraphType>(paths_[index_], app_->recursive_);
        }
      } else {
        if (index_ >= paths_.size()) return *this;
        g_ = read_graph<GraphType>(paths_[index_]);
      }
      GraphType &g = app_->folder_mode_ ? (*iter_).first : g_;
      if (app_->simplify_) g.simplify();
      if (app_->undirected_) g.make_undirected();
      return *this;
    }
  };

public:
  bgl_app(const std::string &desc = "") : CLI::App(desc) {
    get_formatter()->column_width(22);
    get_formatter()->label("REQUIRED", "");

    add_option("paths", paths_, "Input path(s)")->required()->check(CLI::ExistingPath);
    add_flag("-f,--folder", folder_mode_, "Read all graphs in folder(s)");
    add_flag("-r,--recursive", recursive_, "Read all graphs in folder(s) recursively");
    add_flag("-s,--simplify", simplify_, "Simplify graph (remove self loops and multiple edges)");
    add_flag("-u,--undirected", undirected_, "Make graph undirected");
  }

  void parse(int argc, const char *const *argv) {
    CLI::App::parse(argc, argv);
    if (recursive_) folder_mode_ = true;
  }

  template <typename GraphType>
  cui_graph_iterator<GraphType> graph_iterator() const {
    cui_graph_iterator<GraphType> iter(*this);
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
  bool folder_mode_ = false;
  bool recursive_ = false;
  bool simplify_ = false;
  bool undirected_ = false;
};
}  // namespace bgl
