#pragma once
#include <vector>
#include <string>
#include <cstdint>

// stats collected while running an algorithm
struct SortStats {
    std::string algorithm;
    long long   comparisons = 0;
    long long   swaps       = 0;
    double      time_ms     = 0;
    std::vector<std::vector<int>> snapshots;  // array state per step
};

class Sorting {
public:
    static SortStats bubble_sort(std::vector<int> arr, bool capture_steps);
    static SortStats selection_sort(std::vector<int> arr, bool capture_steps);
    static SortStats insertion_sort(std::vector<int> arr, bool capture_steps);
    static SortStats merge_sort(std::vector<int> arr, bool capture_steps);
    static SortStats quick_sort(std::vector<int> arr, bool capture_steps);
    static SortStats heap_sort(std::vector<int> arr, bool capture_steps);
    static SortStats shell_sort(std::vector<int> arr, bool capture_steps);
    static SortStats counting_sort(std::vector<int> arr, bool capture_steps);
};