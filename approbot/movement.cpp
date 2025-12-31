#include "GraphicsCostEstimator_patch.h"
#include <osg/io_utils> 
#include <osgViewer/Viewer>
// #include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/NodeTrackerManipulator>
#include <osgGA/OrbitManipulator>
#include <osgDB/ReadFile>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Depth>
// #include <osg/Geode>
#include <osg/Material>
#include <osg/LOD>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
// #include <osg/ShapeDrawable>
// #include <osg/Texture2D>
// #include <osg/TextureBuffer>
// #include <osg/Image>
// #include <osg/Texture2DArray>
// #include <osg/Multisample> 
#include <osg/LineWidth>  
// #include <osg/Camera>
#include <osg/PositionAttitudeTransform>
#include <osg/ComputeBoundsVisitor>
#include <osg/Notify>
#include <osgViewer/CompositeViewer>
#include <osg/Vec3> 


#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Optimizer>
#include <osg/PolygonMode>

#include <thread>
#include <mutex>
#include "frobot.hpp"

// void makeCadStyleNode(osg::ref_ptr<osg::Group> root,osg::Node* model);
// void makeCadStyleNode(
//     osg::ref_ptr<osg::Group> root,
//     osg::Node* model,
//     const osg::Vec4& edgeColor=
// );

// Struct to represent one full sector (512 bytes)
struct LogEntry {
    int16_t value;  
    char padding[512 - sizeof(int16_t)] = {0};  // Fill remaining bytes
};
#define mflog(file,val) if(file){ LogEntry le; le.value=(int16_t)val; fwrite(&le, 512, 1, file); fflush(file); }
// #define mflog(file,val) if(file){ int16_t value=(int16_t)val; fwrite(&value, 512, 1, file); fflush(file); }
#define mflog(file,val)

// #define _usual_no_load 1
// #include <_usual.hpp> 
#define cot
#define cot1 
// #define cotm 

 
// import _usual;
// export module movement;

// void turn_light_on(int index,int angle);

using namespace std;
using namespace osg;

// export 
struct osgdr;
extern vector<osgdr*> ve;
// osgGA::TrackballManipulator* tmr;
typedef osg::Vec3f vec3f; 
// typedef osg::Vec3f vec3; 
#define Pi 3.141592653589793238462643383279502884L
#define Pi180 0.01745329251

bool posa_debug=0;
bool dbg_force=0; 
float pressuref=0;
vector<vec3> pressure_at;

std::vector<osgdr*> ve;

vvfloat posapool;



nmutex first_mtx("first_mtx",0);
nmutex posa_mtx("posa_mtx",1);
// nmutex posa_counter_mtx("posa_counter_mtx",0);
// nmutex posa_erase_mtx("posa_erase_mtx",0);
vector<lnmutex*> mut;
struct initmut{
    initmut(){
        lop(i,0,20){
            mut.push_back(new lnmutex("mut"+to_string(i),0));
        }
    }
}initmuts;
 

// unordered_map<string,osg::ref_ptr<osg::Node>> stlgroup;
// osg::Group* group = new osg::Group(); 
// struct osgdr;
// vector<osgdr*> ve;
osgdr* carril; 
// vector<std::mutex> mut=vector<std::mutex>(10);


struct ZShift : public osg::NodeVisitor
{
    float dz;

    ZShift(float shift)
        : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), dz(shift) {}

    void apply(osg::Geode& geode) override
    {
        for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Geometry* g = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
            if (!g) continue;

            osg::Vec3Array* v = dynamic_cast<osg::Vec3Array*>(g->getVertexArray());
            if (!v) continue;

            for (auto& p : *v)
                p.z() += dz;

            v->dirty();
            g->dirtyBound();
        }
    }
};


void makeCadStyleNode(
    osg::ref_ptr<osg::Group> root,
    osg::Node* model,
    const osg::Vec4& faceColor = osg::Vec4(0.95f, 0.95f, 0.95f, 1.0f)
)
{
    if (!root || !model)
        return;

    bool isTransparent = faceColor.a() < 1.0f;

    /* ============================================================
       PASS 1: SHADED FACES (CAD SOLID)
       ============================================================ */

    osg::ref_ptr<osg::Node> faceNode =
        dynamic_cast<osg::Node*>(model->clone(osg::CopyOp::DEEP_COPY_ALL));

    osg::StateSet* fs = faceNode->getOrCreateStateSet();
    fs->clear();
    fs->removeAttribute(osg::StateAttribute::PROGRAM);

    // Basic CAD Material State
    fs->setMode(GL_LIGHTING,       osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_ARRAY,    osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    // Transparency Handling
    if (isTransparent) {
        // 1. Enable Blending
        fs->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        
        // 2. Move to the Transparent Bin (Sorted back-to-front)
        // Bin 10 is the standard OSG bin for transparent objects
        fs->setRenderBinDetails(10, "DepthSortedBin");

        // 3. Disable Culling so we can see the "back" of the transparent object
        fs->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    } else {
        fs->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        fs->setRenderBinDetails(0, "RenderBin");
    }

    osg::ref_ptr<osg::Material> faceMat = new osg::Material;
    faceMat->setDiffuse (osg::Material::FRONT_AND_BACK, faceColor);
    faceMat->setAmbient (osg::Material::FRONT_AND_BACK, faceColor);
    faceMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setAlpha(osg::Material::FRONT_AND_BACK, faceColor.a());
    faceMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    fs->setAttributeAndModes(faceMat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(faceNode.get());

    /* ============================================================
       EDGE EXTRACTION (ROBUST, POSITION-BASED)
       ============================================================ */

    struct EdgeKey {
        osg::Vec3 a, b;
        EdgeKey(const osg::Vec3& p, const osg::Vec3& q) {
            if (p.x()<q.x() || (p.x()==q.x() && (p.y()<q.y() || (p.y()==q.y() && p.z()<q.z()))))
            { a=p; b=q; }
            else
            { a=q; b=p; }
        }
        bool operator<(const EdgeKey& o) const {
            if (a.x()!=o.a.x()) return a.x()<o.a.x();
            if (a.y()!=o.a.y()) return a.y()<o.a.y();
            if (a.z()!=o.a.z()) return a.z()<o.a.z();
            if (b.x()!=o.b.x()) return b.x()<o.b.x();
            if (b.y()!=o.b.y()) return b.y()<o.b.y();
            return b.z()<o.b.z();
        }
    };

    struct EdgeInfo {
        osg::Vec3 normalSum;
        unsigned count = 0;
    };

    std::map<EdgeKey, EdgeInfo> edgeMap;

    struct Extractor : public osg::NodeVisitor {
        std::map<EdgeKey, EdgeInfo>& edgeMap;
        Extractor(std::map<EdgeKey, EdgeInfo>& m)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), edgeMap(m) {}

        void addTriangles(osg::Vec3Array* v, osg::DrawElementsUInt* idx)
        {
            for (unsigned i=0;i+2<idx->size();i+=3)
            {
                const osg::Vec3& a = (*v)[(*idx)[i]];
                const osg::Vec3& b = (*v)[(*idx)[i+1]];
                const osg::Vec3& c = (*v)[(*idx)[i+2]];
                osg::Vec3 n = (b-a) ^ (c-a);
                if (n.length2() < 1e-12f) continue;
                n.normalize();
                edgeMap[EdgeKey(a,b)].normalSum += n;
                edgeMap[EdgeKey(a,b)].count++;
                edgeMap[EdgeKey(b,c)].normalSum += n;
                edgeMap[EdgeKey(b,c)].count++;
                edgeMap[EdgeKey(c,a)].normalSum += n;
                edgeMap[EdgeKey(c,a)].count++;
            }
        }

        void apply(osg::Geode& geode) override
        {
            for (unsigned i=0;i<geode.getNumDrawables();++i)
            {
                osg::Geometry* g = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (!g) continue;
                osg::Vec3Array* v = dynamic_cast<osg::Vec3Array*>(g->getVertexArray());
                if (!v) continue;
                for (unsigned p=0;p<g->getNumPrimitiveSets();++p)
                {
                    osg::PrimitiveSet* ps = g->getPrimitiveSet(p);
                    if (ps->getMode()!=GL_TRIANGLES) continue;
                    osg::ref_ptr<osg::DrawElementsUInt> tmp = new osg::DrawElementsUInt(GL_TRIANGLES);
                    if (auto* u = dynamic_cast<osg::DrawElementsUInt*>(ps)) for (auto i:*u) tmp->push_back(i);
                    else if (auto* s = dynamic_cast<osg::DrawElementsUShort*>(ps)) for (auto i:*s) tmp->push_back(i);
                    else if (auto* b = dynamic_cast<osg::DrawElementsUByte*>(ps)) for (auto i:*b) tmp->push_back(i);
                    else if (auto* a = dynamic_cast<osg::DrawArrays*>(ps)) for (int i=0;i<a->getCount();++i) tmp->push_back(a->getFirst()+i);
                    addTriangles(v,tmp.get());
                }
            }
        }
    };

    Extractor ex(edgeMap);
    model->accept(ex);

    /* ============================================================
       BUILD EDGE GEOMETRY
       ============================================================ */

    osg::ref_ptr<osg::Vec3Array> edgeVerts = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUInt> lines = new osg::DrawElementsUInt(GL_LINES);

    const float featureAngle = osg::DegreesToRadians(30.0f);
    const float cosFeature   = std::cos(featureAngle);
    const float coplanarEps  = 0.9999f;

    for (const auto& kv : edgeMap)
    {
        const EdgeKey& e = kv.first;
        const EdgeInfo& info = kv.second;
        if (info.count == 1) {
            lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.a);
            lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.b);
        } else {
            osg::Vec3 avg = info.normalSum;
            avg.normalize();
            float dot = avg * (info.normalSum / (float)info.count);
            if (dot > coplanarEps) continue;
            if (dot < cosFeature) {
                lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.a);
                lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.b);
            }
        }
    }

    osg::ref_ptr<osg::Geometry> edgeGeom = new osg::Geometry;
    edgeGeom->setVertexArray(edgeVerts.get());
    edgeGeom->addPrimitiveSet(lines.get());

    osg::ref_ptr<osg::Geode> edgeGeode = new osg::Geode;
    edgeGeode->addDrawable(edgeGeom.get());

    /* ============================================================
       EDGE RENDER STATE
       ============================================================ */

    osg::StateSet* es = edgeGeode->getOrCreateStateSet();
    es->clear();
    es->removeAttribute(osg::StateAttribute::PROGRAM);

    // Ensure edges are drawn AFTER/OVER the transparent faces
    // We put them in a later bin (11) than the faces (10)
    es->setRenderBinDetails(11, "RenderBin");

    osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::LEQUAL,0,1,true);
    es->setAttributeAndModes(depth.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // Offset the edges toward the camera to prevent "z-fighting" with the faces
    osg::ref_ptr<osg::PolygonOffset> po = new osg::PolygonOffset(-1.0f, -1.0f);
    es->setAttributeAndModes(po.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    es->setMode(GL_LIGHTING,  osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth(1.8f);
    es->setAttributeAndModes(lw.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> edgeMat = new osg::Material;
    edgeMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    es->setAttributeAndModes(edgeMat.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(edgeGeode.get());
}


//vw fine
void makeCadStyleNode_vw1(
    osg::ref_ptr<osg::Group> root,
    osg::Node* model,
    const osg::Vec4& faceColor = osg::Vec4(0.95f, 0.95f, 0.95f, 1.0f)
)
{
    if (!root || !model)
        return;

    /* ============================================================
       PASS 1: SHADED FACES (CAD SOLID)
       ============================================================ */

    osg::ref_ptr<osg::Node> faceNode =
        dynamic_cast<osg::Node*>(model->clone(osg::CopyOp::DEEP_COPY_ALL));

    osg::StateSet* fs = faceNode->getOrCreateStateSet();
    fs->clear();
    fs->removeAttribute(osg::StateAttribute::PROGRAM);

    fs->setMode(GL_LIGHTING,       osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_CULL_FACE,      osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_ARRAY,    osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> faceMat = new osg::Material;
    faceMat->setDiffuse (osg::Material::FRONT_AND_BACK, faceColor);
    faceMat->setAmbient (osg::Material::FRONT_AND_BACK, faceColor);
    faceMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    fs->setAttributeAndModes(
        faceMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
    );

 
    root->addChild(faceNode.get());

    /* ============================================================
       EDGE EXTRACTION (ROBUST, POSITION-BASED)
       ============================================================ */

    struct EdgeKey {
        osg::Vec3 a, b;
        EdgeKey(const osg::Vec3& p, const osg::Vec3& q) {
            if (p.x()<q.x() || (p.x()==q.x() && (p.y()<q.y() || (p.y()==q.y() && p.z()<q.z()))))
            { a=p; b=q; }
            else
            { a=q; b=p; }
        }
        bool operator<(const EdgeKey& o) const {
            if (a.x()!=o.a.x()) return a.x()<o.a.x();
            if (a.y()!=o.a.y()) return a.y()<o.a.y();
            if (a.z()!=o.a.z()) return a.z()<o.a.z();
            if (b.x()!=o.b.x()) return b.x()<o.b.x();
            if (b.y()!=o.b.y()) return b.y()<o.b.y();
            return b.z()<o.b.z();
        }
    };

    struct EdgeInfo {
        osg::Vec3 normalSum;
        unsigned count = 0;
    };

    std::map<EdgeKey, EdgeInfo> edgeMap;

    struct Extractor : public osg::NodeVisitor {
        std::map<EdgeKey, EdgeInfo>& edgeMap;
        Extractor(std::map<EdgeKey, EdgeInfo>& m)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), edgeMap(m) {}

        void addTriangles(osg::Vec3Array* v, osg::DrawElementsUInt* idx)
        {
            for (unsigned i=0;i+2<idx->size();i+=3)
            {
                const osg::Vec3& a = (*v)[(*idx)[i]];
                const osg::Vec3& b = (*v)[(*idx)[i+1]];
                const osg::Vec3& c = (*v)[(*idx)[i+2]];

                osg::Vec3 n = (b-a) ^ (c-a);
                if (n.length2() < 1e-12f)
                    continue;
                n.normalize();

                edgeMap[EdgeKey(a,b)].normalSum += n;
                edgeMap[EdgeKey(a,b)].count++;

                edgeMap[EdgeKey(b,c)].normalSum += n;
                edgeMap[EdgeKey(b,c)].count++;

                edgeMap[EdgeKey(c,a)].normalSum += n;
                edgeMap[EdgeKey(c,a)].count++;
            }
        }

        void apply(osg::Geode& geode) override
        {
            for (unsigned i=0;i<geode.getNumDrawables();++i)
            {
                osg::Geometry* g =
                    dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (!g) continue;

                osg::Vec3Array* v =
                    dynamic_cast<osg::Vec3Array*>(g->getVertexArray());
                if (!v) continue;

                for (unsigned p=0;p<g->getNumPrimitiveSets();++p)
                {
                    osg::PrimitiveSet* ps = g->getPrimitiveSet(p);
                    if (ps->getMode()!=GL_TRIANGLES) continue;

                    osg::ref_ptr<osg::DrawElementsUInt> tmp =
                        new osg::DrawElementsUInt(GL_TRIANGLES);

                    if (auto* u = dynamic_cast<osg::DrawElementsUInt*>(ps))
                        for (auto i:*u) tmp->push_back(i);
                    else if (auto* s = dynamic_cast<osg::DrawElementsUShort*>(ps))
                        for (auto i:*s) tmp->push_back(i);
                    else if (auto* b = dynamic_cast<osg::DrawElementsUByte*>(ps))
                        for (auto i:*b) tmp->push_back(i);
                    else if (auto* a = dynamic_cast<osg::DrawArrays*>(ps))
                        for (int i=0;i<a->getCount();++i)
                            tmp->push_back(a->getFirst()+i);

                    addTriangles(v,tmp.get());
                }
            }
        }
    };

    Extractor ex(edgeMap);
    model->accept(ex);

    /* ============================================================
       BUILD EDGE GEOMETRY
       ============================================================ */

    osg::ref_ptr<osg::Vec3Array> edgeVerts = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUInt> lines =
        new osg::DrawElementsUInt(GL_LINES);

    const float featureAngle = osg::DegreesToRadians(30.0f);
    const float cosFeature   = std::cos(featureAngle);
    const float coplanarEps  = 0.9999f;

    for (const auto& kv : edgeMap)
    {
        const EdgeKey& e = kv.first;
        const EdgeInfo& info = kv.second;

        if (info.count == 1)
        {
            lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.a);
            lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.b);
        }
        else
        {
            osg::Vec3 avg = info.normalSum;
            avg.normalize();

            float dot = avg * (info.normalSum / (float)info.count);
            if (dot > coplanarEps)
                continue;

            if (dot < cosFeature)
            {
                lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.a);
                lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.b);
            }
        }
    }

    osg::ref_ptr<osg::Geometry> edgeGeom = new osg::Geometry;
    edgeGeom->setVertexArray(edgeVerts.get());
    edgeGeom->addPrimitiveSet(lines.get());

    osg::ref_ptr<osg::Geode> edgeGeode = new osg::Geode;
    edgeGeode->addDrawable(edgeGeom.get());

    /* ============================================================
       EDGE RENDER STATE (BLACK)
       ============================================================ */

    osg::StateSet* es = edgeGeode->getOrCreateStateSet();
    es->clear();
 


    es->removeAttribute(osg::StateAttribute::PROGRAM);

    osg::ref_ptr<osg::Depth> depth =
        new osg::Depth(osg::Depth::LEQUAL,0,1,false);
    es->setAttributeAndModes(depth.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    es->setRenderBinDetails(1,"RenderBin");

    osg::ref_ptr<osg::PolygonOffset> po = new osg::PolygonOffset(-1,-1);
    es->setAttributeAndModes(po.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_POLYGON_OFFSET_LINE,
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    es->setMode(GL_LIGHTING,  osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth(1.8f);
    es->setAttributeAndModes(lw.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> edgeMat = new osg::Material;
    edgeMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    es->setAttributeAndModes(edgeMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(edgeGeode.get());
}
void makeCadStyleNode_vt(
    osg::ref_ptr<osg::Group> root,
    osg::Node* model,
    const osg::Vec4& faceColor = osg::Vec4(0.95f, 0.95f, 0.95f, 1.0f)
)
{
    if (!root || !model)
        return;

    /* ============================================================
       PASS 1: SHADED FACES (CAD SOLID)
       ============================================================ */

    osg::ref_ptr<osg::Node> faceNode =
        dynamic_cast<osg::Node*>(model->clone(osg::CopyOp::DEEP_COPY_ALL));

    osg::StateSet* fs = faceNode->getOrCreateStateSet();
    fs->clear();
    fs->removeAttribute(osg::StateAttribute::PROGRAM);

    fs->setMode(GL_LIGHTING,       osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_CULL_FACE,      osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_ARRAY,    osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> faceMat = new osg::Material;
    faceMat->setDiffuse (osg::Material::FRONT_AND_BACK, faceColor);
    faceMat->setAmbient (osg::Material::FRONT_AND_BACK, faceColor);
    faceMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    fs->setAttributeAndModes(
        faceMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
    );

    /* ============================================================
       TRANSPARENCY SUPPORT (WORKS CORRECTLY)
       ============================================================ */

    if (faceColor.a() < 1.0f)
    {
        // Enable blending
        fs->setMode(GL_BLEND,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::BlendFunc> bf =
            new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        fs->setAttributeAndModes(bf.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // Depth test ON, depth write OFF (critical)
        osg::ref_ptr<osg::Depth> d =
            new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
        fs->setAttributeAndModes(d.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // Transparent bin
        fs->setRenderBinDetails(0, "TransparentBin");
    }
    else
    {
        // Opaque: depth write ON
        osg::ref_ptr<osg::Depth> d =
            new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, true);
        fs->setAttributeAndModes(d.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        fs->setRenderBinDetails(0, "RenderBin");
    }

    root->addChild(faceNode.get());

    /* ============================================================
       EDGE EXTRACTION (unchanged)
       ============================================================ */

    struct EdgeKey {
        osg::Vec3 a, b;
        EdgeKey(const osg::Vec3& p, const osg::Vec3& q) {
            if (p < q) { a=p; b=q; } else { a=q; b=p; }
        }
        bool operator<(const EdgeKey& o) const {
            if (a.x()!=o.a.x()) return a.x()<o.a.x();
            if (a.y()!=o.a.y()) return a.y()<o.a.y();
            if (a.z()!=o.a.z()) return a.z()<o.a.z();
            if (b.x()!=o.b.x()) return b.x()<o.b.x();
            if (b.y()!=o.b.y()) return b.y()<o.b.y();
            return b.z()<o.b.z();
        }
    };

    struct EdgeInfo {
        osg::Vec3 normalSum;
        unsigned count = 0;
    };

    std::map<EdgeKey, EdgeInfo> edgeMap;

    struct Extractor : public osg::NodeVisitor {
        std::map<EdgeKey, EdgeInfo>& edgeMap;
        Extractor(std::map<EdgeKey, EdgeInfo>& m)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), edgeMap(m) {}

        void addTriangles(osg::Vec3Array* v, osg::DrawElementsUInt* idx)
        {
            for (unsigned i=0;i+2<idx->size();i+=3)
            {
                const osg::Vec3& a = (*v)[(*idx)[i]];
                const osg::Vec3& b = (*v)[(*idx)[i+1]];
                const osg::Vec3& c = (*v)[(*idx)[i+2]];

                osg::Vec3 n = (b-a) ^ (c-a);
                if (n.length2() < 1e-12f)
                    continue;
                n.normalize();

                edgeMap[EdgeKey(a,b)].normalSum += n;
                edgeMap[EdgeKey(a,b)].count++;

                edgeMap[EdgeKey(b,c)].normalSum += n;
                edgeMap[EdgeKey(b,c)].count++;

                edgeMap[EdgeKey(c,a)].normalSum += n;
                edgeMap[EdgeKey(c,a)].count++;
            }
        }

        void apply(osg::Geode& geode) override
        {
            for (unsigned i=0;i<geode.getNumDrawables();++i)
            {
                osg::Geometry* g =
                    dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (!g) continue;

                osg::Vec3Array* v =
                    dynamic_cast<osg::Vec3Array*>(g->getVertexArray());
                if (!v) continue;

                for (unsigned p=0;p<g->getNumPrimitiveSets();++p)
                {
                    osg::PrimitiveSet* ps = g->getPrimitiveSet(p);
                    if (ps->getMode()!=GL_TRIANGLES) continue;

                    osg::ref_ptr<osg::DrawElementsUInt> tmp =
                        new osg::DrawElementsUInt(GL_TRIANGLES);

                    if (auto* u = dynamic_cast<osg::DrawElementsUInt*>(ps))
                        for (auto i:*u) tmp->push_back(i);
                    else if (auto* s = dynamic_cast<osg::DrawElementsUShort*>(ps))
                        for (auto i:*s) tmp->push_back(i);
                    else if (auto* b = dynamic_cast<osg::DrawElementsUByte*>(ps))
                        for (auto i:*b) tmp->push_back(i);
                    else if (auto* a = dynamic_cast<osg::DrawArrays*>(ps))
                        for (int i=0;i<a->getCount();++i)
                            tmp->push_back(a->getFirst()+i);

                    addTriangles(v,tmp.get());
                }
            }
        }
    };

    Extractor ex(edgeMap);
    model->accept(ex);

    /* ============================================================
       BUILD EDGE GEOMETRY
       ============================================================ */

    osg::ref_ptr<osg::Vec3Array> edgeVerts = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUInt> lines =
        new osg::DrawElementsUInt(GL_LINES);

    const float featureAngle = osg::DegreesToRadians(30.0f);
    const float cosFeature   = std::cos(featureAngle);
    const float coplanarEps  = 0.9999f;

    for (const auto& kv : edgeMap)
    {
        const EdgeKey& e = kv.first;
        const EdgeInfo& info = kv.second;

        if (info.count == 1)
        {
            lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.a);
            lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.b);
        }
        else
        {
            osg::Vec3 avg = info.normalSum;
            avg.normalize();

            float dot = avg * (info.normalSum / (float)info.count);
            if (dot > coplanarEps)
                continue;

            if (dot < cosFeature)
            {
                lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.a);
                lines->push_back(edgeVerts->size()); edgeVerts->push_back(e.b);
            }
        }
    }

    osg::ref_ptr<osg::Geometry> edgeGeom = new osg::Geometry;
    edgeGeom->setVertexArray(edgeVerts.get());
    edgeGeom->addPrimitiveSet(lines.get());

    osg::ref_ptr<osg::Geode> edgeGeode = new osg::Geode;
    edgeGeode->addDrawable(edgeGeom.get());

    /* ============================================================
       EDGE RENDER STATE (BLACK)
       ============================================================ */

    osg::StateSet* es = edgeGeode->getOrCreateStateSet();
    es->clear();

    es->setMode(GL_BLEND,
        osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    es->removeAttribute(osg::StateAttribute::PROGRAM);

    osg::ref_ptr<osg::Depth> depth =
        new osg::Depth(osg::Depth::LEQUAL,0,1,true); // depth write ON
    es->setAttributeAndModes(depth.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // es->setRenderBinDetails(1,"RenderBin");
es->setRenderBinDetails(11,"RenderBin");

    osg::ref_ptr<osg::PolygonOffset> po = new osg::PolygonOffset(-1,-1);
    es->setAttributeAndModes(po.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_POLYGON_OFFSET_LINE,
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    es->setMode(GL_LIGHTING,  osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth(1.8f);
    es->setAttributeAndModes(lw.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> edgeMat = new osg::Material;
    edgeMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    es->setAttributeAndModes(edgeMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(edgeGeode.get());
}


// https://copilot.microsoft.com/chats/6D21xf7MfHhAwjq37r6yK
// osg::ref_ptr<osg::Node> makeCadStyleNode_v1(osg::Node* model)
void makeCadStyleNode_v1(osg::ref_ptr<osg::Group> root,osg::Node* model)
{
    if (!model)
        return;// nullptr;

    // osg::ref_ptr<osg::Group> root = new osg::Group;

    /* ============================================================
       PASS 1: WHITE SHADED FACES
       ============================================================ */

    osg::ref_ptr<osg::Node> faceNode =
        dynamic_cast<osg::Node*>(model->clone(osg::CopyOp::DEEP_COPY_ALL));

    osg::StateSet* fs = faceNode->getOrCreateStateSet();
    fs->clear();
    fs->removeAttribute(osg::StateAttribute::PROGRAM);

    fs->setMode(GL_LIGHTING,       osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_CULL_FACE,      osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_ARRAY,    osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> faceMat = new osg::Material;
    faceMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.95f,0.95f,0.95f,1));
    faceMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0.95f,0.95f,0.95f,1));
    faceMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    fs->setAttributeAndModes(faceMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(faceNode.get());

    /* ============================================================
       PASS 2: CAD EDGES (BOUNDARY + FEATURE)
       ============================================================ */

    osg::ref_ptr<osg::Geode> edgeGeode = new osg::Geode;

    struct EdgeKey {
        unsigned a, b;
        EdgeKey(unsigned x=0, unsigned y=0) {
            if (x < y) { a = x; b = y; }
            else       { a = y; b = x; }
        }
        bool operator<(const EdgeKey& o) const {
            return a < o.a || (a == o.a && b < o.b);
        }
    };

    struct EdgeInfo {
        osg::Vec3 n1, n2;
        unsigned count = 0;
    };

    std::map<EdgeKey, EdgeInfo> edgeMap;

    struct Extractor : public osg::NodeVisitor
    {
        std::map<EdgeKey, EdgeInfo>& edgeMap;

        Extractor(std::map<EdgeKey, EdgeInfo>& m)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), edgeMap(m) {}

        void addTriangles(osg::Vec3Array* v, osg::DrawElementsUInt* idx)
        {
            for (unsigned i = 0; i + 2 < idx->size(); i += 3)
            {
                unsigned a = (*idx)[i];
                unsigned b = (*idx)[i+1];
                unsigned c = (*idx)[i+2];

                osg::Vec3 n = ((*v)[b] - (*v)[a]) ^ ((*v)[c] - (*v)[a]);
                if (n.length2() < 1e-5f)
                    continue;
                n.normalize();

                EdgeKey e1(a,b), e2(b,c), e3(c,a);

                auto& i1 = edgeMap[e1];
                if (i1.count == 0) i1.n1 = n;
                else if (i1.count == 1) i1.n2 = n;
                i1.count++;

                auto& i2 = edgeMap[e2];
                if (i2.count == 0) i2.n1 = n;
                else if (i2.count == 1) i2.n2 = n;
                i2.count++;

                auto& i3 = edgeMap[e3];
                if (i3.count == 0) i3.n1 = n;
                else if (i3.count == 1) i3.n2 = n;
                i3.count++;
            }
        }

        void apply(osg::Geode& geode) override
        {
            for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
            {
                osg::Geometry* g =
                    dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (!g) continue;

                osg::Vec3Array* v =
                    dynamic_cast<osg::Vec3Array*>(g->getVertexArray());
                if (!v) continue;

                for (unsigned p = 0; p < g->getNumPrimitiveSets(); ++p)
                {
                    osg::PrimitiveSet* ps = g->getPrimitiveSet(p);
                    if (ps->getMode() != GL_TRIANGLES) continue;

                    osg::ref_ptr<osg::DrawElementsUInt> tmp =
                        new osg::DrawElementsUInt(GL_TRIANGLES);

                    if (auto* u = dynamic_cast<osg::DrawElementsUInt*>(ps))
                        for (unsigned i = 0; i < u->size(); ++i) tmp->push_back((*u)[i]);
                    else if (auto* s = dynamic_cast<osg::DrawElementsUShort*>(ps))
                        for (unsigned i = 0; i < s->size(); ++i) tmp->push_back((*s)[i]);
                    else if (auto* b = dynamic_cast<osg::DrawElementsUByte*>(ps))
                        for (unsigned i = 0; i < b->size(); ++i) tmp->push_back((*b)[i]);
                    else if (auto* a = dynamic_cast<osg::DrawArrays*>(ps))
                        for (int i = 0; i < a->getCount(); ++i) tmp->push_back(a->getFirst() + i);

                    addTriangles(v, tmp.get());
                }
            }
        }
    };

    Extractor ex(edgeMap);
    model->accept(ex);

    /* ============================================================
       FIND ANY VERTEX ARRAY
       ============================================================ */

    osg::Vec3Array* vertices = nullptr;

    struct VAVisitor : public osg::NodeVisitor {
        osg::Vec3Array* result = nullptr;
        VAVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
        void apply(osg::Geode& geode) override {
            for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
            {
                if (auto* g = dynamic_cast<osg::Geometry*>(geode.getDrawable(i)))
                {
                    if (auto* v = dynamic_cast<osg::Vec3Array*>(g->getVertexArray()))
                    {
                        result = v;
                        return;
                    }
                }
            }
        }
    };

    VAVisitor vav;
    faceNode->accept(vav);
    vertices = vav.result;

    if (!vertices)
        return;// root;

    /* ============================================================
       BUILD EDGE GEOMETRY
       ============================================================ */

    osg::ref_ptr<osg::DrawElementsUInt> lines =
        new osg::DrawElementsUInt(GL_LINES);

    const float featureAngle = osg::DegreesToRadians(30.0f);
    const float coplanarEps  = 0.9995f;
    const float zPlaneEps    = 1e-6f;   // plane proximity for z≈0 filter

    for (auto& kv : edgeMap)
    {
        const EdgeKey& e = kv.first;
        const EdgeInfo& info = kv.second;

        const osg::Vec3& va = (*vertices)[e.a];
        const osg::Vec3& vb = (*vertices)[e.b];

        if (info.count == 1)
        {
            // boundary edge
            lines->push_back(e.a);
            lines->push_back(e.b);
        }
        else if (info.count == 2)
        {
            // SPECIAL CASE: kill internal edges lying on z≈0 plane
            if (std::fabs(va.z()) < zPlaneEps && std::fabs(vb.z()) < zPlaneEps)
                continue;

            float dot = info.n1 * info.n2;

            if (dot > coplanarEps)
                continue; // coplanar, drop diagonal

            if (dot < std::cos(featureAngle))
            {
                // true feature edge
                lines->push_back(e.a);
                lines->push_back(e.b);
            }
        }
    }

    osg::ref_ptr<osg::Geometry> edgeGeom = new osg::Geometry;
    edgeGeom->setVertexArray(vertices);
    edgeGeom->addPrimitiveSet(lines.get());
    edgeGeode->addDrawable(edgeGeom.get());

    /* ============================================================
       EDGE RENDER STATE (NO HIDING, NO FLICKER)
       ============================================================ */

    osg::StateSet* es = edgeGeode->getOrCreateStateSet();
    es->clear();
    es->removeAttribute(osg::StateAttribute::PROGRAM);

    // Depth test ON, depth write OFF
    osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::LEQUAL, 0, 1, false);
    es->setAttributeAndModes(depth.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // Render edges after faces
    es->setRenderBinDetails(1, "RenderBin");

    // Polygon offset & line mode
    osg::ref_ptr<osg::PolygonOffset> po = new osg::PolygonOffset;
    po->setFactor(-1.0f);
    po->setUnits(-1.0f);
    es->setAttributeAndModes(po.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_POLYGON_OFFSET_LINE,
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    es->setMode(GL_LIGHTING,  osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth(1.8f);
    es->setAttributeAndModes(lw.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> edgeMat = new osg::Material;
    edgeMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);
    es->setAttributeAndModes(edgeMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(edgeGeode.get());

    // return root;
}
#include <osg/Node>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/BlendFunc>
//not working
void setTransparency(osg::Node* model, bool val = true)
{
    if (!model) return;

    osg::StateSet* state = model->getOrCreateStateSet();

    // TURN TRANSPARENCY OFF
    if (!val)
    {
        state->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        state->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        state->removeAttribute(osg::StateAttribute::MATERIAL);
        state->removeAttribute(osg::StateAttribute::BLENDFUNC);
        return;
    }

    // TURN TRANSPARENCY ON

    // 1) Material with alpha < 1
    osg::ref_ptr<osg::Material> mat = new osg::Material;
    mat->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    // white with alpha 0.5 – forces visible transparency
    mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.5f));
    mat->setAlpha(osg::Material::FRONT_AND_BACK, 0.5f);

    state->setAttributeAndModes(
        mat.get(),
        osg::StateAttribute::ON |
        osg::StateAttribute::OVERRIDE
    );

    // 2) Standard alpha blending (this was wrong in your code)
    osg::ref_ptr<osg::BlendFunc> bf =
        new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
                           osg::BlendFunc::ONE_MINUS_SRC_ALPHA);

    state->setAttributeAndModes(
        bf.get(),
        osg::StateAttribute::ON |
        osg::StateAttribute::OVERRIDE
    );

    // 3) Enable blending + transparent bin
    state->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

void setCadFacesTransparency(osg::Group* cadRoot, float alpha)
{
    if (!cadRoot || cadRoot->getNumChildren() == 0)
        return;

    // We know makeCadStyleNode puts faces as first child
    osg::Node* faceNode = cadRoot->getChild(0);
    osg::StateSet* fs = faceNode->getOrCreateStateSet();

    osg::ref_ptr<osg::Material> faceMat =
        dynamic_cast<osg::Material*>(fs->getAttribute(osg::StateAttribute::MATERIAL));
    if (!faceMat)
    {
        faceMat = new osg::Material;
        // default white; you can store original color elsewhere if needed
        faceMat->setDiffuse(osg::Material::FRONT_AND_BACK,
                            osg::Vec4(0.95f, 0.95f, 0.95f, alpha));
    }

    osg::Vec4 d = faceMat->getDiffuse(osg::Material::FRONT_AND_BACK);
    d.a() = alpha;
    faceMat->setDiffuse(osg::Material::FRONT_AND_BACK, d);

    fs->setAttributeAndModes(
        faceMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
    );

    if (alpha < 1.0f)
    {
        osg::ref_ptr<osg::BlendFunc> bf =
            new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        fs->setAttributeAndModes(
            bf.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
        );

        fs->setMode(GL_BLEND,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::Depth> dstate =
            new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
        fs->setAttributeAndModes(
            dstate.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
        );

        fs->setRenderBinDetails(0, "TransparentBin");
    }
    else
    {
        // Back to opaque
        fs->setMode(GL_BLEND,
            osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        fs->removeAttribute(osg::StateAttribute::BLENDFUNC);

        osg::ref_ptr<osg::Depth> dstate =
            new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, true);
        fs->setAttributeAndModes(
            dstate.get(),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
        );

        fs->setRenderBinDetails(0, "RenderBin");
    }
}


void settranparency(osg::Node* model, bool enable = true)
{
	return;
	cotm("transp")
    osg::StateSet* ss = model->getOrCreateStateSet();

    if (!enable)
    {
        // Disable transparency cleanly
        ss->removeAttribute(osg::StateAttribute::MATERIAL);
        ss->removeAttribute(osg::StateAttribute::BLENDFUNC);
        ss->setMode(GL_BLEND, osg::StateAttribute::OFF);
        ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        return;
    }

    // --- ENABLE TRANSPARENCY ---
    osg::ref_ptr<osg::Material> mat = new osg::Material;
    mat->setAlpha(osg::Material::FRONT_AND_BACK, 0.5f);
    ss->setAttributeAndModes(mat, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::BlendFunc> bf =
        new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    ss->setAttributeAndModes(bf, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

void settranparency_vold(Node* model,bool val=1){
	osg::StateSet* state2 = model->getOrCreateStateSet();
	state2->clear();	
	osg::ref_ptr<osg::Material> mat2 = new osg::Material;
	mat2->setAlpha(osg::Material::FRONT_AND_BACK, 0.5); //Making alpha channel
	state2->setAttributeAndModes( mat2.get() ,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	if(!val)return;
	osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_DST_COLOR );
	state2->setAttributeAndModes(bf);  
	model->getStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
	model->getStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
}
void settranparency_vp(Node* model,bool val=1){  
// return;
// osg::StateSet* ss = model->getOrCreateStateSet();

// ss->setMode(
//     GL_LIGHTING,
//     osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE
// );

// osg::ref_ptr<osg::Material> mat = new osg::Material;

// osg::Vec4 nearWhite(0.95f, 0.95f, 0.95f, 1.0f);

// mat->setDiffuse (osg::Material::FRONT_AND_BACK, nearWhite);
// mat->setAmbient (osg::Material::FRONT_AND_BACK, nearWhite);
// mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
// mat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
// mat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

// ss->setAttributeAndModes(
//     mat.get(),
//     osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
// );

// ss->setMode(GL_LIGHTING,       osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
// ss->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

// ss->removeAttribute(osg::StateAttribute::MATERIAL);









// return;
// 	osg::StateSet* state2 = model->getOrCreateStateSet();
// 	state2->clear();
	
// 	osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth;
// lw->setWidth(1.8f);

// state2->setAttributeAndModes(
//     lw.get(),
//     osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
// );

// state2->setMode(
//     GL_LINE_SMOOTH,
//     osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE
// );

// state2->setMode(
//     GL_DEPTH_TEST,
//     osg::StateAttribute::ON
// );


// return;

	// osg::ref_ptr<osg::Material> mat2 = new osg::Material;
	// // mat2->setAlpha(osg::Material::FRONT_AND_BACK, 0.5); //Making alpha channel
	// state2->setAttributeAndModes( mat2.get() ,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	// if(!val)return;
	// osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_DST_COLOR );
	// state2->setAttributeAndModes(bf);  
	// model->getStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
	// model->getStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
}

void settranparency_v1(Node* model,bool val=1){ 
	osg::StateSet* state2 = model->getOrCreateStateSet();
	state2->clear();	
	osg::ref_ptr<osg::Material> mat2 = new osg::Material;
	// mat2->setAlpha(osg::Material::FRONT_AND_BACK, 0.5); //Making alpha channel
	state2->setAttributeAndModes( mat2.get() ,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	if(!val)return;
	osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_DST_COLOR );
	state2->setAttributeAndModes(bf);  
	model->getStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
	model->getStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
}

void setalpha(Node* model,float val=0.9){
// https://groups.google.com/g/osg-users/c/APGaJ_5icx8
	osg::StateSet* state2 = model->getOrCreateStateSet(); //Creating material 
	osg::ref_ptr<osg::Material> mat2 = new osg::Material;

	mat2->setAlpha(osg::Material::FRONT_AND_BACK, 0.5); //Making alpha channel
	state2->setAttributeAndModes( mat2.get() ,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	
	osg::BlendFunc* bf = new                        //Blending
	osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
	osg::BlendFunc::ONE_MINUS_DST_COLOR );
	state2->setAttributeAndModes(bf);  
	model->getStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
	model->getStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN ); 
	
	
	state2->clear();
	state2->setAttributeAndModes( mat2.get() ,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	
}

 
// export 
float distance_two_points(vec3* point1,vec3* point2){
	// cot(*point2);
	return sqrt( pow(point2->x()-point1->x(),2) +pow(point2->y()-point1->y(),2) +pow(point2->z()-point1->z(),2)    );	
}

// vector<posv*> pool;









// #include <atomic>
// // export 
// struct posv {
//     vfloat p;
//     vec3 topoint;
//     float x;
//     float y;
//     float z;
//     std::atomic<int> posa_counter{0};  
//     bool cancel = false;
//     int funcn = 0;
//     vbool lock_axle{};
//     bool pause = false;

// 	// posv(){};
//     // Explicitly delete the copy constructor and copy assignment operator
//     // posv(const posv&) = delete;
//     // posv& operator=(const posv&) = delete;

//     // // Allow move constructor and move assignment
//     // posv(posv&&) noexcept = default;
//     // posv& operator=(posv&&) noexcept = default;
// };

void movetoposz(float z, posv* &pov);
void goffset(vec3* offset);

void bound_box();
// void fldbg_pos();
void copy_points_k();
void arm_len_fill();
void movz_ik(float z);

// Warning: detected OpenGL error 'invalid value' at after RenderBin::draw(..)
// void osgdr::redrawarrays(){
// 	// return;
//     osg::DrawArrays* drwl=((osg::DrawArrays*)drw);
//     // drwl->dirty();
//     drwl->set(GL_LINES,0, points->size()); 
//     geometry ->setPrimitiveSet(0,(osg::DrawArrays*)drwl);
// }

// #include <GL/glu.h>
// #if 1
// _checkGLErrors = ONCE_PER_FRAME;
// #else
// _checkGLErrors = ONCE_PER_ATTRIBUTE; 
// #endif

// nmutex rda("rda");
void osgdr::redrawarrays(){
	// return;
	// rda.lock();

// // Suppose you have a state object
// osg::State* pState = new osg::State();

// // Set the error checking mode to ONCE_PER_ATTRIBUTE
// pState->setCheckForGLErrors(osg::State::CheckForGLErrors::ONCE_PER_ATTRIBUTE);


    // Ensure that the draw pointer is not null
    if (drw == nullptr) {
        std::cerr << "Error: drw is null" << std::endl;
        return;
    }
    // GLenum err = glGetError(); // Limpar qualquer erro anterior
    // while (err != GL_NO_ERROR) {
    //     std::cerr << "OpenGL Error (antes): " << gluErrorString(err) << std::endl;
    //     err = glGetError();
	// }
    osg::DrawArrays* drwl=((osg::DrawArrays*)drw);
    // osg::DrawArrays* drwl = dynamic_cast<osg::DrawArrays*>(drw);
    if (drwl == nullptr) {
        std::cerr << "Error: dynamic_cast failed, drw is not a DrawArrays object" << std::endl;
        return;
    }

    // Check if the points array is not empty
    if (points->size() == 0) {
        std::cerr << "Error: points array is empty" << std::endl;
        return;
    }

    // Set the draw mode and number of vertices
    drwl->set(GL_LINES, 0, points->size()); 
    geometry->setPrimitiveSet(0, drwl);
	// rda.unlock();
}

// nmutex rda("rda");
// osg::DrawArrays* drwl;//=new osg::DrawArrays;
// void osgdr::redrawarrays() {
// 	rda.lock();
//     // if (!drw) {
//     //     std::cerr << "Erro: 'drw' é um ponteiro nulo!" << std::endl;
//     //     return;
//     // }
// 	drwl=new osg::DrawArrays;
//     // osg::DrawArrays* drwl=((osg::DrawArrays*)drw);
//     if (!drwl) {
//         std::cerr << "Erro: Cast para osg::DrawArrays falhou!" << std::endl;
//         return;
//     }

//     if (!geometry || !points || points->empty()) {
//         std::cerr << "Erro: Geometria ou pontos inválidos!" << std::endl;
//         return;
//     }

//     int pointCount = points->size();
//     if (pointCount == 0) {
//         std::cerr << "Erro: Nenhum ponto disponível para desenhar!" << std::endl;
//         return;
//     }

//     // Garantir que o número de pontos é par para GL_LINES
//     if (pointCount % 2 != 0) {
//         std::cerr << "Aviso: Número ímpar de pontos. Ajustando para múltiplo de 2." << std::endl;
//         pointCount -= 1;  // Remove um ponto extra
//     }

//     // std::cout << "Redesenhando com " << pointCount << " pontos." << std::endl;

//     drwl->set(GL_LINES, 0, pointCount);
// 	drwl->dirty();
//     geometry->setPrimitiveSet(0, drwl);
// 	// delete drw;
// 	rda.unlock();
// }

osgdr::osgdr(Group* group){	
    geometry= new osg::Geometry; 
	// geometry->setDataVariance(osg::Object::DYNAMIC);

    transform = new osg::MatrixTransform;
    points = new osg::Vec3Array;
	pointsik = new osg::Vec3Array;;
	color = new osg::Vec4Array;
    drw=new osg::DrawArrays;
    group->addChild( transform); 
    group->addChild( geometry);
}

mutex mtxaxis;
void osgdr::newdr(vec3 _axisbegin,vec3 _axisend){
    index=ve.size()-1;
    // cot(index);
    points->push_back(_axisbegin);
    points->push_back(_axisend);
	mtxaxis.lock();
    axisbegin=&points[0][0];
    axisend=&points[0][1];
	mtxaxis.unlock();
    color->push_back(osg::Vec4(1.0,0.0,0.0,1.0));
    // color->push_back(osg::Vec4(1.0,0.0,0.0,1.0));
    geometry ->setVertexArray(points.get());
    geometry ->setColorArray(color.get());
    geometry ->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
    // geometry ->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    // geometry ->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry ->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0, points->size()));
    osg::LineWidth* linew = new osg::LineWidth(5);
    geometry->getOrCreateStateSet()->setAttributeAndModes(linew);
    // transform->addChild(geometry);
    
    lop(i,0,nodesstr.size()){
        nodes.push_back(osgDB::readRefNodeFile(nodesstr[i]));

		makeCadStyleNode_vt(transform,nodes[i],osg::Vec4(0.95f, 0.55f, 0.5f, 1.0f));
		// makeCadStyleNode_vw1(transform,nodes[i],osg::Vec4(0.95f, 0.55f, 0.5f, 1.0f));
		// makeCadStyleNode(transform,nodes[i],osg::Vec4(0.95f, 0.55f, 0.5f, 1.0f));
    // osg::StateSet* ss = nodes[i]->getOrCreateStateSet();

    // // 1) Disable lighting completely
    // ss->setMode(GL_LIGHTING,
    //             osg::StateAttribute::OFF |
    //             osg::StateAttribute::OVERRIDE |
    //             osg::StateAttribute::PROTECTED);

    // // 2) Also disable cull, just in case
    // ss->setMode(GL_CULL_FACE,
    //             osg::StateAttribute::OFF |
    //             osg::StateAttribute::OVERRIDE |
    //             osg::StateAttribute::PROTECTED);

 



        // settranparency(nodes[i].get(),0);
        // transform->addChild(nodes[i].get());
        // transform->accept(*cbv);
    }
    // arm_len_fill();
    
    //pointsik init
    // pointsik = new osg::Vec3Array;
    // pointsik->push_back(points[0][0]);
    // pointsik->push_back(points[0][1]); 
    // axisbeginik=&pointsik[0][0];
    // axisendik=&pointsik[0][1];
    
    // fldbg_pos();
    // cot(*axisbegin);
    // cot(*axisbeginik);
}

void osgdr::gooffset(){
    if(offset==0)offset=new vec3(0,0,0);
    
}



void osgdr::rotate_pos(float newangle){	 
    float nangle=angle-newangle; 
    rotate(nangle*-1);
}
void osgdr::rotate_posk(float newangle){	
    float nangle=angleik-newangle; 
    rotateik(nangle*-1);
}
//ve[index]-> == this
//posa
void osgdr::rotatetoposition(float newangle, posv* &pov,bool manual){	
    thread th([](float newangle, posv* pov,bool manual,osgdr* th ){
        // posv* pov=(posv*)povi;
        if(pov->pause){
            while(pov->pause)sleepms(200);
            // cotm("pause");
        }
        // cot1(8888);
        float precision=1;
        if(th->moving==1){
            // moving=0;
        }
        if(!manual)mut[th->index+1]->lock();
        th->moving=1;
        if(pov->cancel)th->moving=0;
        // cot(angle);
        // cot(newangle);
        // if(newangle>anglemax)return;
        // mut[9]->lock();
        // mtxlock(9);
        float nangle=newangle-th->angle;
        // cot(index);
        // cot(newangle);
        // cot1(angle);
        // cot(nangle);
        // mut[9]->unlock();
        // mtxunlock(9);
        // cot(anglemax);
        // rotate( nangle);
        if(nangle>0){
            // cot1(nangle);
            float dir=1*precision;
            for(;;){			
                if(pressuref>2){
                    // cot1("W");
                    // pov->cancel=1;
                    // pressure_at.push_back(*ve[4]->axisend);
                    // cot(pressure_at.back());
                }	
                if(pov->cancel==1)break;		
                if(pov->pause){
                    while(pov->pause)sleepms(200);
                }
                if(th->moving==0)break;
                if(th->angle>th->anglemax)break;
                if(nangle<=0.1)break;
                th->rotate( dir);
                // cot(nangle);
                // cot(angle);
                nangle-=dir;
                sleepms(20);
            
            }
        }
        if(nangle<0){
            // cot1(nangle);
            float dir=-1*precision;
            for(;;){					
                if(pov->cancel==1)break;		
                if(pov->pause){
                    while(pov->pause)sleepms(200);
                }
                if(th->moving==0)break;
                if(th->angle<th->anglemin)break;
                if(nangle>=-0.1)break;
                th->rotate( dir);
                nangle-=dir;
                sleepms(20);
            
            }
        }
        cot(pov->posa_counter);
        th->moving=0;
        pov->posa_counter--;
        if(!manual)mut[th->index+1]->unlock();
            // posa_counter_mtx.lock();
            // pov->posa_counter--;
            // cot(posa_counter);
            // posa_counter_mtx.unlock();
            // fldbg_pos();
    },newangle,pov,manual,this);
    th.detach( ); 
    

} 
void osgdr::rotate( float _angle ){
// cot(_angle);
    if(_angle==0)return;
    // mtxlock(0);
    first_mtx.lock();
    // cot(index);
    // cot(angle);
    // lop(i,0,ve.size()) cout<<i<<" "<<ve[i]->angle<<"  ";cout<<endl;
    // cot(axisb.length());
    // if(axisb.length()==0)
    vec3 axisb=*axisbegin; 
    // angle+=_angle;
    // cot(angle);
            
    osg::Matrix Tr;
    Tr.makeTranslate( axisb.x(),axisb.y(),axisb.z() );
    osg::Matrix T; 
    T.makeTranslate( -axisb.x(),-axisb.y(),-axisb.z() ); 
        
    lop(i,0,index) {		
        osg::Matrix Ra; 
        Ra.makeRotate( Pi180*-ve[i]->angle, ve[i]->axis ); 
        lop(j,0,points[0].size())points[0][j] = points[0][j]   * T * Ra * Tr   ; 
        transform->setMatrix(transform->getMatrix()   * T * Ra * Tr ); 
    }
    osg::Matrix R; 
    R.makeRotate( Pi180*_angle*rotatedir, axis ); 	 
    lop(j,0,points[0].size())points[0][j] = points[0][j] * T * R * Tr  ; 
    transform->setMatrix(transform->getMatrix()   * T  *  R  * Tr   ); 
        
    for(int i=index-1;i>=0;i--)	{ 
        osg::Matrix Ra; 
        Ra.makeRotate( Pi180*ve[i]->angle, ve[i]->axis ); 
        lop(j,0,points[0].size())points[0][j] = points[0][j]    * T * Ra * Tr   ; 
        transform->setMatrix(transform->getMatrix()   * T * Ra * Tr    );
    } 
    redrawarrays();
    // (osg::DrawArrays*)drw->dirty();
    // (osg::DrawArrays*)drw->set(GL_LINES,0, points->size());
    // geometry ->setPrimitiveSet(0,(osg::DrawArrays*)drw); 
    
    //todas as posteriores teem que rodar tambem 
    lop(i,index+1,ve.size()-0){
        // cot(ve[i]->nodesstr);
                
        lop(jj,0,index)	{		
            osg::Matrix Ra; 
            Ra.makeRotate( Pi180*-ve[jj]->angle, ve[jj]->axis ); 
            lop(j,0,ve[i]->points[0].size())ve[i]->points[0][j] = ve[i]->points[0][j]   * T * Ra * Tr   ; 
            ve[i]->transform->setMatrix(ve[i]->transform->getMatrix()   * T * Ra * Tr ); 
        }
        
        ve[i]->transform->setMatrix(ve[i]->transform->getMatrix()   * T * R * Tr   );
        lop(j,0,ve[i]->points[0].size()){ 
            ve[i]->points[0][j] = ve[i]->points[0][j]   * T * R * Tr   ;  
        }
        
        for(int jj=index-1;jj>=0;jj--)	{		
            osg::Matrix Ra; 
            Ra.makeRotate( Pi180*ve[jj]->angle, ve[jj]->axis ); 
            lop(j,0,ve[i]->points[0].size())ve[i]->points[0][j] = ve[i]->points[0][j]   * T * Ra * Tr   ; 
            ve[i]->transform->setMatrix(ve[i]->transform->getMatrix()   * T * Ra * Tr ); 
        }
        
        ve[i]->redrawarrays();
        // ve[i]->drw->dirty();
        // ve[i]->drw->set(GL_LINES,0, ve[i]->points->size());
        // ve[i]->geometry ->setPrimitiveSet(0,ve[i]->drw); 
    }

    // transform->accept(*cbv);
    angle+=_angle;
    
    //modulo float
    angle = angle - int( angle/360.0 )*360.0 ;
    
    mflog(flog,angle);
    
    //acende luz no fk 
    turn_light_on(index,angle);

    fldbg_pos(); /////
    // bound_box();
    // first_mtx.unlock();
    first_mtx.unlock();
};
void osgdr::rotateik( float _angle ){   
    vec3 axisb=*axisbeginik;  
            
    osg::Matrix Tr;
    Tr.makeTranslate( axisb.x(),axisb.y(),axisb.z() );
    osg::Matrix T; 
    T.makeTranslate( -axisb.x(),-axisb.y(),-axisb.z() ); 
    
    lop(i,0,index){		
        osg::Matrix Ra; 
        Ra.makeRotate( Pi180*-ve[i]->angleik, ve[i]->axis ); 
        lop(j,0,pointsik[0].size())pointsik[0][j] = pointsik[0][j]   * T * Ra * Tr   ; 
        // transform->setMatrix(transform->getMatrix()   * T * Ra * Tr ); 
    }
    osg::Matrix R; 
    R.makeRotate( Pi180*_angle*rotatedir, axis ); 	 
    lop(j,0,pointsik[0].size())pointsik[0][j] = pointsik[0][j] * T * R * Tr  ; 
    // transform->setMatrix(transform->getMatrix()   * T * R * Tr   );
        
    for(int i=index-1;i>=0;i--)	{ 
        osg::Matrix Ra; 
        Ra.makeRotate( Pi180*ve[i]->angleik, ve[i]->axis ); 
        lop(j,0,pointsik[0].size())pointsik[0][j] = pointsik[0][j]    * T * Ra * Tr   ; 
        // transform->setMatrix(transform->getMatrix()   * T * Ra * Tr    );
    } 

    //todas as posteriores teem que rodar tambem 
    lop(i,index+1,ve.size()-0){
        // cot(ve[i]->nodesstr);
                
        lop(jj,0,index)	{		
            osg::Matrix Ra; 
            Ra.makeRotate( Pi180*-ve[jj]->angleik, ve[jj]->axis ); 
            lop(j,0,ve[i]->pointsik[0].size())ve[i]->pointsik[0][j] = ve[i]->pointsik[0][j]   * T * Ra * Tr   ; 
            // ve[i]->transform->setMatrix(ve[i]->transform->getMatrix()   * T * Ra * Tr ); 
        }
        
        // ve[i]->transform->setMatrix(ve[i]->transform->getMatrix()   * T * R * Tr   );
        lop(j,0,ve[i]->pointsik[0].size()){ 
            ve[i]->pointsik[0][j] = ve[i]->pointsik[0][j]   * T * R * Tr   ;  
        }
        
        for(int jj=index-1;jj>=0;jj--)	{		
            osg::Matrix Ra; 
            Ra.makeRotate( Pi180*ve[jj]->angleik, ve[jj]->axis ); 
            lop(j,0,ve[i]->pointsik[0].size())ve[i]->pointsik[0][j] = ve[i]->pointsik[0][j]   * T * Ra * Tr   ; 
            // ve[i]->transform->setMatrix(ve[i]->transform->getMatrix()   * T * Ra * Tr ); 
        }
        
        // ve[i]->drw->dirty();
        // ve[i]->drw->set(GL_LINES,0, ve[i]->points->size());
        // ve[i]->geometry ->setPrimitiveSet(0,ve[i]->drw); 
    }

    angleik+=_angle;

}; 

//returns angles to given point
// posv posik(vec3 topoint,float x=0,float y=0,float z=1000){
posv* osgdr::posik(vec3 topoint, vbool lock_axle){
		copy_points_k();
		// dbg_force=1;
		// fldbg_pos();
		// posa_debug=0;
		float precision=1;
		int sz=ve.size(); 
		vfloat angles(sz);
		lop(i,0,ve.size())
			angles[i]=ve[i]->angle;
		sz+=1; ///eixo z, depois implementar o x e o y
		// vec3 axisb=*axisbegin;
		#define cdist distance_two_points(&topoint,axisendik)
		 
		// cot(axisb);
		// cot(arm_len);
		// lop(i,0,ve.size())cout<<ve[i]->arm_len<<" ";cout<<endl;
		// cot(*axisendik)
		// cot(*axisend) 
		float ld=cdist;
		vfloat cdir(sz);
		lop(i,0,sz)cdir[i]=precision;  //aqui o z tem a mesma precisao k a rotaçao
		// cdir[sz-1]=-20;
		for(int wi=0;wi<1000;wi++){
			for(int vi=sz-2;vi>=0;vi--){ //-2 é o z //o ultimo n tem angulo
			// cot1(lock_axle[vi]);
				if(lock_axle[vi])continue;
				if(vi<sz-1)if( ve[vi]->angleik >  ve[vi]->anglemax || ve[vi]->angleik <  ve[vi]->anglemin ){ 
					cdir[vi]*=-1;
					ve[vi]->rotateik(cdir[vi]*1);
				}
				ve[vi]->rotateik(cdir[vi]);
				if(ld<=cdist){
					cdir[vi]*=-1;
					ve[vi]->rotateik(cdir[vi]); 
				}  
				// cot(ve[vi]->angle);
				angles[vi]=ve[vi]->angleik;
				// cot(vi)
				// pausa
				// cot(angles);
			}
			movz_ik(cdir[sz-1]);
			if(ld<=cdist){
				cdir[sz-1]*=-1;
				movz_ik(cdir[sz-1]); 
			}  
			
			ld=cdist;
			// cot(ld);
			// if(wi%100==0) cot(ld);
			// cot(cdir);
			// cot(angles);
			if(wi%3500==0){
				lop(i,0,sz)cdir[i]=precision;
			}
		}
		// cot(ld);
		// cot(angles);
		// cot(*axisendik)
		// cot(*axisend)
		posv* resp=new posv;
		resp->p=angles;
		// res.z=1000;
		resp->z=ve[0]->axisbeginik->z();
		// posv r(angles,{},0,0,0);
		return resp;
	}


void arm_len_fill(){
	lop(i,0,ve.size()-1){
		ve[i]->arm_len=distance_two_points(ve[i]->axisbegin,ve[i+1]->axisbegin);
		cot(ve[i]->arm_len);
	}
}

void geraeixos(Group* group){ 

	//robot Origin from cad
	vec3 offset(499.5,580,-1000);


	int idx;
	{
		idx=0;
		ve.resize(idx+1);
		ve[idx]=new osgdr(group);  
		ve[idx]->nodesstr.push_back("stl/servo70.stl"); 
		ve[idx]->nodesstr.push_back("stl/servo70_arm.stl"); 
		ve[idx]->nodesstr.push_back("stl/robot_body.stl");
		ve[idx]->nodesstr.push_back("stl/robot_bucket.stl");
		ve[idx]->axis=vec3(0,1,0);
		ve[idx]->anglemax=230;
		ve[idx]->anglemin=-90;
		vec3 axle_point(110,174,-21);

		vec3 v1(offset.x() + axle_point.x(), offset.y() + axle_point.y(), offset.z() + axle_point.z());
		vec3 v2=v1;
		v2.z()+=50;
		ve[idx]->newdr(v1, v2);
	}
	{
		idx=1;
		ve.resize(idx+1);
		ve[idx]=new osgdr(group);  
		ve[idx]->nodesstr.push_back("stl/robot_arm0.stl");
		ve[idx]->nodesstr.push_back("stl/servo70_arm0.stl");
		ve[idx]->axis=vec3(1,0,0);
		ve[idx]->anglemin=-110;
		ve[idx]->anglemax=110;
		vec3 axle_point(5,138,-21);

		vec3 v1(offset.x() + axle_point.x(), offset.y() + axle_point.y(), offset.z() + axle_point.z());
		vec3 v2=v1;
		v2.y()+=50;
		ve[idx]->newdr(v1, v2);
	}
	{
		idx=2;
		ve.resize(idx+1);
		ve[idx]=new osgdr(group);  
		ve[idx]->nodesstr.push_back("stl/robot_arm1.stl"); 
		ve[idx]->axis=vec3(0,0,1); 
		ve[idx]->anglemin=0;
		ve[idx]->anglemax=90;
		vec3 axle_point(-67,135,-59);

		vec3 v1(offset.x() + axle_point.x(), offset.y() + axle_point.y(), offset.z() + axle_point.z());
		vec3 v2=v1;
		v2.x()+=50;
		ve[idx]->newdr(v1, v2);
	}
	{
		idx=3;
		ve.resize(idx+1);
		ve[idx]=new osgdr(group);  
		ve[idx]->nodesstr.push_back("stl/robot_arm2.stl"); 
		ve[idx]->axis=vec3(0,0,1);
		ve[idx]->anglemin=-(320-60);
		ve[idx]->anglemax=60;
		vec3 axle_point(-287,135,-59);

		vec3 v1(offset.x() + axle_point.x(), offset.y() + axle_point.y(), offset.z() + axle_point.z());
		vec3 v2=v1;
		v2.x()+=50;
		ve[idx]->newdr(v1, v2);
	}
	{
		idx=4;
		ve.resize(idx+1);
		ve[idx]=new osgdr(group);  
		ve[idx]->nodesstr.push_back("stl/robot_wrist.stl"); 
		ve[idx]->axis=vec3(0,1,0);
		ve[idx]->anglemin=0;
		ve[idx]->anglemax=170;
		vec3 axle_point(-287,-82,-21);

		vec3 v1(offset.x() + axle_point.x(), offset.y() + axle_point.y(), offset.z() + axle_point.z());
		vec3 v2=v1;
		v2.y()-=50;
		ve[idx]->newdr(v1, v2);
	}


}
void geraeixos_v1(Group* group){ 
	
	// vec3* offset=new vec3(0,0,0);
	//robot offset
	int off_bucket=80; //off_bottom_from_250bucket 130
	vec3* offset=new vec3(610-110,272+200,-300);
	
	
	// carril=new osgdr(group);
	// carril->nodesstr.push_back("stl/robot_nema23.stl"); 
	// carril->offset=offset;
	// carril->newdr(vec3(110,250,-20),vec3(110,250,200));
	
	// {
	// int idx;
	// idx=0;
	// ve.resize(idx+1);
	// ve[idx]=new osgdr(group); 
	// // ve[idx]->nodesstr.push_back("stl/p1_001.stl");
	// // ve[idx]->nodesstr.push_back("stl/p1_002.stl");
	// // ve[idx]->nodesstr.push_back("stl/p2_001.stl");
	// ve[idx]->nodesstr.push_back("stl/p_c_profile corners_001.stl");
	// ve[idx]->nodesstr.push_back("stl/p_c_profile corners_002.stl");
	// ve[idx]->axis=vec3(1,0,0);
	// ve[idx]->newdr(vec3(0,0,0),vec3(0,0,0));
	// }
	// return;

	int idx;
	idx=0;
	ve.resize(idx+1);
	ve[idx]=new osgdr(group); 
	// ve[idx]->nodesstr.push_back("stl/test2.stl");
	ve[idx]->nodesstr.push_back("stl/robot morango_corpo.stl");
	ve[idx]->nodesstr.push_back("stl/robot morango_balde.stl");
	ve[idx]->nodesstr.push_back("stl/robot morango_servospt70.stl"); 
	ve[idx]->axis=vec3(0,1,0);
	ve[idx]->anglemax=230;
	ve[idx]->anglemin=-90;
	ve[idx]->offset=offset;
	ve[idx]->newdr(vec3(110,250-off_bucket,-20),vec3(110,250-off_bucket,200));

	// goffset(offset);return;
	
	idx=1;
	ve.resize(idx+1);
	ve[idx]=new osgdr(group); 
	ve[idx]->nodesstr.push_back("stl/robot morango_servospt70_1.stl");
	ve[idx]->nodesstr.push_back("stl/robot morango_armj1.stl");
	ve[idx]->axis=vec3(1,0,0);
	ve[idx]->anglemin=-20;
	ve[idx]->anglemax=200;
	// ve[idx]->rotatedir=-1;
	ve[idx]->offset=offset;
	ve[idx]->newdr(vec3(0,127.18,-42.0),vec3(0,127.18+50 ,-42.0));
			
	// // idx=2;
	// // ve.resize(idx+1);
	// // ve[idx]=new osgdr(group); 
	// // ve[idx]->nodesstr.push_back("stl/robot morango_armj1_1.stl"); 
	// // ve[idx]->axis=vec3(0,0,1); 
	// // ve[idx]->anglemax=160;
	// // ve[idx]->anglemin=-160;
	// // ve[idx]->offset=offset;
	// // ve[idx]->newdr(vec3(-55,228,0),vec3(-50,228,0));
	
	idx=2;
	ve.resize(idx+1);
	ve[idx]=new osgdr(group); 
	ve[idx]->nodesstr.push_back("stl/robot morango_armj2.stl"); 
	ve[idx]->nodesstr.push_back("stl/robot morango_servospt70_2.stl"); 
	ve[idx]->axis=vec3(0,1,0); 
	ve[idx]->anglemin=0;
	ve[idx]->anglemax=90;
	ve[idx]->offset=offset;
	ve[idx]->newdr(vec3(-66.61,171 ,-42.19),vec3(-150,171 ,-42.19));

	idx=3;
	ve.resize(idx+1);
	ve[idx]=new osgdr(group); 
	ve[idx]->nodesstr.push_back("stl/robot morango_armj3.stl"); 
	ve[idx]->axis=vec3(0,1,0);
	ve[idx]->anglemin=0;
	ve[idx]->anglemax=160;
	ve[idx]->offset=offset;
	ve[idx]->newdr(vec3(-325.77,171,-42.19),vec3(-400,171,-42.19));
	

	
	// goffset(offset);
	// return;


	idx=4;
	ve.resize(idx+1);
	ve[idx]=new osgdr(group); 
	ve[idx]->nodesstr.push_back("stl/robot morango_armj4.stl"); 
	ve[idx]->axis=vec3(1,0,0);
	ve[idx]->anglemin=0;
	ve[idx]->anglemax=170;
	ve[idx]->offset=offset;
	int y=127;
	ve[idx]->newdr(vec3(-459.61,y,-42.19),vec3(-600,y,-42.19));
 


	goffset(offset);

	

	
 
}
osg::ref_ptr<osg::Node> maquete;
osg::ref_ptr<osg::Node> ucs_icon;
vector<osg::ref_ptr<osg::Node>> cube10(8);
bool toggletranspbool=0;
void toggletransp(){
	toggletranspbool=!toggletranspbool;
	lop(i,0,ve.size()){
		lop(j,0,ve[i]->nodes.size()){
			setTransparency(ve[i]->nodes[j],toggletranspbool);
		}
	}

} 

void bound_box(){
	//https://stackoverflow.com/questions/36830660/how-to-get-aabb-bounding-box-from-matrixtransform-node-in-openscenegraph https://groups.google.com/g/osgworks-users/c/K4px3UQXOew https://gamedev.stackexchange.com/questions/60505/how-to-check-for-cube-collisions
	// osg::ComputeBoundsVisitor* cbv=new osg::ComputeBoundsVisitor;
	// ve[0]->transform->accept(*cbv); 
	
	osg::ComputeBoundsVisitor cbv;
	ve[0]->transform->accept(cbv);
	osg::BoundingBox bb = cbv.getBoundingBox(); // in local coords.
	
	osg::ComputeBoundsVisitor cbv3;
	ve[3]->transform->accept(cbv3);
	osg::BoundingBox bb3 = cbv3.getBoundingBox(); // in local coords.
	
	// osg::BoundingBox bb = ve[0]->cbv->getBoundingBox();
	// osg::BoundingBox bb3 = ve[3]->cbv->getBoundingBox();
	 
// osg::Matrix localToWorld = osg::computeLocalToWorld( ve[0]->transform->getParentalNodePaths().front());// node->getParent(0)->getParentalNodePaths()[0] );
// osg::Matrix localToWorld = osg::computeLocalToWorld( ve[0]-> nodes[0]->getParent(0)->getParentalNodePaths()[0] );
 // for ( unsigned int i=0; i<8; ++i ) bb.expandBy( bb.corner(i) * localToWorld );
 
// osg::Matrix localToWorld3 = osg::computeLocalToWorld( ve[3]->transform->getParentalNodePaths().front());// node->getParent(0)->getParentalNodePaths()[0] );
// osg::Matrix localToWorld3 = osg::computeLocalToWorld( ve[3]->nodes[0]->getParent(0)->getParentalNodePaths()[0] );
 // for ( unsigned int i=0; i<8; ++i ) bb3.expandBy( bb3.corner(i) * localToWorld );
 
 
	bool intersects=bb.intersects(bb3);
	cot(intersects);
}

#include <osg/PolygonMode>

#include <osgFX/Scribe>
void loadstlfile(osg::Group* group, const std::string& fname)
{
    // 1. Load STL
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(fname);
    if (!node)
    {
        std::cout << "Failed to load STL: " << fname << std::endl;
        return;
    }

    // ------------------------------------------------------------
    // 2. Recursively remove ALL normals
    // ------------------------------------------------------------

    std::function<void(osg::Node*)> stripNormals = [&](osg::Node* n)
    {
        osg::Geode* geode = dynamic_cast<osg::Geode*>(n);
        if (geode)
        {
            for (unsigned int i = 0; i < geode->getNumDrawables(); ++i)
            {
                osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
                if (geom)
                {
                    geom->setNormalArray(nullptr);
                    geom->setNormalBinding(osg::Geometry::BIND_OFF);

                    for (unsigned int a = 0; a < geom->getNumVertexAttribArrays(); ++a)
                        geom->setVertexAttribArray(a, nullptr);
                }
            }
        }

        osg::Group* g = n->asGroup();
        if (g)
        {
            for (unsigned int i = 0; i < g->getNumChildren(); ++i)
                stripNormals(g->getChild(i));
        }
    };

    stripNormals(node);

    // ------------------------------------------------------------
    // 3. Merge vertices
    // ------------------------------------------------------------
    osgUtil::Optimizer opt;
    opt.optimize(node,
        osgUtil::Optimizer::MERGE_GEOMETRY |
        osgUtil::Optimizer::VERTEX_POSTTRANSFORM);

    // ------------------------------------------------------------
    // 4. Recompute normals with crease angle
    // ------------------------------------------------------------
    osgUtil::SmoothingVisitor sv;
    sv.setCreaseAngle(osg::DegreesToRadians(30.0f));
    node->accept(sv);

    // ------------------------------------------------------------
    // 5. Force lighting ON
    // ------------------------------------------------------------
    osg::StateSet* ss = node->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    ss->setMode(GL_LIGHT0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // ------------------------------------------------------------
    // 6. Add a material
    // ------------------------------------------------------------
    osg::ref_ptr<osg::Material> mat = new osg::Material;
    mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1,1,1,1));
    mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.2,0.2,0.2,1));
    mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.1,0.1,0.1,1));
    ss->setAttributeAndModes(mat, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // ------------------------------------------------------------
    // 7. Add silhouette edges (NO diagonals)
    // ------------------------------------------------------------
    osg::ref_ptr<osgFX::Scribe> scribe = new osgFX::Scribe;
    scribe->setWireframeColor(osg::Vec4(0,0,0,1));   // black outline
    scribe->setWireframeLineWidth(1.5f);
    scribe->addChild(node);

    // ------------------------------------------------------------
    // 8. Attach to scene
    // ------------------------------------------------------------
    group->addChild(scribe);
}



osg::ref_ptr<osg::Group> makeCleanNode(osg::Node* child)
{
    osg::ref_ptr<osg::Group> g = new osg::Group;
    osg::StateSet* ss = g->getOrCreateStateSet();

    // ------------------------------------------------------------
    // 1. Clear all inherited states (critical!)
    // ------------------------------------------------------------
    ss->clear();

    // ------------------------------------------------------------
    // 2. Disable backface culling (fixes black interior)
    // ------------------------------------------------------------
    ss->setMode(GL_CULL_FACE,
                osg::StateAttribute::OFF |
                osg::StateAttribute::OVERRIDE |
                osg::StateAttribute::PROTECTED);

    // ------------------------------------------------------------
    // 3. Enable two-sided lighting (OSG < 3.6 compatible)
    // ------------------------------------------------------------
    ss->setMode(GL_LIGHTING,
                osg::StateAttribute::ON |
                osg::StateAttribute::OVERRIDE |
                osg::StateAttribute::PROTECTED);

    ss->setMode(GL_LIGHT_MODEL_TWO_SIDE,
                osg::StateAttribute::ON |
                osg::StateAttribute::OVERRIDE |
                osg::StateAttribute::PROTECTED);

    // ------------------------------------------------------------
    // 4. Normalize normals (fixes negative scales / transforms)
    // ------------------------------------------------------------
    ss->setMode(GL_NORMALIZE,
                osg::StateAttribute::ON |
                osg::StateAttribute::OVERRIDE |
                osg::StateAttribute::PROTECTED);

    // ------------------------------------------------------------
    // 5. Apply a clean material (front & back)
    // ------------------------------------------------------------
    osg::ref_ptr<osg::Material> mat = new osg::Material;
    mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1,1,1,1));
    mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    mat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    ss->setAttributeAndModes(mat,
        osg::StateAttribute::ON |
        osg::StateAttribute::OVERRIDE |
        osg::StateAttribute::PROTECTED);

    // ------------------------------------------------------------
    // 6. Attach the STL node under this clean wrapper
    // ------------------------------------------------------------
    g->addChild(child);

    return g;
}

void makeCadStyleNode_v0(osg::ref_ptr<osg::Group> root,osg::Node* model)
{
    if (!model)
        return;// nullptr;

    // osg::ref_ptr<osg::Group> root = new osg::Group;

    /* ============================================================
       PASS 1: WHITE SHADED FACES
       ============================================================ */

    osg::ref_ptr<osg::Node> faceNode =
        dynamic_cast<osg::Node*>(model->clone(osg::CopyOp::DEEP_COPY_ALL));

    osg::StateSet* fs = faceNode->getOrCreateStateSet();
    fs->clear();
    fs->removeAttribute(osg::StateAttribute::PROGRAM);

    fs->setMode(GL_LIGHTING,       osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_CULL_FACE,      osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_ARRAY,    osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    fs->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> faceMat = new osg::Material;
    faceMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.95f,0.95f,0.95f,1));
    faceMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0.95f,0.95f,0.95f,1));
    faceMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    faceMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    fs->setAttributeAndModes(faceMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(faceNode.get());

    /* ============================================================
       PASS 2: CAD EDGES (BOUNDARY + FEATURE)
       ============================================================ */

    osg::ref_ptr<osg::Geode> edgeGeode = new osg::Geode;

    struct EdgeKey {
        unsigned a, b;
        EdgeKey(unsigned x=0, unsigned y=0) {
            if (x < y) { a = x; b = y; }
            else       { a = y; b = x; }
        }
        bool operator<(const EdgeKey& o) const {
            return a < o.a || (a == o.a && b < o.b);
        }
    };

    struct EdgeInfo {
        osg::Vec3 normalSum;
        unsigned count = 0;
    };

    std::map<EdgeKey, EdgeInfo> edgeMap;

    struct Extractor : public osg::NodeVisitor
    {
        std::map<EdgeKey, EdgeInfo>& edgeMap;

        Extractor(std::map<EdgeKey, EdgeInfo>& m)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), edgeMap(m) {}

        void addTriangles(osg::Vec3Array* v, osg::DrawElementsUInt* idx)
        {
            for (unsigned i = 0; i + 2 < idx->size(); i += 3)
            {
                unsigned a = (*idx)[i];
                unsigned b = (*idx)[i+1];
                unsigned c = (*idx)[i+2];

                osg::Vec3 n = ((*v)[b] - (*v)[a]) ^ ((*v)[c] - (*v)[a]);
                n.normalize();

                EdgeKey e1(a,b), e2(b,c), e3(c,a);

                edgeMap[e1].normalSum += n; edgeMap[e1].count++;
                edgeMap[e2].normalSum += n; edgeMap[e2].count++;
                edgeMap[e3].normalSum += n; edgeMap[e3].count++;
            }
        }

        void apply(osg::Geode& geode) override
        {
            for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
            {
                osg::Geometry* g =
                    dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (!g) continue;

                osg::Vec3Array* v =
                    dynamic_cast<osg::Vec3Array*>(g->getVertexArray());
                if (!v) continue;

                for (unsigned p = 0; p < g->getNumPrimitiveSets(); ++p)
                {
                    osg::PrimitiveSet* ps = g->getPrimitiveSet(p);
                    if (ps->getMode() != GL_TRIANGLES) continue;

                    osg::ref_ptr<osg::DrawElementsUInt> tmp =
                        new osg::DrawElementsUInt(GL_TRIANGLES);

                    if (auto* u = dynamic_cast<osg::DrawElementsUInt*>(ps))
                    {
                        for (unsigned i = 0; i < u->size(); ++i)
                            tmp->push_back((*u)[i]);
                    }
                    else if (auto* s = dynamic_cast<osg::DrawElementsUShort*>(ps))
                    {
                        for (unsigned i = 0; i < s->size(); ++i)
                            tmp->push_back((*s)[i]);
                    }
                    else if (auto* b = dynamic_cast<osg::DrawElementsUByte*>(ps))
                    {
                        for (unsigned i = 0; i < b->size(); ++i)
                            tmp->push_back((*b)[i]);
                    }
                    else if (auto* a = dynamic_cast<osg::DrawArrays*>(ps))
                    {
                        for (int i = 0; i < a->getCount(); ++i)
                            tmp->push_back(a->getFirst() + i);
                    }

                    addTriangles(v, tmp.get());
                }
            }
        }
    };

    Extractor ex(edgeMap);
    model->accept(ex);

    /* ============================================================
       FIND ANY VERTEX ARRAY
       ============================================================ */

    osg::Vec3Array* vertices = nullptr;

    struct VAVisitor : public osg::NodeVisitor {
        osg::Vec3Array* result = nullptr;
        VAVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
        void apply(osg::Geode& geode) override {
            for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
            {
                if (auto* g = dynamic_cast<osg::Geometry*>(geode.getDrawable(i)))
                {
                    if (auto* v = dynamic_cast<osg::Vec3Array*>(g->getVertexArray()))
                    {
                        result = v;
                        return;
                    }
                }
            }
        }
    };

    VAVisitor vav;
    faceNode->accept(vav);
    vertices = vav.result;

    if (!vertices)
        return; // no geometry found
        // return root; // no geometry found

    /* ============================================================
       BUILD EDGE GEOMETRY
       ============================================================ */

    osg::ref_ptr<osg::DrawElementsUInt> lines =
        new osg::DrawElementsUInt(GL_LINES);

    const float featureAngle = osg::DegreesToRadians(30.0f);

    for (auto& kv : edgeMap)
    {
        const EdgeKey& e = kv.first;
        const EdgeInfo& info = kv.second;

        if (info.count == 1)
        {
            // boundary edge
            lines->push_back(e.a);
            lines->push_back(e.b);
        }
        else if (info.count == 2)
        {
            // feature edge?
            osg::Vec3 n = info.normalSum;
            n.normalize();

            float dot = n * n; // sum of two normals
            if (dot < std::cos(featureAngle))
            {
                lines->push_back(e.a);
                lines->push_back(e.b);
            }
        }
    }

    osg::ref_ptr<osg::Geometry> edgeGeom = new osg::Geometry;
    edgeGeom->setVertexArray(vertices);
    edgeGeom->addPrimitiveSet(lines.get());
    edgeGeode->addDrawable(edgeGeom.get());

    osg::StateSet* es = edgeGeode->getOrCreateStateSet();
    es->clear();
    es->removeAttribute(osg::StateAttribute::PROGRAM);

    es->setMode(GL_LIGHTING,  osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    es->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth;
    lw->setWidth(1.8f);
    es->setAttributeAndModes(lw.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::ref_ptr<osg::Material> edgeMat = new osg::Material;
    edgeMat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    edgeMat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

    es->setAttributeAndModes(edgeMat.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    root->addChild(edgeGeode.get());

    // return root;
}
struct StripNormalsAndColors : public osg::NodeVisitor
{
    StripNormalsAndColors() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}

    void apply(osg::Geode& geode) override
    {
        for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Geometry* g = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
            if (!g) continue;

            // Remove normals
            g->setNormalArray(nullptr);
            g->setNormalBinding(osg::Geometry::BIND_OFF);

            // Remove any Inventor per-vertex color garbage
            g->setColorArray(nullptr);
            g->setColorBinding(osg::Geometry::BIND_OFF);
        }
    }
};

void loadstl(Group* group){
	
	// loadstlfile(group,"stl/test2.stl");

    // maquete = osgDB::readRefNodeFile("stl/test.stl");

    // maquete = osgDB::readRefNodeFile("stl/maquete.stl");
    maquete = osgDB::readRefNodeFile("stl/test2.stl");
// ZShift shifter(-0.0000001f);
// maquete->accept(shifter);



StripNormalsAndColors cleaner;
maquete->accept(cleaner);

makeCadStyleNode(group,maquete,osg::Vec4(0.95f, 0.95f, 0.95f, 0.85f));
// makeCadStyleNode(group,maquete,osg::Vec4(0.95f, 0.95f, 0.95f, 1.0f));
// setCadFacesTransparency(group,0.7);

// osg::ref_ptr<osg::Node> cadNode = makeCadStyleNode_v1(maquete);
// group->addChild(cadNode.get());




// auto clean = makeCleanNode(maquete.get()); group->addChild(clean.get());




    // // maquete = osgDB::readRefNodeFile("stl/maquete.stl");
    // // maquete = osgDB::readRefNodeFile("stl/all_shapes_fused.stl");
	// cotm("readn");
	// settranparency(maquete.get(),0);
	// group->addChild(maquete.get());
// osg::StateSet* ss = maquete->getOrCreateStateSet();

// ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
// ss->setMode(GL_LIGHT0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

// osg::ref_ptr<osg::Material> mat = new osg::Material;
// mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1,1,1,1));
// mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.2,0.2,0.2,1));
// ss->setAttributeAndModes(mat, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

// osgUtil::SmoothingVisitor sv;
// sv.setCreaseAngle(osg::DegreesToRadians(30.0f));
// maquete->accept(sv);


// osg::ref_ptr<osg::Light> light = new osg::Light;
// light->setLightNum(0);
// light->setPosition(osg::Vec4(0,0,0,1));
// light->setDirection(osg::Vec3(0,0,-1));

// osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
// ls->setLight(light);

// // Add to scene graph, not the render camera
// group->addChild(ls);

// // Enable the light
// group->getOrCreateStateSet()->setMode(GL_LIGHT0, osg::StateAttribute::ON);

// // Ensure normals exist
// osgUtil::SmoothingVisitor sv;
// maquete->accept(sv);
 

	
	
    ucs_icon = osgDB::readRefNodeFile("stl/3DUCSICON2.stl");
	settranparency(ucs_icon.get(),1);
	group->addChild(ucs_icon.get());
	
	// int plat_x=8, plat_y=4, plat_z=37, plat_zblock=4;
	int plat_x=4, plat_y=1, plat_z=2, plat_zblock=4;
	vector<vec3*> bdcpos(8);
	lop(i,0,cube10.size()){
		cube10[i] = osgDB::readRefNodeFile("stl/cube10.stl");
		settranparency(cube10[i].get(),1);
		MatrixTransform* trfm=new MatrixTransform;
		group->addChild(trfm);
		trfm->addChild(cube10[i].get());
		int x=i%4;
		int z=0;
		if(i>3)z=1;
		bdcpos[i]=new vec3(150+300*x,150,-150+-300*z);
		cot(*bdcpos[i]);
		osg::Matrix Trf;
		Trf.makeTranslate( bdcpos[i]->x(),bdcpos[i]->y(),bdcpos[i]->z() );
		trfm->setMatrix(trfm->getMatrix() *  Trf );	
	}

    // osg::ref_ptr<osg::Node> estrutura = osgDB::readRefNodeFile("Assembly1.stl");
	// settranparency(estrutura.get(),1);
	// group->addChild(estrutura.get());
	
};

// std::atomic<bool> running(true);
bool all_cancel=0;
int funcid=0;
//topoint faz override ao p
//vai ao posapool p=angles
//1000 é = ao anterior no p e  no z, x=0 e y=0 é =
// o topoint faz override ao p e lock_axle only works with posik
// posa({},{},0,0,0,0,0,{});
 
void posa(vfloat pangles, vec3 topoint, float x, float y, float z,int speedtype,int funcn, vbool lock_axle){

	posv* pov=new posv{pangles,topoint,0,0,z,0,0,funcn,lock_axle};

	vector<vec3> linearpov;
	z=pov->z;
	
	//here can be more complete with all axes topoint 0, we assume robot arm never goes 0,0,0
	if(pov->topoint.x()!=0){ 
		// cout<<"FROM POINT "<<*ve[4]->axisend<<endl;
		// cout<<"TO POINT "<<pov->topoint<<endl; 
		cotm(*ve[4]->axisend,pov->topoint);
		posv* pv=ve[4]->posik(pov->topoint,pov->lock_axle);
		pov->p=pv->p;
		z=pv->z;
		delete pv;
		pv=0;
	}
	pov->posa_counter={pov->p.size()};
	lop(i,0,pov->p.size()){ 
		float newangle= pov->p[i];
		// cot(newangle)
		if(newangle>=999){ 
			pov->posa_counter--; 
			continue;
		}
		// cot1(pov->p.size());
		ve[i]->rotatetoposition(newangle,pov); 
	}
	if(z<1000){  
		pov->posa_counter++;
		movetoposz(z,pov); 
	} 
	while(pov->posa_counter.load()>0){
		sleepms(200);
	}
	delete pov;
	pov=0;
}

void posi(int index,float angle){
	vfloat angles(ve.size());
	// posa_erase_mtx.lock();
	// cot(pool.back().p);
	lop(i,0,ve.size())angles[i]=ve[i]->angle;
	// for(int j=pool.size()-1;j>=0;j--)
		// if(pool[j].p.size()>0){
			// lop(i,0,pool[j].p.size())angles[i]=pool[j].p[i]; 
			// break;
		// }	
	// for(int j=pool.size()-1;j>=0;j--)
	// 	if(pool[j]->p.size()>0){
	// 		lop(i,0,pool[j]->p.size())angles[i]=pool[j]->p[i]; 
	// 		break;
	// 	}
	// posa_erase_mtx.unlock();
	angles[index]=angle;
	// cout<<"ii"<<index<<endl;
	// cot(index);
	// cot(posapool.size());
	// cot(angles);
	posa(angles);
	
}

void posik(vec3 vv, vbool lock_axle){
	// copy_points_k();
	// dbg_force=1;
	// fldbg_pos();
	// cot("copied ik");
	// pos_k(vfloat{0,20,20,70});
	
	// vfloat angles=ve[3]->posik(vv);
	// posa(angles);
	// posa({},vv);
	posa({},vv,0,0,0,0,0,lock_axle);
	return;
	// lop(i,0,ve.size()){
		// ve[i]->rotate_posk(angles[i]);
		// ve[i]->rotatetoposition(angles[i]);
	// }
	// cot(angles);
}

void movz(float z){
	posa({},{},0,0,z);
}

//tem de copiar tambem o angulo
void copy_points_k(){ 
	lop(i,0,ve.size()){ 
		ve[i]->pointsik->clear();
		ve[i]->pointsik->push_back(ve[i]->points[0][0]);
		ve[i]->pointsik->push_back(ve[i]->points[0][1]); 
		ve[i]->axisbeginik=&ve[i]->pointsik[0][0];
		ve[i]->axisendik=&ve[i]->pointsik[0][1]; 
		ve[i]->angleik=ve[i]->angle;
	}
}
void pos_k(vfloat angles){ 
	lop(i,0,angles.size()){ 
		ve[i]->rotate_posk(angles[i]);
	}
	
}

void goffset(vec3* offset){
	osg::Matrix Trf;
	Trf.makeTranslate( offset->x(),offset->y(),offset->z() );
	lop(i,0,ve.size()){
		ve[i]->transform->setMatrix(ve[i]->transform->getMatrix() *  Trf );		
		lop(j,0,ve[i]->points[0].size())ve[i]->points[0][j] = ve[i]->points[0][j]  * Trf  ; //points axisbegin actualiza
        ve[i]->redrawarrays();
		// ve[i]->drw->dirty();
		// ve[i]->drw->set(GL_LINES,0, ve[i]->points->size());
		// ve[i]->geometry ->setPrimitiveSet(0,ve[i]->drw); 
	}
	//carril
	// carril->transform->setMatrix(carril->transform->getMatrix() *  Trf );	
	
	copy_points_k();
}

 void movz_ik(float z){
	if(abs(z)<3)return; //evitate tremelics
	osg::Matrix Trf;
	Trf.makeTranslate( 0,0,z );	
	lop(i,0,ve.size())	
		lop(j,0,ve[i]->pointsik[0].size())ve[i]->pointsik[0][j] = ve[i]->pointsik[0][j]  * Trf  ;

}

void movetoposz(float z, posv* &pov){
	thread th([](float z, posv* pov){
		float currz=(float)((*ve[0]->axisbegin).z());
		// if(abs(currz-z)<3)return; //evitate tremelics
		float rz=z-currz;
		float speed=2;
		// cot (currz);
		// cot(rz);
		float newz=rz;
		int dir=1;
		if(z<=currz){dir=-1;   }
		for(;;){
			// mtxlock(0);
			first_mtx.lock();
			if(pressuref>2){
				// pov->cancel=1;
			}
			// cot(dir);
			// cot(newz);
			if(pov->cancel){first_mtx.unlock();break;}
			if(pov->pause){
				first_mtx.unlock();
				while(pov->pause)sleepms(200);
				first_mtx.lock();
			}
			if(dir==1 && newz<=0){first_mtx.unlock();break;}
			if(dir==-1 && newz>=0){first_mtx.unlock();break;}
			if(dir==1)newz-=speed;else newz+=speed;
			osg::Matrix Trf;
			Trf.makeTranslate( 0,0,speed*dir );	
			lop(i,0,ve.size()){	
				ve[i]->transform->setMatrix(ve[i]->transform->getMatrix() *  Trf );	
				lop(j,0,ve[i]->points[0].size())ve[i]->points[0][j] = ve[i]->points[0][j]  * Trf  ; 
                ve[i]->redrawarrays();
				// ve[i]->drw->dirty();
				// ve[i]->drw->set(GL_LINES,0, ve[i]->points->size());
				// ve[i]->geometry ->setPrimitiveSet(0,ve[i]->drw); 
			}
			// if((*ve[0]->axisbegin).z()>=0 && rotate_pos!=0){
				// ve[0]->rotatetoposition(ve[0]->angle+90,pov);
				// rotate_pos=1;
			// }
			fldbg_pos(); ////
			// first_mtx.unlock();
			first_mtx.unlock();
			sleepms(5);
				// cot("PC");
		}
				
		// posa_counter_mtx.lock();
		// cot(pov->posa_counter);
		pov->posa_counter--;
		// posa_counter_mtx.unlock();
		// fldbg_pos();
		
	},z,pov);
	th.detach();
}