#pragma once

#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include "task.h"

inline const char* DISCOVERY_MSG = "_DISCOVERY_";
inline const uint16_t DISCOVERY_PORT = 1337;
inline const uint16_t WORK_PORT = 1338;
inline const size_t BUFFER_SIZE = 256;
inline const uint8_t TIMEOUT_SEC = 3;
inline const uint8_t MIN_WORKERS = 4;

template <typename T>
inline void SetSockOpt(int fd, int opt, const T& data) {
    if (setsockopt(fd, SOL_SOCKET, opt, &data, sizeof(data)) < 0) {
        perror("setsockopt");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

inline void SetNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

inline void ErrExit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

inline bool ReceiveTask(int conn_fd, Task& task) {
    size_t totalBytesRead = 0;
    size_t structSize = sizeof(Task);
    char* buffer = reinterpret_cast<char*>(&task);

    while (totalBytesRead < structSize) {
        ssize_t bytesRead = recv(conn_fd, buffer + totalBytesRead, structSize - totalBytesRead, 0);
        if (bytesRead <= 0) {
            perror("recv");
            return false;
        }
        totalBytesRead += bytesRead;
    }

    return true;
}

inline bool SendTask(int conn_fd, const Task& task) {
    return send(conn_fd, &task, sizeof(task), MSG_DONTWAIT) == sizeof(task);
}
