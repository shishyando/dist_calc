#include "worker_manager.h"
#include "common.h"

#include <arpa/inet.h>
#include <sys/_types/_socklen_t.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include <optional>

WorkerManager::WorkerManager(double left, double right, double task_size) : task_manager_(TaskManager(left, right, task_size)) {
}

void WorkerManager::Broadcast() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ErrExit("socket");
    }
    SetSockOpt<int>(sock, SO_BROADCAST, 1);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    if (sendto(sock, DISCOVERY_MSG, strlen(DISCOVERY_MSG), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(sock);
        ErrExit("sendto");
    }
    close(sock);
}

int WorkerManager::GetAcceptorSock() {
    int acceptor_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptor_sock < 0) {
        ErrExit("socket");
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(acceptor_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(acceptor_sock);
        ErrExit("bind");
    }
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    SetSockOpt(acceptor_sock, SO_RCVTIMEO, timeout);
    SetNonBlocking(acceptor_sock);
    return acceptor_sock;
}

int WorkerManager::GetAcceptorEpoll(int acceptor_sock) {
    // Create an epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        close(acceptor_sock);
        ErrExit("epoll_create1");
    }

    // Add the listen socket to the epoll instance
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = acceptor_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, acceptor_sock, &event) == -1) {
        close(acceptor_sock);
        close(epoll_fd);
        ErrExit("epoll_ctl: acceptor_sock");
    }
    return epoll_fd;
}

void WorkerManager::DistributeTasks() {
    int acceptor_sock = GetAcceptorSock();
    Broadcast();

    int epoll_fd = GetAcceptorEpoll(acceptor_sock);
    const uint8_t MAX_EPOLL_EVENTS = 25;
    struct epoll_event events[MAX_EPOLL_EVENTS];

    while (task_manager_.UnsolvedTasks() > 0) {
        int n = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, TIMEOUT_SEC * 1000);
        if (n == -1) {
            ErrExit("epoll_wait");
        } else if (n == 0 || (task_manager_.UnassignedTasks() > 0 && workers_.size() < MIN_WORKERS)) {
            // starvation, rebroadcast for new discoveries
            Broadcast();
            continue;
        }

        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == acceptor_sock) {
                // someone wants to be discovered
                while (true) {
                    struct sockaddr_in addr;
                    int conn_fd = accept(acceptor_sock, reinterpret_cast<sockaddr*>(&addr), nullptr);
                    if (conn_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }

                    epoll_event event;
                    event.events = EPOLLIN | EPOLLOUT;
                    event.data.fd = conn_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) == -1) {
                        perror("epoll_ctl: conn_fd");
                        close(conn_fd);
                        continue;
                    }

                    AddFreeWorker(conn_fd);
                }
            } else {
                int worker_sock = events[i].data.fd;

                if (events[i].events & EPOLLHUP || events[i].events & EPOLLRDHUP || events[i].events & EPOLLERR) {
                    // handle drop
                    ExcludeWorker(worker_sock);
                } else if (events[i].events & EPOLLIN) {
                    // read
                    Task result;
                    if (ReceiveTask(worker_sock, result)) {
                        // ok, give him the next task
                        task_manager_.SetResult(result);
                        AddFreeWorker(worker_sock);
                    } else {
                        // unreadable data
                        ExcludeWorker(worker_sock);
                    }
                }
            }
        }
    }
    for (const auto& [sock, _] : workers_) {
        close(sock);
    }
    workers_.clear();

    close(acceptor_sock);
    close(epoll_fd);
}

void WorkerManager::AddFreeWorker(int conn_fd) {
    workers_[conn_fd] = std::nullopt;
    jobless_.emplace(conn_fd);
    TryAssignTasks();
}

void WorkerManager::ExcludeWorker(int conn_fd) {
    close(conn_fd);
    jobless_.erase(conn_fd);
    auto nh = workers_.extract(conn_fd);
    if (!nh.empty() && nh.mapped().has_value()) {
        task_manager_.AddTask(nh.mapped().value());
    }
    TryAssignTasks();
}

void WorkerManager::TryAssignTasks() {
    while (task_manager_.UnassignedTasks() > 0 && !jobless_.empty()) {
        Task task = task_manager_.GetTask();
        int sock = *jobless_.begin();
        if (SendTask(sock, task)) {
            workers_[sock] = task;
            jobless_.erase(sock);
            task_manager_.PopTask();
        }
    }
}

TaskResult WorkerManager::CollectResult() {
    return task_manager_.GetResult();
}
