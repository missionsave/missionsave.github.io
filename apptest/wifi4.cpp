// g++ -std=c++20 wifi4.cpp -o wifi4 -lfltk ../common/general.cpp -I../common

#include <iostream>
#include <cstdlib>
#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <algorithm> // Para std::all_of
#include <cctype>    // Para ::isdigit
#include <sstream>   // Para std::istringstream

#include "general.hpp"
using namespace std;

std::string wifi_ssid;
Fl_Input *password_input;

// Executa comando e retorna saída
std::string exec(const char* cmd) {
    char buffer[128];
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Verifica se a rede-alvo está conectada (corrigido)
bool is_target_connected(const std::string& id_str) {
    std::string status = exec("wpa_cli -i wlp2s0 status");
    std::istringstream iss(status);
    std::string line;
    bool is_completed = false;
    bool is_correct_id = false;
    while (std::getline(iss, line)) {
        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);
            if (key == "wpa_state" && value == "COMPLETED") {
                is_completed = true;
            } else if (key == "id" && value == id_str) {
                is_correct_id = true;
            }
        }
    }
    return is_completed && is_correct_id;
}

// Tenta conectar com a senha fornecida (corrigido)
void try_connect(Fl_Widget*, void*) {
    std::string password = password_input->value();
    trim(password);
    std::cout << "Tentando conectar à rede '" << wifi_ssid << "'...\n";

    // 1. Desconecta de redes existentes e limpa configurações
    // system("wpa_cli -i wlp2s0 disconnect > /dev/null 2>&1");
    // system("wpa_cli -i wlp2s0 remove_network all > /dev/null 2>&1");

    // 2. Adiciona rede temporária
    std::string id_str = exec("wpa_cli -i wlp2s0 add_network");
    id_str.erase(id_str.find_last_not_of(" \n\r\t") + 1);
    if (id_str.empty() || !std::all_of(id_str.begin(), id_str.end(), ::isdigit)) {
        std::cerr << "Erro: Não foi possível adicionar a rede. Verifique a interface Wi-Fi.\n";
        return;
    }
    std::cout << "ID da rede temporária: " << id_str << "\n";

	pausa

    // 3. Configura a rede
    system(("wpa_cli -i wlp2s0 set_network " + id_str + " ssid '\"" + wifi_ssid + "\"' > /dev/null").c_str());
    system(("wpa_cli -i wlp2s0 set_network " + id_str + " psk '\"" + password + "\"' > /dev/null").c_str());
    system(("wpa_cli -i wlp2s0 select_network " + id_str + " > /dev/null").c_str());
    system(("wpa_cli -i wlp2s0 enable_network " + id_str + " > /dev/null").c_str());

    // 4. Verifica conexão (com timeout de 10 segundos)
    bool connected = false;
    for (int i = 0; i < 20; i++) {
        if (is_target_connected(id_str)) {
            connected = true;
            break;
        }
        sleepms(500);
    }

    // 5. Resultado
    if (connected) {
        std::cout << "[SUCESSO] Conexão estabelecida! Salvando...\n";
        system("wpa_cli -i wlp2s0 save_config > /dev/null");
        // Fl::quit();
    } else {
        std::cerr << "[ERRO] Falha na conexão (senha incorreta ou rede indisponível).\n";
        std::string final_status = exec("wpa_cli -i wlp2s0 status");
        std::cerr << "Status final:\n" << final_status << "\n";
        system(("wpa_cli -i wlp2s0 remove_network " + id_str + " > /dev/null").c_str());
    }
}

// Janela FLTK para inserir senha (inalterada)
void ask_password(const std::string& ssid) {
    wifi_ssid = ssid;
    Fl_Window *window = new Fl_Window(300, 150, "Conectar à rede Wi-Fi");
    Fl_Box *label = new Fl_Box(20, 20, 260, 30, ("Senha para: " + ssid).c_str());
    password_input = new Fl_Input(20, 60, 260, 30);
    // password_input->type(FL_SECRET_INPUT);
    Fl_Button *connect_btn = new Fl_Button(100, 100, 100, 30, "Conectar");
    connect_btn->callback(try_connect);
    window->end();
    window->show();
    Fl::run();
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <SSID>\n";
        return 1;
    }
    string ar = argv[1];
    trim(ar);
    cotm(ar);
    ask_password(ar);
    return 0;
}