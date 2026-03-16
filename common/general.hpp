#ifndef GENERAL_HPP
#define GENERAL_HPP 1

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <atomic>


typedef std::vector<char> vchar;
typedef std::vector<std::vector<char>> vvchar;
typedef std::vector<unsigned char> vuchar;
typedef std::vector<std::vector<unsigned char>> vvuchar;
typedef std::vector<unsigned short> vushort;
typedef std::vector<std::vector<unsigned short>> vvushort;
typedef std::vector<float> vfloat;
typedef std::vector<std::vector<float>> vvfloat;
typedef std::vector<bool> vbool;
typedef std::vector<std::string> vstring;
typedef std::vector<int> vint;
typedef std::vector<std::vector<int>> vvint;

#ifndef RGB
#define RGB(r, g, b) ((int)(((unsigned char)(r) | ((unsigned short)((unsigned char)(g)) << 8)) | (((unsigned int)(unsigned char)(b)) << 16)))
#endif

#define lop(var,from,to)for(int var=(from);var<(to);var++)
#define pausa getchar();
#define rprintf( ... )({ char buffer[256000]; sprintf (buffer, __VA_ARGS__);  buffer; })

#define mathRound(n,d) ({float _pow10=pow(10,d); floorf((n) * _pow10 + 0.5) / _pow10;})

#define sortGiveIndexAsc(res,vals,sz,...)std::vector<int> res##I(sz); std::size_t nidx(0);    std::generate(std::begin(res##I), std::end(res##I), [&]{ return nidx++; });    std::sort(  std::begin(res##I),   std::end(res##I), [&](int i1, int i2) { return vals[i1] < vals[i2]; } );
#define sortGiveIndexDesc(res,vals,sz,...)std::vector<int> res##I(sz); std::size_t nidx(0);    std::generate(std::begin(res##I), std::end(res##I), [&]{ return nidx++; });    std::sort(  std::begin(res##I),   std::end(res##I), [&](int i1, int i2) { return vals[i1] > vals[i2]; } );

template<typename T>
int indexOf(const std::vector<T>& vec, const T& value) {
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it != vec.end()) {
        return std::distance(vec.begin(), it);
    }
    return -1;  // Indicates value not found
}
template<typename T>
int indexOfMax(const std::vector<T>& vec) {
    auto it = std::max_element(vec.begin(), vec.end());
    if (it != vec.end()) {
        return std::distance(vec.begin(), it);
    }
    return -1;  // Indicates empty vector
} 
template<typename T>
int indexOfMin(const std::vector<T>& vec) {
    auto it = std::min_element(vec.begin(), vec.end());
    if (it != vec.end()) {
        return std::distance(vec.begin(), it);
    }
    return -1;  // Indicates empty vector
}

// #define go_up(n) { \
//     for (int i = 0; i < n; i++) { \
//         printf("\033[A");   /* Move up one logical line */ \
//         printf("\r\033[2K"); /* Clear the entire line */ \
//     } \
//     fflush(stdout); \
// }

extern std::string cotmlastoutput;
extern bool scotmup;
#include <iostream>
#include <string>
#if defined(__linux__)
#include <sys/ioctl.h>
#endif
#include <unistd.h>


int get_terminal_width();

void go_up_and_clear_line(const std::string& last_text) ;
#define go_up {go_up_and_clear_line(cotmlastoutput);}


void cotm_function(const std::string &args_names, const std::string &args_values);
#define cotm(...) cotm_function(#__VA_ARGS__, get_args_string(__VA_ARGS__)); 

// #define cotm(...) cotm_function(#__VA_ARGS__, get_args_string(__VA_ARGS__)); 

// #define cotm(...) {scotmup=0; cotmstd(#__VA_ARGS__); }

#define cotmupset {if(scotmup==0)cout<<endl;}
#define cotmup {scotmup=1;}
// Overload operator<< for std::vector<T>
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i + 1 < vec.size()) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}
template <typename... Args>
std::string get_args_string(Args&&... args) {
    std::stringstream ss;
    ((ss << args << "⟁ "), ...); // Fold expression to format values
    std::string result = ss.str();
    if (!result.empty()) result.pop_back(); // Remove trailing comma
    return result;
}

std::vector<std::string> split(const std::string &s, const std::string &delimiter = ",");
void sleepms(int ms);

std::vector<int> splitToVint(const std::string& str, std::string delimiter);

struct lnmutex {
    std::condition_variable lua_funcs_cv;
    bool lua_funcs_locked = false;
    std::mutex mutex_;
    std::string name_;
    bool debug_;
    bool islock = false;
    lnmutex(const std::string& name, bool debug = false);
    void lock();
    void unlock();
    void exit();
    std::string getName() const; 
};
 
struct nmutex {
    std::mutex mutex_;
    std::string name_;
    std::atomic<int> waiting_threads{0};
    bool debug_;
    bool islock = false;
    nmutex(const std::string& name, bool debug = false);
    void lock();
    void unlock();
    bool try_lock();
    std::string getName() const;
};

void replace_All(std::string & data, std::string toSearch, std::string replaceStr);


std::string joinv(const std::vector<int>& vec, std::string delimiter);

bool Contains(std::string text, std::string target = "world");



struct combR{
    bool cached=false;
    std::vector<unsigned int*> cache;
    std::vector<int> ranges;
    std::vector<int> restoR;
    unsigned long long range;
    int k;//=ranges.size();
    combR(){}
    ~combR(){}
    combR(int n,int K,bool tocache=false);
    combR(std::vector <int> Ranges,bool tocache=false);
    std::vector<int> toComb(unsigned int csn);
    void toComb(unsigned int csn,std::vector<int>&res);
    vvint toCombV(vint &hist);
    void toComb(int* res,unsigned int csn);
    unsigned long long combNumCombinIrregular();
    unsigned long long toCsn(int *comb);
    unsigned long long toCsn(vint comb); 
	
// 	vvint comb_bin_sorted1(){
// 		vector<vector<int>> combs(range);
//         for (int i = 0; i < range; i++) {
//             combs[i] = toComb(i);
//         }
//         sort(combs.begin(), combs.end(), [](vector<int> a, vector<int> b) {
//             int count_a = count(a.begin(), a.end(), 1);
//             int count_b = count(b.begin(), b.end(), 1);
//             return count_a < count_b || (count_a == count_b && a < b);
//         });
// 		return combs;
// 	}
int countMatches(const std::vector<int>& a, const std::vector<int>& b) {
    int count = 0;
    for (int i = 0; i < a.size(); i++) {
        if (a[i] == b[i]) {
            count++;
        }
    }
    return count;
}

// vector<vector<int>> comb_bin_sorted() {
//     vector<vector<int>> combs;
//     for (int i = 0; i < 16; i++) {
//         combs.push_back(toComb(i));
//     }
//     sort(combs.begin(), combs.end(), [&](const vector<int>& a, const vector<int>& b) {
//         if (countMatches(a, b) > 0) {
//             return countMatches(a, b) > countMatches(b, a);
//         } else {
//             return a < b;
//         }
//     });
//     return combs;
// }
};


void perf(std::string p="");
void perf1(std::string p="");
void perf2(std::string p="");

void appwdir();

void trim(std::string& str);

#define func_fps(_fps,thecontent){  \
    static std::chrono::steady_clock::time_point last_event = std::chrono::steady_clock::now(); \
    auto now = std::chrono::steady_clock::now();\
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event).count();\
    if(elapsed>(1000.0/(_fps))){\
        {thecontent} \
        last_event = now;\
    }}



void copy_html(const std::string& html);



#include <vector>
#include <bitset>

inline bool hasDuplicates(const std::vector<int>& v) {
    static std::bitset<1000001> seen; // adjust size
    seen.reset();

    for (int x : v) {
        if (seen[x]) return true;
        seen[x] = true;
    }
    return false;
}



#include <filesystem>
#include <ctime>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <sys/stat.h>
#endif

inline std::time_t get_last_access(const std::filesystem::path& p) {
#if defined(_WIN32)

    HANDLE h = CreateFileW(
        p.wstring().c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (h == INVALID_HANDLE_VALUE)
        return 0;

    FILETIME atime;
    if (!GetFileTime(h, NULL, &atime, NULL)) {
        CloseHandle(h);
        return 0;
    }
    CloseHandle(h);

    ULARGE_INTEGER ull;
    ull.LowPart  = atime.dwLowDateTime;
    ull.HighPart = atime.dwHighDateTime;

    // Convert Windows FILETIME (100ns since 1601) → Unix time_t (seconds since 1970)
    return static_cast<std::time_t>((ull.QuadPart - 116444736000000000ULL) / 10000000ULL);

#elif defined(__APPLE__) || defined(__linux__)

    struct stat s;
    if (stat(p.c_str(), &s) != 0)
        return 0;

    return s.st_atime;   // last access time (POSIX)

#else
    return 0; // Unsupported platform
#endif
}



#include <string>
#include <ctime>
#include <filesystem>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

inline bool set_access_time(const std::filesystem::path& path, std::time_t new_atime) {
#if defined(_WIN32)
    // Convert time_t → Windows FILETIME (100ns intervals since 1601)
    LONGLONG ll = Int32x32To64(new_atime, 10000000) + 116444736000000000LL;

    FILETIME ft;
    ft.dwLowDateTime  = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);

    HANDLE h = CreateFileW(
        path.wstring().c_str(),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (h == INVALID_HANDLE_VALUE)
        return false;

    // Set only access time, keep creation & modification unchanged
    FILETIME dummy = {0,0};
    BOOL ok = SetFileTime(h, NULL, &ft, NULL);

    CloseHandle(h);
    return ok;

#else
    // Linux / macOS: use utimensat()
    struct timespec times[2];

    // atime
    times[0].tv_sec  = new_atime;
    times[0].tv_nsec = 0;

    // keep mtime unchanged
    times[1].tv_nsec = UTIME_OMIT;

    return utimensat(AT_FDCWD, path.c_str(), times, 0) == 0;
#endif
}



#endif // GENERAL_HPP