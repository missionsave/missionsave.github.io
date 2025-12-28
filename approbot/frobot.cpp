
// #include <X11/Xlib.h>
// #include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Gl_Window.H>
// #include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
// #include <FL/Fl_Scroll.H>
// #include <Fl/Fl_Value_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Help_View.H>
// #include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H> 
// #include <FL/Fl_Choice.H>
// #include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <malloc.h>
#include <filesystem>
#include <functional>

#include "GraphicsCostEstimator_patch.h"
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/OrbitManipulator>
#include <osgGA/TrackballManipulator>
#include <osg/PolygonMode>
#include <osg/LineWidth>
#include <osg/Material>

#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/Version>

#include <osgFX/Outline>

#include <osgUtil/SmoothingVisitor>
#include <osg/LightModel>
#include <osg/PolygonOffset>
#include <osg/ComputeBoundsVisitor>
#include <osgFX/Cartoon>

#include <osg/ShadeModel>
// #include "fl_scintilla.hpp"
#include "frobot.hpp"

Fl_Double_Window* win;
Fl_Double_Window* flt; 
Fl_Double_Window* flbd; 
Fl_Menu_Bar* menu;
struct scint;
extern scint* editor;
void scint_init(int x,int y,int w,int h); 

#define cot1
using namespace std;
namespace fs = std::filesystem;
using namespace osg;

vector<vector<Fl_Button*>> btp;
struct ViewerFLTK;
ViewerFLTK* osggl;
osg::Group* group;
// int Break=0;
bool makvisible=1;
bool robotvisible=1;

#if defined(__linux__)
int startstream(Window _win);
#endif


// bool is_minimized = false;
std::atomic<bool> is_minimized = false;
auto last_event = std::chrono::steady_clock::now(); 


osg::Vec3d calculateWorldIntersectionPivot(int mouseX, int mouseY, osgViewer::Viewer* viewer)
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
        new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, mouseX, mouseY);
    osgUtil::IntersectionVisitor iv(intersector.get());
    viewer->getCamera()->accept(iv);

    if (intersector->containsIntersections()) {
        return intersector->getFirstIntersection().getWorldIntersectPoint();
    } else {
        // fallback: centro da bounding box da cena
        osg::BoundingSphere bs = viewer->getSceneData()->getBound();
        return bs.center();
    }
}





// nmutex rdam("rda");
// add an toggle function to enable/disable to liberate ram:
struct ViewerFLTK : public Fl_Gl_Window, public osgViewer::Viewer {

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;

    static ViewerFLTK* instance;

	osgGA::TrackballManipulator* tmr;

    // Add variables for right mouse panning
    bool rightMousePressed;
    int lastMouseX, lastMouseY;

osg::ref_ptr<osg::Group> _sceneRoot;
osg::ref_ptr<osg::MatrixTransform> _rootXform;
osg::Vec3d _pivot;
osg::Vec3d _pivot_local;
bool _leftDown = false;
int _lastDragX = 0, _lastDragY = 0;



    static void Timer_CB(void*) {

        if (instance) instance->redraw();  // Redraw sempre ativo

        Fl::repeat_timeout(1.0 / 10, Timer_CB, nullptr);

    } 
osg::ref_ptr<osgFX::Outline> _outlineEffect;

void applyGlobalGhostedEdges_v1(bool on=1)
{
    if (on)
    {
        if (!_outlineEffect)
        {
            _outlineEffect = new osgFX::Outline;
            _outlineEffect->setWidth(2.0f);                     // espessura da edge
            _outlineEffect->setColor(osg::Vec4(0,0,0,1));       // cor da edge (preto)
        }

        // Mover cena para dentro do efeito
        if (_outlineEffect->getNumChildren() == 0)
        {
            while (_rootXform->getNumChildren() > 0)
            {
                osg::ref_ptr<osg::Node> child = _rootXform->getChild(0);
                _rootXform->removeChild(child);
                _outlineEffect->addChild(child);
            }

            _rootXform->addChild(_outlineEffect);
        }

        // Faces brancas opacas
        osg::StateSet* ss = _outlineEffect->getOrCreateStateSet();

        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1,1,1,1));   // branco opaco
        mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.9,0.9,0.9,1));

        ss->setAttributeAndModes(mat, osg::StateAttribute::ON);
        ss->setMode(GL_BLEND, osg::StateAttribute::OFF);       // sem transparência
        ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);
    }
    else
    {
        if (_outlineEffect && _outlineEffect->getNumChildren() > 0)
        {
            for (unsigned i = 0; i < _outlineEffect->getNumChildren(); ++i)
                _rootXform->addChild(_outlineEffect->getChild(i));

            _outlineEffect->removeChildren(0, _outlineEffect->getNumChildren());
        }

        if (_outlineEffect)
            _rootXform->removeChild(_outlineEffect);
    }
}

void applyGlobalGhostedEdges_v2(bool on = true)
{
    if (on)
    {
        // --------------------------------------------------------------------
        // 1) Create outline effect if needed
        // --------------------------------------------------------------------
        if (!_outlineEffect)
        {
            _outlineEffect = new osgFX::Outline;
            _outlineEffect->setWidth(1.8f);
            _outlineEffect->setColor(osg::Vec4(0, 0, 0, 1));
        }

        // --------------------------------------------------------------------
        // 2) Move all children under outline (only once)
        // --------------------------------------------------------------------
        if (_outlineEffect->getNumChildren() == 0)
        {
            while (_rootXform->getNumChildren() > 0)
            {
                osg::ref_ptr<osg::Node> child = _rootXform->getChild(0);
                _rootXform->removeChild(child);
                _outlineEffect->addChild(child);
            }

            _rootXform->addChild(_outlineEffect);
        }

        // --------------------------------------------------------------------
        // 3) Ensure valid normals (fixes black faces)
        // --------------------------------------------------------------------
        osgUtil::SmoothingVisitor smoother;
        _outlineEffect->accept(smoother);

        // --------------------------------------------------------------------
        // 4) Robust global render state
        // --------------------------------------------------------------------
        osg::StateSet* ss = _outlineEffect->getOrCreateStateSet();

        // --- Lighting ON
        ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // --- Two-sided lighting (fix back faces)
        osg::ref_ptr<osg::LightModel> lm = new osg::LightModel;
        lm->setTwoSided(true);
        ss->setAttributeAndModes(lm, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // --- Normalize normals (fix negative scales)
        ss->setMode(GL_NORMALIZE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // --------------------------------------------------------------------
        // 5) Solid white material (driver-safe)
        // --------------------------------------------------------------------
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
        // mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0.85f, 0.85f, 0.85f, 1));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);

        ss->setAttributeAndModes(mat, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // --------------------------------------------------------------------
        // 6) Force opaque rendering (no ghost transparency)
        // --------------------------------------------------------------------
        ss->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);

        // --------------------------------------------------------------------
        // 7) Guarantee at least one light source
        // --------------------------------------------------------------------
        osg::ref_ptr<osg::Light> light = new osg::Light;
        light->setLightNum(0);
        light->setPosition(osg::Vec4(0, 0, 10, 1));
        light->setDiffuse(osg::Vec4(1, 1, 1, 1));
        light->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1));

        osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
        ls->setLight(light);

        if (!_outlineEffect->containsNode(ls))
            _outlineEffect->addChild(ls);
    }
    else
    {
        // --------------------------------------------------------------------
        // Restore original scene
        // --------------------------------------------------------------------
        if (_outlineEffect)
        {
            for (unsigned i = 0; i < _outlineEffect->getNumChildren(); ++i)
                _rootXform->addChild(_outlineEffect->getChild(i));

            _outlineEffect->removeChildren(0, _outlineEffect->getNumChildren());
            _rootXform->removeChild(_outlineEffect);
        }
    }
}
// osg::ref_ptr<osgFX::Outline> _outlineEffect;
osg::ref_ptr<osg::Group> _wireOverlay;

void applyGlobalGhostedEdges_v3(bool on = true)
{
    if (on)
    {
        // --- Create outline effect if needed ---
        if (!_outlineEffect)
        {
            _outlineEffect = new osgFX::Outline;
            _outlineEffect->setWidth(2.0f);
            _outlineEffect->setColor(osg::Vec4(0,0,0,1));
        }

        // --- Move scene under outline once ---
        if (_outlineEffect->getNumChildren() == 0)
        {
            while (_rootXform->getNumChildren() > 0)
            {
                osg::ref_ptr<osg::Node> c = _rootXform->getChild(0);
                _rootXform->removeChild(c);
                _outlineEffect->addChild(c);
            }
            _rootXform->addChild(_outlineEffect);
        }

        // --- Ensure normals are valid ---
        osgUtil::SmoothingVisitor sv;
        _outlineEffect->accept(sv);

        // --- Solid white material ---
        osg::StateSet* ss = _outlineEffect->getOrCreateStateSet();
        osg::ref_ptr<osg::Material> m = new osg::Material;
        m->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1,1,1,1));
        m->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.85f,0.85f,0.85f,1));
        ss->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // --- Add wireframe overlay for full edges ---
        if (!_wireOverlay)
        {
            osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;
            pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

            osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth(1.0f);

            osg::ref_ptr<osg::PolygonOffset> po = new osg::PolygonOffset(-1.0f, -1.0f);

            osg::StateSet* wss = new osg::StateSet;
            wss->setAttributeAndModes(pm, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            wss->setAttributeAndModes(lw, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            wss->setAttributeAndModes(po, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

            _wireOverlay = new osg::Group;
            _wireOverlay->setStateSet(wss);

            // share children (no clones)
            for (unsigned i = 0; i < _outlineEffect->getNumChildren(); ++i)
                _wireOverlay->addChild(_outlineEffect->getChild(i));

            _outlineEffect->addChild(_wireOverlay);
        }
    }
    else
    {
        // --- Restore original scene ---
        if (_outlineEffect)
        {
            for (unsigned i = 0; i < _outlineEffect->getNumChildren(); ++i)
                if (_outlineEffect->getChild(i) != _wireOverlay)
                    _rootXform->addChild(_outlineEffect->getChild(i));

            _outlineEffect->removeChildren(0, _outlineEffect->getNumChildren());
            _rootXform->removeChild(_outlineEffect);
            _wireOverlay = nullptr;
        }
    }
}

void applyGlobalGhostedEdges(bool on = true)
{
    if (on)
    {
        // 1. Create outline effect if needed
        if (!_outlineEffect)
        {
            _outlineEffect = new osgFX::Outline;
            _outlineEffect->setWidth(3.0f); // Thicker for visibility
            _outlineEffect->setColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Pure black
        }

        // 2. Move children under outline (only once)
        if (_outlineEffect->getNumChildren() == 0)
        {
            while (_rootXform->getNumChildren() > 0)
            {
                osg::ref_ptr<osg::Node> child = _rootXform->getChild(0);
                _rootXform->removeChild(child);
                _outlineEffect->addChild(child);
            }
            _rootXform->addChild(_outlineEffect);
        }

        // 3. Configure state for sharp, persistent edges
        osg::StateSet* ss = _outlineEffect->getOrCreateStateSet();

        // Disable lighting (for pure outline color)
        ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

        // Disable depth test (to avoid occlusion)
        ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

        // Enable polygon offset (to avoid depth fighting)
        ss->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        ss->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // Disable blending (for sharp edges)
        ss->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

        // Force opaque rendering and late render bin
        ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        ss->setRenderBinDetails(11, "RenderBin");

        // Use a flat material (to avoid color overrides)
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 0.0f);
        ss->setAttributeAndModes(mat, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // Disable line smoothing (for sharp edges)
        ss->setMode(GL_LINE_SMOOTH, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    }
    else
    {
        // Restore original scene
        if (_outlineEffect)
        {
            for (unsigned i = 0; i < _outlineEffect->getNumChildren(); ++i)
                _rootXform->addChild(_outlineEffect->getChild(i));
            _outlineEffect->removeChildren(0, _outlineEffect->getNumChildren());
            _rootXform->removeChild(_outlineEffect);
        }
    }
}

void applyOCCStyleEdges(bool on = true)
{
    static osg::ref_ptr<osg::Group> cadRoot;
    static osg::ref_ptr<osg::Group> faceGroup;
    static osg::ref_ptr<osg::Group> edgeGroup;

    if (on)
    {
        if (!cadRoot)
        {
            cadRoot  = new osg::Group;
            faceGroup = new osg::Group;
            edgeGroup = new osg::Group;

            cadRoot->addChild(faceGroup);
            cadRoot->addChild(edgeGroup);
        }

        // ------------------------------------------------------------
        // Move original scene under CAD root
        // ------------------------------------------------------------
        if (cadRoot->getNumChildren() == 2 && faceGroup->getNumChildren() == 0)
        {
            while (_rootXform->getNumChildren() > 0)
            {
                osg::ref_ptr<osg::Node> n = _rootXform->getChild(0);
                _rootXform->removeChild(n);
                faceGroup->addChild(n);
            }
            _rootXform->addChild(cadRoot);
        }

        // ------------------------------------------------------------
        // FACE PASS (flat white, depth-correct)
        // ------------------------------------------------------------
        osg::StateSet* fs = faceGroup->getOrCreateStateSet();

        fs->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        fs->setMode(GL_BLEND, osg::StateAttribute::OFF);

        osg::ref_ptr<osg::Material> fmat = new osg::Material;
        fmat->setDiffuse(osg::Material::FRONT_AND_BACK,
                         osg::Vec4(1,1,1,1));
        fs->setAttributeAndModes(fmat,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::PolygonOffset> fpo = new osg::PolygonOffset;
        fpo->setFactor(1.0f);
        fpo->setUnits(1.0f);
        fs->setAttributeAndModes(fpo,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        fs->setRenderingHint(osg::StateSet::OPAQUE_BIN);

        // ------------------------------------------------------------
        // EDGE PASS (OCCT-style visible edges)
        // ------------------------------------------------------------
        osg::StateSet* es = edgeGroup->getOrCreateStateSet();

        es->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        es->setMode(GL_BLEND, osg::StateAttribute::OFF);

        osg::ref_ptr<osg::Depth> depth = new osg::Depth;
        depth->setFunction(osg::Depth::LEQUAL);
        depth->setWriteMask(false); // critical
        es->setAttributeAndModes(depth,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth;
        lw->setWidth(2.0f);  // visually similar to OCCT
        es->setAttributeAndModes(lw,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::Material> emat = new osg::Material;
        emat->setDiffuse(osg::Material::FRONT_AND_BACK,
                         osg::Vec4(0,0,0,1));
        es->setAttributeAndModes(emat,
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // ------------------------------------------------------------
        // Extract edges from geometry (once)
        // ------------------------------------------------------------
        if (edgeGroup->getNumChildren() == 0)
        {
            struct EdgeExtractor : public osg::NodeVisitor
            {
                osg::Group* target;

                EdgeExtractor(osg::Group* g)
                    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), target(g) {}

                void apply(osg::Geode& geode) override
                {
                    for (unsigned i = 0; i < geode.getNumDrawables(); ++i)
                    {
                        osg::Geometry* src =
                            dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                        if (!src) continue;

                        osg::ref_ptr<osg::Geometry> edges =
                            new osg::Geometry(*src, osg::CopyOp::DEEP_COPY_ALL);

                        edges->getPrimitiveSetList().clear();
                        edges->addPrimitiveSet(
                            new osg::DrawArrays(GL_LINES, 0,
                                edges->getVertexArray()->getNumElements()));

                        osg::ref_ptr<osg::Geode> eg = new osg::Geode;
                        eg->addDrawable(edges);
                        target->addChild(eg);
                    }
                }
            };

            EdgeExtractor ex(edgeGroup);
            faceGroup->accept(ex);
        }
    }
    else
    {
        if (cadRoot)
        {
            while (faceGroup->getNumChildren() > 0)
                _rootXform->addChild(faceGroup->getChild(0));

            faceGroup->removeChildren(0, faceGroup->getNumChildren());
            edgeGroup->removeChildren(0, edgeGroup->getNumChildren());
            _rootXform->removeChild(cadRoot);
        }
    }
}


struct FixNearPlane : public osg::Camera::DrawCallback
{
    virtual void operator()(osg::RenderInfo& ri) const override
    {
        osg::Camera* cam = ri.getCurrentCamera();
        if (!cam) return;

        double fovy, aspect, zNear, zFar;
        if (cam->getProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar))
        {
            // Force a safe near plane every frame
            cam->setProjectionMatrixAsPerspective(
                fovy, aspect,
                0.5,      // <-- THIS fixes the Z=0 diagonals
                zFar
            );
        }
    }
};

    ViewerFLTK(int x, int y, int w, int h, const char* label = 0): Fl_Gl_Window(x, y, w, h, label), osgViewer::Viewer(), rightMousePressed(false) {  // Ensure osgViewer::Viewer constructor is called


        // this->setUpViewerAsEmbeddedInWindow(x, y, w, h);



        //  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

        //  traits->alpha = false;

        //  traits->depth = false;

        //  traits->stencil = false;

        //  traits->doubleBuffer = true;

            

        // mode(FL_OPENGL3 |FL_RGB );
        // mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // mode(FL_OPENGL3 |FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // mode(FL_OPENGL3 |FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

        _gw = new osgViewer::GraphicsWindowEmbedded(x, y, w, h);
		// In your window/camera setup (constructor or init), create the embedded graphics window
// osg::GraphicsContext::Traits* traits = new osg::GraphicsContext::Traits;
// traits->x = 0;
// traits->y = 0;
// traits->width = this->w();
// traits->height = this->h();
// traits->windowDecoration = false;
// traits->doubleBuffer = true;
// traits->sharedContext = 0;

// _gw = new osgViewer::GraphicsWindowEmbedded(traits);

//   setLightingMode(osg::View::HEADLIGHT);      

        getCamera()->setViewport(new osg::Viewport(0, 0, w, h));

        // getCamera()->setProjectionMatrixAsPerspective(

        //     30.0f, static_cast<double>(w) / static_cast<double>(h), 1.0f, 10000.0f);

        getCamera()->setGraphicsContext(_gw);

        getCamera()->setDrawBuffer(GL_BACK);

        getCamera()->setReadBuffer(GL_BACK);

		getCamera()->setProjectionMatrixAsPerspective(
    45.0,
    static_cast<double>(w) / static_cast<double>(h),
    0.5,      // near plane
    50000.0   // far plane
);


getCamera()->setPostDrawCallback(new FixNearPlane);



		// getCamera()->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f)); // RGBA
		// getCamera()->setClearColor(osg::Vec4(1.0f, 0.5f, 0.5f, 0.5f)); // RGBA

		// getCamera()->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

        setRunFrameScheme( osgViewer::ViewerBase::ON_DEMAND );

        setRunMaxFrameRate(20.0);

        setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);



        instance = this;

        tmr=new osgGA::TrackballManipulator;//(   osgGA::StandardManipulator::DEFAULT_SETTINGS |  osgGA::StandardManipulator::SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT | osgGA::StandardManipulator::COMPUTE_HOME_USING_BBOX );

		tmr->setAnimationTime(0.2); // Smooth transitions

// Calculate the scene's bounding sphere
osg::BoundingSphere boundingSphere=computeSceneBoundingSphere();
// (You'll need to compute this from your scene)

// Set the center of rotation to the bounding sphere center
tmr->setCenter(boundingSphere.center()); 
        setCameraManipulator(tmr);
// setCameraManipulator(nullptr);

		

        Fl::add_timeout(1.0 / 24, Timer_CB, nullptr);

    }



    void resize(int x, int y, int w, int h) override {
        if (_gw.valid()) {
            _gw->getEventQueue()->windowResize(x, y, w, h);
            _gw->resized(x, y, w, h);
			setbar5per();
        }
        Fl_Gl_Window::resize(x, y, w, h);
    }
	osg::BoundingSphere computeSceneBoundingSphere() {
    osg::BoundingSphere bs;
    if (getSceneData()) {
        bs = getSceneData()->getBound();
    }
    return bs;
}
void setbar5per() {
    // Static HUD camera, root, and bar geometry
    static osg::ref_ptr<osg::Camera> hudCam;
    static osg::ref_ptr<osg::Group> hudRoot;
    static osg::ref_ptr<osg::Geode> barGeode;

    // Get current window size from FLTK
    int viewportWidth = this->w();
    int viewportHeight = this->h();

    // On first call, create HUD camera and add to scene
    if (!hudCam) {
        hudRoot = new osg::Group;

        hudCam = new osg::Camera;
        hudCam->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        hudCam->setClearMask(GL_DEPTH_BUFFER_BIT);
        hudCam->setRenderOrder(osg::Camera::POST_RENDER);
        hudCam->setAllowEventFocus(false);
        hudCam->setViewMatrix(osg::Matrix::identity());
        hudCam->addChild(hudRoot);

        // Add HUD camera to your main scene graph (rootNode is your scene's root)
        _rootXform->addChild(hudCam);
    }

    // Update the HUD camera projection matrix to match current size
    hudCam->setProjectionMatrix(osg::Matrix::ortho2D(0, viewportWidth, 0, viewportHeight));

    // Remove the previous bar if it exists
    if (barGeode) {
        hudRoot->removeChild(barGeode);
        barGeode = nullptr;
    }

    float barWidth = viewportWidth * 0.05f;

    // Define red bar vertices (in screen coordinates)
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(viewportWidth - barWidth, 0, 0));
    vertices->push_back(osg::Vec3(viewportWidth,           0, 0));
    vertices->push_back(osg::Vec3(viewportWidth,   viewportHeight, 0));
    vertices->push_back(osg::Vec3(viewportWidth - barWidth, viewportHeight, 0));

    // Set color with transparency (alpha < 1.0)
    float alpha = 0.4f; // 40% opaque
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, alpha)); // transparent red

    // Create the geometry
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    // Create a new Geode and attach geometry
    barGeode = new osg::Geode;
    barGeode->addDrawable(geom);

    // Enable transparency and disable lighting
    osg::StateSet* stateSet = barGeode->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF); // Prevent black bar
    osg::ref_ptr<osg::Depth> depth = new osg::Depth;
    depth->setWriteMask(false);
    stateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);

    // Add the bar to the HUD root group
    hudRoot->addChild(barGeode);
}

void updateRotationCenter() {
    osg::BoundingSphere boundingSphere;
    // Compute bounding sphere of your scene here
    
    if (tmr) {
        tmr->setCenter(boundingSphere.center());
        // Optional: recalculate home position
        // tmr->home(0); // 0 for no animation
    }
}
    bool _rotating = false;
    osg::Vec3d _eye0;
    osg::Vec3d _center0;
    osg::Vec3d _up0;
    osg::Vec3d _arcballV0;
    int _vpW = 1;
    int _vpH = 1;


	static osg::Vec3d projectToArcball(int x, int y, int w, int h)
{
    double nx = (2.0 * x - w) / w;
    double ny = (h - 2.0 * y) / h;

    double len2 = nx*nx + ny*ny;
    double nz = (len2 <= 1.0) ? std::sqrt(1.0 - len2) : 0.0;

    osg::Vec3d v(nx, ny, nz);
    v.normalize();
    return v;
}

int handle(int event) override
{
    if (event == FL_SHOW && is_minimized) {
        last_event = std::chrono::steady_clock::now();
        is_minimized = 0;
    }

    if (is_minimized)
        return Fl_Gl_Window::handle(event);

    if (event == FL_KEYUP || event == FL_KEYDOWN) {
        if (Fl::event_key() == FL_Escape)
            return 0;
    }

    if (event == FL_PUSH)
        Fl::focus(this);

    if (event == FL_MOUSEWHEEL) {
        int dy = Fl::event_dy();
        if (dy == 0) return 0;

        int mouseX = Fl::event_x();
        int mouseY = Fl::event_y();
        int windowWidth = w();
        int windowHeight = h();

        osg::Vec3d eye, center, up;
        tmr->getTransformation(eye, center, up);

        osg::Matrixd viewMatrix = getCamera()->getViewMatrix();
        osg::Matrixd projMatrix = getCamera()->getProjectionMatrix();

        osg::Vec3d viewDir = center - eye;
        double distance = viewDir.length();
        viewDir.normalize();

        double zoomFactor = (dy > 0) ? 1.1 : 0.9;
        double scale = (1.0 - zoomFactor);

        osg::Matrixd invView = osg::Matrixd::inverse(viewMatrix);
        osg::Vec3d camRight(invView(0,0), invView(0,1), invView(0,2));
        osg::Vec3d camUp   (invView(1,0), invView(1,1), invView(1,2));

        double px = mouseX - windowWidth * 0.5;
        double py = windowHeight * 0.5 - mouseY;

        double fovy, aspectRatio, zNear, zFar;
        projMatrix.getPerspective(fovy, aspectRatio, zNear, zFar);
        double pixelSize = 2.0 * distance * tan(osg::DegreesToRadians(fovy * 0.5)) / windowHeight;

        osg::Vec3d panOffset =
            camRight * (px * pixelSize * scale) +
            camUp    * (py * pixelSize * scale);

        osg::Vec3d zoomOffset = viewDir * distance * scale;

        tmr->setTransformation(
            eye    + zoomOffset + panOffset,
            center + zoomOffset + panOffset,
            up
        );
        return 1;
    }

    static int _lastMouseX = 0, _lastMouseY = 0;

    if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
        if (_gw.valid())
            _gw->getEventQueue()->mouseButtonPress(Fl::event_x(), Fl::event_y(), Fl::event_button());

        _lastMouseX = Fl::event_x();
        _lastMouseY = Fl::event_y();

        int mx = _lastMouseX;
        int my = _lastMouseY;
        int winH = h();
        int osgMouseY = winH - 1 - my;

        osg::Vec3d pivotWorld = calculateWorldIntersectionPivot(mx, osgMouseY, this);
        osggl->_pivot = pivotWorld;

        return 1;
    }

    if (event == FL_DRAG && Fl::event_button() == FL_LEFT_MOUSE) {
        osg::Vec3d rotationCenter = osggl->_pivot;

        int currentX = Fl::event_x();
        int currentY = Fl::event_y();

        float percentX = float(_lastMouseX) / float(w());
        if (percentX > 0.95f) {
            float dy = currentY - _lastMouseY;
            _lastMouseY = currentY;
            float angle = osg::DegreesToRadians(-dy * 0.1f);

            osg::Vec3d eye, center, up;
            tmr->getTransformation(eye, center, up);
            osg::Vec3d viewDir = center - eye;
            viewDir.normalize();

            osg::Quat roll(angle, viewDir);
            tmr->setTransformation(eye, center, roll * up);
            return 1;
        }

        float dx = currentX - _lastMouseX;
        float dy = currentY - _lastMouseY;
        _lastMouseX = currentX;
        _lastMouseY = currentY;

        osg::Vec3d eye, center, up;
        tmr->getTransformation(eye, center, up);

        osg::Vec3d viewDir = center - eye;
        viewDir.normalize();
        osg::Vec3d right = viewDir ^ up;
        right.normalize();

        osg::Quat rotH(osg::DegreesToRadians(-dx * 0.2f), up);
        osg::Quat rotV(osg::DegreesToRadians(-dy * 0.2f), right);
        osg::Quat totalRot = rotH * rotV;

        osg::Vec3d eyeOffset    = eye    - rotationCenter;
        osg::Vec3d centerOffset = center - rotationCenter;

        tmr->setTransformation(
            rotationCenter + totalRot * eyeOffset,
            rotationCenter + totalRot * centerOffset,
            totalRot * up
        );
        return 1;
    }

    if (event == FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE) {
        if (_gw.valid())
            _gw->getEventQueue()->mouseButtonRelease(Fl::event_x(), Fl::event_y(), Fl::event_button());
        return 1;
    }

    if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE) {
        rightMousePressed = true;
        lastMouseX = Fl::event_x();
        lastMouseY = Fl::event_y();
        return 1;
    }

    if (event == FL_DRAG && rightMousePressed && Fl::event_button() == FL_RIGHT_MOUSE) {
        int dx = Fl::event_x() - lastMouseX;
        int dy = Fl::event_y() - lastMouseY;

        osg::Vec3d eye, center, up;
        tmr->getTransformation(eye, center, up);

        osg::Vec3d viewDir = center - eye;
        viewDir.normalize();
        osg::Vec3d right = viewDir ^ up;
        right.normalize();
        osg::Vec3d trueUp = right ^ viewDir;
        trueUp.normalize();

        double panFactor = (center - eye).length() * 0.001;
        osg::Vec3d pan =
            right  * (-dx * panFactor) +
            trueUp * ( dy * panFactor);

        tmr->setTransformation(eye + pan, center + pan, up);

        lastMouseX = Fl::event_x();
        lastMouseY = Fl::event_y();
        return 1;
    }

    if (event == FL_RELEASE && Fl::event_button() == FL_RIGHT_MOUSE) {
        rightMousePressed = false;
        return 1;
    }

    return Fl_Gl_Window::handle(event);
}


    void draw() override {  

        if(!is_minimized && !done()) {
    		// Fl::wait(0.01);			
            frame(); 
			// damage()
		}

    }

     
// --- Camera animation state ---
bool _animating = false;
double _animDuration = 0.4;

std::chrono::steady_clock::time_point _animStart;

osg::Vec3d _animEye0, _animCenter0, _animUp0;
osg::Vec3d _animEye1, _animCenter1, _animUp1;


static void ViewAnim_CB(void* data) {
    ViewerFLTK* v = (ViewerFLTK*)data;
    if (!v->_animating || !v->tmr) return;

    auto now = std::chrono::steady_clock::now();
    double t = std::chrono::duration<double>(now - v->_animStart).count() / v->_animDuration;
    if (t >= 1.0) t = 1.0;

    osg::Vec3d eye    = v->_animEye0    + (v->_animEye1    - v->_animEye0)    * t;
    osg::Vec3d center = v->_animCenter0 + (v->_animCenter1 - v->_animCenter0) * t;
    osg::Vec3d up     = v->_animUp0     + (v->_animUp1     - v->_animUp0)     * t;
    up.normalize();

    v->tmr->setTransformation(eye, center, up);
    v->redraw();

    if (t < 1.0) {
        Fl::repeat_timeout(1.0/60.0, ViewAnim_CB, v);
    } else {
        v->_animating = false;
    }
}



void startViewAnimation(const osg::Vec3d& eye1,
                        const osg::Vec3d& center1,
                        const osg::Vec3d& up1)
{
    if (!tmr) return;

    tmr->getTransformation(_animEye0, _animCenter0, _animUp0);
    _animEye1    = eye1;
    _animCenter1 = center1;
    _animUp1     = up1;

    _animStart = std::chrono::steady_clock::now();
    _animating = true;

    Fl::remove_timeout(ViewAnim_CB, this);
    Fl::add_timeout(1.0/60.0, ViewAnim_CB, this);
}



};
ViewerFLTK* ViewerFLTK::instance = nullptr;

struct sbts {
    std::string label;
    std::function<void()> func;
    int id;
    double v[6];
    void* occv = nullptr;
    int idx = 0;
    Fl_Button* occbtn = nullptr;

static void call(Fl_Widget*, void* data) {
    sbts* s = (sbts*)data;
    auto* o = (ViewerFLTK*)s->occv;

    if (s->func) { s->func(); return; }

    osg::Vec3d eye, center, up;
    o->tmr->getTransformation(eye, center, up);

    osg::Vec3d dir(s->v[0], s->v[1], s->v[2]);
    osg::Vec3d upv(s->v[3], s->v[4], s->v[5]);
    dir.normalize();
    upv.normalize();

    double dist = (eye - center).length();
    osg::Vec3d newEye = center + dir * dist;

    o->startViewAnimation(newEye, center, upv);
}



};
vector<sbts> sbt;
static const struct {
    const char* name;
    double y[6];
    double z[6];
} sbt_all[] = {
    {"Front",  { 0,0,1, 0,1,0 }, { 0,1,0, 0,0,1 }},
    {"Back",   { 0,0,-1,0,1,0 }, { 0,-1,0,0,0,1 }},
    {"Top",    { 0,1,0, 1,0,0 }, { 0,0,1, 0,1,0 }},
    {"Bottom", { 0,-1,0,-1,0,0 },{ 0,0,-1,0,1,0 }},
    {"Left",   { -1,0,0,0,1,0 }, { -1,0,0,0,0,1 }},
    {"Right",  { 1,0,0,0,1,0 },  { 1,0,0,0,0,1 }},
    {"Iso",    { -1,1,1,0,1,0 }, { -1,1,1,0,0,1 }},
    {"Isor",   { 1,1,-1,0,1,0 }, { 1,-1,1,0,0,1 }},
};
void sbtset(bool zdirup=0)
{
    sbt.clear();

    for(const auto& e : sbt_all) {
        sbts s;
        s.label = e.name;
        s.id = 1;
        const double* src = zdirup ? e.z : e.y;
        for(int i=0;i<6;i++) s.v[i] = src[i];
        sbt.push_back(s);
    }

    sbt.push_back(sbts{
        "Invert d",
        [osggl]{
            osg::Vec3d eye,center,up;
            osggl->tmr->getHomePosition(eye,center,up);
            osg::Vec3d dir = center - eye;
            osg::Vec3d newEye = center + dir;
            osggl->tmr->setHomePosition(newEye,center,up);
            osggl->tmr->home(1.0);
        }
    });

    sbt.push_back(sbts{
        "Align",
        [osggl]{ osggl->tmr->home(1.0); }
    });
}
void drawbuttons(float x,float y,float w,int h)
{
    sbtset(0);
    float w1 = ceil(w / sbt.size());

    for(size_t i=0;i<sbt.size();i++) {
        sbt[i].occv = osggl;
        sbt[i].idx  = i;
        sbt[i].occbtn = new Fl_Button(x+w1*i,y,w1,h,sbt[i].label.c_str());
        sbt[i].occbtn->callback(sbts::call,&sbt[i]);
    }
}

#if 1 //WMOVING
vbool wmoving(4);
osg::Vec3 fix_center_bug;
void wmove(){	
	// return;
   // cot(wmoving);
	osg::Vec3 eye, center, up;
	osggl->getCamera()->getViewMatrixAsLookAt( eye, center, up,50 ); 
	center=fix_center_bug;
	// tmr->setAutoComputeHomePosition(0);
	if(wmoving[0]){
		center.z()+=40; 
		eye.z()+=40; 
	}
	if(wmoving[1]){
		center.z()-=40; 
		eye.z()-=40; 
	}
	if(wmoving[2]){
		center.x()-=40; 
		eye.x()-=40; 
	}
	if(wmoving[3]){
		center.x()+=40; 
		eye.x()+=40; 
	}
	osggl->tmr->setHomePosition( eye, center, up );
	// tm->home(0.0);
	// tm->setPivot(Vec3f(0,0,0));
	// osggl->setCameraManipulator(osggl->tmr);
	fix_center_bug=center;
}
int OnKeyPress(int Key, Fl_Window *MyWindow) { 
	if(Fl::focus()!=osggl)return Fl::handle_(Key, MyWindow);
	if(Key == FL_KEYDOWN) {
		// cot( Fl::event_key());
		int dkey=Fl::event_key();
		if(dkey==FL_Escape)return 1;
		if(dkey==119)wmoving[0]=1;//w
		if(dkey==115)wmoving[1]=1;//s
		if(dkey==97)wmoving[2]=1;//a
		if(dkey==100)wmoving[3]=1;//d
		wmove();		 
      // cot( Fl::event_text());
	//   osggl->handle(Key);
	//   osggl->getEventQueue()->keyPress(Fl::event_key());
	// 	osggl->_gw->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol)Fl::event_key());
 
   }
   if(Key == FL_KEYUP) {
		int dkey=Fl::event_key();
		if(dkey==FL_Escape)return 1;
		if(dkey==119)wmoving[0]=0;//w
		if(dkey==115)wmoving[1]=0;//s
		if(dkey==97)wmoving[2]=0;//a
		if(dkey==100)wmoving[3]=0;//d
		wmove();
		osggl->_gw->getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol)Fl::event_key());
      // cot( Fl::event_text());
 
   }
   return Fl::handle_(Key, MyWindow);
}
#endif
void getview(){
	#define mathRound(n,d) ({float _pow10=pow(10,d); floorf((n) * _pow10 + 0.5) / _pow10;})
	osg::Vec3 eye, center, up;
	osggl->getCamera()->getViewMatrixAsLookAt( eye, center, up );
	cotm(eye);
	cotm(center);
	cotm(up); 
	cout<<"view1: "<<mathRound(eye.x(),1)<<" "<<mathRound(eye.y(),1)<<" "<<mathRound(eye.z(),1)<<" "<<mathRound(center.x(),1)<<" "<<mathRound(center.y(),1)<<" "<<mathRound(center.z(),1)<<" "<<mathRound(up.x(),1)<<" "<<mathRound(up.y(),1)<<" "<<mathRound(up.z(),1)<<" "<<endl;
	cout<<"view( "<<mathRound(eye.x(),1)<<" , "<<mathRound(eye.y(),1)<<" , "<<mathRound(eye.z(),1)<<" , "<<mathRound(center.x(),1)<<" , "<<mathRound(center.y(),1)<<" , "<<mathRound(center.z(),1)<<" , "<<mathRound(up.x(),1)<<" , "<<mathRound(up.y(),1)<<" , "<<mathRound(up.z(),1)<<" )"<<endl;
}

static Fl_Menu_Item items[] = {
	// Main menu items
	{ "&File", 0, 0, 0, FL_SUBMENU },
		{ "&Alpha", FL_COMMAND + 'a', ([](Fl_Widget *, void* v){ 	  
			toggletransp();	  
		}), (void*)menu },
		
		{ "Cancel all", 0, ([](Fl_Widget *, void* v){  
				all_cancel=1;  
		}), (void*)menu },
		
		{ "Pause", 0, ([](Fl_Widget *fw, void* v){ 	  
			Fl_Menu_Item* btpause= const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)fw)->find_item("&File/Pause"));
			if(btpause->value()>0)
			// 	lop(i,0,pool.size())
			// 		pool[i]->pause=1;
			// else
			// 	lop(i,0,pool.size())
			// 		pool[i]->pause=0;	
			
			cot1(btpause->value());  
		}), (void*)menu, FL_MENU_TOGGLE },

		{ "&Rota", 0, ([](Fl_Widget *, void* v){ 
			// flgl->resize(flgl->x(),flgl->y(),200,200); 
				// ve[1]->rotate( 10);
				// cot(*ve[1]->axisbegin ); 
				// cot(*ve[1]->axisend );  		
			thread([]{
				lop(i,0,1900000000){
					sleepms(10); 
					// if(Break)return;
					// ve[0]->rotate( 1);
					if(ve[0]->angle>180)ve[0]->dir=-1;
					if(ve[0]->angle<-140)ve[0]->dir=1;
					ve[0]->rotate( 1*ve[0]->dir);
					// cot(ve[0]->angle);
					
					
					if(ve[1]->angle>90)ve[1]->dir=-1;
					if(ve[1]->angle<-90)ve[1]->dir=1;
					ve[1]->rotate( 2*ve[1]->dir); 
					// cot(*ve[0]->axisend );
				}
			}).detach();
		}), (void*)menu },
		{ "&Get view", FL_COMMAND  + 'g', ([](Fl_Widget *, void* v){  
			// ve[0]->rotate(Vec3f(0,0,1),45); 
			getview();
			// Break=1;
			
				// ve[0]->rotate( 10); 
				// cot(*ve[1]->axisbegin ); 
				// cot(*ve[1]->axisend );  
		}), (void*)menu },
		{ "Inverse kinematics", 0, ([](Fl_Widget *, void* v){ 	
			// ve[ve.size()-1]->rotate_posk(10);	
			// dbg_pos(); /////
			ve[3]->posik(vec3(150,150,-150));
		}), (void*)menu },
		{ "Quit", 0, ([](Fl_Widget *, void* v){cotm("exit"); exit(0);}) },
		{ 0 },
	
	{ "&View", 0, 0, 0, FL_SUBMENU },
		{ "&osg window visible", 0, ([](Fl_Widget *, void* v){ 	
			// ViewerFLTK::instance = nullptr;
			sleepms(500);
			osggl->hide();
			// osggl->destroyOpenGLContext();
			// malloc_trim(0); 
			// delete osggl;	
		}), (void*)menu },

		
		{ "&Maquete visible", 0, ([](Fl_Widget *, void* v){ 	
			makvisible = !makvisible;
			maquete->setNodeMask(makvisible ? 0xffffffff : 0x0);	
		}), (void*)menu },
		
		#if defined(__linux__)
		{ "&startstream", 0, ([](Fl_Widget *, void* v){ 	
			thread([win]{	
				Fl_Window* w=(Fl_Window*)win;		
			startstream(fl_xid(w));	
			}).detach();
		}), (void*)menu },
		#endif

		
		{ "Robot visible", 0, ([](Fl_Widget *, void* v){ 	
			robotvisible = !robotvisible;
			lop(i,0,ve.size()) lop(j,0,ve[i]->nodes.size())
				ve[i]->nodes[j]->setNodeMask(robotvisible ? 0xffffffff : 0x0);	
		}), (void*)menu ,FL_MENU_DIVIDER},
		
		{ "Time debug", 0, ([](Fl_Widget *, void* v){ 	
			// // Config cfg =  serialize(pool);
			// // cot(cfg);
			// time_f();	
			// pressuref=4;
			// try{
			// 	elevator_go(3);
			// }catch(...){}
		}), (void*)menu },

		{ "Fit", 0, ([](Fl_Widget *, void* v){ 
			ViewerFLTK* view=osggl;
			if ( view->getCamera() ){
				// http://ahux.narod.ru/olderfiles/1/OSG3_Cookbook.pdf
				double _distance=-1; float _offsetX=0, _offsetY=0;
				osg::Vec3d eye, center, up;
				view->getCamera()->getViewMatrixAsLookAt( eye, center,
				up );
	
				osg::Vec3d lookDir = center - eye; lookDir.normalize();
				osg::Vec3d side = lookDir ^ up; side.normalize();
	
				const osg::BoundingSphere& bs = view->getSceneData()->getBound();
				if ( _distance<0.0 ) _distance = bs.radius() * 3.0;
				center = bs.center();
				center -= (side * _offsetX + up * _offsetY) * 0.1;
				fix_center_bug=center;
				// up={0,1,0};
				osggl->tmr->setHomePosition( center-lookDir*_distance, center, up );
				osggl->setCameraManipulator(osggl->tmr);
				getview();
			}	
		}), (void*)menu },
		{ 0 },
	
	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "&About v2", 0, ([](Fl_Widget *, void* v){ 
			// lua_init();
			// getallsqlitefuncs();
			string val=getfunctionhelp();
			cot1(val);
			fl_message(val.c_str());
		}), (void*)menu },
		{ 0 },
	
	{ 0 } // End marker
};

struct vix{int index;float angle; };
vector<vector<vix*>> vixs;	
mutex mtxlight;
struct LightChange {
	int index, highlight;
};
void light_ui_cb(void* data) {
	auto info = static_cast<LightChange*>(data);
	int index = info->index;
	int highlight = info->highlight;
	delete info; 
	if (btp.size() <= (size_t)index) return;
	if (btp[index].size() == 0) return;

	for (size_t j = 0; j < btp[index].size(); ++j) {
		if (btp[index][j]) {
			btp[index][j]->color(j == (size_t)highlight ? FL_GREEN : FL_GRAY);
			btp[index][j]->redraw();
		}
	}
}
void turn_light_on(int index, int angle) {
	if (is_minimized) return;
	if (vixs.empty() || vixs[index].empty()) return;

	mtxlight.lock();
	int ang10 = angle / 10 * 10;

	for (size_t i = 0; i < vixs[index].size(); ++i) {
		if ((int)vixs[index][i]->angle == ang10) {
			Fl::awake(light_ui_cb, new LightChange{index, static_cast<int>(i)});
			break;
		}
	}

	mtxlight.unlock();
}

void lightview(int w,int h){
	//botoes dos angulos
	// fldbg=new Fl_Help_View(w*(0.3+0.45),h-(h*0.15),w-w*(0.3+0.45),h*0.15);
	win->begin();
	Fl_Double_Window* flpos=new Fl_Double_Window(w*(0.3+0.45), h*.5, w-w*(0.3+0.45), h*0.2);   
	// Fl_Double_Windowc* flpos=new Fl_Double_Windowc(800, 310, 300, 480-300);   
	Fl_Box* flposbox=new Fl_Box(0, 0, 300, 120);   
	// cot(flpos->w());
	// vector<vector<Fl_Button*>> btp(ve.size());
	// struct vix{int index;float angle; };	
	// vector<vector<vix*>> vixs(> btp(ve.size())
	btp=vector<vector<Fl_Button*>> (ve.size()); 
	vixs=vector<vector<vix*>> (ve.size());
		// cot(ve.size());
	lop(vei,0,ve.size()){
		// cot(button_width);
		// string bs=
		float btn=ve[vei]->anglemin/10.0*10;
		int btnsz=( ve[vei]->anglemax/10.0 ) - btn/10+1;
		// cot(btnsz);
		// cot(btn);
		// cot(ve[vei]->anglemax/10.0*10);
		btp[vei]=vector<Fl_Button*>(btnsz);
		vixs[vei]=vector<vix*>(btnsz);
		float button_width=flpos->w()/(float)btp[vei].size();
		lop(i,0,btp[vei].size()){ 
			const char* button_val=to_string(i).c_str();
			btp[vei][i]=new Fl_Button(i*button_width,20*vei,button_width,20); 
				// btp[vei][i]->color(FL_GREEN);
			// btp[i]->copy_label(button_val);
			btp[vei][i]->copy_label(to_string((int)abs(btn/10)).c_str() );
		// cot(to_string((int)abs(btn/10)));
			vix* vsend=new vix({vei,btn});
			vixs[vei][i]=vsend;
			btp[vei][i]->callback([](Fl_Widget *, void* v){ 
				vix* vv=(vix*)v; 
				// cot(vv->index);
				// cot(vv->angle);
				posv* p=new posv;
				// first_mtx.unlock();
				// mtxunlock(vv->index+1);
				// mut[vv->index+1]->unlock();
				ve[vv->index]->rotatetoposition(vv->angle,p,1);
				// ve[vv->index]->rotatetoposition(vv->angle,p); //solved dont kwnow why could lead to future problems see mutexs
				// threadDetach([&]{
					// sleepms(10000);
					// delete p;
				// });
				// ve[vv->index]->rotate_pos(vv->angle);
			},(void*)vsend);
			btn+=10;
		}
	}
	flpos->resizable(flposbox);
	flpos->end();
	flpos->show();
	win->end();
   
}

Fl_Help_View* fldbg; 
extern mutex mtxaxis;
void fldbg_pos_cb(void* data) {
    auto html = static_cast<std::string*>(data);
    if (fldbg) fldbg->value(html->c_str());
    delete html;
}

void fldbg_pos() {
    if (is_minimized) return; 
    std::stringstream strm;
    {
        std::lock_guard<std::mutex> lock(mtxaxis);
        strm << "<html>";

        lop(i, 0, ve.size()) {
            if (i >= 0 && i <= 3)
                strm << "idx" << i << "\t" << (int)(ve[i]->axisbegin->x()) << ",\t"
                     << (int)(ve[i]->axisbegin->y()) << ",\t"
                     << (int)(ve[i]->axisbegin->z());
            strm << "<br>";
            if (i == 4)
                strm << "idxend" << i << "\t" << (int)(ve[i]->axisend->x()) << "\t"
                     << (int)(ve[i]->axisend->y()) << "\t"
                     << (int)(ve[i]->axisend->z()) << std::endl;
        }
        strm << "<br>posa ";
		lop(i, 0, ve.size())
			strm << i << ":" << (int)(ve[i]->angle) << "\t";

        strm << "</html>";
    }
	// cotm(strm.str())

    Fl::awake(fldbg_pos_cb, new std::string(std::move(strm.str())));
}


int flx=0;
int flk=0;
int event_handler_idle(int e) {
	// cotm(Fl::event_button());
	if(Fl::event_x()!=flx || Fl::event_key()!=flk){
		flx=Fl::event_x();
		flk=Fl::event_key();
    	last_event = std::chrono::steady_clock::now();
	}
	if (e == FL_KEYDOWN && Fl::event_key() == FL_Escape) {
        printf("ESCAPE global capturado\n");
        return 1; // Evento tratado, não propaga
    }

    return 0;
}
void check_idle(void*) { 
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_event).count();
    
    if (elapsed >= 20 && !is_minimized) { // secs
		win->iconize(); 
        is_minimized = 1;   
    }  
    Fl::repeat_timeout(10.0, check_idle);
}

#if defined(__linux__)
#include <sys/inotify.h>
void realtimevars(string path){
	path="lua/cfg.lua";
	int fd = inotify_init();  // no IN_NONBLOCK
	int wd = inotify_add_watch(fd, path.c_str(), IN_MODIFY);

	char buf[1024];

	while (true) {
		int len = read(fd, buf, sizeof(buf));  // blocks here
		if (len > 0) {
			// process event
		}
	}
}
#endif


void fill_menu(){
	
menu->add("View/Toggle Projection", FL_CTRL + 'p', [](Fl_Widget*, void* ud) {
    ViewerFLTK* v = static_cast<ViewerFLTK*>(ud);
    osg::ref_ptr<osg::Camera> camera = v->getCamera();
    if (!camera || !camera->getViewport()) return;

    static bool isPerspective = true;

    // Perspective params
    static double fovy = 45.0;
    static double zNearPersp = 0.1;
    static double zFarPersp = 10000.0;

    // Ortho fallback size (used if bounds fail)
    static double fallbackOrthoSize = 100.0;

    double w = camera->getViewport()->width();
    double h = camera->getViewport()->height();
    double aspect = w / h;

    if (isPerspective) {
        // ----- Switch to Orthographic -----
        // Compute scene bounds for a fitting ortho frustum
        osg::ComputeBoundsVisitor cbv;
        v->getSceneData()->accept(cbv);  // assuming getSceneData() gives root node
        osg::BoundingBox bb = cbv.getBoundingBox();

        double orthoSize = 100.0;  // default fallback
        if (bb.valid()) {
            double extentX = bb.xMax() - bb.xMin();
            double extentY = bb.yMax() - bb.yMin();
            double extentZ = bb.zMax() - bb.zMin();
            // Use the larger horizontal/vertical extent, add padding
            double maxXY = std::max(extentX, extentY);
            orthoSize = maxXY * 1.2;  // 20% padding
            if (orthoSize < 1.0) orthoSize = fallbackOrthoSize;
        } else {
            orthoSize = fallbackOrthoSize;
        }

        double left   = -orthoSize / 2.0;
        double right  =  orthoSize / 2.0;
        double bottom = -orthoSize / (2.0 * aspect);
        double top    =  orthoSize / (2.0 * aspect);

        camera->setProjectionMatrixAsOrtho(left, right, bottom, top, -1000.0, 1000.0);
    } else {
        // ----- Switch to Perspective -----
        camera->setProjectionMatrixAsPerspective(fovy, aspect, zNearPersp, zFarPersp);
    }

    // Toggle mode
    isPerspective = !isPerspective;

    // Crucial: recenter the view on the scene
    if (v->getCameraManipulator()) {
        v->getCameraManipulator()->home(0.0);
    }

    v->redraw();

}, osggl, FL_MENU_TOGGLE | FL_MENU_VALUE);
}

struct HeadlightCallback : public osg::NodeCallback {
    osg::Camera* cam;
    osg::Light* light;

    HeadlightCallback(osg::Camera* c, osg::Light* l)
        : cam(c), light(l) {}

    void operator()(osg::Node* node, osg::NodeVisitor* nv) override {
        osg::Matrixd inv = osg::Matrixd::inverse(cam->getViewMatrix());

        osg::Vec3 pos = inv.getTrans();
        osg::Vec3 dir = -inv.getRotate() * osg::Vec3(0,0,1);

        light->setPosition(osg::Vec4(pos, 1.0f));
        light->setDirection(dir);

        traverse(node, nv);
    }
};

int main() { 
    // Fl::add_handler(event_handler_idle); 
    // Fl::add_timeout(10.0, check_idle); 
	std::cout << "FLTK version: "
              << FL_MAJOR_VERSION << "."
              << FL_MINOR_VERSION << "."
              << FL_PATCH_VERSION << std::endl;
	std::cout << "OpenSceneGraph version: " << osgGetVersion() << std::endl;
	Fl::lock();
	appwdir();
	osg::setNotifyLevel(osg::FATAL);
	// osg::DisplaySettings::instance()->setShaderHint(osg::DisplaySettings::NO_SHADER);
	osg::DisplaySettings::instance()->setNumMultiSamples(0);
	
// Enable Vertex Buffer Objects (preferred modern approach)
// osg::DisplaySettings::instance()->setUseVertexBufferObjects(true);

// For backward compatibility, you might also need:
// osg::DisplaySettings::instance()->setUseDisplayList(false);
 
    int w=1024;
	int h=576;
	// cotm(w,h);
    Fl::scheme("oxy");	
	// Fl::visual(FL_RGB | FL_DOUBLE | FL_DEPTH); 
	Fl::use_high_res_GL(0);  
	win=new Fl_Double_Window(0,0,w,h,"frobot");     
    win->callback([](Fl_Widget *widget, void* ){		
		if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
			return;
		menu->find_item("&File/Quit")->do_callback(menu);
	});
    menu = new Fl_Menu_Bar(0, 0,w,22);  
	menu->menu(items); 
  
    Fl_Group* content = new Fl_Group(0, 22, w, h-22); 

    osggl=new ViewerFLTK(w*0.3,  22, w*0.45, h*1-22*2); 
	
	// editor = new fl_scintilla(0, 22, w*0.3, h-22); 
	// win->resizable(win); win->show();  Fl::run(); return 0;
 
	Fl::event_dispatch(OnKeyPress);
	osgViewer::StatsHandler *sh = new osgViewer::StatsHandler;
	sh->setKeyEventTogglesOnScreenStats('p');
    osggl->addEventHandler(sh);
    // osggl->addEventHandler(new osgViewer::StatsHandler); 
	// tmr=new osgGA::TrackballManipulator;

	// group = new osg::Group();
	// // osg::Group* group = new osg::Group();
	// loadstl(group);
	// geraeixos(group);
	// osggl->setSceneData(group);


	fill_menu();

osggl->_sceneRoot = new osg::Group();
osggl->_rootXform = new osg::MatrixTransform;
osggl->_rootXform->setMatrix(osg::Matrix::identity());

// Aqui criamos o grupo de objetos visíveis
osg::ref_ptr<osg::Group> group = new osg::Group();
loadstl(group.get()); // continua usando a função que já tens
geraeixos(group.get());
// osg::Vec3d worldCenter = group->getBound().center(); // está em world coords


// Adiciona ao transform e define no viewer
osggl->_rootXform->addChild(group.get());
osggl->_sceneRoot->addChild(osggl->_rootXform.get());

osg::Vec3d worldCenter = group->getBound().center();

// converte para espaço local do _rootXform (como está identity, são iguais)
osggl->_pivot_local = worldCenter;


osggl->setSceneData(osggl->_rootXform);
// osggl->setSceneData(osggl->_sceneRoot.get());



osg::Matrixd M = osggl->_rootXform->getMatrix(); // matriz atual do transform
osg::Matrixd invM = osg::Matrixd::inverse(M);
osggl->_pivot = worldCenter * invM;  // agora está no mesmo espaço do transform
// opcional: definir o pivot inicial como o centro da cena
// osggl->_pivot = group->getBound().center();

// osg::ref_ptr<osg::ShapeDrawable> marker = new osg::ShapeDrawable(new osg::Sphere(osggl->_pivot, 50.0));
// marker->setColor(osg::Vec4(1, 0, 0, 1));
// osg::ref_ptr<osg::Geode> g = new osg::Geode;
// g->addDrawable(marker.get());
// osggl->_rootXform->addChild(g); // adiciona ao mesmo espaço







	fldbg=new Fl_Help_View(w*(0.3+0.45),h*.7,w-w*(0.3+0.45),h*0.30);
	// fldbg->textcolor(FL_RED);
	fldbg->textfont(5);
	fldbg->textsize(14);
	fldbg->value("<font face=Arial > <br></font>");
	string fldbgstr="<b>Alt+z run</b><br><b>Alt+x run line</b>";
	fldbg->value(fldbgstr.c_str());
	fldbg->show();

	scint_init(0, 22, w*0.3, h-22);
	// fl_scintilla* editor = new fl_scintilla(0, 22, w*0.3, h-22); 
	// sql3_init();
	lightview(w,h);

	// init rotations
if(0){
	cotm(ve.size());
	if(!fs::exists("log"))
		fs::create_directory("log");
	lop(i,0,ve.size()){ 
		string fname="log/ve"+to_string(i);
		if(fs::exists(fname)){
			std::uintmax_t file_size = fs::file_size(fname);
			if(file_size>0){
				FILE *file = fopen(fname.c_str(), "rb");
				fseek(file, -512, SEEK_END);
				int16_t value;
				fread(&value, sizeof(value), 1, file);
				cotm(i,value);
				fclose(file);
				ve[i]->rotate((float)value);
				if(file_size>1024*1024*128){
					fs::remove(fname);
					FILE *file = fopen(fname.c_str(), "ab");
					fwrite(&value, sizeof(value), 1, file);
					fflush(file);
					fclose(file);
				}
			}
		}
		ve[i]->flog=fopen(fname.c_str(), "ab");
	}
}		

	menu->find_item("&View/Fit")->do_callback(menu);

  
	// win->clear_visible_focus(); 	 
	// win->color(0x7AB0CfFF);
	// win->color(FL_RED);
	win->resizable(content);
	 
	
	win->position(0,0);
	// win->position(Fl::w()/2-win->w()/2,10);
	// osggl->applyOCCStyleEdges();
	// osggl->applyGlobalGhostedEdges_v1();



// 	osg::StateSet* ss = osggl->getCamera()->getOrCreateStateSet();
// ss->setMode(GL_LIGHT0, osg::StateAttribute::ON);

// osg::ref_ptr<osg::Light> light = new osg::Light;
// light->setLightNum(0);
// light->setPosition(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));  // at camera eye
// light->setDirection(osg::Vec3(0.0f, 0.0f, -1.0f));      // forward

// osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
// ls->setLight(light);

// osggl->getCamera()->addChild(ls);

// osg::StateSet* ss = group->getOrCreateStateSet();

// ss->setMode(GL_LIGHT1, osg::StateAttribute::OFF);
// ss->setMode(GL_LIGHT2, osg::StateAttribute::OFF);
// ss->setMode(GL_LIGHT3, osg::StateAttribute::OFF);
// ss->setMode(GL_LIGHT4, osg::StateAttribute::OFF);
// ss->setMode(GL_LIGHT5, osg::StateAttribute::OFF);
// ss->setMode(GL_LIGHT6, osg::StateAttribute::OFF);
// ss->setMode(GL_LIGHT7, osg::StateAttribute::OFF);

// 	// Create the light
// osg::ref_ptr<osg::Light> light = new osg::Light;
// light->setLightNum(0);
// light->setPosition(osg::Vec4(0,0,0,1));
// light->setDirection(osg::Vec3(0,0,-1));

// // Create the LightSource
// osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
// ls->setLight(light);

// // Add to scene graph
// group->addChild(ls);

// // Enable GL_LIGHT0
// group->getOrCreateStateSet()->setMode(GL_LIGHT0, osg::StateAttribute::ON);

// // Add callback so the light follows the camera
// ls->addUpdateCallback(new HeadlightCallback(osggl->getCamera(), light));





	osggl->setbar5per();
	Fl_Group::current(content);
	drawbuttons(osggl->x(),osggl->h()+22*1,osggl->w(),22);
	// cotm(osggl->x(),osggl->h()+22*1,osggl->w(),22);
	win->show(); 

	// int x, y, _w, _h;
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22);
 
	// malloc_trim(0);
    return Fl::run(); 
}