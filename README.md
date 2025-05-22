# kgraphviz

**Drop-in C++11 replacement for Python’s [`graphviz`](https://graphviz.readthedocs.io/en/stable/) — write DOT code in C++ like a Pythonista.**

**kgraphviz** is a lightweight, header-only C++11 library for constructing and rendering [Graphviz DOT](https://graphviz.org/) graphs programmatically.  
Inspired by the Python `graphviz` package, it brings expressive syntax and intuitive APIs to modern C++ development.

---



## ✨ Features

- 🧩 Header-only, dependency-free, C++11 compatible
- 🎯 API modeled after Python’s `graphviz` (`node()`, `edge()`, `render()`, etc.)
- 📄 Automatically escapes/quotes DOT identifiers
- 🖼️ Renders to PNG, SVG, PDF, etc. via system `dot` binary
- 🔍 Supports both **directed** (`DiGraph`) and **undirected** (`Graph`) graphs
- 🧱 Supports subgraphs and attribute customization
- ⚙️ Cross-platform: Linux, macOS, Windows (including WSL)

---



## 🚀 Quickstart

```cpp
#include <kgraphviz/graph.hpp>

int main() {
    kgraphviz::DiGraph dot("MyGraph");

    // Add labeled nodes
    dot.node("A", "Start");
    dot.node("B", "Process");
    dot.node("C", "End");

    // Add edges (with optional attributes)
    dot.edge("A", "B");
    dot.edge("B", "C", {{"color", "blue"}, {"label", "next"}});

    // Render to file (format inferred from extension)
    dot.render("output.svg");

    // Or view using system default viewer
    dot.view();
}
````

You’ll get a nicely formatted SVG image rendered by Graphviz.

---



## 📦 API Overview

```cpp
kgraphviz::DiGraph dot("name");     // Directed graph
kgraphviz::Graph g("name");         // Undirected graph

dot.node("id", "label");            // Add node with label
dot.edge("A", "B");                 // Add edge
dot.edge("A", "B", {{"label", "x"}}); // Edge with attributes
dot.set_graph_attr("rankdir", "LR"); // Set graph-level attribute
dot.subgraph(sub);                 // Add subgraph
dot.render("out.svg");             // Render to file
dot.render_to_memory();           // Render to memory as vector<uint8_t>
dot.view();                        // Open with default viewer
```

All identifiers and strings are automatically escaped for DOT format.
Attributes are passed via `std::map<std::string, std::string>` (alias: `AttrMap`).

---



## 🛠️ Installation

`kgraphviz` is **header-only** — just add it to your project and include:

```cpp
#include <kgraphviz/graph.hpp>
```

Make sure Graphviz (`dot`) is available in your system PATH.

- On Debian/Ubuntu: `sudo apt install graphviz`
- On macOS: `brew install graphviz`
- On Windows: install Graphviz and ensure `dot.exe` is in your PATH

---



## 📁 File Structure

```text
.
├── include/
│   └── kgraphviz/
│       ├── graph.hpp         // Main Graph & DiGraph APIs
│       ├── source.hpp        // Source: render from raw DOT string
│       ├── options.hpp       // Render options (format, engine, etc.)
│       ├── exceptions.hpp    // Custom exception types
│       └── detail/
│           ├── render.hpp    // Internal render logic (dot command)
│           ├── viewer.hpp    // Platform viewer (open, start, etc.)
│           ├── tmpfile.hpp   // Temp file helpers
│           └── run_command.hpp // Command execution helpers (stdout/stderr capture)
├── LICENSE
└── README.md
```

All public headers are under include/kgraphviz/ — just add -Iinclude to your compiler flags.

---



## 📄 License

This project is licensed under the **MIT License**.
Contributions welcome via pull requests or issues!

---



## 💡 Acknowledgements

Inspired by the elegant interface of [Python’s graphviz](https://github.com/xflr6/graphviz).
Built with ❤️ for C++11 minimalism and clean tooling.

---



## 🎁 Pacman Dependency Graph Easter Egg (Arch Only)

If you're using Arch Linux, you're in for a treat!

We’ve included a special example: **`pacman_deps_graph.cpp`** — a small program that uses `pacman -Q` and `pacman -Qi` to analyze all installed packages on your system, and then uses `kgraphviz` to generate a complete dependency graph.



🛠️ How to use:

```bash
g++ -std=c++11 -Iinclude -O2 pacman_deps_graph.cpp -o pacman_deps_graph
./pacman_deps_graph
# by the way, it may takes five or ten minutes
```

This will generate a file called:

```
pacman_deps.svg
```

...a visual representation of **your system's entire package dependency graph**, have fun!

> Note: This only works on Arch-based systems with pacman installed. On other distributions, the program will likely fail due to `pacman` not being found.

