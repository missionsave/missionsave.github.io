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

#include <iostream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/sha.h>

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
bool wait_scan_done(struct nl_sock* sock, int if_index) {
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

    // Conecta à rede especificada usando SSID e PSK em hexadecimal
    bool connect(const std::string& ifname, const std::string& ssid, const std::string& psk_hex) {
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





// Função principal com fluxo otimizado
int main() {
    std::cout << "🚀 Iniciando scanner Wi-Fi...\n";
    
    // Descobre interface
    std::string iface = find_wifi_interface();
    if (iface.empty()) {
        std::cerr << "❌ Nenhuma interface Wi-Fi encontrada\n";
        return 1;
    }
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
        return 1;
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
    
	startMonitor();

    return 0;
}