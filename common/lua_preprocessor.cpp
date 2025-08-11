#include <lua.hpp>
#include <vector>
#include <string>
#include <regex>

using namespace std;

class LuaRequireOverride {
    vector<string> target_funcs;
    regex pattern;
    
    void build_regex() {
        string pattern_str = "(";
        for (size_t i = 0; i < target_funcs.size(); i++) {
            if (i != 0) pattern_str += "|";
            pattern_str += target_funcs[i];
        }
        pattern_str += ")\\s*\\(\\s*([a-zA-Z_][a-zA-Z0-9_]*)\\s*,";
        pattern = regex(pattern_str);
    }
    
public:
    LuaRequireOverride(const vector<string>& funcs) : target_funcs(funcs) {
        build_regex();
    }

    static int custom_loader(lua_State* L) {
        // Get our class instance from registry
        lua_getfield(L, LUA_REGISTRYINDEX, "__lua_require_override");
        auto* self = (LuaRequireOverride*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        
        const char* modname = luaL_checkstring(L, 1);
        
        // 1. Find the file using package.searchpath
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "searchpath");
        lua_pushvalue(L, 1);  // modname
        lua_call(L, 1, 1);
        
        if (lua_isnil(L, -1)) {
            return 1;  // Not found
        }
        
        const char* filename = lua_tostring(L, -1);
        
        // 2. Read file content
        FILE* f = fopen(filename, "rb");
        if (!f) {
            lua_pushnil(L);
            lua_pushfstring(L, "cannot open %s", filename);
            return 2;
        }
        
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        string buf(len, '\0');
        fread(&buf[0], 1, len, f);
        fclose(f);
        
        // 3. Apply transformations
        string result;
        result.reserve(buf.size() * 1.5); // Reserve extra space
        
        smatch matches;
        string::const_iterator search_start = buf.cbegin();
        
        while (regex_search(search_start, buf.cend(), matches, self->pattern)) {
            // Copy prefix
            result.append(search_start, matches[0].first);
            
            // Rebuild the function call with quoted first arg
            result.append(matches[1].str());  // function name
            result.append("(\"");
            result.append(matches[2].str());  // first arg
            result.append("\",");
            
            search_start = matches[0].second;
        }
        
        // Copy remaining content
        result.append(search_start, buf.cend());
        
        // 4. Load transformed chunk
        if (luaL_loadbuffer(L, result.c_str(), result.size(), filename) != LUA_OK) {
            return lua_error(L);
        }
        
        // 5. Call the chunk
        lua_call(L, 0, 1);
        return 1;
    }

    static void override_require(lua_State* L, const vector<string>& funcs) {
        // Create and store our processor
        auto* processor = new LuaRequireOverride(funcs);
        
        // Store in registry
        lua_pushlightuserdata(L, processor);
        lua_setfield(L, LUA_REGISTRYINDEX, "__lua_require_override");
        
        // Get package.loaders table
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "searchers");
        lua_remove(L, -2);  // remove package table
        
        // Create new loader function
        lua_pushcfunction(L, custom_loader);
        
        // Insert at position 2 (right after preloader)
        lua_pushvalue(L, -1);  // copy function
        lua_rawseti(L, -3, 2); // package.searchers[2] = our loader
        
        lua_pop(L, 2);  // pop searchers table and function copy
    }
};

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    // Override require to handle these functions
    vector<string> funcs = {"cfunction", "init", "setup"};
    LuaRequireOverride::override_require(L, funcs);
    
    // Now these will work without quotes:
    // require("mod") where mod.lua contains:
    //   local x = cfunction(k, 1, 2)
    //   init(config, params)
    // Will become:
    //   local x = cfunction("k", 1, 2)
    //   init("config", params)
    
    luaL_dofile(L, "main.lua");
    lua_close(L);
    return 0;
}



