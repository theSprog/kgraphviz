#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>

#include "include/kgraphviz/graph.hpp"

// Run a command and return its output
inline std::string run_cmd(const std::string& cmd) {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (! pipe) throw std::runtime_error("Failed to run command: " + cmd);
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    if (pclose(pipe) != 0) {
        std::cerr << "⚠️  Warning: command failed: " << cmd << std::endl;
    }
    return result;
}

// Parse output of `pacman -Qi <pkg>` and extract dependencies
std::vector<std::string> get_direct_deps(const std::string& pkg_name) {
    std::vector<std::string> deps;
    std::istringstream iss(run_cmd("pacman -Qi " + pkg_name));
    std::string line;
    bool collecting = false;

    while (std::getline(iss, line)) {
        if (line.find("Depends On") == 0) {
            collecting = true;
            auto pos = line.find(':');
            if (pos != std::string::npos) {
                std::istringstream dep_stream(line.substr(pos + 1));
                std::string dep;
                while (dep_stream >> dep) {
                    deps.push_back(dep);
                }
            }
        } else if (collecting && (line.empty() || line[0] != ' ')) {
            break;
        } else if (collecting) {
            std::istringstream dep_stream(line);
            std::string dep;
            while (dep_stream >> dep) {
                deps.push_back(dep);
            }
        }
    }

    // Remove version constraints
    for (auto& dep : deps) {
        dep = dep.substr(0, dep.find_first_of("<=>"));
    }

    deps.erase(std::remove(deps.begin(), deps.end(), "None"), deps.end());
    return deps;
}

// Get all installed packages
std::vector<std::string> get_all_installed_packages() {
    std::vector<std::string> result;
    std::istringstream iss(run_cmd("pacman -Qq"));
    std::string line;
    while (std::getline(iss, line)) {
        if (! line.empty()) result.push_back(line);
    }
    return result;
}

// Build dependency graph and reverse mapping
void build_dep_graph(std::map<std::string, std::vector<std::string>>& deps_map) {
    auto all_pkgs = get_all_installed_packages();
    std::size_t total = all_pkgs.size();

    std::cout << "📦 Found " << total << " installed packages.\n";

    for (std::size_t i = 0; i < total; ++i) {
        const auto& pkg = all_pkgs[i];

        std::cout << "🔍 [" << (i + 1) << "/" << total << "] Analyzing: " << pkg << std::endl;

        auto deps = get_direct_deps(pkg);
        deps_map[pkg] = deps;

        std::cout << "    └─ " << deps.size() << " direct deps";
        if (! deps.empty()) {
            std::cout << ": ";
            for (std::size_t j = 0; j < std::min<std::size_t>(deps.size(), 5); ++j) {
                std::cout << deps[j];
                if (j + 1 < deps.size()) std::cout << ", ";
            }
            if (deps.size() > 5) std::cout << "...";
        }
        std::cout << std::endl;
    }
}

int main() {
    try {
        std::cout << "⏳ Analyzing installed packages..." << std::endl;
        std::map<std::string, std::vector<std::string>> deps_map;
        build_dep_graph(deps_map);

        std::cout << "🌐 Building graph..." << std::endl;
        kgraphviz::DiGraph g("PacmanDeps");

        for (const auto& pair : deps_map) {
            const std::string& pkg = pair.first;
            const auto& deps = pair.second;

            g.node(pkg);
            for (const auto& dep : deps) {
                g.node(dep);
                g.edge(pkg, dep);
            }
        }

        std::cout << "🖼️  Rendering to pacman_deps.svg..." << std::endl;
        g.render("pacman_deps.svg");

        std::cout << "✅ Done." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "❌ Error: " << ex.what() << std::endl;
        return 1;
    }
}
