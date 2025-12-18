#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/extended_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>

#include <filesystem>
#include <chrono>
#include <iostream>
#include <string>
#include <algorithm>

using namespace ogdf;
namespace fs = std::filesystem;

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

static bool isSupportedExt(const fs::path &p) {
    auto ext = lower(p.extension().string());
    return ext == ".graphml" || ext == ".gml" || ext == ".dot" || ext == ".gv";
}

static void printHeader() {
    std::cout << "instance,source,n,m,planar,read_ms,planarity_ms\n";
}

static void processFile(const std::string &path, bool onlyPlanar) {
    Graph G;

    auto t0 = std::chrono::steady_clock::now();
    // OGDF infere o formato pela extensão (GraphML/GML/DOT etc.) :contentReference[oaicite:3]{index=3}
    bool ok = GraphIO::read(G, path);
    auto t1 = std::chrono::steady_clock::now();

    if (!ok) return; // arquivo inválido/inesperado

    auto t2 = std::chrono::steady_clock::now();
    bool planar = isPlanar(G);
    auto t3 = std::chrono::steady_clock::now();

    if (onlyPlanar && !planar) return;

    auto read_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto plan_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    std::cout << path << ",file,"
              << G.numberOfNodes() << ","
              << G.numberOfEdges() << ","
              << (planar ? 1 : 0) << ","
              << read_ms << ","
              << plan_ms << "\n";
}

static void genPlanar(int count, int n, int m) {
    for (int i = 1; i <= count; i++) {
        Graph G;

        auto t0 = std::chrono::steady_clock::now();
        // Gera grafo planar simples e conectado :contentReference[oaicite:4]{index=4}
        randomPlanarConnectedGraph(G, n, m);
        auto t1 = std::chrono::steady_clock::now();

        auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Por segurança (e para log), ainda dá pra checar planaridade:
        auto t2 = std::chrono::steady_clock::now();
        bool planar = isPlanar(G);
        auto t3 = std::chrono::steady_clock::now();
        auto plan_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

        std::cout << "gen_" << i << ",gen,"
                  << G.numberOfNodes() << ","
                  << G.numberOfEdges() << ","
                  << (planar ? 1 : 0) << ","
                  << gen_ms << ","
                  << plan_ms << "\n";
    }
}

static void usage() {
    std::cerr <<
    "Uso:\n"
    "  bench --dir <pasta> [--only-planar] > out.csv\n"
    "  bench --gen-planar --count C --n N --m M > out.csv\n";
}

int main(int argc, char **argv) {
    std::string dir;
    bool onlyPlanar = false;
    bool gen = false;
    int count = 0, n = 0, m = 0;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--dir" && i+1 < argc) dir = argv[++i];
        else if (a == "--only-planar") onlyPlanar = true;
        else if (a == "--gen-planar") gen = true;
        else if (a == "--count" && i+1 < argc) count = std::stoi(argv[++i]);
        else if (a == "--n" && i+1 < argc) n = std::stoi(argv[++i]);
        else if (a == "--m" && i+1 < argc) m = std::stoi(argv[++i]);
        else { usage(); return 2; }
    }

    printHeader();

    if (gen) {
        if (count <= 0 || n <= 0 || m < 0) { usage(); return 2; }
        genPlanar(count, n, m);
        return 0;
    }

    if (!dir.empty()) {
        for (auto &p : fs::recursive_directory_iterator(dir)) {
            if (!p.is_regular_file()) continue;
            if (!isSupportedExt(p.path())) continue;
            processFile(p.path().string(), onlyPlanar);
        }
        return 0;
    }

    usage();
    return 2;
}
