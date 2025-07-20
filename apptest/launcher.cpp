#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <cstdio>
#include <memory>
#include <string>
#include <array>
#include <cstdlib>
#include <unordered_map>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <regex>
using namespace std;

struct Launcher{
	// Check if a window with the given title substring exists
	bool windowExists(const std::string& title) {
		std::array<char, 256> buffer;
		std::string output;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(
			popen("wmctrl -l", "r"), pclose);
		if (!pipe) return false;
		while (fgets(buffer.data(), buffer.size(), pipe.get())) {
			output += buffer.data();
		}
		return output.find(title) != std::string::npos;
	}

	// Activate the window whose title matches
	void activateWindow(const std::string& title) {
		std::string cmd = "wmctrl -a \"" + title + "\"";
		system(cmd.c_str());
	}

	#include <string>
	#include <regex>

	// Removes placeholders like %F, %U, %f, etc.
	std::string parseExecCommand(const std::string& execLine) {
		std::string cleaned = std::regex_replace(execLine, std::regex("%[fFuUdDnNickvm]"), "");
		return cleaned;
	}

	// Launch SciTE in the background
	void launch(std::string desktopExecLine) {
		std::string command = parseExecCommand(desktopExecLine) + " &";
		system(command.c_str());
	}


	unordered_map <string,string> uapps;

	void fillApplications(std::unordered_map<std::string, std::string>& uapps) {
		const char* command =
			"grep -rl 'Type=Application' /usr/share/applications ~/.local/share/applications 2>/dev/null | "
			"xargs grep -L 'NoDisplay=true' 2>/dev/null | "
			"xargs grep -L 'Hidden=true' 2>/dev/null | "
			"xargs grep -hE '^Name=|^Exec=' 2>/dev/null | "
			"paste - -";

		std::array<char, 512> buffer;
		std::string output;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
		if (!pipe) return;

		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			output += buffer.data();
		}

		std::istringstream stream(output);
		std::string line;

		std::regex placeholder_re("%[fFuUdDnNickvm]");

		while (std::getline(stream, line)) {
			size_t name_pos = line.find("Name=");
			size_t exec_pos = line.find("Exec=");
			if (name_pos == std::string::npos || exec_pos == std::string::npos)
				continue;

			std::string name = line.substr(name_pos + 5, exec_pos - name_pos - 6);
			std::string exec = line.substr(exec_pos + 5);

			// Remove placeholders like %F
			exec = std::regex_replace(exec, placeholder_re, "");

			uapps[name] = exec;
		}
	}


	// FLTK button callback
	void on_click(Fl_Widget*, void*) {
		const std::string winTitle = "Panel";
		if (windowExists(winTitle)) {
			activateWindow(winTitle);
		} else {
			launch(uapps[winTitle]);
		}
	}
};
#pragma region 
	Launcher* lnc;
	vector<string> pinnedApps={"Google Chrome","Visual Studio Code","SciTE Text Editor","super@dell: /home/super/msv/apptest"};
#pragma endregion

#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <sstream>

std::vector<std::string> getOpenWindowTitles() {
    std::vector<std::string> titles;
    const char* command = "wmctrl -l";

    std::array<char, 512> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) return titles;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());

        // Skip empty or malformed lines
        if (line.size() < 5) continue;

        // Find the window title (after 4th field)
        std::istringstream iss(line);
        std::string field;
        int fieldCount = 0;
        std::string title;

        while (iss >> field) {
            ++fieldCount;
            if (fieldCount >= 4) {
                // Read the rest of the line as title
                std::getline(iss, title);
                break;
            }
        }

        // Clean leading spaces from title
        title.erase(0, title.find_first_not_of(" \t\n\r"));

        if (!title.empty()) {
            titles.push_back(title);
        }
    }

    return titles;
}


#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>

struct WindowInfo {
    std::string window_id;
    std::string title;
    int pid;
    std::string process_name;
};

std::string getProcessName(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/comm");
    std::string name;
    if (file.is_open()) {
        std::getline(file, name);
    }
    return name;
}

std::vector<WindowInfo> getOpenWindowsInfo() {
    std::vector<WindowInfo> windows;
    const char* command = "wmctrl -lp";

    std::array<char, 512> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) return windows;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());

        std::istringstream iss(line);
        WindowInfo info;

        iss >> info.window_id; // 0xID
        std::string desktop_num;		
        iss >> desktop_num;    // Desktop number (ignored)
		if (desktop_num == "-1") {
			continue; // Skip sticky windows (appear on all desktops)
		}
        iss >> info.pid;       // PID
        std::string hostname;
        iss >> hostname;       // Hostname (ignored)

        std::string title;
        std::getline(iss, title);
        title.erase(0, title.find_first_not_of(" \t\n\r")); // Trim leading spaces
        info.title = title;

        info.process_name = getProcessName(info.pid);

        windows.push_back(info);
    }

    return windows;
}

#include <string>
#include <cstdio>
#include <memory>
#include <sstream>

int getPidFromWindowTitle(const std::string& titleSubstring) {
    const char* command = "wmctrl -lp";
    std::array<char, 512> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);

    if (!pipe) return -1;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());

        std::istringstream iss(line);
        std::string window_id;
        std::string desktop_num;
        int pid;
        std::string hostname;
        std::string title;

        iss >> window_id;
        iss >> desktop_num;

        if (desktop_num == "-1") {
            continue; // Skip sticky windows
        }

        iss >> pid;
        iss >> hostname;
        std::getline(iss, title);

        // Clean leading spaces
        title.erase(0, title.find_first_not_of(" \t\n\r"));

        // Check if the titleSubstring is found in the window title
        if (title.find(titleSubstring) != std::string::npos) {
            return pid; // Found matching window
        }
    }

    return -1; // Not found
}



int main() {
	lnc=new Launcher;
	lnc->fillApplications(lnc->uapps);

	auto windows = getOpenWindowsInfo();
    for (const auto& win : windows) {
        std::cout << win.process_name << " [" << win.pid << "] "
                  << win.window_id << " => "
                  << win.title << '\n';
    }
	cout<<"windows "<<windows.size()<<"\n";


	// auto titles = getOpenWindowTitles();
    // for (const auto& title : titles) {
    //     std::cout << title << '\n';
    // }

    // for (const auto& [name, exec] : lnc->uapps) {
    //     std::cout << name << " => " << exec << '\n';
    // }
	cout<<"uapps "<<lnc->uapps.size()<<endl;

	for(int i=0;i<pinnedApps.size();i++){
		cout<<pinnedApps[i]<<" => []"<<getPidFromWindowTitle(pinnedApps[i])<<endl;
	} 







    Fl_Window* win = new Fl_Window(440, 100, "Launcher");
	vector<Fl_Button*> vbtn(pinnedApps.size());
	for(int i=0;i<pinnedApps.size();i++){
    	vbtn[i] = new Fl_Button(i*120, 0, 120, 24, pinnedApps[i].c_str());
		vbtn[i]->callback([](Fl_Widget* widget){

			const std::string winTitle = std::string(((Fl_Button*)widget)->label());
			if (lnc->windowExists(winTitle)) {
				lnc->activateWindow(winTitle);
			} else {
				lnc->launch(lnc->uapps[winTitle]);
			}
		});
	}
    // btn->callback(on_click);
    win->end();
    win->show();
    return Fl::run();
}
