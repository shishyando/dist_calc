#!/bin/bash

if [ "$ENABLE_PACKET_LOSS" == "true" ]; then
  iptables -A INPUT -m statistic --mode random --probability 0.35 -j DROP
  iptables -A OUTPUT -m statistic --mode random --probability 0.35 -j DROP
fi

if [ "$ENABLE_PACKET_DUPLICATION" == "true" ]; then
  if ! tc qdisc show dev eth0 | grep -q "netem"; then
    tc qdisc add dev eth0 root netem duplicate 20%
  fi
fi

exec "$@"