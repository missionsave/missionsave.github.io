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

#include "general.hpp"

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
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> tmp(size + 1);
    if (_NSGetExecutablePath(tmp.data(), &size) != 0) return {};
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

static std::vector<std::string> candidate_paths(const std::string& filename) {
    std::vector<std::string> paths;
    auto base = app_dir();
    paths.push_back(join_path(base, filename));
#if defined(__APPLE__)
    paths.push_back(join_path(join_path(base, "../Resources"), filename));
    paths.push_back(join_path(join_path(base, "../../Resources"), filename));
#endif
    paths.push_back(join_path(join_path(base, "fonts"), filename));
    return paths;
}

#if defined(_WIN32)
static std::wstring utf8_to_wstring(const std::string& utf8) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring wstr(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wstr[0], wlen);
    return wstr;
}
#endif

static bool register_font_file(const std::string& absPath) {
#if defined(_WIN32)
    std::wstring path = utf8_to_wstring(absPath);
    int added = AddFontResourceExW(path.c_str(), FR_PRIVATE, 0);
    if (added > 0) {
        SendMessageW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
        sleepms(100);
        return true;
    }
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
    if (!FcInit()) return false;
    FcConfig* cfg = FcConfigGetCurrent();
    if (!cfg) cfg = FcInitLoadConfigAndFonts();
    FcBool ok = FcConfigAppFontAddFile(cfg, reinterpret_cast<const FcChar8*>(absPath.c_str()));
    FcConfigBuildFonts(cfg);
    return ok == FcTrue;
#endif
}

std::string load_app_font(const std::string& filename) {
    for (const auto& p : candidate_paths(filename)) {
        FILE* f = std::fopen(p.c_str(), "rb");
        if (!f) continue;
        std::fclose(f);

        if (!register_font_file(p))
            continue;

#if defined(_WIN32)
        std::wstring widePath = utf8_to_wstring(p);
        wchar_t name[LF_FACESIZE] = {};
        DWORD size = sizeof(name);

        BOOL ok = GetFontResourceInfoW(widePath.c_str(), &size, name, 1);
        if (ok && name[0]) {
            char converted[LF_FACESIZE];
            WideCharToMultiByte(CP_UTF8, 0, name, -1, converted, sizeof(converted), nullptr, nullptr);
            return std::string(converted);
        }
        auto slash = p.find_last_of("/\\");
        auto dot = p.find_last_of(".");
        return p.substr(slash + 1, dot - slash - 1);

#elif defined(__APPLE__)
        CFStringRef cfPath = CFStringCreateWithCString(nullptr, p.c_str(), kCFStringEncodingUTF8);
        CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath, kCFURLPOSIXPathStyle, false);
        CFRelease(cfPath);
        CTFontDescriptorRef descriptor = CTFontManagerCreateFontDescriptorFromDataURL(url);
        CFRelease(url);
        if (!descriptor) return filename;
        CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, nullptr);
        CFRelease(descriptor);
        if (!font) return filename;
        CFStringRef name = CTFontCopyPostScriptName(font);
        CFRelease(font);
        if (!name) return filename;
        char buffer[256];
        CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        CFRelease(name);
        return std::string(buffer);

#else
        auto slash = p.find_last_of("/\\");
        auto dot = p.find_last_of(".");
        return p.substr(slash + 1, dot - slash - 1);
#endif
    }

    return {};
}
