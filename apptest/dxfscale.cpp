// g++ dxfscale.cpp ../common/general.cpp -I../common -o dxfscale -std=c++20
/*
remove_dim_scale.cpp

Simple C++ tool that edits an ASCII DXF (R12/R13/R14/R15-compatible) file to:
 - set header $DIMSCALE (overall dimension scale) to 1.0 if present
 - set DIMSTYLE table entries' DIMLFAC (group code 144) to 1.0 if present
 - remove per-entity dimvar overrides stored in 1002 "{...}" chunks (commonly used
   to store per-dimension overrides)

Usage: ./remove_dim_scale input.dxf output.dxf

This is a line-oriented tolerant editor (it operates on group-code/value pairs).
It is conservative: if it cannot confidently parse a pair it will copy it unchanged.

Limitations:
 - Only handles ASCII DXF files (not binary)
 - Assumes group codes and values are on separate lines (standard DXF ASCII)
 - Does not attempt to interpret binary or embedded objects or write handles
 - Attempts to be robust for common DXF R12..R15 variants

Author: ChatGPT (GPT-5 Thinking mini)
*/

#include <bits/stdc++.h>
using namespace std;

static string trim(const string &s) {
    size_t a = 0; while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size(); while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input.dxf output.dxf\n";
        return 1;
    }
    string inpath = argv[1];
    string outpath = argv[2];

    ifstream fin(inpath);
    if (!fin) { cerr << "Failed to open input: " << inpath << '\n'; return 2; }

    // Read all lines (preserve exact lines for output)
    vector<string> lines;
    string line;
    while (std::getline(fin, line)) lines.push_back(line);
    fin.close();

    vector<string> out;
    out.reserve(lines.size());

    // We'll process the file as pairs: group-code line, value line
    // but be robust if file ends unexpectedly.
    bool in_header_section = false;
    bool in_tables_section = false;
    bool in_dimstyle_table = false;
    bool skipping_1002_block = false; // when true, skip lines until matching 1002 '}' encountered

    for (size_t i = 0; i < lines.size();) {
        string code = trim(lines[i]);
        string value = (i + 1 < lines.size()) ? lines[i+1] : string();

        // If currently skipping a 1002 override block, check for end condition
        if (skipping_1002_block) {
            // We skip the current pair (code+value). Look for 1002 with value "}" to stop.
            if (code == "1002") {
                string vtrim = trim(value);
                if (!vtrim.empty() && (vtrim == "}" || vtrim == "\}")) {
                    // end of override block: skip this closing line too and resume
                    skipping_1002_block = false;
                    i += 2; // move past this pair
                    continue;
                }
            }
            i += 2; // skip
            continue;
        }

        // Detect section changes by looking for SECTION/ENDSEC pairs and TABLES/TABLE
        if (code == "0" && trim(value) == "SECTION") {
            // copy the SECTION pair
            out.push_back(lines[i]); out.push_back(lines[i+1]); i += 2;
            // next pair should be 2 <section name>
            if (i+1 < lines.size()) {
                string c2 = trim(lines[i]); string v2 = trim(lines[i+1]);
                if (c2 == "2") {
                    string secname = v2;
                    if (secname == "HEADER") { in_header_section = true; in_tables_section = false; in_dimstyle_table = false; }
                    else { in_header_section = false; }
                    if (secname == "TABLES") { in_tables_section = true; } else { in_tables_section = false; }
                }
                // copy them as well
                if (i < lines.size()) { out.push_back(lines[i]); }
                if (i+1 < lines.size()) { out.push_back(lines[i+1]); }
                i += 2;
            }
            continue;
        }

        if (code == "0" && trim(value) == "ENDSEC") {
            // leave any table/dimstyle state
            in_header_section = false;
            in_tables_section = false;
            in_dimstyle_table = false;
            out.push_back(lines[i]); if (i+1 < lines.size()) out.push_back(lines[i+1]); i += 2; continue;
        }

        // Detect TABLE/DIMSTYLE start and end inside TABLES section
        if (in_tables_section && code == "0" && trim(value) == "TABLE") {
            out.push_back(lines[i]); out.push_back(lines[i+1]); i += 2;
            // next group likely 2 DIMSTYLE
            if (i+1 < lines.size()) {
                string c2 = trim(lines[i]); string v2 = trim(lines[i+1]);
                out.push_back(lines[i]); out.push_back(lines[i+1]);
                i += 2;
                if (c2 == "2" && v2 == "DIMSTYLE") {
                    in_dimstyle_table = true;
                }
            }
            continue;
        }
        if (in_dimstyle_table && code == "0" && trim(value) == "ENDTAB") {
            in_dimstyle_table = false;
            out.push_back(lines[i]); out.push_back(lines[i+1]); i += 2; continue;
        }

        // Remove per-entity dimvar overrides stored in 1002 "{...}" chunks.
        if (code == "1002") {
            string vtrim = trim(value);
            if (!vtrim.empty() && (vtrim.front() == '{' || vtrim == "{")) {
                // start skipping until matching 1002 with '}'
                skipping_1002_block = true;
                i += 2; // skip this opening pair
                continue;
            }
        }

        // In HEADER section: handle $DIMSCALE variable (group code 9 then 40)
        if (in_header_section && code == "9") {
            string varname = trim(value);
            // copy the 9 line and lookahead to next pair
            out.push_back(lines[i]);
            if (i+1 < lines.size()) {
                out.push_back(lines[i+1]);
            }
            i += 2;

            if (varname == "$DIMSCALE") {
                // next pair should be group code 40 with a value: we need to set it to 1.0
                if (i+1 < lines.size()) {
                    string next_code = trim(lines[i]);
                    // write modified value
                    if (next_code == "40") {
                        out.push_back(lines[i]); // code line
                        out.push_back(string("1.0"));
                        i += 2;
                    } else {
                        // not expected; just continue without modification
                    }
                }
            }
            continue;
        }

        // In DIMSTYLE table records, set DIMLFAC (group code 144) to 1.0
        if (in_dimstyle_table) {
            if (code == "144") {
                // replace value with 1.0
                out.push_back(lines[i]); // code line (144)
                out.push_back(string("1.0"));
                i += 2; continue;
            }
        }

        // Default: copy the pair as-is
        out.push_back(lines[i]);
        if (i+1 < lines.size()) out.push_back(lines[i+1]);
        i += 2;
    }

    // Write output
    ofstream fout(outpath);
    if (!fout) { cerr << "Failed to open output: " << outpath << '\n'; return 3; }
    for (auto &s : out) fout << s << '\n';
    fout.close();

    cerr << "Done. Wrote: " << outpath << '\n';
    return 0;
}


