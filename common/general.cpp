#include "general.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <thread>

#include <ctime>
#include <chrono>
#include <iomanip>
#include <algorithm> // Necessário para std::find
#include <atomic>


using namespace std;

// Function to split a string by commas
std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(s.substr(start)); // Last token
    return tokens;
} 

std::vector<int> splitToVint(const std::string& str, std::string delimiter) {
    std::vector<int> result;

    vstring sp=split(str,delimiter);
    result=vint(sp.size());
    lop(i,0,sp.size())
        result[i]=atoi(sp[i].c_str());
    return result;

    size_t start = 0, end;

    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(std::stoi(str.substr(start, end - start)));
        start = end + delimiter.length();
    }

    result.push_back(std::stoi(str.substr(start))); // Add the last element
    return result;
}

// Implementation of cotm_function 
void cotm_function(const std::string &args_names, const std::string &args_values) { 
    std::vector<std::string> names = split(args_names);
    std::vector<std::string> values = split(args_values,"⟁");

	stringstream strm;
    // Get current time
    auto timeval = std::time(nullptr);
    auto tm = *std::localtime(&timeval);
    strm << std::put_time(&tm, "%H:%M:%S ") << " ";

    // Print each variable and its value
    for (size_t i = 0; i < names.size(); ++i) {
        if(names[i].back() == '"'){strm << names[i]<<"\t"; continue;}
        strm << names[i] << "=" << values[i] << "\t";
    }
    strm << std::endl;
	if(scotmup)go_up_and_clear_line(cotmlastoutput);
	scotmup=0;
	cotmlastoutput=strm.str();
	cout<<cotmlastoutput;
}
bool scotmup=0;
std::string cotmlastoutput="";
int get_terminal_width() {
	#if defined(__linux__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    } 
	#endif	
		
	return 80;  // Fallback if detection fails
}

void go_up_and_clear_line(const std::string& last_text) {
    int width = get_terminal_width();
    int text_length = last_text.length();
    int rows = (text_length + width - 1) / width;  // ceil division

    for (int i = 0; i < rows; ++i) {
        std::cout << "\033[A"     // Move up
                  << "\r\033[2K"; // Clear line
    }
    // std::cout.flush();
}

void trim(std::string& str) {
    // Left trim
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));

    // Right trim
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end());
}
void sleepms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    // struct timespec ts;
    // ts.tv_sec = ms / 1000;           // Segundos
    // ts.tv_nsec = (ms % 1000) * 1000000L; // Milissegundos convertidos para nanossegundos

    // if (nanosleep(&ts, nullptr) < 0) {
    //     perror("nanosleep failed");
    // }
}

lnmutex::lnmutex(const std::string& name, bool debug) : name_(name), debug_(debug) {}

void lnmutex::lock() {
    std::unique_lock<std::mutex> lock(mutex_);
    lua_funcs_locked = true;
}

void lnmutex::unlock() {
    lua_funcs_locked = false;
    lua_funcs_cv.notify_all();
}

// #include <pthread.h>
void lnmutex::exit() {
    cotm("exit thread (not working)");
    // pthread_exit(nullptr);
    // throw std::runtime_error("Exiting thread via exception");
}

std::string lnmutex::getName() const {
    return name_;
}

nmutex::nmutex(const std::string& name, bool debug) : name_(name), debug_(debug) {}

void nmutex::lock() { 
    waiting_threads++; 
    islock = true;
    if (debug_) std::cout << name_ << " prev\n";
    mutex_.lock();
    if (debug_) std::cout << name_ << " next\n"; 
}   

void nmutex::unlock() { 
    waiting_threads--;
    islock = false;  
    mutex_.unlock();  
}       
   
bool nmutex::try_lock() {
    return mutex_.try_lock();
}

std::string nmutex::getName() const {
    return name_;
} 

void replace_All(std::string & data, std::string toSearch, std::string replaceStr){
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while( pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos =data.find(toSearch, pos + replaceStr.size());
    }
}

std::string joinv(const std::vector<int>& vec, std::string delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << vec[i];
    }
    return oss.str();
}
bool Contains(std::string text, std::string target ){

	if (text.find(target) != std::string::npos) {
		return 1;
		std::cout << "Found\n";
	} else {
		return 0;
		std::cout << "Not found\n";
	}
	return 0;
}

combR::combR(int n,int K,bool tocache){
    k=K;
    ranges=vector<int>(k);
    for(int i=0;i<k;i++)ranges[i]=n;
    range = combNumCombinIrregular();
    restoR=ranges;
    for(int i=k-2;i>=0;i--)restoR[i]=ranges[i]*restoR[i+1];
    if(tocache){
        cache.resize(k);
        for(int c=0;c<k;c++)cache[c]=new unsigned int [range];
        for(unsigned int  i=0;i<range;i++){
            vector<int> cf=toComb(i);
            for(int c=0;c<k;c++)cache[c][i]=cf[c];
        }
        cached=true;
    }
}
combR::combR(vector <int> Ranges,bool tocache){
    ranges=Ranges;
    k=ranges.size();
    range = combNumCombinIrregular();
    restoR=ranges;
    for(int i=k-2;i>=0;i--)restoR[i]=ranges[i]*restoR[i+1];
    if(tocache){
        cache.resize(k);
        for(int c=0;c<k;c++)cache[c]=new unsigned int [range];
        for(unsigned int  i=0;i<range;i++){
            vector<int> cf=toComb(i);
            for(int c=0;c<k;c++)cache[c][i]=cf[c];
        }
        cached=true;
    }
}
vector<int> combR::toComb(unsigned int csn){
    vector<int>res(k);
    res[k-1]=csn%restoR[k-1];
    for(int i=0;i<k-1;i++)res[i]=csn/restoR[i+1]%ranges[i];
    return res;
}
void combR::toComb(unsigned int csn,vector<int>&res){
    res[k-1]=csn%restoR[k-1];
    for(int i=0;i<k-1;i++)res[i]=csn/restoR[i+1];
    for(int i=1;i<k-1;i++)res[i]%=ranges[i];
}
void combR::toComb(int* res,unsigned int csn){
    res[k-1]=csn%restoR[k-1];
    for(int i=0;i<k-1;i++)res[i]=csn/restoR[i+1];
    for(int i=1;i<k-1;i++)res[i]%=ranges[i];
}
unsigned long long combR::combNumCombinIrregular(){
    unsigned long long res=1;
    for(int i=0;i<ranges.size();i++)res*=ranges[i];
    return res;
}
unsigned long long  combR::toCsn(int *comb){
    unsigned long long pos = 0;
    unsigned long long rangeval = range;
    for (int l = 0; l < k; l++) {
        int figura = comb[l];
        unsigned long long sector = rangeval / ranges[l];
        rangeval = sector;
        pos += sector * figura;
    }
    return pos;
}
unsigned long long  combR::toCsn(vint comb){
	return toCsn(&comb[0]);
}




struct performance{ 
    std::chrono::steady_clock::time_point pclock;
    const char* pname; 
    performance(const char*  name=""); //
    void p(const char* prefix=""); //
};

performance::performance(const char*  name){
    pname=name;
    pclock=std::chrono::steady_clock::now();
}
void performance::p(const char* prefix){
	if(prefix==0){
		pclock=std::chrono::steady_clock::now();
		printf("benchmark\n");
		return;
	}
	auto end = std::chrono::steady_clock::now();
	auto elapsed = end - pclock;
	printf("%s %.9f sec\n",prefix,std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()/1000000000.0);
	// printf("%s %s %f sec\n",pname,prefix,((double)clock()-pclock)/CLOCKS_PER_SEC);
	fflush(stdout);
	pclock=std::chrono::steady_clock::now();
}
performance perfinit;
void perf(string p){
    if(p==""){perfinit.pclock=std::chrono::steady_clock::now();return;}
    perfinit.p(p.c_str());
} 
performance perfinit1;
void perf1(string p){
    if(p==""){perfinit1.pclock=std::chrono::steady_clock::now();return;}
    perfinit1.p(p.c_str());
} 
performance perfinit2;
void perf2(string p){
    if(p==""){perfinit2.pclock=std::chrono::steady_clock::now();return;}
    perfinit2.p(p.c_str());
} 


 
namespace fs = std::filesystem;
#ifdef _WIN32
extern "C" __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void* hModule, char* lpFilename, unsigned long nSize);
#endif

void appwdir() {
    fs::path exePath;
#ifdef _WIN32
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    exePath = fs::path(path).parent_path();
#elif __linux__
    char path[1024];
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (count != -1) {
        path[count] = '\0';
        exePath = fs::path(path).parent_path();
    }
#endif
    std::cout << "Executable Directory: " << exePath << std::endl;
    fs::current_path(exePath);  // Muda diretamente o diretório de trabalho
    std::cout << "Current Working Directory: " << fs::current_path() << std::endl; 
}



#ifndef onlystd

#if defined(__linux__) 
#include <X11/Xlib.h> 
#include <X11/Xatom.h>
#endif
void copy_html(const std::string& html)
{
#if defined(_WIN32)
// ---------------------- WINDOWS -----------------------------
    std::string header =
        "Version:0.9\r\n"
        "StartHTML:00000000\r\n"
        "EndHTML:00000000\r\n"
        "StartFragment:00000000\r\n"
        "EndFragment:00000000\r\n";

    std::string full =
        header +
        "<html><body><!--StartFragment-->" +
        html +
        "<!--EndFragment--></body></html>";

    auto start_html = header.size();
    auto start_frag = full.find("<!--StartFragment-->");
    auto end_frag   = full.find("<!--EndFragment-->") + strlen("<!--EndFragment-->");
    auto end_html   = full.size();

    char hdr[200];
    sprintf(hdr,
        "Version:0.9\r\n"
        "StartHTML:%08d\r\n"
        "EndHTML:%08d\r\n"
        "StartFragment:%08d\r\n"
        "EndFragment:%08d\r\n",
        (int)start_html, (int)end_html, (int)start_frag, (int)end_frag);

    memcpy(&full[0], hdr, strlen(hdr));

    OpenClipboard(nullptr);
    EmptyClipboard();

    UINT fmt = RegisterClipboardFormatA("HTML Format");
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, full.size() + 1);
    memcpy(GlobalLock(hMem), full.c_str(), full.size() + 1);
    GlobalUnlock(hMem);

    SetClipboardData(fmt, hMem);
    CloseClipboard();

#elif defined(__APPLE__)
// ---------------------- MACOS -------------------------------
    @autoreleasepool {
        NSPasteboard* pb = [NSPasteboard generalPasteboard];
        [pb clearContents];
        NSString* nsHtml = [NSString stringWithUTF8String:html.c_str()];
        [pb setString:nsHtml forType:NSHTMLPboardType];
    }

#elif defined(__linux__)
// ---------------------- LINUX / X11 -------------------------
//  #if defined(__linux__)
// #include <X11/Xlib.h>
// #include <X11/Xatom.h>

// void copy_html(const std::string& html)
std::thread([html]() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;

    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
                                     0, 0, 1, 1, 0, 0, 0);

    Atom clip     = XInternAtom(dpy, "CLIPBOARD", False);
    Atom targets  = XInternAtom(dpy, "TARGETS", False);
    Atom htmlAtom = XInternAtom(dpy, "text/html", False);
    Atom utf8     = XInternAtom(dpy, "UTF8_STRING", False);

    XSetSelectionOwner(dpy, clip, win, CurrentTime);

    // Event loop for selection requests
    for (;;) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == SelectionRequest) {
            XSelectionRequestEvent* req = &ev.xselectionrequest;

            XEvent res;
            memset(&res, 0, sizeof(res));
            res.xselection.type      = SelectionNotify;
            res.xselection.display   = req->display;
            res.xselection.requestor = req->requestor;
            res.xselection.selection = req->selection;
            res.xselection.time      = req->time;
            res.xselection.target    = req->target;
            res.xselection.property  = None;

            if (req->target == targets) {
                Atom list[2] = { htmlAtom, utf8 };
                XChangeProperty(dpy, req->requestor, req->property,
                                XA_ATOM, 32, PropModeReplace,
                                (unsigned char*)list, 2);
                res.xselection.property = req->property;
            }
            else if (req->target == htmlAtom) {
                XChangeProperty(dpy, req->requestor, req->property,
                                htmlAtom, 8, PropModeReplace,
                                (unsigned char*)html.c_str(),
                                html.size());
                res.xselection.property = req->property;
            }

            XSendEvent(dpy, req->requestor, False, 0, &res);
            XFlush(dpy);
        }
        else if (ev.type == SelectionClear) {
            break; // lost clipboard ownership
        }
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}).detach();

#else
    #error Unsupported platform
#endif
}
#endif
