// load_app_font.hpp / .cpp
// C++17; Windows: link Gdi32.lib; Linux: link fontconfig (e.g., -lfontconfig)

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <limits>
#include <algorithm>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#elif defined(__linux__)
  #include <unistd.h>
  #include <limits.h>
  #include <fontconfig/fontconfig.h>
#else
  #error "Only Windows and Linux supported here."
#endif

 

// Read entire file into memory
std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open font file: " + path);

    f.seekg(0, std::ios::end);
    std::streamoff sz = f.tellg();
    if (sz <= 0 || sz == std::streamoff(-1))
        throw std::runtime_error("Invalid font file size (tellg failed)");

    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    if (!f.read(reinterpret_cast<char*>(buf.data()), sz))
        throw std::runtime_error("Failed to read font file data");
    
    return buf;
}

 uint16_t be16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] << 8 | p[1]);
}
 uint32_t be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

// Convert UTF‑16BE to UTF‑8 (minimal, handles BMP)
 std::string utf16be_to_utf8(const uint8_t* data, size_t bytes) {
    std::string out;
    out.reserve(bytes); // rough
    for (size_t i = 0; i + 1 < bytes; ) {
        uint16_t code = be16(data + i);
        i += 2;
        // handle surrogate pairs
        if (code >= 0xD800 && code <= 0xDBFF) {
            if (i + 1 >= bytes) break;
            uint16_t low = be16(data + i);
            if (low < 0xDC00 || low > 0xDFFF) continue;
            i += 2;
            uint32_t cp = 0x10000u + (((code - 0xD800u) << 10) | (low - 0xDC00u));
            if (cp <= 0x10FFFFu) {
                out.push_back(char(0xF0 | ((cp >> 18) & 0x07)));
                out.push_back(char(0x80 | ((cp >> 12) & 0x3F)));
                out.push_back(char(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(char(0x80 | (cp & 0x3F)));
            }
            continue;
        }
        uint32_t cp = code;
        if (cp < 0x80) {
            out.push_back(char(cp));
        } else if (cp < 0x800) {
            out.push_back(char(0xC0 | (cp >> 6)));
            out.push_back(char(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(char(0xE0 | (cp >> 12)));
            out.push_back(char(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(char(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

// Extract family (nameID=1) from TTF/OTF 'name' table; fallback to full name (nameID=4)
 std::string extract_ttf_family(const std::vector<uint8_t>& buf) {
    if (buf.size() < 12) throw std::runtime_error("Font too small");
    const uint8_t* p = buf.data();
    uint32_t sfnt = be32(p + 0);
    // Accept 'OTTO' (CFF/OTF), 0x00010000 (TTF), 'true' (rare) or 'typ1' (very rare)
    if (!(sfnt == 0x00010000u || sfnt == 0x4F54544Fu /*OTTO*/ || sfnt == 0x74727565u /*true*/ || sfnt == 0x74797031u /*typ1*/))
        throw std::runtime_error("Unrecognized sfnt");
    uint16_t numTables = be16(p + 4);
    if (buf.size() < 12 + size_t(numTables) * 16) throw std::runtime_error("Corrupt table directory");
    const uint8_t* dir = p + 12;
    uint32_t nameOffset = 0, nameLength = 0;
    for (uint16_t i = 0; i < numTables; ++i) {
        const uint8_t* e = dir + i * 16;
        uint32_t tag = be32(e + 0);
        if (tag == 0x6E616D65u) { // 'name'
            nameOffset = be32(e + 8);
            nameLength = be32(e + 12);
            break;
        }
    }
    if (nameOffset == 0 || nameOffset + nameLength > buf.size())
        throw std::runtime_error("'name' table not found");

    const uint8_t* nt = p + nameOffset;
    if (nameLength < 6) throw std::runtime_error("Corrupt 'name' table");
    uint16_t format = be16(nt + 0);
    uint16_t count  = be16(nt + 2);
    uint16_t stringOffset = be16(nt + 4);
    if (nameLength < 6 + count * 12) throw std::runtime_error("Corrupt 'name' records");

    auto get_string = [&](uint16_t len, uint16_t off, bool utf16) -> std::string {
        uint32_t absOff = nameOffset + stringOffset + off;
        if (absOff + len > buf.size()) return {};
        const uint8_t* s = p + absOff;
        if (utf16) return utf16be_to_utf8(s, len);
        // Mac Roman fallback: treat as Latin-1-ish for simplicity
        return std::string(reinterpret_cast<const char*>(s), reinterpret_cast<const char*>(s) + len);
    };

    std::string bestFamily, fullName;

    for (uint16_t i = 0; i < count; ++i) {
        const uint8_t* rec = nt + 6 + i * 12;
        uint16_t platformID = be16(rec + 0);
        uint16_t encodingID = be16(rec + 2);
        uint16_t /*languageID*/ _lang = be16(rec + 4);
        uint16_t nameID     = be16(rec + 6);
        uint16_t length     = be16(rec + 8);
        uint16_t offset     = be16(rec + 10);

        bool isUnicode = (platformID == 0) ||
                         (platformID == 3 && (encodingID == 1 || encodingID == 10));
        bool isMacRoman = (platformID == 1 && encodingID == 0);

        if (nameID == 1 && isUnicode) {
            auto s = get_string(length, offset, true);
            if (!s.empty()) bestFamily = s;
        } else if (nameID == 4 && isUnicode) {
            auto s = get_string(length, offset, true);
            if (!s.empty()) fullName = s;
        } else if (nameID == 1 && bestFamily.empty() && isMacRoman) {
            auto s = get_string(length, offset, false);
            if (!s.empty()) bestFamily = s;
        } else if (nameID == 4 && fullName.empty() && isMacRoman) {
            auto s = get_string(length, offset, false);
            if (!s.empty()) fullName = s;
        }
    }

    if (!bestFamily.empty()) return bestFamily;
    if (!fullName.empty()) return fullName;
    throw std::runtime_error("Could not extract font family name");
}

 std::string exe_dir() {
#ifdef _WIN32
    wchar_t wpath[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, wpath, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) throw std::runtime_error("GetModuleFileNameW failed");
    std::wstring ws(wpath, wpath + n);
    size_t pos = ws.find_last_of(L"\\/");
    std::wstring wdir = (pos == std::wstring::npos) ? L"." : ws.substr(0, pos);
    int len = WideCharToMultiByte(CP_UTF8, 0, wdir.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wdir.c_str(), -1, out.data(), len, nullptr, nullptr);
    return out;
#elif defined(__linux__)
    char path[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (n <= 0) throw std::runtime_error("readlink /proc/self/exe failed");
    path[n] = '\0';
    std::string s(path);
    size_t pos = s.find_last_of("/\\");
    return (pos == std::string::npos) ? std::string(".") : s.substr(0, pos);
#endif
}

 std::string join_path(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    char sep =
#ifdef _WIN32
        '\\';
#else
        '/';
#endif
    if (a.back() == '/' || a.back() == '\\') return a + b;
    return a + sep + b;
}

 

// Returns the face (family) name, e.g. "DejaVu Sans"
std::string load_app_font(const std::string& filename) {
    // Build absolute path next to executable
    const std::string path = join_path(exe_dir(), filename);

    // Read font and extract family name first (so we can return it even if registration is a no-op)
    const auto bytes = read_file(path);
    const std::string family = extract_ttf_family(bytes);

    // Register font for current process
#ifdef _WIN32
    // Prefer file-based registration (simpler lifetime). FR_PRIVATE keeps it process-local.
    int added = 0;
    {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
        std::wstring wpath(wlen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath.data(), wlen);
        // MultiByteToWideChar writes trailing null; ensure string is null-terminated in c_str
        added = AddFontResourceExW(wpath.c_str(), FR_PRIVATE, nullptr);
    }
    // 'added' can be 0 if already loaded or registration failed. We still return the family name.
#elif defined(__linux__)
    FcInit();
    FcConfig* cfg = FcConfigGetCurrent();
    if (cfg) {
        FcBool ok = FcConfigAppFontAddFile(cfg, reinterpret_cast<const FcChar8*>(path.c_str()));
        if (ok) {
            FcConfigBuildFonts(cfg);
        }
        // If it fails, the font might already be visible or fontconfig not available; we still return the name.
    }
#endif

    return family;
}
