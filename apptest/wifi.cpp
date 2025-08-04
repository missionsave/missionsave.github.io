//  g++ wifi.cpp -lnl-3 -lnl-genl-3 -o wifi -I/usr/include/libnl3 -lcrypto
//  g++ wifi.cpp -l:libnl-3.a -l:libnl-genl-3.a -o wifi -I/usr/include/libnl3 -l:libcrypto.a


#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>

#include <net/if.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <iomanip>
#include <map>
#include <chrono>

struct WiFiNetwork {
    std::string ssid;
    int signal_dbm;
    int frequency;
};
std::string iface ="wlan0";

#include <iostream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void disableWiFi() {
    // Attempt to stop services commonly used for WiFi
    const char* services[] = {
        "systemctl stop NetworkManager",
        "systemctl stop wpa_supplicant",
        "systemctl stop iwd",
        "killall wpa_supplicant",
        "killall iwd",
        "killall NetworkManager"
    };

    for (const auto& cmd : services) {
        std::system(cmd);
    }

    std::cout << "[INFO] WiFi services disabled (temporarily).\n";
}

void setDNS() {
    int ret = std::system("resolvectl dns wlan0 8.8.8.8");  // Try with resolvectl first
    if (ret != 0) {
        // Fallback: overwrite /etc/resolv.conf (requires root)
        std::ofstream resolv("/etc/resolv.conf");
        if (resolv.is_open()) {
            resolv << "nameserver 8.8.8.8\n";
            resolv.close();
            std::cout << "[INFO] DNS set to 8.8.8.8 via /etc/resolv.conf\n";
        } else {
            std::cerr << "[ERROR] Failed to write /etc/resolv.conf. Try running as root.\n";
        }
    } else {
        std::cout << "[INFO] DNS set to 8.8.8.8 using resolvectl.\n";
    }
}

std::string getLocalIP() {
    struct ifaddrs *ifaddr, *ifa;
    std::string localIP = "0.0.0.0";

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return localIP;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        std::string iface(ifa->ifa_name);
        if (iface != "lo") { // skip loopback
            char ip[INET_ADDRSTRLEN];
            void* addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);
            localIP = ip;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return localIP;
}


std::string generate_psk(const std::string& ssid, const std::string& passphrase) {
    const int iterations = 4096;
    const int key_len = 32;
    unsigned char psk[key_len];

    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.c_str(),
        passphrase.length(),
        reinterpret_cast<const unsigned char*>(ssid.c_str()),
        ssid.length(),
        iterations,
        key_len,
        psk
    );

    std::ostringstream oss;
    for (int i = 0; i < key_len; ++i)
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)psk[i];

    return oss.str();
}

#if 1
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/route/link.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/route/link.h>

#include <linux/rfkill.h>
#include <linux/if_link.h>   // IFLA_OPERSTATE, IFLA_CARRIER
#include <linux/if.h>        // IF_OPER_UP, IF_OPER_UNKNOWN

#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

static int onLinkChange(struct nl_msg *msg, void *arg) {
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    // cast explícito em C++
    struct ifinfomsg *ifi = (struct ifinfomsg*) nlmsg_data(nlh);

    struct nlattr *attrs[IFLA_MAX + 1];
    nla_parse(attrs, IFLA_MAX,
              nlmsg_attrdata(nlh, sizeof(*ifi)),
              nlmsg_attrlen(nlh, sizeof(*ifi)),
              NULL);

    if (!attrs[IFLA_IFNAME])
        return NL_OK;

    // cast explícito
    const char *ifname = (const char*) nla_data(attrs[IFLA_IFNAME]);
    if (strcmp(ifname, "wlan0") != 0)
        return NL_OK;

    uint8_t oper = attrs[IFLA_OPERSTATE]
        ? nla_get_u8(attrs[IFLA_OPERSTATE])
        : IF_OPER_UNKNOWN;

    uint8_t carrier = attrs[IFLA_CARRIER]
        ? nla_get_u8(attrs[IFLA_CARRIER])
        : 1;

    if (oper == IF_OPER_UP && carrier) {
        printf("[NETLINK] %s UP + sinal\n", ifname);
    } else {
        printf("[NETLINK] %s DOWN ou sem sinal\n", ifname);
    }
    return NL_OK;
}

void startMonitor() {
    // 1) Netlink
    struct nl_sock *sock = nl_socket_alloc();
    nl_connect(sock, NETLINK_ROUTE);
    nl_socket_modify_cb(sock, NL_CB_MSG_IN, NL_CB_CUSTOM, onLinkChange, NULL);
    nl_socket_add_memberships(sock, RTNLGRP_LINK, 0);

    // 2) rfkill
    int rfk_fd = open("/dev/rfkill", O_RDONLY | O_CLOEXEC);
    if (rfk_fd < 0) {
        perror("open /dev/rfkill");
        nl_socket_free(sock);
        return;
    }

    // 3) poll()
    int nl_fd = nl_socket_get_fd(sock);
    struct pollfd fds[2] = {
        { .fd = nl_fd,  .events = POLLIN },
        { .fd = rfk_fd, .events = POLLIN }
    };

    while (true) {
        if (poll(fds, 2, -1) < 0) {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            nl_recvmsgs_default(sock);
        }

        if (fds[1].revents & POLLIN) {
            struct rfkill_event ev;
            if (read(rfk_fd, &ev, sizeof(ev)) == sizeof(ev)
                && ev.type == RFKILL_TYPE_WLAN
                && ev.op == RFKILL_OP_CHANGE) 
            {
                if (ev.soft || ev.hard) {
                    printf("[RFKILL] wlan%d BLOQUEADA (soft=%u hard=%u)\n",
                           ev.idx, ev.soft, ev.hard);
                } else {
                    printf("[RFKILL] wlan%d DESBLOQUEADA\n", ev.idx);
                }
            }
        }
    }

    close(rfk_fd);
    nl_socket_free(sock);
}

#endif

std::vector<WiFiNetwork> scan_results;

// Função para executar comandos shell e capturar output
std::string exec_command(const char* cmd) {
    char buffer[256];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "error";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Encontra interface Wi-Fi de maneira mais robusta
std::string find_wifi_interface() {
    // Primeiro tenta com iw
    // std::string iw_result = exec_command("iw dev | awk '/Interface/ {print $2}'");
    // if (!iw_result.empty() && iw_result.find("error") == std::string::npos) {
    //     std::istringstream iss(iw_result);
    //     std::string iface;
    //     while (std::getline(iss, iface)) {
    //         if (!iface.empty() && iface.back() == '\n')
    //             iface.pop_back();
    //         if (!iface.empty()) return iface;
    //     }
    // }
    
    // Fallback para o método original
    struct nl_sock* sock = nl_socket_alloc();
    if (!sock || genl_connect(sock)) {
        if (sock) nl_socket_free(sock);
        return "";
    }
    
    int nl80211_id = genl_ctrl_resolve(sock, "nl80211");
    if (nl80211_id < 0) {
        nl_socket_free(sock);
        return "";
    }

    struct nl_msg* msg = nlmsg_alloc();
    if (!msg) {
        nl_socket_free(sock);
        return "";
    }
    
    genlmsg_put(msg, 0, 0, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);
    
    std::string result;
    auto cb = [](struct nl_msg* msg, void* arg) -> int {
        std::string* out = static_cast<std::string*>(arg);
        struct nlmsghdr* nlh = nlmsg_hdr(msg);
        struct genlmsghdr* gnlh = static_cast<struct genlmsghdr*>(nlmsg_data(nlh));
        struct nlattr* tb[NL80211_ATTR_MAX + 1] = {};
        
        nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), nullptr);
        
        if (tb[NL80211_ATTR_IFNAME] && tb[NL80211_ATTR_IFTYPE]) {
            int iftype = nla_get_u32(tb[NL80211_ATTR_IFTYPE]);
            if (iftype == NL80211_IFTYPE_STATION) {
                *out = reinterpret_cast<char*>(nla_data(tb[NL80211_ATTR_IFNAME]));
                return NL_STOP;
            }
        }
        return NL_OK;
    };
    
    nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, cb, &result);
    nl_send_auto(sock, msg);
    nl_recvmsgs_default(sock);
    
    nlmsg_free(msg);
    nl_socket_free(sock);
    return result;
}

// Obtém SSID atual com tratamento de erros
std::string get_current_ssid(const std::string& iface) {
    if (iface.empty()) return "";
    
    std::string cmd = "iwgetid " + iface + " -r 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    
    char buffer[128];
    if (!fgets(buffer, sizeof(buffer), pipe)) {
        pclose(pipe);
        return "";
    }
    pclose(pipe);
    
    std::string ssid(buffer);
    if (!ssid.empty() && ssid.back() == '\n')
        ssid.pop_back();
    return ssid;
}

// Dispara o scan com tratamento robusto de erros
bool trigger_scan(struct nl_sock* sock, int driver_id, int if_index) {
    struct nl_msg* msg = nlmsg_alloc();
    if (!msg) {
        std::cerr << "❌ Failed to allocate trigger scan message" << std::endl;
        return false;
    }
    
    genlmsg_put(msg, 0, 0, driver_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);
    
    // Configuração otimizada de scan
    nla_put(msg, NL80211_ATTR_SCAN_SSIDS, 0, "");  // SSID wildcard
    
    // Adiciona frequências específicas para scan mais rápido
    struct nlattr* freqs = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);
    if (freqs) {
        // Frequências comuns 2.4GHz
        for (int freq = 2412; freq <= 2472; freq += 5) {
            nla_put_u32(msg, NL80211_ATTR_SCAN_FREQUENCIES, freq);
        }
        // Frequências 5GHz
        const int five_ghz[] = {5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320, 5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640, 5660, 5680, 5700, 5720, 5745, 5765, 5785, 5805, 5825};
        for (int freq : five_ghz) {
            nla_put_u32(msg, NL80211_ATTR_SCAN_FREQUENCIES, freq);
        }
        nla_nest_end(msg, freqs);
    }
    
    int ret = nl_send_auto(sock, msg);
    nlmsg_free(msg);
    
    if (ret < 0) {
        std::cerr << "❌ Trigger scan failed: " << nl_geterror(ret) << std::endl;
        return false;
    }
    return true;
}

// Implementação alternativa de timeout para versões mais antigas do libnl
// Implementação alternativa de timeout para versões muito antigas do libnl
static void socket_timeout(struct nl_sock* sock, int timeout_ms) {
    // Versões muito antigas do libnl não têm funções de timeout diretas
    // Usaremos um workaround definindo o buffer de recepção
    nl_socket_set_buffer_size(sock, 32768, 32768);
    
    // Workaround: definir porta local para evitar conflitos
    nl_socket_set_local_port(sock, 0);
    
    // Em versões muito antigas, não há como definir timeout diretamente
    // O timeout será controlado na lógica de recepção
}

// Aguarda conclusão do scan com timeout preciso
// Aguarda conclusão do scan com timeout preciso
bool wait_scan_done_v1(struct nl_sock* sock, int if_index) {
    using namespace std::chrono;
    auto start = steady_clock::now();
    const int timeout_ms = 4000; // 4 segundos
    
    while (duration_cast<milliseconds>(steady_clock::now() - start).count() < timeout_ms) {
        // Verifica status do scan usando comando iw para maior confiabilidade
        std::string cmd = "iw dev " + std::to_string(if_index) + " scan status 2>&1 | grep -q 'Scan complete'";
        if (system(cmd.c_str()) == 0) {
            return true;
        }
        
        // Verifica se há mensagens disponíveis sem bloquear
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(nl_socket_get_fd(sock), &rfds);
        
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000; // 200ms
        
        int ret = select(nl_socket_get_fd(sock) + 1, &rfds, NULL, NULL, &tv);
        if (ret > 0) {
            // Temos dados para ler
            nl_recvmsgs_default(sock);
        } else if (ret < 0) {
            // Erro no select
            break;
        }
    }
    return false;
}


bool wait_scan_done(struct nl_sock* sock, int if_index) {
    using namespace std::chrono;
    auto start = steady_clock::now();
    const int timeout_ms = 4000; // 4 seconds

    bool scan_done = false;

    // Callback to process incoming scan messages
    auto handler = [](struct nl_msg* msg, void* arg) -> int {
        struct nlmsghdr* hdr = nlmsg_hdr(msg);
        if (hdr->nlmsg_type == NL80211_CMD_NEW_SCAN_RESULTS) {
            bool* done = static_cast<bool*>(arg);
            *done = true;
            return NL_STOP;
        }
        if (hdr->nlmsg_type == NL80211_CMD_SCAN_ABORTED) {
            std::cerr << "[ERROR] Scan aborted.\n";
            return NL_STOP;
        }
        return NL_OK;
    };

    nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, handler, &scan_done);

    while (duration_cast<milliseconds>(steady_clock::now() - start).count() < timeout_ms) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(nl_socket_get_fd(sock), &rfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000; // 200ms

        int ret = select(nl_socket_get_fd(sock) + 1, &rfds, nullptr, nullptr, &tv);
        if (ret > 0) {
            nl_recvmsgs_default(sock);
            if (scan_done) break;
        } else if (ret < 0) {
            perror("select");
            break;
        }
    }

    return scan_done;
}

// Processa resultados do scan com parsing melhorado
static int scan_handler(struct nl_msg* msg, void* arg) {
    struct nlmsghdr* nlh = nlmsg_hdr(msg);
    struct genlmsghdr* gnlh = static_cast<struct genlmsghdr*>(nlmsg_data(nlh));
    struct nlattr* tb[NL80211_ATTR_MAX + 1] = {};
    struct nlattr* bss[NL80211_BSS_MAX + 1] = {};
    
    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), nullptr);
    
    if (!tb[NL80211_ATTR_BSS]) return NL_SKIP;
    nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], nullptr);
    
    WiFiNetwork net;
    bool valid = false;
    
    // Processa sinal
    if (bss[NL80211_BSS_SIGNAL_MBM]) {
        net.signal_dbm = nla_get_s32(bss[NL80211_BSS_SIGNAL_MBM]) / 100;
        valid = true;
    }
    
    // Processa frequência
    if (bss[NL80211_BSS_FREQUENCY]) {
        net.frequency = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
    }
    
    // Processa SSID
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
        uint8_t* ies = static_cast<uint8_t*>(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
        int len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
        
        for (int i = 0; i < len; ) {
            uint8_t id = ies[i];
            uint8_t elen = ies[i+1];
            
            if (i + 2 + elen > len) break;
            
            if (id == 0 && elen > 0 && elen <= 32) {  // SSID element
                net.ssid.assign(reinterpret_cast<char*>(&ies[i+2]), elen);
                break;
            }
            i += 2 + elen;
        }
    }
    
    // Filtra redes ocultas e inválidas
    if (valid && !net.ssid.empty() && net.ssid != "\x00") {
        scan_results.push_back(net);
    }
    
    return NL_OK;
}

#ifndef NL80211_ATTR_CRYPTO
#define NL80211_ATTR_CRYPTO 55
#endif

#ifndef NL80211_CRYPTO_ATTR_KEY
#define NL80211_CRYPTO_ATTR_KEY 1
#endif

#ifndef NL80211_CRYPTO_ATTR_KEY_TYPE
#define NL80211_CRYPTO_ATTR_KEY_TYPE 5
#endif

#ifndef NL80211_KEYTYPE_PSK
#define NL80211_KEYTYPE_PSK 2
#endif

struct WiFiConnector {
    struct nl_sock* sock;
    int nl80211_id;

    WiFiConnector() {
        sock = nl_socket_alloc();
        genl_connect(sock);
        nl80211_id = genl_ctrl_resolve(sock, "nl80211");
    }

    ~WiFiConnector() {
        if (sock) nl_socket_free(sock);
    }

	bool connect(const std::string& ifname, const std::string& ssid, const std::string& psk_hex) {
    int ifindex = if_nametoindex(ifname.c_str());
    if (ifindex == 0) return false;

    struct nl_msg* msg = nlmsg_alloc();
    if (!msg) return false;

    genlmsg_put(msg, 0, 0, nl80211_id, 0, 0, NL80211_CMD_CONNECT, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);
    nla_put(msg, NL80211_ATTR_SSID, ssid.size(), ssid.c_str());

    // Converter PSK hexadecimal para bytes
    unsigned char psk[32];
    for (int i = 0; i < 32; ++i) {
        unsigned int byte;
        sscanf(psk_hex.c_str() + 2 * i, "%2x", &byte);
        psk[i] = static_cast<unsigned char>(byte);
    }

    nla_put(msg, NL80211_ATTR_PMK, 32, psk);

    // Opcional: também no bloco CRYPTO
    struct nlattr* crypto = nla_nest_start(msg, NL80211_ATTR_CRYPTO);
    nla_put(msg, NL80211_CRYPTO_ATTR_KEY, 32, psk);
    nla_nest_end(msg, crypto);

    // Callback para receber resposta do kernel
    bool success = false;
    auto ack_handler = [](struct nl_msg* msg, void* arg) -> int {
        bool* ok = static_cast<bool*>(arg);
        *ok = true;
        return NL_STOP;
    };
    nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, ack_handler, &success);

    int err = nl_send_auto_complete(sock, msg);
    nlmsg_free(msg);

    // Esperar resposta
    if (err >= 0)
        nl_recvmsgs_default(sock);

    return success;
}

    // Conecta à rede especificada usando SSID e PSK em hexadecimal
    bool connect_v1(const std::string& ifname, const std::string& ssid, const std::string& psk_hex) {
        int ifindex = if_nametoindex(ifname.c_str());
        if (ifindex == 0) return false;

        struct nl_msg* msg = nlmsg_alloc();
        genlmsg_put(msg, 0, 0, nl80211_id, 0, 0, NL80211_CMD_CONNECT, 0);
        nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);
        nla_put(msg, NL80211_ATTR_SSID, ssid.size(), ssid.c_str());

        // Nested attribute para a chave de criptografia
        struct nlattr* crypto = nla_nest_start(msg, NL80211_ATTR_CRYPTO);
        unsigned char psk[32];
        for (int i = 0; i < 32; ++i) {
            unsigned int byte;
            sscanf(psk_hex.c_str() + 2 * i, "%2x", &byte);
            psk[i] = static_cast<unsigned char>(byte);
        }
        nla_put(msg, NL80211_CRYPTO_ATTR_KEY, 32, psk);
        nla_put_u32(msg, NL80211_CRYPTO_ATTR_KEY_TYPE, 2); // 2 = PSK
        nla_nest_end(msg, crypto);

        int err = nl_send_auto_complete(sock, msg);
        nlmsg_free(msg);
        return (err >= 0);
    }
};

// Exemplo de uso da struct no seu código existente:
//
// WiFiConnector connector;
// std::string ssid = "MinhaRedeWiFi";
// std::string pass = "minhasenha123";
// std::string psk = generate_psk(ssid, pass);
// if (connector.connect("wlan0", ssid, psk)) {
//     std::cout << "Conectando a " << ssid << "...\n";
// } else {
//     std::cerr << "Falha ao iniciar conexão\n";
// }




#include <cstdlib>
#include <string>
#include <iostream>

bool connectWiFi(const std::string& iface, const std::string& ssid, const std::string& psk) {
    std::string cmd = "sudo iw dev " + iface + " connect \"" + ssid + "\" key 0:" + psk;
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Falha ao conectar-se à rede WiFi." << std::endl;
        return false;
    }
    return true;
}

#include <cstdlib>
#include <fstream>
#include <thread>
#include <chrono>

bool connectWithWpaSupplicant(const std::string& iface, const std::string& ssid, const std::string& psk) {
    std::string confFile = "/tmp/wpa_supplicant.conf";

    // Cria o arquivo temporário
    std::ofstream conf(confFile);
    if (!conf) return false;
    conf << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n";
    conf << "network={\n";
    conf << "    ssid=\"" << ssid << "\"\n";
    conf << "    psk=" << psk << "\n";
    conf << "}\n";
    conf.close();

    // Inicia wpa_supplicant em background
    // std::string startCmd = "sudo wpa_supplicant -i" + iface + " -c" + confFile;
    std::string startCmd = "sudo wpa_supplicant -B -i" + iface + " -c" + confFile;
    if (std::system(startCmd.c_str()) != 0) return false;

    // Espera 3 segundos pela conexão
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Opcional: obtém IP via DHCP (1 tentativa)
    std::string dhcpCmd = "sudo dhclient -1 " + iface;
    std::system(dhcpCmd.c_str());

    // Encerra wpa_supplicant (se quiser liberar RAM)
    // std::string killCmd = "sudo pkill -f 'wpa_supplicant -i" + iface + "'";
    // std::system(killCmd.c_str());

    return true;
}
// #include <wpa_ctrl.h>
// #include <cstdio>
// #include <cstring>

// bool connectToWiFi(const char* iface, const char* ssid, const char* psk) {
//     struct wpa_ctrl* ctrl;
//     char cmd[256], reply[1024];
//     size_t reply_len;

//     // Conecta ao wpa_supplicant (socket UNIX)
//     ctrl = wpa_ctrl_open(iface);
//     if (!ctrl) {
//         perror("Failed to connect to wpa_supplicant");
//         return false;
//     }

//     // Adiciona uma nova rede
//     snprintf(cmd, sizeof(cmd), "ADD_NETWORK");
//     if (wpa_ctrl_request(ctrl, cmd, strlen(cmd), reply, &reply_len, NULL) < 0) {
//         wpa_ctrl_close(ctrl);
//         return false;
//     }
//     int net_id = atoi(reply);

//     // Configura SSID
//     snprintf(cmd, sizeof(cmd), "SET_NETWORK %d ssid \"%s\"", net_id, ssid);
//     if (wpa_ctrl_request(ctrl, cmd, strlen(cmd), reply, &reply_len, NULL) < 0) {
//         wpa_ctrl_close(ctrl);
//         return false;
//     }

//     // Configura PSK
//     snprintf(cmd, sizeof(cmd), "SET_NETWORK %d psk \"%s\"", net_id, psk);
//     if (wpa_ctrl_request(ctrl, cmd, strlen(cmd), reply, &reply_len, NULL) < 0) {
//         wpa_ctrl_close(ctrl);
//         return false;
//     }

//     // Habilita a rede
//     snprintf(cmd, sizeof(cmd), "ENABLE_NETWORK %d", net_id);
//     if (wpa_ctrl_request(ctrl, cmd, strlen(cmd), reply, &reply_len, NULL) < 0) {
//         wpa_ctrl_close(ctrl);
//         return false;
//     }

//     // Salva a configuração
//     wpa_ctrl_request(ctrl, "SAVE_CONFIG", 11, reply, &reply_len, NULL);

//     wpa_ctrl_close(ctrl);
//     return true;
// }
// Função principal com fluxo otimizado
int main() {

	// disableWiFi();
	// sleep(4);
	// std::system("sudo wpa_supplicant -u -s -O DIR=/run/wpa_supplicant GROUP=netdev &"); 
    std::cout << "🚀 Iniciando scanner Wi-Fi...\n";
    
    // Descobre interface
    iface = find_wifi_interface();
    if (iface.empty()) {
        std::cerr << "❌ Nenhuma interface Wi-Fi encontrada\n";
        return 1;
    }
	// trim(iface);
    std::cout << "🧭 Interface: " << iface << "\n";
    
    // Verifica estado da interface
    // std::cout << "🔍 Verificando estado da interface...\n";
    // std::string iw_status = exec_command(("iw dev " + iface + " link").c_str());
    // std::cout << (iw_status.empty() ? "  - Interface não conectada\n" : iw_status);
    
    // Obtém SSID atual
    std::string connected_ssid = get_current_ssid(iface);
    if (!connected_ssid.empty()) {
        std::cout << "🔗 Conectado a: " << connected_ssid << "\n";
    }
    
    // Prepara socket netlink
    struct nl_sock* sock = nl_socket_alloc();
    if (!sock) {
        std::cerr << "❌ Falha ao alocar socket\n";
        return 1;
    }
    
    // Configura timeout (alternativo para versões antigas do libnl)
    socket_timeout(sock, 2000);  // 2 segundos
    
    if (genl_connect(sock)) {
        std::cerr << "❌ Falha ao conectar ao generic netlink\n";
        nl_socket_free(sock);
        return 1;
    }
    
    int driver_id = genl_ctrl_resolve(sock, "nl80211");
    if (driver_id < 0) {
        std::cerr << "❌ Driver nl80211 não disponível\n";
        nl_socket_free(sock);
        return 1;
    }
    
    int if_index = if_nametoindex(iface.c_str());
    if (!if_index) {
        std::cerr << "❌ Falha ao obter índice da interface\n";
        nl_socket_free(sock);
        return 1;
    }
    
    // Dispara o scan
    std::cout << "🔄 Iniciando scan...\n";
    if (!trigger_scan(sock, driver_id, if_index)) {
        std::cerr << "❌ Falha ao iniciar scan\n";
        nl_socket_free(sock);
        return 1;
    }
    
    // Aguarda conclusão
    std::cout << "⏱️ Aguardando resultados (máx 3 segundos)...\n";
    if (!wait_scan_done(sock, if_index)) {
        std::cerr << "⚠️ Scan não completou dentro do tempo\n";
        // Continua mesmo com timeout - pode haver resultados parciais
    }
    
    // Obtém resultados
    std::cout << "📊 Coletando redes...\n";
    struct nl_msg* msg = nlmsg_alloc();
    genlmsg_put(msg, 0, 0, driver_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);
    
    scan_results.clear();
    nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, scan_handler, nullptr);
    nl_send_auto(sock, msg);
    nl_recvmsgs_default(sock);
    
    // Limpeza
    nlmsg_free(msg);
    nl_socket_free(sock);
    
    // Processa resultados
    if (scan_results.empty()) {
        std::cout << "❌ Nenhuma rede encontrada. Verifique:\n";
        std::cout << "   - Permissões (execute como root)\n";
        std::cout << "   - Estado da interface (ifconfig " << iface << ")\n";
        std::cout << "   - Driver Wi-Fi (lsmod | grep iwl)\n";
        // return 1;
    }
    
    // Remove duplicatas e ordena por sinal
    std::sort(scan_results.begin(), scan_results.end(),
        [](const WiFiNetwork& a, const WiFiNetwork& b) {
            return a.signal_dbm > b.signal_dbm;
        });
    
    auto last = std::unique(scan_results.begin(), scan_results.end(),
        [](const WiFiNetwork& a, const WiFiNetwork& b) {
            return a.ssid == b.ssid;
        });
    scan_results.erase(last, scan_results.end());
    
    // Exibe resultados
    std::cout << "\n📶 Redes encontradas (" << scan_results.size() << "):\n";
    std::cout << std::left << std::setw(25) << "SSID" 
              << std::setw(10) << "Sinal" 
              << "Freq\n";
    std::cout << "----------------------------------------\n";
    
    for (const auto& net : scan_results) {
        std::string freq = (net.frequency > 4000) ? "5 GHz" : "2.4 GHz";
        std::cout << std::left << std::setw(25) 
                  << (net.ssid.empty() ? "[Oculto]" : net.ssid.substr(0, 24))
                  << std::setw(10) << (std::to_string(net.signal_dbm) + " dBm")
                  << freq;
        
        if (net.ssid == connected_ssid) {
            std::cout << "  🔗";
        }
        std::cout << "\n";
    }
    


std::string ssid = "Vodafone-6CDAF1";
std::string pass = "passsw";
std::string psk = generate_psk(ssid, pass);
connectWithWpaSupplicant(iface,ssid,psk);
// connectWiFi(iface,ssid,psk);
startMonitor();
return 0;
sleep(10);


if(0){	WiFiConnector connector;
std::string ssid = "Vodafone-6CDAF1";
std::string pass = "passsw";
std::string psk = generate_psk(ssid, pass);
std::cout<<"psk "<<psk<<std::endl;
if (connector.connect(iface, ssid, psk)) {
    std::cout << "Conectando a " << ssid << "...\n";
} else {
    std::cerr << "Falha ao iniciar conexão\n";
}
	sleep(20);
	// startMonitor();
}

    return 0;
}

// sudo iw dev wlp2s0 connect SSID key 0:PSK
// sudo iw dev wlp2s0 connect Vodafone-6CDAF1 key 0:70e35f11f344aa4adbb7eb050d9686fd374496e0977fa61f9c4736b290c0b780


// sudo dhclient -1 -nw -cf <(echo 'supersede domain-name-servers 8.8.8.8;')

// dhclient -1 wlp2s0


