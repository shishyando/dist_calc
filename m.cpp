#include "worker_manager.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <left> <right> <task_size>" << std::endl;
        return 1;
    }
    const double left = std::stod(argv[1]);
    const double right = std::stod(argv[2]);
    const double task_size = std::stod(argv[3]);
    std::cerr << "Running master: "
              << "left = " << left << ", right = " << right << ", task_size = " << task_size << std::endl;

    WorkerManager worker_manager(left, right, task_size);
    worker_manager.DistributeTasks();
    std::cout << "Result: " << worker_manager.CollectResult() << std::endl;
    return 0;
}
