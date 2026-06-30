#pragma once
#include <vector>
#include "sorting.h"

struct BenchmarkInput {
    int    size;
    double sorted_ratio;   // 0.0 = random, 1.0 = fully sorted
    double duplicate_ratio;
    double variance;       // spread of values
};

class Benchmark {
public:
    // generate a random array based on input characteristics
    static std::vector<int> generate_array(const BenchmarkInput& input);

    // run all 8 algorithms on the same array, return all stats
    static std::vector<SortStats> run_all(std::vector<int> arr, bool capture_steps);

    // extract ML features from an array (for prediction)
    static BenchmarkInput extract_features(const std::vector<int>& arr);
};