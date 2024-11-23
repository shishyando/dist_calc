#pragma once

#include "task.h"

#include <cassert>
#include <queue>
#include <optional>

class TaskManager {
public:
    TaskManager(double left, double right, double delta);
    Task GetTask();
    void PopTask();
    size_t UnassignedTasks();
    size_t UnsolvedTasks();
    TaskResult GetResult();
    void SetResult(const Task& result);
    void AddTask(const Task& task);

private:
    size_t tasks_left_;
    std::vector<std::optional<TaskResult>> results_;
    std::queue<Task> pending_tasks_;
};
