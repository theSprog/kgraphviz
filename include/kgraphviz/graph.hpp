#pragma once
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <utility>

#include "options.hpp"

#include "detail/tmpfile.hpp"
#include "detail/viewer.hpp"
#include "detail/render.hpp"

namespace kgraphviz {
using AttrMap = std::map<std::string, std::string>;

class BaseGraph {
    struct Statement {
        enum class Type {
            RawLine,
            Node,
            Edge,
            Subgraph
        };
        Type type;

        // RawLine
        std::string raw;

        // Node
        std::string node_name;
        std::map<std::string, std::string> node_attrs;

        // Edge
        std::string tail, head;
        std::map<std::string, std::string> edge_attrs;

        // Subgraph
        std::shared_ptr<BaseGraph> subgraph;

        static Statement make_raw(const std::string& r) {
            Statement s;
            s.type = Type::RawLine;
            s.raw = r;
            return s;
        }

        static Statement make_node(const std::string& name, const std::map<std::string, std::string>& attrs) {
            Statement s;
            s.type = Type::Node;
            s.node_name = name;
            s.node_attrs = attrs;
            return s;
        }

        static Statement
        make_edge(const std::string& t, const std::string& h, const std::map<std::string, std::string>& attrs) {
            Statement s;
            s.type = Type::Edge;
            s.tail = t;
            s.head = h;
            s.edge_attrs = attrs;
            return s;
        }

        static Statement make_subgraph(const std::shared_ptr<BaseGraph>& g) {
            Statement s;
            s.type = Type::Subgraph;
            s.subgraph = g;
            return s;
        }

        std::string to_string(const BaseGraph& owner, int indent_level) const {
            const std::string indent(indent_level * 4, ' ');
            std::ostringstream oss;

            switch (type) {
                case Type::RawLine:
                    oss << indent << raw << "\n";
                    break;

                case Type::Node:
                    oss << indent << owner.escape_id(node_name);
                    if (! node_attrs.empty()) {
                        oss << " [" << owner.format_attrs(node_attrs) << "]";
                    }
                    oss << ";\n";
                    break;

                case Type::Edge:
                    oss << indent << owner.escape_id(tail) << " " << owner.edge_op() << " " << owner.escape_id(head);
                    if (! edge_attrs.empty()) {
                        oss << " [" << owner.format_attrs(edge_attrs) << "]";
                    }
                    oss << ";\n";
                    break;

                case Type::Subgraph:
                    oss << subgraph->to_string(indent_level); // recursive
                    break;
            }

            return oss.str();
        }
    };


  protected:
    std::string graph_name_;
    bool strict_ = false;
    bool directed_ = false;

    std::string comment_;
    AttrMap graph_attr_;
    AttrMap node_attr_;
    AttrMap edge_attr_;

    std::vector<Statement> statements_;

    std::string edge_op() const {
        return directed_ ? "->" : "--";
    }

  public:
    BaseGraph(const std::string& name = "G", bool strict = false, bool directed = false)
        : graph_name_(name), strict_(strict), directed_(directed) {}

    void set_graph_attr(const std::string& key, const std::string& value) {
        graph_attr_[key] = value;
    }

    void set_node_attr(const std::string& key, const std::string& value) {
        node_attr_[key] = value;
    }

    void set_edge_attr(const std::string& key, const std::string& value) {
        edge_attr_[key] = value;
    }

    void
    node(const std::string& name, const std::string& label = "", const std::map<std::string, std::string>& attrs = {}) {
        auto merged = attrs;
        if (! label.empty()) merged["label"] = label;
        statements_.push_back(Statement::make_node(name, merged));
    }

    void edge(const std::string& tail, const std::string& head, const std::map<std::string, std::string>& attrs = {}) {
        statements_.push_back(Statement::make_edge(tail, head, attrs));
    }

    void edges(const std::vector<std::pair<std::string, std::string>>& pairs,
               const std::map<std::string, std::string>& attrs = {}) {
        for (const auto& p : pairs) {
            edge(p.first, p.second, attrs);
        }
    }

    void subgraph(const BaseGraph& sub) {
        statements_.push_back(Statement::make_subgraph(std::make_shared<BaseGraph>(sub)));
    }

    void save_to(const std::string& path) const {
        std::ofstream ofs(path.c_str());
        if (! ofs) {
            throw std::runtime_error("Failed to open file for writing: " + path);
        }
        ofs << to_string();
    }

    void save_to(std::ostream& os) const {
        os << to_string();
    }

    std::string to_string(int indent_level = 0) const {
        const std::string indent(indent_level * 4, ' ');
        std::ostringstream oss;

        if (! comment_.empty() && indent_level == 0) {
            oss << "// " << comment_ << "\n";
        }

        if (indent_level == 0) {
            if (strict_) oss << "strict ";
            oss << (directed_ ? "digraph " : "graph ") << escape_id(graph_name_) << " {\n";
        } else {
            oss << indent << "subgraph " << escape_id("cluster_" + graph_name_) << " {\n";
        }

        if (! graph_attr_.empty()) {
            oss << indent << "    graph [" << format_attrs(graph_attr_) << "];\n";
        }
        if (! node_attr_.empty()) {
            oss << indent << "    node [" << format_attrs(node_attr_) << "];\n";
        }
        if (! edge_attr_.empty()) {
            oss << indent << "    edge [" << format_attrs(edge_attr_) << "];\n";
        }

        for (const auto& stmt : statements_) {
            oss << stmt.to_string(*this, indent_level + 1);
        }

        oss << indent << "}\n";
        return oss.str();
    }

    void render(const std::string& output_path, const RenderOptions& render_options_ = RenderOptions()) const {
        Renderer::render_from_string(to_string(), output_path, render_options_);
    }

    std::vector<uint8_t> render_to_memory(const RenderOptions& render_options_ = RenderOptions()) const {
        return Renderer::render_from_string_to_memory(to_string(), render_options_);
    }

    void view(const RenderOptions& render_options_ = RenderOptions()) const {
        std::string output_path = TmpFile::generate_path(DefaultFormat);
        Renderer::render_from_string(to_string(), output_path, render_options_);
        Viewer::view(output_path);
    }

    void set_comment(const std::string& comment) {
        comment_ = comment;
    }

  private:
    static inline std::string escape_id(const std::string& id) {
        // 允许字母、数字、下划线，不加引号
        if (id.empty()) return "\"\"";

        for (char ch : id) {
            if (! std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
                goto need_escape;
            }
        }
        return id;

    need_escape:
        std::ostringstream oss;
        oss << "\"";
        for (char ch : id) {
            if (ch == '"')
                oss << "\\\"";
            else
                oss << ch;
        }
        oss << "\"";
        return oss.str();
    }

    static inline std::string format_attrs(const AttrMap& attrs) {
        std::ostringstream oss;
        bool first = true;
        for (const auto& kv : attrs) {
            if (! first) oss << ", ";
            first = false;
            oss << escape_id(kv.first) << "=" << escape_id(kv.second);
        }
        return oss.str();
    }
};

class Graph : public BaseGraph {
  public:
    Graph(const std::string& name = "G", bool strict = false) : BaseGraph(name, strict, false) {}
};

class DiGraph : public BaseGraph {
  public:
    DiGraph(const std::string& name = "DG", bool strict = false) : BaseGraph(name, strict, true) {}
};

} // namespace kgraphviz
