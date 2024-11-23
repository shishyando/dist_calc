#include "task_manager.h"

#include <cassert>
#include <iostream>
// #define DEBUG

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
    results_.assign(id, std::nullopt);
    tasks_left_ = id;
}

Task TaskManager::GetTask() {
#ifdef DEBUG
    std::cerr << "TaskManager::GetTask" << std::endl;
#endif
    return pending_tasks_.front();
}

void TaskManager::PopTask() {
#ifdef DEBUG
    std::cerr << "TaskManager::PopTask" << std::endl;
#endif
    pending_tasks_.pop();
}

size_t TaskManager::UnassignedTasks() {
#ifdef DEBUG
    std::cerr << "TaskManager::UnassignedTasks " << pending_tasks_.size() << std::endl;
#endif
    return pending_tasks_.size();
}

size_t TaskManager::UnsolvedTasks() {
#ifdef DEBUG
    std::cerr << "TaskManager::UnsolvedTasks " << tasks_left_ << std::endl;
#endif
    return tasks_left_;
}

TaskResult TaskManager::GetResult() {
#ifdef DEBUG
    std::cerr << "TaskManager::GetResult" << std::endl;
#endif
    assert(UnsolvedTasks() == 0 && "Not all tasks are solved");
    TaskResult total = 0.0;
    for (auto& r : results_) {
        total += r.value();
    }
    return total;
}

void TaskManager::SetResult(const Task& result) {
#ifdef DEBUG
    std::cerr << "TaskManager::SetResult" << std::endl;
#endif
    if (results_.at(result.id).has_value()) {
        return;
    }
    results_.at(result.id) = result.res;
    --tasks_left_;
    return;
}

void TaskManager::AddTask(const Task& task) {
#ifdef DEBUG
    std::cerr << "TaskManager::AddTask" << std::endl;
#endif
    pending_tasks_.emplace(task);
}
