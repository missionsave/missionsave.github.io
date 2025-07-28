#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>

// Função para encontrar o caminho da fonte usando fc-match
std::string findFontPath(const std::string& fontName) {
    std::string command = "fc-match \"" + fontName + "\" file | head -n 1";
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Erro ao executar fc-match! (Não fatal, apenas para verificação)" << std::endl;
        return "";
    }
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);

    size_t pos_file_prefix = result.find(":file=");
    if (pos_file_prefix != std::string::npos) {
        result = result.substr(pos_file_prefix + 6);
    }
    size_t first = result.find_first_not_of(" \t\n\r");
    size_t last = result.find_last_not_of(" \t\n\r");
    if (std::string::npos == first || std::string::npos == last) {
        return "";
    }
    return result.substr(first, (last - first + 1));
}

int main(int argc, char **argv) {
    Fl_Window *window = new Fl_Window(400, 200, "Exemplo de Checkmark com Noto Font");

    // Tentativas de nomes de fontes em ordem de preferência
    std::vector<const char*> preferred_font_names = {
        "Noto Sans Symbols",
        "Noto Sans Symbols2",
        "Noto Sans",          // Genérica
        "Noto Color Emoji",   // Para emojis, mas muitas vezes tem símbolos
        "DejaVu Sans"         // Boa fonte genérica Unicode no Linux
    };

    const char* selected_font_name = nullptr;
    Fl_Font target_fltk_font_id = FL_HELVETICA; // Fallback padrão

    // Iterar pelas fontes preferidas e tentar carregar a primeira encontrada
    for (const char* font_name_candidate : preferred_font_names) {
        std::string found_path = findFontPath(std::string(font_name_candidate) + ":style=Regular");
        if (!found_path.empty()) {
            std::cout << "Fonte '" << font_name_candidate << "' encontrada em: " << found_path << std::endl;
            selected_font_name = font_name_candidate;
            
            // Tenta mapear o nome da fonte para um slot FLTK
            // AQUI estamos usando FL_FREE_FONT, que é o primeiro slot livre.
            // Para versões muito antigas, o retorno pode ser void, não atribuível.
            Fl::set_font(FL_FREE_FONT, selected_font_name);
            target_fltk_font_id = FL_FREE_FONT;
            break; // Sai do loop assim que uma fonte é encontrada e mapeada
        } else {
            std::cout << "Fonte '" << font_name_candidate << "' não encontrada usando fc-match." << std::endl;
        }
    }

    if (!selected_font_name) {
        Fl::warning("Nenhuma fonte Noto ou alternativa encontrada/carregada. Usando FL_HELVETICA.");
        // target_fltk_font_id já é FL_HELVETICA
    } else {
        std::cout << "FLTK tentará usar a fonte: " << selected_font_name << std::endl;
    }

    Fl_Box *box = new Fl_Box(20, 50, 360, 100);
    box->box(FL_FLAT_BOX);

    // O caractere de checkmark Unicode (U+2713)
    const char* text_content = "Tarefa Concluída: \xE2\x9C\x93"; // UTF-8 para U+2713 (✓)
    // Se você quiser testar o emoji de checkmark completo:
    // const char* text_content = "Tarefa Concluída: \xE2\x9C\x85"; // UTF-8 para U+2705 (✅)
    // Se "Noto Color Emoji" estiver instalada, esta é a forma mais robusta de obter emojis.

    box->label(text_content);
    box->labelfont(target_fltk_font_id); // Aplica o ID da fonte FLTK
    box->labelsize(36);
    box->labelcolor(FL_DARK_GREEN);
    box->align(FL_ALIGN_CENTER);

    window->end();
    window->show(argc, argv);

    return Fl::run();
}