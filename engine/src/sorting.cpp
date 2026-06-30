#include "../include/sorting.h"
#include <chrono>
#include <algorithm>

/*
 * =============================================================
 *  AlgoVision — sorting.cpp
 * =============================================================
 *  8 classic sorting algorithms instrumented to count
 *  comparisons and swaps, and optionally capture a snapshot
 *  of the array after every meaningful operation (for the
 *  frontend to animate step by step).
 *
 *  Each function takes the array BY VALUE (a copy) so the
 *  original input is never mutated; lets us run all 8
 *  algorithms on the same starting array for a fair race.
 * =============================================================
 */

static void snap(SortStats& s, const std::vector<int>& arr, bool capture) {
    if (capture) s.snapshots.push_back(arr);
}

SortStats Sorting::bubble_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "bubble_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            s.comparisons++;
            if (arr[j] > arr[j+1]) {
                std::swap(arr[j], arr[j+1]);
                s.swaps++;
                swapped = true;
                snap(s, arr, capture_steps);
            }
        }
        if (!swapped) break;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

SortStats Sorting::selection_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "selection_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i+1; j < n; j++) {
            s.comparisons++;
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        if (min_idx != i) {
            std::swap(arr[i], arr[min_idx]);
            s.swaps++;
            snap(s, arr, capture_steps);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

SortStats Sorting::insertion_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "insertion_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    int n = arr.size();
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0) {
            s.comparisons++;
            if (arr[j] <= key) break;
            arr[j+1] = arr[j];
            s.swaps++;
            j--;
            snap(s, arr, capture_steps);
        }
        arr[j+1] = key;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

static void merge(std::vector<int>& arr, int l, int m, int r,
                   SortStats& s, bool capture) {
    std::vector<int> left(arr.begin()+l, arr.begin()+m+1);
    std::vector<int> right(arr.begin()+m+1, arr.begin()+r+1);

    int i=0, j=0, k=l;
    while (i < (int)left.size() && j < (int)right.size()) {
        s.comparisons++;
        if (left[i] <= right[j]) arr[k++] = left[i++];
        else                     arr[k++] = right[j++];
        s.swaps++;
        snap(s, arr, capture);
    }
    while (i < (int)left.size())  { arr[k++] = left[i++];  s.swaps++; snap(s,arr,capture); }
    while (j < (int)right.size()) { arr[k++] = right[j++]; s.swaps++; snap(s,arr,capture); }
}

static void merge_sort_helper(std::vector<int>& arr, int l, int r,
                               SortStats& s, bool capture) {
    if (l >= r) return;
    int m = l + (r-l)/2;
    merge_sort_helper(arr, l, m, s, capture);
    merge_sort_helper(arr, m+1, r, s, capture);
    merge(arr, l, m, r, s, capture);
}

SortStats Sorting::merge_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "merge_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    if (!arr.empty())
        merge_sort_helper(arr, 0, arr.size()-1, s, capture_steps);

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

static int partition(std::vector<int>& arr, int low, int high,
                      SortStats& s, bool capture) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        s.comparisons++;
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
            s.swaps++;
            snap(s, arr, capture);
        }
    }
    std::swap(arr[i+1], arr[high]);
    s.swaps++;
    snap(s, arr, capture);
    return i+1;
}

static void quick_sort_helper(std::vector<int>& arr, int low, int high,
                               SortStats& s, bool capture) {
    if (low < high) {
        int pi = partition(arr, low, high, s, capture);
        quick_sort_helper(arr, low, pi-1, s, capture);
        quick_sort_helper(arr, pi+1, high, s, capture);
    }
}

SortStats Sorting::quick_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "quick_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    if (!arr.empty())
        quick_sort_helper(arr, 0, arr.size()-1, s, capture_steps);

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

static void heapify(std::vector<int>& arr, int n, int i,
                     SortStats& s, bool capture) {
    int largest = i, l = 2*i+1, r = 2*i+2;
    if (l < n) { s.comparisons++; if (arr[l] > arr[largest]) largest = l; }
    if (r < n) { s.comparisons++; if (arr[r] > arr[largest]) largest = r; }
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        s.swaps++;
        snap(s, arr, capture);
        heapify(arr, n, largest, s, capture);
    }
}

SortStats Sorting::heap_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "heap_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    int n = arr.size();
    for (int i = n/2 - 1; i >= 0; i--)
        heapify(arr, n, i, s, capture_steps);

    for (int i = n-1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        s.swaps++;
        snap(s, arr, capture_steps);
        heapify(arr, i, 0, s, capture_steps);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

SortStats Sorting::shell_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "shell_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    int n = arr.size();
    for (int gap = n/2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j = i;
            while (j >= gap) {
                s.comparisons++;
                if (arr[j-gap] <= temp) break;
                arr[j] = arr[j-gap];
                s.swaps++;
                j -= gap;
                snap(s, arr, capture_steps);
            }
            arr[j] = temp;
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}

SortStats Sorting::counting_sort(std::vector<int> arr, bool capture_steps) {
    SortStats s; s.algorithm = "counting_sort";
    auto t0 = std::chrono::high_resolution_clock::now();

    if (arr.empty()) { s.time_ms = 0; return s; }

    int max_val = *std::max_element(arr.begin(), arr.end());
    int min_val = *std::min_element(arr.begin(), arr.end());
    int range = max_val - min_val + 1;

    std::vector<int> count(range, 0);
    for (int x : arr) { count[x - min_val]++; s.comparisons++; }

    int idx = 0;
    for (int i = 0; i < range; i++) {
        while (count[i] > 0) {
            arr[idx++] = i + min_val;
            count[i]--;
            s.swaps++;
            snap(s, arr, capture_steps);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    s.time_ms = std::chrono::duration<double, std::milli>(t1-t0).count();
    return s;
}