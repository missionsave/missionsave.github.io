// gcc -O2 msvclip.cpp -o msvclip -lX11 -lXfixes && strip msvclip && pkill msvclip && sudo cp ./msvclip /usr/local/bin && msvclip &
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Stream-based prepending to keep RAM usage near zero
void save_to_file(const char* text, size_t len) {
    char path[512];
    snprintf(path, sizeof(path), "%s/msvclip.txt", getenv("HOME"));
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    FILE* f_out = fopen(temp_path, "wb");
    if (!f_out) return;

    // Write new content
    fwrite(text, 1, len, f_out);
    fputc('\0', f_out);

    // Stream old content directly from disk to disk
    FILE* f_in = fopen(path, "rb");
    if (f_in) {
        char buffer[4096];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), f_in)) > 0) {
            fwrite(buffer, 1, n, f_out);
        }
        fclose(f_in);
    }

    fclose(f_out);
    rename(temp_path, path);
}
void handle_selection(Display* display, Window window, Atom property) {
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    
    // Request plain text, which matches what happens when you "paste normally"
    Atom target_type = XInternAtom(display, "text/plain;charset=utf-8", False);
    if (target_type == None) {
        target_type = XInternAtom(display, "text/plain", False);
    }

    XConvertSelection(display, clipboard, target_type, property, window, CurrentTime);
    XFlush(display);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.type == SelectionNotify && event.xselection.selection == clipboard) {
            if (event.xselection.property == None) return;

            Atom actual_type;
            int actual_format;
            unsigned long n_items, bytes_after;
            unsigned char* data = NULL;

            // Read the property data safely
            XGetWindowProperty(display, window, property, 0, 65536, False,
                               AnyPropertyType, &actual_type, &actual_format,
                               &n_items, &bytes_after, &data);

            if (data) {
                if (n_items > 0) {
                    int is_text = 1;
                    
                    // FIXED: Scan up to n_items - 1.
                    // If the very last byte is a null terminator '\0', that is normal for strings!
                    // We only reject if a null byte is hidden INSIDE the actual text body.
                    unsigned long scan_limit = (n_items > 0) ? n_items - 1 : 0;
                    for (unsigned long i = 0; i < scan_limit; i++) {
                        if (data[i] == '\0') {
                            is_text = 0;
                            break;
                        }
                    }

                    if (is_text) {
                        // Explicit C++ style cast to fix the compiler error
                        save_to_file(reinterpret_cast<char*>(data), n_items);
                    }
                }

                XFree(data);
            }

            XDeleteProperty(display, window, property);
            return;
        }
    }
}
void handle_selectionv1(Display* display, Window window, Atom property) {
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    Atom utf8 = XInternAtom(display, "UTF8_STRING", False);

    XConvertSelection(display, clipboard, utf8, property, window, CurrentTime);
    XFlush(display);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.type == SelectionNotify && event.xselection.selection == clipboard) {
            if (event.xselection.property == None) return;

            Atom actual_type;
            int actual_format;
            unsigned long n_items, bytes_after;
            unsigned char* data = NULL;

            // Read property (limiting to ~256KB per grab to prevent RAM spikes)
            XGetWindowProperty(display, window, property, 0, 65536, False,
                               AnyPropertyType, &actual_type, &actual_format,
                               &n_items, &bytes_after, &data);

			if (data) {
				if (n_items > 0) {

					// Reject binary data: do NOT save if any '\0' is present
					int is_text = 1;
					for (unsigned long i = 0; i < n_items; i++) {
						if (data[i] == '\0') {
							is_text = 0;
							break;
						}
					}

					if (is_text) {
						save_to_file((char*)data, n_items);
					}
				}

				XFree(data);
			}

            XDeleteProperty(display, window, property);
            return;
        }
    }
}
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Struct to easily map LaTeX macro strings to Unicode outputs
typedef struct {
    const char *macro;
    const char *unicode;
} MathSymbol;

// Add any common LaTeX symbol shortcuts you need rendered here!
MathSymbol symbol_map[] = {
    {"\\approx", "≈"},
    {"\\neq",    "≠"},
    {"\\le",     "≤"},
    {"\\ge",     "≥"},
    {"\\times",  "×"},
    {"\\div",    "÷"},
    {"\\alpha",  "α"},
    {"\\beta",   "β"},
    {"\\gamma",  "γ"},
    {"\\infty",  "∞"},
    {"\\pm",     "±"}
};
int symbol_map_len = sizeof(symbol_map) / sizeof(MathSymbol);

void append_formatted_math(char *dest, int *d_idx, const char *src, int start, int end) {
    for (int i = start; i < end; i++) {
        
        // 1. Check for Macro Commands (e.g., \approx)
        if (src[i] == '\\') {
            bool found_macro = false;
            for (int s = 0; s < symbol_map_len; s++) {
                int m_len = strlen(symbol_map[s].macro);
                // Ensure there's enough room left in the substring to check for a match
                if (i + m_len <= end && strncmp(&src[i], symbol_map[s].macro, m_len) == 0) {
                    strcat(dest, symbol_map[s].unicode);
                    *d_idx += strlen(symbol_map[s].unicode);
                    i += m_len - 1; // Advance loop counter past the macro string
                    found_macro = true;
                    break;
                }
            }
            if (found_macro) continue;
        }

        // 2. Check for Superscripts (e.g., ^2 -> ²)
        if (src[i] == '^' && i + 1 < end) {
            char next = src[i + 1];
            const char *sup = NULL;
            if (next == '1') sup = "¹";
            else if (next == '2') sup = "²";
            else if (next == '3') sup = "³";
            else if (next == '4') sup = "⁴";
            else if (next == '5') sup = "⁵";
            else if (next == '6') sup = "⁶";
            else if (next == '7') sup = "⁷";
            else if (next == '8') sup = "⁸";
            else if (next == '9') sup = "⁹";
            else if (next == '0') sup = "⁰";
            else if (next == 'n') sup = "ⁿ";

            if (sup) {
                strcat(dest, sup);
                *d_idx += strlen(sup);
                i++; // Skip character flag
                continue;
            }
        }

        // 3. Check for Subscripts (e.g., _2 -> ₂)
        if (src[i] == '_' && i + 1 < end) {
            char next = src[i + 1];
            const char *sub = NULL;
            if (next == '1') sub = "₁";
            else if (next == '2') sub = "₂";
            else if (next == '3') sub = "₃";
            else if (next == '4') sub = "₄";
            else if (next == '5') sub = "₅";
            else if (next == '6') sub = "₆";
            else if (next == '7') sub = "₇";
            else if (next == '8') sub = "₈";
            else if (next == '9') sub = "₉";
            else if (next == '0') sub = "₀";

            if (sub) {
                strcat(dest, sub);
                *d_idx += strlen(sub);
                i++; // Skip character flag
                continue;
            }
        }

        // 4. Default execution: Copy regular math symbols (+, -, =, variables) verbatim
        dest[(*d_idx)++] = src[i];
        dest[*d_idx] = '\0';
    }
}

void format_latex_equations(const char *input, char *output) {
    int len = strlen(input);
    int i = 0;
    int out_idx = 0;
    output[0] = '\0';

    while (i < len) {
        // Parse \( ... \) inline math blocks
        if (i + 1 < len && input[i] == '\\' && input[i + 1] == '(') {
            int start_math = i + 2;
            int end_math = -1;
            for (int j = start_math; j < len - 1; j++) {
                if (input[j] == '\\' && input[j + 1] == ')') {
                    end_math = j;
                    break;
                }
            }
            if (end_math != -1) {
                append_formatted_math(output, &out_idx, input, start_math, end_math);
                i = end_math + 2;
                continue;
            }
        }

        // Parse $ ... $ single dollar inline blocks (like $CO_2$)
        // Be careful to step over $$ display math if it happens to show up 
        if (input[i] == '$' && (i == 0 || input[i - 1] != '$') && (i + 1 < len && input[i + 1] != '$')) {
            int start_math = i + 1;
            int end_math = -1;
            for (int j = start_math; j < len; j++) {
                if (input[j] == '$') {
                    end_math = j;
                    break;
                }
            }
            if (end_math != -1) {
                append_formatted_math(output, &out_idx, input, start_math, end_math);
                i = end_math + 1;
                continue;
            }
        }

        // Parse $$ ... $$ display math blocks
        if (i + 1 < len && input[i] == '$' && input[i + 1] == '$') {
            int start_math = i + 2;
            int end_math = -1;
            for (int j = start_math; j < len - 1; j++) {
                if (input[j] == '$' && input[j + 1] == '$') {
                    end_math = j;
                    break;
                }
            }
            if (end_math != -1) {
                append_formatted_math(output, &out_idx, input, start_math, end_math);
                i = end_math + 2;
                continue;
            }
        }

        // Default layout: Keep formatting styles, lists, and text untouched
        output[out_idx++] = input[i++];
        output[out_idx] = '\0';
    }
}




int msvclip() {
    Display* display = XOpenDisplay(NULL);
    if (!display) return 1;

    int event_base, error_base;
    if (!XFixesQueryExtension(display, &event_base, &error_base)) return 1;

    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
    Atom property = XInternAtom(display, "CLIPBOARD_PROPERTY", False);
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);

    XFixesSelectSelectionInput(display, window, clipboard, XFixesSetSelectionOwnerNotifyMask);

    while (1) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == event_base + XFixesSelectionNotify) {
            XFixesSelectionNotifyEvent* sev = (XFixesSelectionNotifyEvent*)&event;
            if (sev->subtype == XFixesSetSelectionOwnerNotify) {
                handle_selection(display, window, property);
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}

int main(int argc, char *argv[]) {
    // Check if the user provided a string argument
    if (argc < 2) {
        // fprintf(stderr, "Error: Missing text argument.\n");
        // fprintf(stderr, "Usage: %s 'your text here'\n", argv[0]);
		msvclip();
        return 0;
    }

	char input[4096] = {0};
    char result[4096] = {0};

    // Check if the --f flag is passed as the first argument
    if (argc >= 2 && strcmp(argv[1], "--f") == 0) {
        int ch, idx = 0;
        // Read directly from stdin (e.g., from a pipe)
        while ((ch = getchar()) != EOF && idx < sizeof(input) - 1) {
            input[idx++] = ch;
        }
		format_latex_equations(input, result);
    	printf("%s", result); 
	    return 0;
    }

 

    return 0;
}