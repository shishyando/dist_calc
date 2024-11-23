#include "common.h"
#include "task.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
sockaddr_in CatchBroadcast() {
    char buffer[BUFFER_SIZE];
    int udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_sock < 0) {
        ErrExit("UDP socket creation failed");
    }
    sockaddr_in udp_addr;
    std::memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(DISCOVERY_PORT);
    if (bind(udp_sock, (sockaddr*)&udp_addr, sizeof(udp_addr)) < 0) {
        ErrExit("bind");
    }
    std::cout << "Waiting for UDP broadcast..." << std::endl;
    sockaddr_in master_addr;
    socklen_t senderLen = sizeof(master_addr);
    if (recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&master_addr, &senderLen) < 0) {
        ErrExit("recvfrom");
    }
    close(udp_sock);
    std::cout << "Broadcast received from " << inet_ntoa(master_addr.sin_addr) << std::endl;
    return master_addr;
}

int ConnectToMaster(sockaddr_in master_addr) {
    int master_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (master_sock < 0) {
        ErrExit("socket");
    }
    master_addr.sin_port = htons(WORK_PORT);
    if (connect(master_sock, (sockaddr*)&master_addr, sizeof(master_addr)) < 0) {
        ErrExit("connect");
    }
    return master_sock;
}

}  // namespace

int main() {
    sockaddr_in master_addr = CatchBroadcast();
    int master_sock = ConnectToMaster(master_addr);

    Task task;
    if (!ReceiveTask(master_sock, task)) {
        ErrExit("ReceiveTask");
    }
    task.Evaluate();
    if (!SendTask(master_sock, task)) {
        ErrExit("SendTask");
    }

    close(master_sock);
    return 0;
}
