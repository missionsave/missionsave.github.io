// g++ wifi_monitor.cpp -o wifi_monitor
//sudo iwlist wlp2s0 scan | grep 'ESSID'

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#define NL_BUFSIZE 8192

void process_link_event(const struct nlmsghdr* nlh) {
    struct ifinfomsg* ifi = (struct ifinfomsg*) NLMSG_DATA(nlh);

    // Interface status
    bool is_up = ifi->ifi_flags & IFF_RUNNING;

    std::cout << "Interface index: " << ifi->ifi_index
              << (is_up ? " [CONNECTED]" : " [DISCONNECTED]") << std::endl;
}

int main() {
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Bind to RTMGRP_LINK group (interface state changes)
    struct sockaddr_nl sa = {};
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK;

    if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    char buf[NL_BUFSIZE];

    std::cout << "Listening for Wi-Fi link events (Ctrl+C to exit)..." << std::endl;

    while (true) {
        ssize_t len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv");
            continue;
        }

        for (struct nlmsghdr* nlh = (struct nlmsghdr*) buf;
             NLMSG_OK(nlh, (unsigned int)len);
             nlh = NLMSG_NEXT(nlh, len)) {

            if (nlh->nlmsg_type == NLMSG_DONE)
                break;

            if (nlh->nlmsg_type == NLMSG_ERROR)
                continue;

            if (nlh->nlmsg_type == RTM_NEWLINK || nlh->nlmsg_type == RTM_DELLINK) {
                process_link_event(nlh);
            }
        }
    }

    close(sock);
    return 0;
}
