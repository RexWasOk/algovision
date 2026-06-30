#pragma once
#include <vector>
#include <string>
#include <unordered_map>

// edge in the graph
struct Edge {
    int to;
    int weight;
};

// stats collected while running a graph algorithm
struct GraphStats {
    std::string algorithm;
    long long   nodes_visited = 0;
    double      time_ms       = 0;
    std::vector<int> visit_order;       // order nodes were visited
    std::vector<int> path;              // final path (if applicable)
    int          total_cost = -1;
};

class Graphs {
public:
    // adjacency list: node -> list of edges
    using AdjList = std::unordered_map<int, std::vector<Edge>>;

    static GraphStats bfs(const AdjList& graph, int start, int end);
    static GraphStats dfs(const AdjList& graph, int start, int end);
    static GraphStats dijkstra(const AdjList& graph, int start, int end);
    static GraphStats astar(const AdjList& graph, int start, int end,
                            const std::unordered_map<int, std::pair<double,double>>& positions);
};