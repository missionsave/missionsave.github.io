#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_draw.H>

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

int main(int argc, char **argv) {
    Fl_Window *window = new Fl_Window(500, 350, "FLTK Noto & Emoji Example");

    // 1. Find the index of an available emoji font
    int emoji_font_index = -1;
    int num_fonts = Fl::set_fonts(0); // Initialize font list and get count

    for (int i = 0; i < num_fonts; ++i) {
        const char* font_name = Fl::get_font_name(i);
        if (font_name) {
            std::string name_str = font_name;
            // Look for common emoji font names (case-insensitive where possible)
            if (name_str.find("Noto Color Emoji") != std::string::npos ||
            // if (name_str.find("Noto Color Emoji") != std::string::npos ||
                name_str.find("Segoe UI Emoji") != std::string::npos ||
                name_str.find("Apple Color Emoji") != std::string::npos) {
                emoji_font_index = i;
                break; // Found one, stop searching
            }
        }
    }

    if (emoji_font_index == -1) {
        std::cerr << "Warning: No dedicated emoji font found by name. Emojis might not display correctly." << std::endl;
        // Fallback to FL_FREE_FONT (Noto Sans if set, or default) if no emoji font is found
        // This is important so other text still renders.
        Fl::set_font(FL_FREE_FONT, "Noto Sans"); 
    } else {
        std::cout << "Detected emoji font at index: " << emoji_font_index << " (" << Fl::get_font_name(emoji_font_index) << ")" << std::endl;
        // Optionally, you can assign this emoji font to a dedicated FLTK font slot
        // e.g., Fl::set_font(FL_SCREEN_BOLD, Fl::get_font_name(emoji_font_index));
        // Then use FL_SCREEN_BOLD later for emoji-heavy content.

        // Set Noto Sans as the default for FL_FREE_FONT for general text
        Fl::set_font(FL_FREE_FONT, "Noto Sans");
    }

    // --- Button setup ---
    // If emoji_font_index is found, try using it directly for the button's emoji
    // Otherwise, it will fallback to FL_FREE_FONT's fallback.


	stringstream strm;
	strm<<u8"a"<<"test";
	// strm<<u8"😊"<<"test";

    Fl_Button *button = new Fl_Button(50, 50, 180, 40, u8"😊");
	button->copy_label(strm.str().c_str());
    // Fl_Button *button = new Fl_Button(50, 50, 180, 40, u8"😊");
    if (emoji_font_index != -1) {
        // button->labelfont(FL_FREE_FONT);
        button->labelfont(emoji_font_index); // Use the detected emoji font for button
    } else {
        button->labelfont(FL_FREE_FONT); // Fallback to Noto Sans or system default
    }
    button->labelsize(20);

    // --- Text Display setup ---
    Fl_Box *heading = new Fl_Box(FL_DOWN_BOX, 50, 120, 400, 30, u8"Emoji Text Display:");
    heading->labelfont(FL_FREE_FONT); // Use Noto Sans for the heading
    heading->labelsize(16);
    heading->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
    Fl_Text_Display *display = new Fl_Text_Display(50, 160, 400, 150);
    display->buffer(buffer);

    // Set the text display's font to the emoji font if found.
    // This will make all text in the display attempt to use the emoji font first.
    if (emoji_font_index != -1) {
        display->textfont(emoji_font_index);
    } else {
        display->textfont(FL_FREE_FONT); // Fallback to Noto Sans or system default
    }
    display->textsize(22);

    // Emojis and text
    std::string emoji_text = u8"Hello FLTK! 👋\n"
                             u8"Emojis are fun: ✨🚀🎉❤️👍😊🤷‍♀️🤯\n"
                             u8"Flags: 🇵🇹🇬🇧🇺🇸🇯🇵🇨🇳\n" // These are the challenging ones without HarfBuzz
                             u8"Animals: 🐶🐱🐭🐹🐰🦊🐻🐼🐨🐯🦁🐒🐦🐧🐸🐢🐍🦎🦖🦕🐙🦀🐠🐳🐬🦋🐌🐛🐜🐝🐞🕷🕸🦂🦟🦠\n"
                             u8"Food: 🍎🍐🍊🍋🍌🍉🍇🍓🍈🍒🍑🥭🍍🥥🥝🍅🍆🥑🥦🥒🌶🌽🥕🥔🍠🥐🍞🥖🥨🧀🥚🍳🥞🥓🥩🍗🍖🍕🍔🍟🌭🥪🌮🌯🥙🧆🥫🍝🍜🍲🍛🍣🍱🥟🍤🍙🍚🍘🍥🥠🥮🍢🍡🍧🍨🍦🥧🧁🍰🎂🍮🍭🍬🍫🍿🍩🍪🌰🥜🍯🥛🍼☕🍵🍶🍾🍷🍸🍹🍺🍻🥂🥃🥤🧃🧉🧊\n"
                             u8"And more! 😎🥳🌍🌟";

    buffer->text(emoji_text.c_str());

    window->end();
    window->show(argc, argv);

    // Print out available fonts for debugging (already in previous code)
    std::cout << "\n--- Available FLTK fonts (mapped to system fonts) ---" << std::endl;
    int current_num_fonts = Fl::set_fonts(0); 
    for (int i = 0; i < current_num_fonts; ++i) {
        const char* font_name = Fl::get_font_name(i);
        if (font_name) {
             std::cout << "  " << i << ": " << font_name << std::endl;
        } else {
             std::cout << "  " << i << ": (null or unknown name)" << std::endl;
        }
    }
    std::cout << "---------------------------------------------------" << std::endl;

    return Fl::run();
}