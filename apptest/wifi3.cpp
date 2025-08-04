// g++ wifi3.cpp ../common/general.cpp -I../common -o wifi3 -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi $(pkg-config --cflags --libs  dbus-1)

// sudo setcap cap_net_admin,cap_net_raw+ep ./wifi3


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <dbus/dbus.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include "general.hpp"

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>       // IFF_RUNNING
#include <linux/if_link.h> // ifinfomsg (no conflito com net/if.h)

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000
#endif

#include <unistd.h>
#include <iostream>
#include <thread>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

void start_dhclient_with_dns(const std::string& dns) {
    const std::string conf_path = "/tmp/dhclient_custom.conf";

    // 1. Criar ficheiro de config temporário
    std::ofstream conf(conf_path);
    if (!conf) {
        std::cerr << "Erro ao criar config temporário\n";
        return;
    }
    conf << "supersede domain-name-servers " << dns << ";\n";
    conf.close();

    // 2. Criar filho
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Código do filho — executa o comando
        const char* argv[] = {
            "sudo",                    // precisa do path absoluto se "sudo" não estiver em /usr/bin
            "dhclient",
            "-1",
            "-nw",
            "-cf",
            conf_path.c_str(),
            nullptr
        };

        execvp("sudo", const_cast<char* const*>(argv));
        perror("execvp falhou");
        _exit(127);  // se exec falhar
    } else {
        // Código do pai — espera o filho terminar
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "dhclient saiu com código " << WEXITSTATUS(status) << "\n";
        } else {
            std::cerr << "dhclient terminou de forma inesperada\n";
        }
    }
}


void listen_netlink() {
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0) {
        perror("socket");
        return;
    }

    sockaddr_nl addr{};
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return;
    }

    std::cout << "Listening for link changes...\n";

    char buf[4096];
    while (true) {
        int len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) continue;

        nlmsghdr *nh;
        for (nh = (nlmsghdr*)buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
            // if (nh->nlmsg_type == RTM_NEWLINK){
			// 	cotm("RTM_NEWLINK")

			// 	//implement only if dhclient not in execution
			// 	start_dhclient_with_dns("8.8.8.8");
			// }
			if (nh->nlmsg_type == RTM_NEWLINK) {
				struct ifinfomsg *ifi = (struct ifinfomsg*)NLMSG_DATA(nh);

				bool link_up = ifi->ifi_flags & IFF_RUNNING;
				bool physical_up = ifi->ifi_flags & IFF_LOWER_UP;

				cout<<"RTM_NEWLINK: ifindex = " << ifi->ifi_index
					<< " running=" << link_up
					<< " lower_up=" << physical_up<<endl;

				if (!link_up || !physical_up) {
					cotm("Link down → maybe disconnected");
					// Aqui podes matar dhclient se quiser, ou reagir
				} else {
					cotm("Link up → talvez reconectado");
					// Aqui podes iniciar dhclient, se não estiver já ativo
				}
			}
			if(nh->nlmsg_type == RTM_DELLINK) { 
				cotm("RTM_DELLINK")
            }
        }
    }

    close(sock);
}







// Globals
static DBusConnection* conn = nullptr;
static std::string iface_path;
static std::vector<std::string> bss_paths;
static std::vector<std::string> ssids;
static Fl_Browser* browser;


// Checa erro D-Bus e encerra se necessário
void check_error(DBusError &err) {
    if (dbus_error_is_set(&err)) {
        std::cerr << "D-Bus Error: " << err.name << " - " << err.message << std::endl;
        dbus_error_free(&err);
        exit(1);
    }
}



// // Mata possíveis gerenciadores de Wi-Fi conflitantes
// void kill_managers() {
//     const char* services[] = {
//         // "systemctl stop wpa_supplicant",
//         "killall wpa_supplicant",
// 		// "systemctl stop NetworkManager", 
//         // "systemctl stop iwd",
//         "killall wpa_supplicant",
//         "killall iwd",
//         "killall NetworkManager",
// 		"killall dhclient"
//     };

//     for (const auto& cmd : services) {
//         std::system(cmd);
//     }

//     std::cout << "[INFO] WiFi services disabled (temporarily).\n";
//     system("pkill -9 NetworkManager wpa_supplicant connman wicd 2>/dev/null");
//     sleep(1);
// }
void kill_managers() {
	return;
    const char* commands[] = {
        "killall -q NetworkManager",
        "killall -q wpa_supplicant",
        "killall -q iwd",
        "killall -q connman",
        "killall -q wicd"
    };

    for (const auto& cmd : commands) {
        std::system(cmd);
    }

    std::cout << "[INFO] Gerenciadores de WiFi terminados (se estavam ativos).\n";
    sleep(1);
}


// Inicia wpa_supplicant em modo D-Bus
// void start_wpa_supplicant() {
//     system("wpa_supplicant -u -s -O DIR=/run/wpa_supplicant GROUP=netdev &");
//     sleep(1);
// }
void start_wpa_supplicant() {
    // Usa o grupo netdev para evitar precisar de root
    std::system("wpa_supplicant -B -u -O /run/wpa_supplicant/ -G netdev");
    sleep(2);
}

// Conecta ao barramento system D-Bus
void connect_dbus() {
    DBusError err;
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    check_error(err);
}

// Cria interface wlp2s0 via D-Bus
void create_interface(const char* ifname) {
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        "/fi/w1/wpa_supplicant1",
        "fi.w1.wpa_supplicant1",
        "CreateInterface");
    if (!msg) {
        std::cerr << "Failed to create D-Bus message\n";
        exit(1);
    }

    DBusMessageIter args, dict, entry, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dict);

    // Ifname
    const char* key_if = "Ifname";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_if);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &ifname);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    // Driver
    const char* key_drv = "Driver";
    const char* driver = "nl80211";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_drv);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &driver);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    dbus_message_iter_close_container(&args, &dict);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);

    const char* path = nullptr;
    DBusMessageIter riter;
    if (dbus_message_iter_init(reply, &riter) &&
        dbus_message_iter_get_arg_type(&riter) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&riter, &path);
    }
    dbus_message_unref(reply);

    if (!path) {
        std::cerr << "CreateInterface returned null path\n";
        exit(1);
    }
    iface_path = path;
    std::cout << "Interface created at: " << iface_path << std::endl;
}

// Dispara Scan via D-Bus
void trigger_scan() {
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "fi.w1.wpa_supplicant1.Interface",
        "Scan");

    if (!msg) {
        std::cerr << "Cannot create Scan message\n";
        exit(1);
    }

    DBusMessageIter args, dict, entry, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dict);

    // Type=active
    const char* key = "Type";
    const char* val = "active";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &val);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    dbus_message_iter_close_container(&args, &dict);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);
    if (reply) dbus_message_unref(reply);

    std::cout << "[INFO] Scan started...\n";
    sleep(2);
}

// Lê propriedade array of object paths "BSSs"
std::vector<std::string> get_bss_list() {
    std::vector<std::string> list;
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "org.freedesktop.DBus.Properties",
        "Get");
    if (!msg) exit(1);

    const char* iface = "fi.w1.wpa_supplicant1.Interface";
    const char* prop = "BSSs";
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);

    DBusMessageIter rit, var, arr;
    dbus_message_iter_init(reply, &rit);
    dbus_message_iter_recurse(&rit, &var);
    dbus_message_iter_recurse(&var, &arr);

    while (dbus_message_iter_get_arg_type(&arr) == DBUS_TYPE_OBJECT_PATH) {
        const char* path;
        dbus_message_iter_get_basic(&arr, &path);
        list.emplace_back(path);
        dbus_message_iter_next(&arr);
    }
    dbus_message_unref(reply);
    return list;
}

// Lê propriedade "SSID" de um BSS
std::string get_ssid(const std::string& bss_path) {
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        bss_path.c_str(),
        "org.freedesktop.DBus.Properties",
        "Get");
    const char* iface = "fi.w1.wpa_supplicant1.BSS";
    const char* prop = "SSID";
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);

    DBusMessageIter rit, var, arr;
    dbus_message_iter_init(reply, &rit);
    dbus_message_iter_recurse(&rit, &var);
    dbus_message_iter_recurse(&var, &arr);

    std::string ssid;
    while (dbus_message_iter_get_arg_type(&arr) == DBUS_TYPE_BYTE) {
        uint8_t b;
        dbus_message_iter_get_basic(&arr, &b);
        ssid.push_back(static_cast<char>(b));
        dbus_message_iter_next(&arr);
    }
    dbus_message_unref(reply);
    return ssid;
}

// Callback para conectar à rede
void connect_to_network(const std::string& ssid, const std::string& password) {
    DBusError err;
    dbus_error_init(&err);

    // 1. Adiciona nova rede com propriedades
    DBusMessage* add_msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "fi.w1.wpa_supplicant1.Interface",
        "AddNetwork");

    DBusMessageIter args, dict, entry, variant, array_iter;
    dbus_message_iter_init_append(add_msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dict);

    // Adiciona "ssid"
    const char* key_ssid = "ssid";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_ssid);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "ay", &variant);
    dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "y", &array_iter);
    for (char c : ssid) {
        uint8_t byte = c;
        dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_BYTE, &byte);
    }
    dbus_message_iter_close_container(&variant, &array_iter);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    // Adiciona "psk"
    const char* key_psk = "psk";
    const char* psk_value = password.c_str();
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_psk);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &psk_value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    dbus_message_iter_close_container(&args, &dict);

    DBusMessage* add_reply = dbus_connection_send_with_reply_and_block(conn, add_msg, -1, &err);
    dbus_message_unref(add_msg);
    check_error(err);

    const char* net_path = nullptr;
    if (dbus_message_iter_init(add_reply, &args) &&
        dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&args, &net_path);
    }
    std::string network_path = net_path ? net_path : "";
    dbus_message_unref(add_reply);

    if (network_path.empty()) {
        std::cerr << "Failed to create network path\n";
        return;
    }

    // 2. Seleciona a rede
    DBusMessage* sel_msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "fi.w1.wpa_supplicant1.Interface",
        "SelectNetwork");

    dbus_message_iter_init_append(sel_msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &network_path);

    DBusMessage* sel_reply = dbus_connection_send_with_reply_and_block(conn, sel_msg, -1, &err);
    dbus_message_unref(sel_msg);
    check_error(err);
    if (sel_reply) dbus_message_unref(sel_reply);

    std::cout << "Connecting to network: " << ssid << std::endl;
}

// Callback—ao clicar rede, pede senha e conecta
void browser_cb(Fl_Widget* w, void*) {
    int idx = browser->value();  // 1-based
    if (idx < 1 || idx > (int)ssids.size()) return;

    // Janela de senha
    Fl_Window* pwd_win = new Fl_Window(300,100,"Password");
    Fl_Input* input = new Fl_Input(10,10,280,25,"Password:");
    Fl_Button* btn = new Fl_Button(110,50,80,30,"Connect");
    pwd_win->end();
    pwd_win->show();

    btn->callback([](Fl_Widget* w, void* v) {
        Fl_Input* in = (Fl_Input*)v;
        std::string password = in->value();
        int idx = browser->value() - 1;
        std::string ssid = ssids[idx];

        // Fecha a janela de senha
        Fl_Window* win = (Fl_Window*)in->parent();
        win->hide();
        delete win;

        // Conecta à rede
        connect_to_network(ssid, password);

        // Mostra mensagem de confirmação
        Fl_Window* ok = new Fl_Window(200,80,"Connecting");
        Fl_Box* fb = new Fl_Box(20,20,160,20,"Connecting to network...");
        Fl_Button* bn = new Fl_Button(60,50,80,25,"OK");
        bn->callback([](Fl_Widget* w, void*) {

// sudo dhclient -1 -nw -cf <(echo 'supersede domain-name-servers 8.8.8.8;')
            w->parent()->hide();
            delete w->parent();
        });
        ok->end();
        ok->show();
    }, input);
}

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
std::string find_wifi_interface() {
    // Primeiro tenta com iw
    std::string iw_result = exec_command("iw dev | awk '/Interface/ {print $2}'");
    if (!iw_result.empty() && iw_result.find("error") == std::string::npos) {
        std::istringstream iss(iw_result);
        std::string iface;
        while (std::getline(iss, iface)) {
            if (!iface.empty() && iface.back() == '\n')
                iface.pop_back();
            if (!iface.empty()) return iface;
        }
    }
	return "wlan0";
}
int main() {
    // 1. Para outros gerenciadores e inicia o wpa_supplicant
    kill_managers();
    start_wpa_supplicant();

    // 2. Conecta D-Bus e cria interface
    connect_dbus(); 
    create_interface(find_wifi_interface().c_str());
    // create_interface("wlp2s0");

    // 3. Escaneia e obtém BSSs
    trigger_scan();
    sleep(3);
    bss_paths = get_bss_list();

    std::cout << "Found " << bss_paths.size() << " networks\n";
    
    // 4. Extrai SSIDs
    ssids.clear();
    for (auto &p : bss_paths) {
        ssids.push_back(get_ssid(p));
    }

    // 5. Monta UI FLTK
    Fl_Window* win = new Fl_Window(300, 400, "Wi-Fi Networks");
    browser = new Fl_Browser(10, 10, 280, 380);
    for (auto &s : ssids) browser->add(s.c_str());
    browser->type(FL_HOLD_BROWSER);
    browser->callback(browser_cb);
    win->end();
    win->show();

	std::thread listener(listen_netlink);
    listener.detach();

    return Fl::run();
}