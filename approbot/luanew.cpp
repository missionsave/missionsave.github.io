#ifdef _WIN32 
#include <lua.hpp>
	#else
#include <lua5.4/lua.hpp> 
#endif
#include <unordered_map>
#include "frobot.hpp"
using namespace std;


nmutex lua_mtx("lua_mtx",1);
nmutex lua_funcs_mtx("lua_funcs_mtx",0);

int movz(lua_State* L){
	lua_funcs_mtx.lock(); 
    if(all_cancel){lua_funcs_mtx.unlock(); return 0;}

	// while(pool.size()>0)sleepms(500);
	float n1=lua_tonumber(L,1);
	// cotm(n1);
	// movetoposz( n1 );;
	posa({},{},0,0,n1);	
	// while(pool.size()>0)sleepms(500);
	lua_funcs_mtx.unlock(); 
	return 1;
}
int posadebug(lua_State* L){
	lua_funcs_mtx.lock(); 
	posa_debug=lua_tonumber(L,1); 
	lua_funcs_mtx.unlock(); 
	return 1;
}
// int cancel_all(lua_State* L){ 
// 	lop(i,0,pool.size()){
// 		pool[i]->cancel=1;
// 	} 
// 	lua_funcs_mtx.lock(); 
// 	all_cancel=1;
// 	lua_funcs_mtx.unlock(); 
// 	return 1;
// }
// void wait
int posa(lua_State* L){
	lua_funcs_mtx.lock(); 
    if(all_cancel){lua_funcs_mtx.unlock(); return 0;}

	// while(pool.size()>0)sleepms(500);
	// cot(lua_gettop(L));
	int sz=lua_gettop(L); 
	vfloat p(sz);
	lop(i,0,sz){  
		if(lua_tostring (L,i+1)==NULL)
			p[i]=1000;
		else
			p[i]=lua_tonumber(L,i+1 );
	}
	// posa(p );
	posa(p,{},0,0,1000,0,0);
	if(all_cancel){
		// all_cancel=0;
		lua_mtx.unlock();
		luaL_error(L, "Script execution interrupted!");
	}
	// while(running==1)sleepms(500);
	lua_funcs_mtx.unlock(); 
	return 1;
}
int posi(lua_State* L){
	lua_funcs_mtx.lock(); 
    if(all_cancel){lua_funcs_mtx.unlock(); return 0;}

	// while(pool.size()>0)sleepms(500);
	// cot(lua_gettop(L));
	int sz=lua_gettop(L); 
	vfloat p(sz);
	lop(i,0,sz)p[i]=lua_tonumber(L,i+1);

	posi(p[0],p[1]); 
	lua_funcs_mtx.unlock(); 
	return 1;
}
int pik2(lua_State* L){
	lua_funcs_mtx.lock();
    if(all_cancel){lua_funcs_mtx.unlock(); return 0;}

	// cot(lua_gettop(L));
	int sz=lua_gettop(L); 
	vfloat p(sz);
	lop(i,0,sz)p[i]=lua_tonumber(L,i+1);

	// pik2(vec3(p[0],p[1],p[2]));
	
	
	lua_funcs_mtx.unlock(); 
	return 1;
}
int close(lua_State* L){
    luaL_error(L, "close() called!");
	return 0;
}
int posaik(lua_State* L){
	lua_funcs_mtx.lock();
    if(all_cancel){lua_funcs_mtx.unlock(); return 0;}

	// while(pool.size()>0)sleepms(500);
	// cot(lua_gettop(L));
	int sz=lua_gettop(L); 
	vfloat p(3);
	// lop(i,0,3)cot1(lua_tonumber(L,i+1));
	lop(i,0,3)p[i]=lua_tonumber(L,i+1);
	vbool lock_axle(ve.size());
	lop(i,3,sz)lock_axle[i-3]=lua_tonumber(L,i+1);
	
// cot1(lock_axle);pausa
	posik(vec3(p[0],p[1],p[2]),lock_axle);	
	// while(running==1)sleepms(500);
	lua_funcs_mtx.unlock(); 
	return 1;
} 
int update_axl(lua_State* L) {
    lua_newtable(L);  // Create a new table for axl
    int x=ve[0]->axisbegin->x();
    for (int i = 0; i < 5; i++) {
        lua_newtable(L);  // Subtable for ve[i]->axisbegin

        lua_pushinteger(L, ve[i]->axisbegin->x()-x);
        lua_setfield(L, -2, "x");

        lua_pushinteger(L, ve[i]->axisbegin->y());
        lua_setfield(L, -2, "y");

        lua_pushinteger(L, ve[i]->axisbegin->z());
        lua_setfield(L, -2, "z");

        lua_rawseti(L, -2, i + 1);  // Insert subtable into axl at index i+1
    }

    lua_setglobal(L, "axl");  // Set the global variable axl

    return 0;  // No return values to Lua
}




lua_State* L;

#define setluafunc(val,desc){ \
	lua_pushcfunction(L, val); \ 
	lua_setglobal(L, #val); \
	if (luafuncs.count(val) == 0)luafuncs[val]=desc; \
}
unordered_map<lua_CFunction ,string> luafuncs;

string getfunctionhelp(){ 
	string val;
	for (const auto& pair : luafuncs) {
		// lua_CFunction func = pair.first; 
		val+=(pair.second)+"\n\n";	 
	}
	return val;
}
void lua_hook(lua_State* L, lua_Debug* ar) {
    int cl=(ar->currentline);
    // if (lua_getstack(L, 0, ar)) {
        lua_getinfo(L, "nSl", ar);
        cotm(cl,ar->source,ar->name) 
    // }
}
struct init_lua{
    init_lua(){
        init();
    }
    void init(){
        if(L)return;
        cotm("init_lua")
        L=luaL_newstate();
        luaL_openlibs(L); 

        // lua_sethook(L, lua_hook, LUA_MASKLINE, 0);
        
        // Get current package.path
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "path");
        const char* current_path = lua_tostring(L, -1);

        // Construct new path
        std::string new_path = "./lua/?.lua;";
        new_path += current_path;

        // Set package.path
        lua_pop(L, 1); // pop old path string
        lua_pushstring(L, new_path.c_str());
        lua_setfield(L, -2, "path");
        lua_pop(L, 1); // pop 'package' table
    

        setluafunc(posa,"posa(axle1,axle2,...)\
            Sets the rotation angle of the servos of the arm.");

        setluafunc(posaik,"posaik(axle1,axle2,...)\
            Sets the rotation angle of the servos of the arm.");

        setluafunc(movz,"movz(distance)\
            Sets the rotation angle of the servos of the arm.");

        // setluafunc(getTableFromCpp,"getTableFromCpp()\
        //     Gets the points actualized.");

        setluafunc(update_axl,"update_axl()\
            Gets the points actualized.");

        setluafunc(close,"close()\
            Exit script.");
    }
}init_luas;

void lua_str(string str,bool isfile){	
    thread([](string str,bool isfile){ 
		lua_mtx.lock();
        init_luas.init();
        // cotm(lua_mtx.waiting_threads.load())
        if(all_cancel && lua_mtx.waiting_threads.load()<=1){
            all_cancel=0;
            lua_mtx.unlock();
            return;
        }
        if(all_cancel){lua_mtx.unlock(); return;}

        int status;
        if (isfile) {
            status = luaL_loadfile(L, str.c_str());
        } else {
            status = luaL_loadstring(L, str.c_str());
        }

        if (status == LUA_OK) {
            if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
                std::cerr << "Runtime error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        } else {
            std::cerr << "Load error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }

        // cotm(2)
        lua_close(L);
        L=0;
		lua_mtx.unlock();
	},str,isfile).detach(); 

}