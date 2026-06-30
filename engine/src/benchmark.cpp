#include "../include/benchmark.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric>

/*
 * =============================================================
 *  AlgoVision — benchmark.cpp
 * =============================================================
 *  Generates synthetic arrays with controllable characteristics
 *  (sorted ratio, duplicate ratio, variance) for both training
 *  the ML model and running live benchmarks.
 *
 *  run_all() races all 8 sorting algorithms on identical input
 *  for a fair head to head comparison.
 *
 *  extract_features() computes the same 4 features used to
 *  train the ML model, so the live frontend can get a real
 *  time prediction on any array the user pastes in.
 * =============================================================
 */

std::vector<int> Benchmark::generate_array(const BenchmarkInput& input) {
    std::mt19937 rng(std::random_device{}());
    std::vector<int> arr(input.size);

    int range = std::max(1, (int)(input.variance * 1000));
    std::uniform_int_distribution<> dist(0, range);

    int unique_count = std::max(1,
        (int)(input.size * (1.0 - input.duplicate_ratio)));
    std::vector<int> pool(unique_count);
    for (int i = 0; i < unique_count; i++) pool[i] = dist(rng);

    std::uniform_int_distribution<> pool_dist(0, unique_count-1);
    for (int i = 0; i < input.size; i++)
        arr[i] = pool[pool_dist(rng)];

    // sort a portion based on sorted_ratio
    int sorted_count = (int)(input.size * input.sorted_ratio);
    if (sorted_count > 0)
        std::sort(arr.begin(), arr.begin() + sorted_count);

    return arr;
}

std::vector<SortStats> Benchmark::run_all(std::vector<int> arr, bool capture_steps) {
    std::vector<SortStats> results;
    results.push_back(Sorting::bubble_sort(arr, capture_steps));
    results.push_back(Sorting::selection_sort(arr, capture_steps));
    results.push_back(Sorting::insertion_sort(arr, capture_steps));
    results.push_back(Sorting::merge_sort(arr, capture_steps));
    results.push_back(Sorting::quick_sort(arr, capture_steps));
    results.push_back(Sorting::heap_sort(arr, capture_steps));
    results.push_back(Sorting::shell_sort(arr, capture_steps));
    results.push_back(Sorting::counting_sort(arr, capture_steps));
    return results;
}

BenchmarkInput Benchmark::extract_features(const std::vector<int>& arr) {
    BenchmarkInput f;
    f.size = arr.size();

    if (arr.empty()) {
        f.sorted_ratio = 0; f.duplicate_ratio = 0; f.variance = 0;
        return f;
    }

    // sorted ratio: fraction of adjacent pairs that are in order
    int sorted_pairs = 0;
    for (int i = 0; i + 1 < (int)arr.size(); i++)
        if (arr[i] <= arr[i+1]) sorted_pairs++;
    f.sorted_ratio = arr.size() > 1 ?
        (double)sorted_pairs / (arr.size()-1) : 1.0;

    // duplicate ratio
    std::vector<int> sorted_copy = arr;
    std::sort(sorted_copy.begin(), sorted_copy.end());
    int unique_count = std::unique(sorted_copy.begin(), sorted_copy.end())
                        - sorted_copy.begin();
    f.duplicate_ratio = 1.0 - ((double)unique_count / arr.size());

    // variance (normalized)
    double mean = std::accumulate(arr.begin(), arr.end(), 0.0) / arr.size();
    double sq_sum = 0;
    for (int x : arr) sq_sum += (x-mean)*(x-mean);
    double variance = sq_sum / arr.size();
    f.variance = std::min(1.0, std::sqrt(variance) / 1000.0);

    return f;
}