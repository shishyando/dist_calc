#pragma once

#include "task.h"
#include "task_manager.h"
#include "worker.h"

#include <netinet/in.h>
#include <unordered_map>
#include <unordered_set>

class WorkerManager {
public:
    WorkerManager(double left, double right, double task_size);

    void DistributeTasks();
    TaskResult CollectResult();

private:
    int GetAcceptorSock();
    void Broadcast();
    int GetAcceptorEpoll(int acceptor_sock);
    void AddFreeWorker(int conn_fd);
    void TryAssignTasks();
    void ExcludeWorker(int conn_fd);

    std::unordered_map<int, std::optional<Task>> workers_;
    std::unordered_set<int> jobless_;
    TaskManager task_manager_;
};
