#pragma once

#include <cstddef>

using TaskId = size_t;
using TaskResult = double;

struct Task {
    double left = 0.0;
    double right = 0.0;
    TaskId id = 0;
    TaskResult res = 0.0;

    void Evaluate() {
        res = right - left;
    }
};
