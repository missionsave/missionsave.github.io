#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h>  // para sleep()
using namespace std;

struct WifiEntry {
    std::string ssid;
    std::string bssid;
    int signal;
    int freq;
};

std::vector<WifiEntry> scan_wifi(const std::string& iface) {
    std::string cmd1 = "wpa_cli -i " + iface + " scan";
    system(cmd1.c_str());
    sleep(2);

    std::string cmd2 = "wpa_cli -i " + iface + " scan_results";
    FILE* pipe = popen(cmd2.c_str(), "r");
    if (!pipe) return {};

    std::vector<WifiEntry> results;
    char buffer[1024];
    bool skip_header = true;

    while (fgets(buffer, sizeof(buffer), pipe)) {
        if (skip_header) {
            skip_header = false;
            continue;
        }

        std::istringstream line(buffer);
        WifiEntry entry;
        std::string flags;

        line >> entry.bssid >> entry.freq >> entry.signal >> flags;
        std::getline(line, entry.ssid);

        // Limpa espaços antes do SSID
        size_t start = entry.ssid.find_first_not_of(" \t");
        if (start != std::string::npos)
            entry.ssid = entry.ssid.substr(start);
        else
            entry.ssid.clear();

        results.push_back(entry);
    }

    pclose(pipe);

    std::sort(results.begin(), results.end(), [](const WifiEntry& a, const WifiEntry& b) {
        return a.signal > b.signal;
    });

    return results;
}

// iw dev | awk '$1=="Interface"{print $2}'

// Custom browser class to draw a border around selected items
class BorderSelectBrowser : public Fl_Browser {
public:
    // Constructor
    BorderSelectBrowser(int X, int Y, int W, int H, const char* L = 0)
        : Fl_Browser(X, Y, W, H, L) {}

protected:
    // Override the draw_item method without relying on internal Fl_Browser functions
    void draw_item(int n, int X, int Y, int W, int H, const char* l) const {
        
        
        // Check if the item at index 'n' is currently selected.
        if (selected(n)) {
            // If it is, set the border color and draw a rectangle border
            fl_color(FL_BLUE);
            fl_rect(X, Y, W, H);
        }
    }
};


class MyBrowser : public Fl_Browser {
public:
    MyBrowser(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Browser(X, Y, W, H, L) {}

int handle(int event) override{
		// cout<<event<<"\n";
    switch (event) {
        case FL_PUSH:{
		// int clicked = which(Fl::event_x(), Fl::event_y());
		// return Fl_Browser::handle(event);
		break;
		}
        case FL_RELEASE:
			// int line =  (position());

int scrollPos = position();
int line_height = full_height()/ size(); // or use a fixed value
// int line_height = this->textsize(); // or use a fixed value
int relative_y = Fl::event_y() - this->y() +scrollPos;
int clicked_line = relative_y / line_height;
clicked_line++;


        if (clicked_line > 0 && clicked_line <= size()) {
            void* data_ptr = data(clicked_line);
            if (data_ptr) {
                // Cast back to your original type
                WifiEntry* obj = static_cast<WifiEntry*>(data_ptr);
				cout<<obj->ssid<<endl;
            }
        }



			// cout<<full_height()<<endl;
			// cout<<selected(2)<<endl;
			int x = Fl::event_x()-this->x();
			int y = Fl::event_x()-this->y();
			select(clicked_line); ///test only

		break;
			// int line=
            int selected = value(); // 1-based index of selected line
			// cout<<line<<endl;
			// cout<<selected<<endl;
            if (selected > 0) {
                const char* item_text = text(selected);
                std::cout << x<<" Selected line: " << selected
                          << ", Text: " << (item_text ? item_text : "(null)") << std::endl;
				// this->text(selected, "New content for line 3");
            }
            break;
    }
    return Fl_Browser::handle(event);
}


// void draw() override {
//         fl_push_clip(x(), y(), w(), h());

//         for (int i = 1; i <= size(); ++i) {
//             int Y = yposition(i);
//             if (Y + textsize() < y() || Y > y() + h()) continue;

//             fl_color(FL_WHITE); // Background: always white
//             fl_rectf(x(), Y, w(), textsize());

//             fl_color(FL_BLACK); // Text: always black
//             const char* txt = text(i);
//             if (txt)
//                 drawtext(txt, x() + 2, Y, w(), textsize(), align(), i);
//         }

//         fl_pop_clip();
//     }

};


int main(int argc, char **argv) {
    Fl_Window* win = new Fl_Window(400, 300, "WiFi Scanner");
    // BorderSelectBrowser* browser = new BorderSelectBrowser(10, 10, 380, 280);
    // CustomBrowser* browser = new CustomBrowser(10, 10, 380, 280);
    MyBrowser* browser = new MyBrowser(20, 40, 380, 280);
    // Fl_Browser* browser = new Fl_Browser(10, 10, 380, 280);
    // browser->type(0);
    // browser->type(FL_SELECT_BROWSER);
	// browser->selection_color(browser->color()); 
	// browser->box(FL_NO_BOX);
	// browser->type(FL_HOLD_BROWSER);
    // browser->selection_color(FL_BACKGROUND_COLOR); 
    // Set column widths (M column, S column, SSID column, Freq column)
    browser->column_widths(new int[]{20, 20, 200, 60, 0}); // Last 0 terminates array
    
    // Add column headers
    browser->add("@b@uM\t@b@uS\t@b@uSSID\t@b@uFreq");

    std::vector<WifiEntry> entries = scan_wifi("wlp2s0");
// for(int ti=0;ti<30;ti++){
    for (const WifiEntry& e : entries) {
        stringstream strm;
        strm << "@B1@uM\t"  // First column (M) with background 1
             << "@B2@uS\t"  // Second column (S) with background 2
             << "@B3@u" << e.ssid << "\t"  // SSID with background 2
             << "@B4@u" << e.freq;          // Frequency with background 2
        browser->add(strm.str().c_str());
        browser->data(browser->size(), (void*)&e);
    }
// }
	// browser->item_draw(custom_draw);

    win->end();
    win->show(argc, argv);
    return Fl::run();
}
