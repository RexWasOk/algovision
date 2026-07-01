#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "../include/sorting.h"
#include "../include/graphs.h"
#include "../include/benchmark.h"

/*
 * =============================================================
 *  AlgoVision — main.cpp
 * =============================================================
 *  CLI interface for the Python bridge. Python runs this binary
 *  as a subprocess and parses JSON from stdout.
 *  All diagnostics go to stderr so stdout stays clean JSON.
 *
 *  Commands:
 *    algovision race <size> <sorted_ratio> <dup_ratio> <variance>
 *    algovision features <comma,separated,array>
 *    algovision sort <algorithm> <comma,separated,array>
 *    algovision graph <algorithm> <txt_file>
 *
 *  Graph text file format (simpler than JSON, trivial to parse):
 *    line 1: start end num_edges num_positions
 *    next num_edges lines: from to weight
 *    next num_positions lines: node_id x y
 * =============================================================
 */

std::string sort_stats_json(const SortStats& s) {
    std::ostringstream o;
    o << "{\"algorithm\":\"" << s.algorithm << "\","
      << "\"comparisons\":" << s.comparisons << ","
      << "\"swaps\":"       << s.swaps       << ","
      << "\"time_ms\":"     << s.time_ms     << "}";
    return o.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: algovision <command> [args]\n";
        return 1;
    }

    std::string command = argv[1];

    // ── race: run all 8 algorithms on synthetic array ────────
    if (command == "race") {
        if (argc < 6) { std::cerr << "Missing args\n"; return 1; }

        BenchmarkInput input;
        input.size            = std::stoi(argv[2]);
        input.sorted_ratio    = std::stod(argv[3]);
        input.duplicate_ratio = std::stod(argv[4]);
        input.variance        = std::stod(argv[5]);

        std::vector<int> arr = Benchmark::generate_array(input);
        std::vector<SortStats> results = Benchmark::run_all(arr, false);

        std::cout << "[";
        for (int i = 0; i < (int)results.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << sort_stats_json(results[i]);
        }
        std::cout << "]\n";
        return 0;
    }

    // ── features: extract ML features from array ────────────
    if (command == "features") {
        if (argc < 3) { std::cerr << "Missing array\n"; return 1; }

        std::vector<int> arr;
        std::istringstream ss(argv[2]);
        std::string token;
        while (std::getline(ss, token, ','))
            arr.push_back(std::stoi(token));

        BenchmarkInput f = Benchmark::extract_features(arr);
        std::cout << "{"
                  << "\"size\":"             << f.size             << ","
                  << "\"sorted_ratio\":"     << f.sorted_ratio     << ","
                  << "\"duplicate_ratio\":"  << f.duplicate_ratio  << ","
                  << "\"variance\":"         << f.variance
                  << "}\n";
        return 0;
    }

    // ── sort: run one algorithm with step capture ────────────
    if (command == "sort") {
        if (argc < 4) { std::cerr << "Missing args\n"; return 1; }
        std::string algo = argv[2];

        std::vector<int> arr;
        std::istringstream ss(argv[3]);
        std::string token;
        while (std::getline(ss, token, ','))
            arr.push_back(std::stoi(token));

        SortStats s;
        if      (algo == "bubble")    s = Sorting::bubble_sort(arr, true);
        else if (algo == "selection") s = Sorting::selection_sort(arr, true);
        else if (algo == "insertion") s = Sorting::insertion_sort(arr, true);
        else if (algo == "merge")     s = Sorting::merge_sort(arr, true);
        else if (algo == "quick")     s = Sorting::quick_sort(arr, true);
        else if (algo == "heap")      s = Sorting::heap_sort(arr, true);
        else if (algo == "shell")     s = Sorting::shell_sort(arr, true);
        else if (algo == "counting")  s = Sorting::counting_sort(arr, true);
        else { std::cerr << "Unknown algorithm\n"; return 1; }

        std::cout << "{"
                  << "\"algorithm\":\"" << s.algorithm << "\","
                  << "\"comparisons\":" << s.comparisons << ","
                  << "\"swaps\":"       << s.swaps       << ","
                  << "\"time_ms\":"     << s.time_ms     << ","
                  << "\"snapshots\":[";
        for (int i = 0; i < (int)s.snapshots.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << "[";
            for (int j = 0; j < (int)s.snapshots[i].size(); j++) {
                if (j > 0) std::cout << ",";
                std::cout << s.snapshots[i][j];
            }
            std::cout << "]";
        }
        std::cout << "]}\n";
        return 0;
    }

    // ── graph: run pathfinding on simple text format ─────────
    if (command == "graph") {
        if (argc < 4) { std::cerr << "Missing args\n"; return 1; }
        std::string algo    = argv[2];
        std::string txtfile = argv[3];

        std::ifstream f(txtfile);
        if (!f.is_open()) {
            std::cerr << "Cannot open graph file: " << txtfile << "\n";
            return 1;
        }

        // line 1: start end num_edges num_positions
        int start_node, end_node, num_edges, num_positions;
        f >> start_node >> end_node >> num_edges >> num_positions;

        // next num_edges lines: from to weight
        Graphs::AdjList graph;
        for (int i = 0; i < num_edges; i++) {
            int from, to, weight;
            f >> from >> to >> weight;
            graph[from].push_back({to, weight});
        }

        // next num_positions lines: node_id x y
        std::unordered_map<int, std::pair<double,double>> positions;
        for (int i = 0; i < num_positions; i++) {
            int node_id;
            double x, y;
            f >> node_id >> x >> y;
            positions[node_id] = {x, y};
        }

        // run chosen algorithm
        GraphStats s;
        if      (algo == "bfs")
            s = Graphs::bfs(graph, start_node, end_node);
        else if (algo == "dfs")
            s = Graphs::dfs(graph, start_node, end_node);
        else if (algo == "dijkstra")
            s = Graphs::dijkstra(graph, start_node, end_node);
        else if (algo == "astar")
            s = Graphs::astar(graph, start_node, end_node, positions);
        else {
            std::cerr << "Unknown graph algorithm: " << algo << "\n";
            return 1;
        }

        // output results as JSON
        std::cout << "{\"algorithm\":\"" << s.algorithm << "\","
                  << "\"nodes_visited\":" << s.nodes_visited << ","
                  << "\"total_cost\":"    << s.total_cost    << ","
                  << "\"visit_order\":[";
        for (int i = 0; i < (int)s.visit_order.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << s.visit_order[i];
        }
        std::cout << "],\"path\":[";
        for (int i = 0; i < (int)s.path.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << s.path[i];
        }
        std::cout << "]}\n";
        return 0;
    }

    std::cerr << "Unknown command: " << command << "\n";
    return 1;
}