// font_loader.cpp  static std::string load_app_font(const std::string& filename)
#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <CoreFoundation/CoreFoundation.h>
  #include <CoreText/CoreText.h>
  #include <limits.h>
  #include <unistd.h>
#else // Linux/Unix
  #include <unistd.h>
  #include <limits.h>
  #include <fontconfig/fontconfig.h>
#endif

// namespace appfont {

static std::string dirname_of(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? std::string(".") : path.substr(0, pos);
}

static std::string join_path(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
#if defined(_WIN32)
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    if (a.back() == '/' || a.back() == '\\') return a + b;
    return a + sep + b;
}

static std::string executable_path() {
#if defined(_WIN32)
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    return std::string(buf, (len ? len : 0));
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size); // get required size
    std::vector<char> tmp(size + 1);
    if (_NSGetExecutablePath(tmp.data(), &size) != 0) return {};
    // realpath to resolve symlinks
    char resolved[PATH_MAX];
    if (realpath(tmp.data(), resolved)) return std::string(resolved);
    return std::string(tmp.data());
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len > 0) { buf[len] = '\0'; return std::string(buf); }
    return {};
#endif
}

static std::string app_dir() {
    return dirname_of(executable_path());
}

// Try common locations relative to the executable
static std::vector<std::string> candidate_paths(const std::string& filename) {
    std::vector<std::string> paths;
    auto base = app_dir();
    paths.push_back(join_path(base, filename));                 // alongside executable
#if defined(__APPLE__)
    // In a bundled .app: MyApp.app/Contents/MacOS/<exe>
    // Resources are at:  MyApp.app/Contents/Resources/
    paths.push_back(join_path(join_path(base, "../Resources"), filename));
    paths.push_back(join_path(join_path(base, "../../Resources"), filename)); // in case of alternate layouts
#endif
    paths.push_back(join_path(join_path(base, "fonts"), filename)); // ./fonts/<file>
    return paths;
}

// Platform-specific registration
static bool register_font_file(const std::string& absPath) {
#if defined(_WIN32)
    int added = AddFontResourceExA(absPath.c_str(), FR_PRIVATE, 0);
    if (added > 0) {
        SendMessageA(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
        return true;
    }
    return false;

#elif defined(__APPLE__)
    CFStringRef cfPath = CFStringCreateWithCString(nullptr, absPath.c_str(), kCFStringEncodingUTF8);
    if (!cfPath) return false;
    CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath, kCFURLPOSIXPathStyle, false);
    CFRelease(cfPath);
    if (!url) return false;
    CFErrorRef error = nullptr;
    bool ok = CTFontManagerRegisterFontsForURL(url, kCTFontManagerScopeProcess, &error);
    if (error) CFRelease(error);
    CFRelease(url);
    return ok;

#else
    // Linux/Unix with Fontconfig
    if (!FcInit()) return false;
    FcConfig* cfg = FcConfigGetCurrent();
    if (!cfg) cfg = FcInitLoadConfigAndFonts();
    FcBool ok = FcConfigAppFontAddFile(cfg, reinterpret_cast<const FcChar8*>(absPath.c_str()));
    // Optional: rebuild font set for immediate availability
    FcConfigBuildFonts(cfg);
    return ok == FcTrue;
#endif
}

// Load font by filename located in app dir (or typical subdirs). Returns absolute path if loaded.
std::string load_app_font(const std::string& filename) {
    for (const auto& p : candidate_paths(filename)) {
        // Quick existence check
        FILE* f = std::fopen(p.c_str(), "rb");
        if (!f) continue;
        std::fclose(f);
        if (register_font_file(p)) return p;
    }
    return {};
}

// } // namespace appfont
