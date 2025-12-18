#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>

#include <ogdf/fileformats/GraphIO.h>

#include <ogdf/basic/simple_graph_alg.h>      // isSimple(), makeSimple()
#include <ogdf/basic/extended_graph_alg.h>    // isPlanar()
#include <ogdf/planarlayout/PlanarStraightLayout.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using namespace ogdf;

static std::string expandTilde(std::string path) {
    if (!path.empty() && path[0] == '~') {
        const char *home = std::getenv("HOME");
        if (home) path = std::string(home) + path.substr(1);
    }
    return path;
}

static bool isGraphML(const fs::path &p) {
    auto ext = p.extension().string();
    for (auto &c : ext) c = (char)std::tolower((unsigned char)c);
    return ext == ".graphml";
}

int main(int argc, char **argv) {
    // Defaults (se você não passar args)
    fs::path inputDir  = expandTilde("~/benchmarks/rome_planar_only/rome");
    fs::path outputDir = expandTilde("~/benchmarks/rome_planar_only/rome_svg");

    if (argc >= 2) inputDir  = expandTilde(argv[1]);
    if (argc >= 3) outputDir = expandTilde(argv[2]);

    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        std::cerr << "Diretorio de entrada invalido: " << inputDir << "\n";
        return 1;
    }
    fs::create_directories(outputDir);

    size_t ok = 0, fail = 0, skipped = 0;

    for (const auto &entry : fs::directory_iterator(inputDir)) {
        if (!entry.is_regular_file()) continue;
        const fs::path inFile = entry.path();
        if (!isGraphML(inFile)) continue;

        Graph G;

        // Lê (formato inferido pela extensão .graphml) :contentReference[oaicite:4]{index=4}
        if (!GraphIO::read(G, inFile.string())) {
            std::cerr << "[FAIL] nao consegui ler: " << inFile.filename() << "\n";
            fail++;
            continue;
        }

        // Pré-condição do PlanarStraightLayout: planar e simples :contentReference[oaicite:5]{index=5}
        if (!isSimple(G)) {
            makeSimple(G); // remove self-loops e paralelas :contentReference[oaicite:6]{index=6}
        }

        if (!isPlanar(G)) {
            std::cerr << "[SKIP] nao planar (inesperado no seu filtro): " << inFile.filename() << "\n";
            skipped++;
            continue;
        }

        GraphAttributes GA(G,
            GraphAttributes::nodeGraphics |
            GraphAttributes::edgeGraphics |
            GraphAttributes::nodeLabel
        );

        // Tamanho padrão dos nós (muitos layouts assumem isso)
        for (node v = G.firstNode(); v; v = v->succ()) {
            GA.width(v)  = 10.0;
            GA.height(v) = 10.0;
            // Se quiser rotular com índice:
            // GA.label(v) = std::to_string(v->index()).c_str();
        }

        // Layout planar com arestas retas (sem cruzamentos) :contentReference[oaicite:7]{index=7}
        PlanarStraightLayout psl;
        psl.separation(20.0); // distância mínima (ajuda a “espalhar”)
        psl.call(GA);

        fs::path outFile = outputDir / (inFile.stem().string() + ".svg");

        // Exporta SVG :contentReference[oaicite:8]{index=8}
        if (!GraphIO::write(GA, outFile.string(), GraphIO::drawSVG)) {
            std::cerr << "[FAIL] nao consegui escrever: " << outFile.filename() << "\n";
            fail++;
            continue;
        }

        ok++;
        std::cout << "[OK] " << inFile.filename() << " -> " << outFile.filename() << "\n";
    }

    std::cout << "\nResumo: OK=" << ok << " FAIL=" << fail << " SKIP=" << skipped << "\n";
    std::cout << "Saida em: " << outputDir << "\n";
    return (fail > 0) ? 2 : 0;
}
