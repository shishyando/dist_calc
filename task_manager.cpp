#include "task_manager.h"

#include <cassert>

TaskManager::TaskManager(double left, double right, double delta) {
    double cur_left = left;
    size_t id = 0;
    while (cur_left < right) {
        double cur_right = std::min(cur_left + delta, right);
        Task task{.left = cur_left, .right = cur_right, .id = id};
        AddTask(std::move(task));
        cur_left += delta;
        ++id;
    }
}

Task TaskManager::GetTask() {
    return pending_tasks_.front();
}

void TaskManager::PopTask() {
    pending_tasks_.pop();
}

size_t TaskManager::UnassignedTasks() {
    return pending_tasks_.size();
}

size_t TaskManager::UnsolvedTasks() {
    return tasks_left_;
}

TaskResult TaskManager::GetResult() {
    assert(UnsolvedTasks() == 0);
    TaskResult total = 0.0;
    for (auto& r : results_) {
        total += r.value();
    }
    return total;
}

void TaskManager::SetResult(const Task& result) {
    if (results_[result.id].has_value()) {
        return;
    }
    results_[result.id] = result.res;
    return;
}

void TaskManager::AddTask(const Task& task) {
    pending_tasks_.emplace(task);
    ++tasks_left_;
}
