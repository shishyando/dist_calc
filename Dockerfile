FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    iputils-ping \
    iproute2 \
    net-tools \
    iptables

WORKDIR /app
COPY . .

COPY set_iptables.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/set_iptables.sh

RUN cmake . && make

ENTRYPOINT ["/usr/local/bin/set_iptables.sh"]
