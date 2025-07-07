#ifndef FROBOT_H
#define FROBOT_H

#include "general.hpp"
#include <osg/Vec3> 
#include <osg/Group>
#include <stdio.h> 

void turn_light_on(int index,int angle);
void fldbg_pos();

#define Dmovement 1 
#if Dmovement
typedef osg::Vec3f vec3; 
float distance_two_points(vec3* point1,vec3* point2);
void loadstl(osg::Group* group);
void geraeixos(osg::Group* group);
void posa(vfloat pangles, vec3 topoint=vec3(0,0,0), float x=0, float y=0, float z=1000,int speedtype=0,int funcn=0, vbool lock_axle={});
void posi(int index,float angle);
void posik(vec3 vv, vbool lock_axle);
struct posv {
    vfloat p;
    vec3 topoint;
    float x;
    float y;
    float z;
    std::atomic<int> posa_counter{0};  
    bool cancel = false;
    int funcn = 0;
    vbool lock_axle{};
    bool pause = false;
};
struct osgdr {  
	int index=0;
	vec3* offset=0;
	FILE* flog=0;
	float angle=0;
	float anglestart=0;
	float anglemax=0;
	float anglemin=0;
	int dir=1;
	int rotatedir=1;
	float angleik=0;
	float arm_len=0;
	bool moving=0;
	vec3 *axisbegin;
	vec3 *axisend;
	vec3 axis;
	vec3 *axisbeginik;
	vec3 *axisendik;
	std::vector<osg::ref_ptr<osg::Node>> nodes;
	vstring nodesstr;
	osg::ref_ptr<osg::Vec3Array> points;
	osg::ref_ptr<osg::Vec3Array> pointsik;
	osg::ref_ptr<osg::Vec4Array> color;
	osg::ref_ptr<osg::Geometry> geometry; 
	// osg::ref_ptr<osg::Geometry> geometry= new osg::Geometry; 
    osg::MatrixTransform* transform;
    void* drw;
    // osg::DrawArrays* drw; 
	// osg::DrawArrays* drw=new osg::DrawArrays;
	// osg::ComputeBoundsVisitor* cbv=new osg::ComputeBoundsVisitor;
    osgdr(osg::Group* group);
    void newdr(vec3 _axisbegin = vec3(30, 0, 0), vec3 _axisend = vec3(0, 0, 0));
    void redrawarrays();
    void gooffset();
    void rotate_pos(float newangle);
    void rotate_posk(float newangle);
    void rotatetoposition(float newangle, posv* &pov, bool manual = false);
    void rotate(float _angle);
    void rotateik(float _angle);
    posv* posik(vec3 topoint, vbool lock_axle={});
};   

extern std::vector<osgdr*> ve;
extern osg::ref_ptr<osg::Node> maquete;
extern osg::ref_ptr<osg::Node> ucs_icon;
extern bool posa_debug;
extern float pressuref;
extern bool all_cancel;
 

extern bool toggletranspbool;
void toggletransp();
   
#endif
      
#define Dlua 1
#if Dlua 
void lua_str(std::string str,bool isfile=1);
// void scint_init(int w,int h);
// void sql3_init();
// void fparse_str(std::string text, int linepos);
// void luaL_loadstring_arg_thread( std::string str,vstring args={},bool fromfunc=0);
std::string getfunctionhelp();
// std::string getallsqlitefuncs();
#endif

#endif // FROBOT_H