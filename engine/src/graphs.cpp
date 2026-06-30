#include "../include/graphs.h"
#include <queue>
#include <stack>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <limits>
#include <unordered_set>

/*
 * =============================================================
 *  AlgoVision — graphs.cpp
 * =============================================================
 *  4 graph traversal/pathfinding algorithms instrumented to
 *  track visit order and total path cost. Used to animate
 *  pathfinding on a frontend grid.
 * =============================================================
 */

GraphStats Graphs::bfs(const AdjList& graph, int start, int end) {
    GraphStats s; s.algorithm = "bfs";
    auto t0 = std::chrono::high_resolution_clock::now();

    std::queue<int> q;
    std::unordered_map<int,int> parent;
    std::unordered_set<int> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        int curr = q.front(); q.pop();
        s.visit_order.push_back(curr);
        s.nodes_visited++;

        if (curr == end) break;

        auto it = graph.find(curr);
        if (it != graph.end()) {
            for (const Edge& e : it->second) {
                if (!visited.count(e.to)) {
                    visited.insert(e.to);
                    parent[e.to] = curr;
                    q.push(e.to);
                }
            }
        }
    }

    // reconstruct path
    if (parent.count(end) || start == end) {
        int curr = end;
        while (curr != start) {
            s.path.push_back(curr);
            curr = parent[curr];
        }
        s.path.push_back(start);
        std::reverse(s.path.begin(), s.path.end());
        s.total_cost = (int)s.path.size() - 1;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

GraphStats Graphs::dfs(const AdjList& graph, int start, int end) {
    GraphStats s; s.algorithm = "dfs";
    auto t0 = std::chrono::high_resolution_clock::now();

    std::stack<int> st;
    std::unordered_map<int,int> parent;
    std::unordered_set<int> visited;

    st.push(start);

    while (!st.empty()) {
        int curr = st.top(); st.pop();
        if (visited.count(curr)) continue;

        visited.insert(curr);
        s.visit_order.push_back(curr);
        s.nodes_visited++;

        if (curr == end) break;

        auto it = graph.find(curr);
        if (it != graph.end()) {
            for (const Edge& e : it->second) {
                if (!visited.count(e.to)) {
                    if (!parent.count(e.to)) parent[e.to] = curr;
                    st.push(e.to);
                }
            }
        }
    }

    if (parent.count(end) || start == end) {
        int curr = end;
        while (curr != start) {
            s.path.push_back(curr);
            curr = parent[curr];
        }
        s.path.push_back(start);
        std::reverse(s.path.begin(), s.path.end());
        s.total_cost = (int)s.path.size() - 1;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

GraphStats Graphs::dijkstra(const AdjList& graph, int start, int end) {
    GraphStats s; s.algorithm = "dijkstra";
    auto t0 = std::chrono::high_resolution_clock::now();

    std::unordered_map<int,int> dist;
    std::unordered_map<int,int> parent;
    using PQItem = std::pair<int,int>; // (distance, node)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[start] = 0;
    pq.push({0, start});

    std::unordered_set<int> visited;

    while (!pq.empty()) {
        auto [d, curr] = pq.top(); pq.pop();
        if (visited.count(curr)) continue;
        visited.insert(curr);
        s.visit_order.push_back(curr);
        s.nodes_visited++;

        if (curr == end) break;

        auto it = graph.find(curr);
        if (it != graph.end()) {
            for (const Edge& e : it->second) {
                int new_dist = d + e.weight;
                if (!dist.count(e.to) || new_dist < dist[e.to]) {
                    dist[e.to] = new_dist;
                    parent[e.to] = curr;
                    pq.push({new_dist, e.to});
                }
            }
        }
    }

    if (parent.count(end) || start == end) {
        int curr = end;
        while (curr != start) {
            s.path.push_back(curr);
            curr = parent[curr];
        }
        s.path.push_back(start);
        std::reverse(s.path.begin(), s.path.end());
        s.total_cost = dist.count(end) ? dist[end] : -1;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

GraphStats Graphs::astar(const AdjList& graph, int start, int end,
        const std::unordered_map<int, std::pair<double,double>>& positions) {
    GraphStats s; s.algorithm = "astar";
    auto t0 = std::chrono::high_resolution_clock::now();

    auto heuristic = [&](int node) -> double {
        if (!positions.count(node) || !positions.count(end)) return 0;
        auto [x1,y1] = positions.at(node);
        auto [x2,y2] = positions.at(end);
        return std::sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    };

    std::unordered_map<int,double> g_score;
    std::unordered_map<int,int> parent;
    using PQItem = std::pair<double,int>; // (f_score, node)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    g_score[start] = 0;
    pq.push({heuristic(start), start});

    std::unordered_set<int> visited;

    while (!pq.empty()) {
        auto [f, curr] = pq.top(); pq.pop();
        if (visited.count(curr)) continue;
        visited.insert(curr);
        s.visit_order.push_back(curr);
        s.nodes_visited++;

        if (curr == end) break;

        auto it = graph.find(curr);
        if (it != graph.end()) {
            for (const Edge& e : it->second) {
                double tentative_g = g_score[curr] + e.weight;
                if (!g_score.count(e.to) || tentative_g < g_score[e.to]) {
                    g_score[e.to] = tentative_g;
                    parent[e.to] = curr;
                    pq.push({tentative_g + heuristic(e.to), e.to});
                }
            }
        }
    }

    if (parent.count(end) || start == end) {
        int curr = end;
        while (curr != start) {
            s.path.push_back(curr);
            curr = parent[curr];
        }
        s.path.push_back(start);
        std::reverse(s.path.begin(), s.path.end());
        s.total_cost = g_score.count(end) ? (int)g_score[end] : -1;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}