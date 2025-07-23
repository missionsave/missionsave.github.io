#ifndef GENERAL_HPP
#define GENERAL_HPP

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

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
#include <sys/ioctl.h>
#include <unistd.h>

int get_terminal_width();

void go_up_and_clear_line(const std::string& last_text) ;
#define go_up {go_up_and_clear_line(cotmlastoutput);}


void cotm_function(const std::string &args_names, const std::string &args_values);
#define cotm(...) cotm_function(#__VA_ARGS__, get_args_string(__VA_ARGS__)); 

// #define cotm(...) cotm_function(#__VA_ARGS__, get_args_string(__VA_ARGS__)); 

// #define cotm(...) {scotmup=0; cotmstd(#__VA_ARGS__); }
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

void appwdir();

void trim(std::string& str);

#endif // GENERAL_HPP