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

// Globals
static DBusConnection* conn = nullptr;
static std::string iface_path;
static std::vector<std::string> bss_paths;
static std::vector<std::string> ssids;
static Fl_Browser* browser;



// Callback for network up
void on_network_up(void* user_data) {
    std::cout << "[INFO] Network is UP!" << std::endl;
    // Optional: Update FLTK UI
    Fl::lock();
    if (user_data) {
        auto* label = static_cast<Fl_Box*>(user_data);
        label->copy_label("Network Status: Connected");
        label->redraw();
    }
    Fl::unlock();
    Fl::awake();
}

// Callback for network down
void on_network_down(void* user_data) {
    std::cout << "[INFO] Network is DOWN!" << std::endl;
    // Optional: Update FLTK UI
    Fl::lock();
    if (user_data) {
        auto* label = static_cast<Fl_Box*>(user_data);
        label->copy_label("Network Status: Disconnected");
        label->redraw();
    }
    Fl::unlock();
    Fl::awake();
}

// Checa erro D-Bus e encerra se necessário
void check_error(DBusError &err) {
    if (dbus_error_is_set(&err)) {
        std::cerr << "D-Bus Error: " << err.name << " - " << err.message << std::endl;
        dbus_error_free(&err);
        exit(1);
    }
}

// Function to detect network up/down events using D-Bus signals
// Parameters:
// - net_up_cb: Callback function to call when network is up (state: "completed")
// - net_down_cb: Callback function to call when network is down (state: "disconnected")
// - user_data: Optional user data to pass to callbacks
void detect_network_status(void (*net_up_cb)(void*), void (*net_down_cb)(void*), void* user_data) {
    DBusError err;
    dbus_error_init(&err);

    // D-Bus signal handler for PropertiesChanged
    static auto signal_filter = [](DBusConnection* connection, DBusMessage* message, void* user_data) -> DBusHandlerResult {
        auto* callbacks = static_cast<std::pair<void (*)(void*), void (*)(void*)>*>(user_data);

        if (dbus_message_is_signal(message, "org.freedesktop.DBus.Properties", "PropertiesChanged")) {
            DBusMessageIter args, dict, entry, variant;
            const char* interface_name;

            if (!dbus_message_iter_init(message, &args)) return DBUS_HANDLER_RESULT_HANDLED;
            if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) return DBUS_HANDLER_RESULT_HANDLED;
            dbus_message_iter_get_basic(&args, &interface_name);

            if (std::string(interface_name) != "fi.w1.wpa_supplicant1.Interface") return DBUS_HANDLER_RESULT_HANDLED;

            dbus_message_iter_next(&args);
            if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) return DBUS_HANDLER_RESULT_HANDLED;
            dbus_message_iter_recurse(&args, &dict);

            while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
                dbus_message_iter_recurse(&dict, &entry);
                const char* key;
                dbus_message_iter_get_basic(&entry, &key);

                if (std::string(key) == "State") {
                    dbus_message_iter_next(&entry);
                    dbus_message_iter_recurse(&entry, &variant);
                    const char* state;
                    dbus_message_iter_get_basic(&variant, &state);
                    std::cout << "[INFO] Network State: " << state << std::endl;

                    if (std::string(state) == "completed" && callbacks->first) {
                        callbacks->first(user_data);
                    } else if (std::string(state) == "disconnected" && callbacks->second) {
                        callbacks->second(user_data);
                    }
                }
                dbus_message_iter_next(&dict);
            }
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    };

    // Register signal match rule
    std::string match_rule = "type='signal',interface='org.freedesktop.DBus.Properties',path=" + iface_path;
    dbus_bus_add_match(conn, match_rule.c_str(), &err);
    check_error(err);

    // Register signal filter with callbacks as user data
    auto* cb_pair = new std::pair<void (*)(void*), void (*)(void*)>(net_up_cb, net_down_cb);
    dbus_connection_add_filter(conn, signal_filter, cb_pair, [](void* data) { delete static_cast<std::pair<void (*)(void*), void (*)(void*)>*>(data); });

    // Ensure D-Bus messages are processed (non-blocking)
    dbus_connection_flush(conn);
}




// Mata possíveis gerenciadores de Wi-Fi conflitantes
void kill_managers() {
    const char* services[] = {
        "systemctl stop wpa_supplicant",
        "killall wpa_supplicant",
		"systemctl stop NetworkManager",
        // "systemctl stop wpa_supplicant",
        "systemctl stop iwd",
        "killall wpa_supplicant",
        "killall iwd",
        "killall NetworkManager",
		"killall dhclient"
    };

    for (const auto& cmd : services) {
        std::system(cmd);
    }

    std::cout << "[INFO] WiFi services disabled (temporarily).\n";
    system("pkill -9 NetworkManager wpa_supplicant connman wicd 2>/dev/null");
    sleep(1);
}

// Inicia wpa_supplicant em modo D-Bus
void start_wpa_supplicant() {
    system("wpa_supplicant -u -s -O DIR=/run/wpa_supplicant GROUP=netdev &");
    sleep(1);
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

    return Fl::run();
}