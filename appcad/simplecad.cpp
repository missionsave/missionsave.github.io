
#include "includes.hpp"
#include <Aspect_NeutralWindow.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_Workspace.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <Geom_Circle.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>

#include <FL/Fl.H>

#include <AIS_ColoredDrawer.hxx>
#include <Aspect_DisplayConnectionDefinitionError.hxx>

#include <AIS_AnimationCamera.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_SelectableObject.hxx>
#include <SelectMgr_ViewerSelector.hxx>

#include <V3d_View.hxx>
#include <Aspect_Window.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBndLib.hxx>

#include <Graphic3d_Camera.hxx>

#include <TopoDS_Shape.hxx>
#include <BOPAlgo_BOP.hxx>
#include <BOPAlgo_Options.hxx>

#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <set>
#include <vector>
#include <cmath>
#include <AIS_Shape.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
// ----------------- assumed OCC_Viewer minimal interface -----------------
// Keep these as comments; your real OCC_Viewer already has equivalents.
//
// struct OCC_Viewer {
//   Handle(V3d_View) m_view;
//   Handle(AIS_InteractiveContext) m_context;
//   std::vector<TopoDS_Shape> vshapes; // optional for HLR display etc.
//   gp_Vec end_proj_global, end_up_global;
//   Handle(AIS_AnimationCamera) CurrentAnimation;
//   void colorisebtn();
//   void projectAndDisplayWithHLR(const std::vector<TopoDS_Shape>& v);
//   void redraw();
// };
 
#pragma region globals

#include "fl_browser_msv.hpp"
#define flwindow Fl_Window  
// #define flwindow Fl_Gl_Window  

#define cotm2(...) cotm_function(#__VA_ARGS__, get_args_string(__VA_ARGS__)); 
#define cotm1(...) cotm_function(#__VA_ARGS__, get_args_string(__VA_ARGS__)); 
#define cotm1(...) 
#define cotm(...)
// #ifdef __linux__
// #define flwindow Fl_Double_Window
// #endif
void PrintLocation(const TopLoc_Location& loc)
{
    if (loc.IsIdentity()) {
        std::cout << "Location: Identity" << std::endl;
        return;
    }

    const gp_Trsf& t = loc.Transformation();

    // Translation
    gp_XYZ tr = t.TranslationPart();
    std::cout << "Translation: "
              << tr.X() << ", "
              << tr.Y() << ", "
              << tr.Z() << std::endl;

    // Rotation matrix
    gp_Mat r = t.VectorialPart();
    std::cout << "Rotation: " ;
    for (int i = 1; i <= 3; ++i) {
        std::cout << "  ";
        for (int j = 1; j <= 3; ++j)
            std::cout << r.Value(i, j) << ",";
        std::cout << " ";
    }

    // Scale
    std::cout << "Scale: " << t.ScaleFactor() << std::endl;
}
void rotateShapeByMouse(
    Handle(AIS_Shape) toRotate,
    Handle(AIS_InteractiveContext) context,
    const gp_Pnt& center,
    const gp_Dir& axisDir,
    float dx,
    float dy)
{
    if (toRotate.IsNull() || context.IsNull())
        return;

    double angle = dx * 0.002;

    gp_Ax1 axis(center, axisDir);

    gp_Trsf trsf;
    trsf.SetRotation(axis, angle);

    TopoDS_Shape original = toRotate->Shape();
    BRepBuilderAPI_Transform op(original, trsf, true);
    TopoDS_Shape rotated = op.Shape();

    toRotate->Set(rotated);

    context->Redisplay(toRotate, true);
}

TopoDS_Shape FixShape(const TopoDS_Shape& s) {
    ShapeFix_Shape fixer(s);
    fixer.Perform();
    return fixer.Shape();
}
#include <ShapeUpgrade_UnifySameDomain.hxx>

TopoDS_Shape CleanShape(const TopoDS_Shape& s) {
    ShapeUpgrade_UnifySameDomain unify(s, true, true, true);
    unify.Build();
    return unify.Shape();
}

void gopart(const string& str);

void WriteBinarySTL(const TopoDS_Shape& shape, const std::string& filename);
std::string translate_shorthand(std::string_view src); 
void install_shorthand_searcher(lua_State* L);

std::string FaceToString(const TopoDS_Face& face);
std::string SerializeCompact(const std::string& partName,
                              int index,
                              const gp_Dir& normal,
                              const gp_Pnt& centroid,
                              double area);
void GetAlignedCameraVectors(const Handle(V3d_View)& view,
                             gp_Vec& end_proj_global,
                             gp_Vec& end_up_global);


TopoDS_Shape ApplyFilletToAllEdges(const TopoDS_Shape shape, Standard_Real radius);





void ReplaceShapeInCompound(
    TopoDS_Compound& compound,
    TopoDS_Shape& targetShape,
    const std::function<TopoDS_Shape(const TopoDS_Shape&)>& modifier
);

#include <Graphic3d_Camera.hxx>
#include <Graphic3d_Mat4d.hxx>
#include <V3d_View.hxx>
#include <cmath>
#include <GL/gl.h> // or appropriate GL header for your platform

// Build a Reverse-Z perspective projection matrix (OpenGL convention)
Graphic3d_Mat4d BuildReverseZProjection(const Handle(Graphic3d_Camera)& cam)
{
    const double zNear  = cam->ZNear();
    const double zFar   = cam->ZFar();
    const double fovY   = cam->FOVy();
    const double aspect = cam->Aspect();

    const double f = 1.0 / std::tan(fovY * 0.5);

    Graphic3d_Mat4d proj;

    // Initialize all elements to 0
    for (int r = 0; r < 4; ++r)
      for (int c = 0; c < 4; ++c)
        proj.SetValue(r, c, 0.0);

    // Standard perspective terms (X/Y)
    proj.SetValue(0, 0, f / aspect);
    proj.SetValue(1, 1, f);

    // Reverse-Z terms (Z row)
    // Map far -> depth 0, near -> depth 1
    proj.SetValue(2, 2,  zNear / (zFar - zNear));
    proj.SetValue(2, 3,  (zFar * zNear) / (zFar - zNear));

    // Perspective divide row
    proj.SetValue(3, 2, -1.0);

    return proj;
}

void ApplyReverseZ(const Handle(V3d_View)& view)
{
    Handle(Graphic3d_Camera) cam = view->Camera();

    // Build and assign custom projection matrix (if you intercept upload path)
    Graphic3d_Mat4d proj = BuildReverseZProjection(cam);
    // If you have a hook to set a custom projection, do it here.
    // Otherwise, you'll patch the GL state in the viewer.

    // Flip depth range and comparison for Reverse-Z
    glEnable(GL_DEPTH_TEST);
    glDepthRange(1.0, 0.0);
    glDepthFunc(GL_GREATER);

    // Keep near/far tight for best precision
    // cam->SetZNear(1.0);
    // cam->SetZFar(1000.0);
    view->SetCamera(cam);
    view->Redraw();
}
void FixZPrecisionAndGhostLines_793 (
    const Handle(AIS_InteractiveContext)& ctx,
    const Handle(V3d_View)&                view,
    const Handle(AIS_InteractiveObject)&   shape)
{
    /* ===============================
     * 1. Compute scene bounding box
     * =============================== */

    Bnd_Box box;

    AIS_ListOfInteractive list;
    ctx->DisplayedObjects(list);

    for (AIS_ListOfInteractive::Iterator it(list); it.More(); it.Next())
    {
        Bnd_Box b;
        it.Value()->BoundingBox(b);
        box.Add(b);
    }

    if (box.IsVoid())
        return;

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt pmin(xmin, ymin, zmin);
    gp_Pnt pmax(xmax, ymax, zmax);
    Standard_Real diag = pmin.Distance(pmax);

    if (diag <= Precision::Confusion())
        diag = 1000.0;


    /* ===============================
     * 2. Optimal Z range
     * =============================== */

    Standard_Real zNear = Max(1.0, diag * 0.02);
    Standard_Real zFar  = diag * 1.5;

    Handle(Graphic3d_Camera) cam = view->Camera();

    view->SetAutoZFitMode(Standard_False);
    cam->SetZRange(zNear, zFar);
    cam->InvalidateProjection();


    /* ===============================
     * 3. Polygon offset for edges
     * =============================== */

    Handle(Prs3d_Drawer) dr = new Prs3d_Drawer();

    Handle(Prs3d_LineAspect) la =
        new Prs3d_LineAspect(Quantity_NOC_BLACK,
                             Aspect_TOL_SOLID,
                             1.0);

    Handle(Graphic3d_AspectLine3d) asp = la->Aspect();
    asp->SetPolygonOffsets(Aspect_POM_Line, 1.0f, -1.0f);

    dr->SetLineAspect(la);
    ctx->SetLocalAttributes(shape, dr, Standard_True);


    /* ===============================
     * 4. Redraw
     * =============================== */

    // view->Redraw();
}


std::string SerializeFaceInvariant(const TopoDS_Face& face);
gp_Ax3 get_face_local_cs(const TopoDS_Face& face);

std::string to_string_trim(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    std::string s = oss.str();

    // if there’s a decimal point, strip trailing zeros
    if (auto pos = s.find('.'); pos != std::string::npos) {
        while (!s.empty() && s.back() == '0')
            s.pop_back();
        // if the last character is now the dot, remove it too
        if (!s.empty() && s.back() == '.')
            s.pop_back();
    }
    return s;
}
class FixedHeightWindow : public Fl_Window { 
public:
    int fixed_height;
    bool in_resize; // Prevent infinite recursion
	Fl_Group* parent=0;
    FixedHeightWindow(int x, int y, int w, int h, const char* label = 0) 
        : Fl_Window(x, y, w, h, label), fixed_height(h), in_resize(false) {}
    
	void flush(){
		Fl_Window::flush();
	}
    void resize(int X, int Y, int W, int H) override {
        if (in_resize) return; // Prevent recursion        
        in_resize = true;
        
		// if(parent==0)
		 parent=window();
        int bottom_y = parent->h() - fixed_height;
        
        // Only allow width to change, keep height fixed, and stick to bottom
        if (H != fixed_height || Y != bottom_y) {
            Fl_Window::resize(X, bottom_y, W, fixed_height);
        } else {
            Fl_Window::resize(X, Y, W, H);
        }
        // // Only allow width to change, keep height fixed
        // if (H != fixed_height) {
        //     Fl_Window::resize(X, Y, W, fixed_height);
        // } else {
        //     Fl_Window::resize(X, Y, W, H);
        // }
		
		// damage(FL_DAMAGE_ALL); 
		// redraw();
        // flush();
        in_resize = false;
    }
};
Fl_Double_Window* win;
Fl_Menu_Bar* menu;
Fl_Group* content; 
fl_browser_msv* fbm;
Fl_Help_View* helpv; 
FixedHeightWindow* woccbtn;

lua_State* L;
static std::unique_ptr<sol::state> G;

extern string currfilename;
 
bool studyRotation=0;
gp_Pnt pickcenterforstudyrotation;
gp_Dir pickcenteraxisDir ; // circle normal
gp_Trsf initLoc;
Handle(AIS_Shape) study_trg;


 
#pragma endregion globals

#pragma region help 
struct shelpv{
	string pname="";
	string point="";
	string error="";
	string edge="";
	string mass="";

	void upd(){
	std::string html = R"(
<html>
<body marginwidth=0 marginheight=0 topmargin=0 leftmargin=0><font face=Arial > 
<b><font color="Red">Part</font> 
$pname<br> $point $edge <br>$mass
</font>
</body>
</html>
)";
	replace_All(html,"$point",point);
	replace_All(html,"$pname",pname);
	replace_All(html,"$edge",edge);
	replace_All(html,"$mass",mass);

	helpv->value(html.c_str());
}

}help;

#pragma endregion help

// auto fmt = [](double v) {
//     if (std::fabs(v) < 1e-9) v = 0; // treat near-zero as zero
//     std::ostringstream oss;
//     oss << std::defaultfloat << v;
//     return oss.str();
// };
    auto fmt = [](double v,int precision=2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << v;
        std::string s = oss.str();
        // strip trailing zeros and optional decimal point
        if (s.find('.') != std::string::npos) {
            while (!s.empty() && s.back() == '0') s.pop_back();
            if (!s.empty() && s.back() == '.') s.pop_back();
        }
        return s;
    };

std::string lua_error_with_line(lua_State* L, const std::string& msg) {
	lua_Debug ar;
	if (lua_getstack(L, 1, &ar)) {	// level 1 = caller of this function
		lua_getinfo(L, "Sl", &ar);	// S = source, l = current line
		std::ostringstream oss;
		oss << msg << " (Lua: " << ar.short_src << ":" << ar.currentline << ")";
		return oss.str();
	}
	return msg;
}
class AIS_NonSelectableShape : public AIS_Shape {

   public:
	AIS_NonSelectableShape(const TopoDS_Shape& s) : AIS_Shape(s) {
		//  SetSelectionPriority(0);
        // SetAutoHilight(false); //terminate called after throwing an instance of 'Standard_NotImplemented'

	}

	// void ComputeSelection(const Handle(SelectMgr_Selection)& sel,
    //                       const Standard_Integer mode) override {
	// 	// Do nothing -> no selectable entities created

	// }
};

void open_cb() {
	Fl_File_Chooser chooser(".", "*", Fl_File_Chooser::SINGLE, "Escolha um arquivo");
	chooser.show();
	while (chooser.shown()) Fl::wait();
	if (chooser.value()) {
		printf("Arquivo selecionado:  %s\n", chooser.value());
	}
}

struct OCC_Viewer : public flwindow {
#pragma region initialization
	Handle(Aspect_DisplayConnection) m_display_connection;
	Handle(OpenGl_GraphicDriver) m_graphic_driver;
	Handle(V3d_Viewer) m_viewer;
	Handle(OpenGl_Context) aCtx;
	Handle(AIS_InteractiveContext) m_context;
    Handle(OpenGl_FrameBuffer) m_fbo;
	// Handle(OpenGl_Context) ctx;
	Handle(OpenGl_Context) glCtx;
	Handle(OpenGl_View) glView;
	Handle(OpenGl_Context)      m_glContext;  // cached shared context
	Handle(OpenGl_View)         m_glView;    // cached OpenGl_View
	Handle(V3d_View) m_view;
	Handle(AIS_Trihedron) trihedron0_0_0;
	bool m_initialized = false;
	bool hlr_on = false;
	std::vector<TopoDS_Shape> vshapes;
	std::vector<Handle(AIS_Shape)> vaShape;
	Handle(AIS_NonSelectableShape) visible_;
	Handle(AIS_Shape) hidden_;

	Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 0.2);
	Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.5);
	Handle(Prs3d_LineAspect) highlightaspect = new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 5.0);
	Handle(Prs3d_Drawer) customDrawerp = new Prs3d_Drawer();

	//dummy origin
	TopLoc_Location Origin;
	gp_Trsf Origintrsf;

	OCC_Viewer(int X, int Y, int W, int H, const char* L = 0) : flwindow(X, Y, W, H, L) {
		// Try to request a 32-bit depth buffer when available
		// #if FL_ABI_VERSION < 10500
		// #  define FL_DEPTH32 FL_DEPTH
		// #endif

		// int mode32 =
		// 	FL_RGB8 | FL_ALPHA | FL_DOUBLE |
		// 	(FL_ABI_VERSION >= 10500 ? FL_DEPTH32 : FL_DEPTH) |
		// 	FL_ACCUM | FL_STENCIL | FL_MULTISAMPLE;

		// mode(mode32);

		// if (!can_do(mode32)) {
		// 	fprintf(stderr, "FL_DEPTH32 not supported, falling back to FL_DEPTH\n");

		// 	int mode24 =
		// 		FL_RGB8 | FL_ALPHA | FL_DOUBLE |
		// 		FL_DEPTH |
		// 		FL_ACCUM | FL_STENCIL | FL_MULTISAMPLE;

		// 	mode(mode24);
		// }

		// Request a 32-bit depth buffer if supported
		
        // cotm2(mode( FL_OPENGL3 | FL_DEPTH32 ));
		// cotm2("mode")
        // mode( FL_DEPTH | FL_STENCIL);
        // mode( FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // mode( FL_RGB8|FL_ALPHA | FL_DOUBLE |FL_DEPTH | FL_ACCUM | FL_STENCIL | FL_MULTISAMPLE);
        // mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_ALPHA | FL_STENCIL | FL_MULTISAMPLE);
        // mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // Fl::gl_visual(32);
		Fl::add_timeout(10, idle_refresh_cb, 0);
	}
void initialize_opencascade_() {
    // Get native window handle and create OCCT window
#ifdef _WIN32
    Fl::wait();
    make_current();
    HWND hwnd = (HWND)fl_xid(this);
    Handle(WNT_Window) wind = new WNT_Window(hwnd);
    m_display_connection = new Aspect_DisplayConnection("");
#else
    Window win = (Window)fl_xid(this);
    Display* display = fl_display;
    m_display_connection = new Aspect_DisplayConnection(display);
    Handle(Xw_Window) wind = new Xw_Window(m_display_connection, win);
#endif

    // Create graphic driver and viewer
    m_graphic_driver = new OpenGl_GraphicDriver(m_display_connection);

    m_viewer = new V3d_Viewer(m_graphic_driver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_context = new AIS_InteractiveContext(m_viewer);

    m_view = m_viewer->CreateView();
    m_view->SetWindow(wind);

    // Rendering quality settings
    // m_context->SetDeviationCoefficient(0.01);
    // m_context->SetDeviationAngle(0.5 * M_PI / 180.0);

    // Immediate mode off for better control
    m_view->SetImmediateUpdate(Standard_False);

    // Highlight and selection setup
    SetupHighlightLineType(m_context);

    // Trihedron (axes)
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_BLACK, 0.08);

    gp_Ax2 axes(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
    Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(axes);
    trihedron0_0_0 = new AIS_Trihedron(placement);
    trihedron0_0_0->SetSize(25.0);
    m_context->Display(trihedron0_0_0, Standard_False);

    // Selection tolerance
    m_context->MainSelector()->AllowOverlapDetection(0);
    m_context->SetPixelTolerance(2);

    // Edge aspects (custom line style/thickness)
    m_context->DefaultDrawer()->SetLineAspect(edgeAspect);
    m_context->DefaultDrawer()->SetSeenLineAspect(edgeAspect);
    m_context->DefaultDrawer()->SetFaceBoundaryAspect(edgeAspect);
    m_context->DefaultDrawer()->SetWireAspect(edgeAspect);
    m_context->DefaultDrawer()->SetUnFreeBoundaryAspect(edgeAspect);
    m_context->DefaultDrawer()->SetFreeBoundaryAspect(edgeAspect);

    // Background and rendering params
    m_view->SetBackgroundColor(Quantity_NOC_GRAY90);

    m_view->ChangeRenderingParams().IsTransparentShadowEnabled = Standard_False;
    m_view->ChangeRenderingParams().ToEnableDepthPrepass = Standard_True;

    m_view->SetAutoZFitMode(Standard_True, 0.99);

    // Force initial resize and fit
    m_view->MustBeResized();
    m_view->FitAll();

    // Cache OpenGL context and view
    m_glContext = m_graphic_driver->GetSharedContext();
    m_glView    = Handle(OpenGl_View)::DownCast(m_view->View());

    if (m_glContext.IsNull() || m_glView.IsNull()) {
        std::cerr << "Failed to retrieve OpenGL context or view!" << std::endl;
        return;
    }

    // Create custom FBO with 32-bit float depth
    m_fbo = new OpenGl_FrameBuffer();

    Standard_Integer w = this->w();
    Standard_Integer h = this->h();

    make_current();  // Ensure FLTK context is current before FBO init

    if (!m_fbo->Init(m_glContext,Graphic3d_Vec2i(w, h), GL_SRGB8_ALPHA8, GL_DEPTH32F_STENCIL8, 0))
							  {
        std::cerr << "Failed to initialize 32F depth FBO!" << std::endl;
        m_fbo.Nullify();
        // Continue with on-screen rendering as fallback
    }
	 else 
	{
        // Assign the FBO to the view — now OCCT will render into it
        m_glView->SetFBO(m_fbo);

        // Optional: verify depth precision
        m_fbo->BindBuffer(m_glContext);
		GLenum err = glewInit();
        GLint depthBits = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_DEPTH_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
                                              &depthBits);
        std::cout << "Custom FBO depth precision: " << depthBits << " bits (should be 32)" << std::endl;
        m_fbo->UnbindBuffer(m_glContext);
    }

    setbar5per();
    toggle_shaded_transp(currentMode);
// pausa
    m_initialized = true;

    redraw();  // Trigger first draw
}




	void initialize_opencascade() {
// Fl::wait();
// pausa
		
// GLint depthBits = 0;
// glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//                                       GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
// GLenum err = glGetError();
// if (err == GL_NO_ERROR) {
//     printf("Actual depth bits: %d\n", depthBits);
// } else {
//     printf("Query error: %u\n", err);
// }
// pausa
		// perf();
		// Get native window handle
#ifdef _WIN32
		// Fl::wait();
		// make_current();
		// while(!valid())sleepms(200);
		// sleepms(2000);
		this->show();
		make_current();
		HWND hwnd = (HWND)fl_xid(this);
		Handle(WNT_Window) wind = new WNT_Window(hwnd);
		m_display_connection = new Aspect_DisplayConnection("");
#else
		Window win = (Window)fl_xid((this));
		Display* display = fl_display;


// if (!glXQueryExtension(display, nullptr, nullptr)) {
//     std::cerr << "GLX not available on this X server" << std::endl;
//     // fallback: disable OCCT viewer or use offscreen rendering
// }



		m_display_connection = new Aspect_DisplayConnection(display);
		Handle(Xw_Window) wind = new Xw_Window(m_display_connection, win);
// pausa
// 		// Handle(Xw_Window) wind;
// 		bool success = false;
// for (unsigned mode : {FL_OPENGL3 | FL_DEPTH32,
//                       FL_OPENGL3 | FL_DEPTH,
//                       FL_OPENGL3 | FL_DEPTH}) {
//     this->mode(mode);
//     this->make_current();
//     try {
//         wind = new Xw_Window(m_display_connection, win);
//         success = true;
//         break;
//     } catch (Standard_Failure&) {
//         // try next mode
//     }
// }
// if (!success) {
//     throw std::runtime_error("No suitable GL context found for OCCT");
// }
		
#endif
		m_graphic_driver = new OpenGl_GraphicDriver(m_display_connection);

// m_graphic_driver->ChangeOptions().buffersNoSwap = Standard_True;
	
		m_viewer = new V3d_Viewer(m_graphic_driver);
		m_viewer->SetDefaultLights();
		m_viewer->SetLightOn();
		m_context = new AIS_InteractiveContext(m_viewer);
		m_view = m_viewer->CreateView();
		// zghost();
		// m_view->SetZClippingType(V3d_ZCLIP_NOTHING); // or tighter clipping
		// m_view->SetZSize(0.00000000001); // refine near/far ratio
		m_view->SetWindow(wind);

			// m_context->SetDeviationCoefficient(0.01);
			// m_context->SetDeviationAngle(0.5 * M_PI / 180.0);

if(0){
GLenum err = glewInit();
if (err != GLEW_OK) {
    std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
}
m_view->MustBeResized();
    // Cache OpenGL context and view
    m_glContext = m_graphic_driver->GetSharedContext();
    m_glView    = Handle(OpenGl_View)::DownCast(m_view->View());

    if (m_glContext.IsNull() || m_glView.IsNull()) {
        std::cerr << "Failed to retrieve OpenGL context or view!" << std::endl;
        return;
    }

	m_fbo = new OpenGl_FrameBuffer();

    if (!m_fbo.IsNull()) {
        bool success = createOrResizeFBO(w(), h());

        if (success) {
            m_glView->SetFBO(m_fbo);
        } else {
            m_fbo.Nullify();
        }
    }
	cotm2(w(),h());
	zghost();
}
if(0){	

 m_view->MustBeResized(); // FORÇA criação do contexto GL

    /* ---------- OpenGL context (CORRETO EM 7.9.x) ---------- */
    // Handle(OpenGl_Context) glCtx =
    glCtx= m_graphic_driver->GetSharedContext();

    if (glCtx.IsNull())
    {
        std::cerr << "OpenGL context not available\n";
        return;
    }

make_current(); // FLTK — OBRIGATÓRIO

// Handle(OpenGl_Context) ctx =
//     m_graphic_driver->GetSharedContext();

// if (ctx.IsNull() || !ctx->IsValid())
// {
//     std::cerr << "OpenGL context invalid\n";
//     return;
// }

std::cout << "GL version: "
          << (const char*)glGetString(GL_VERSION) << std::endl;

std::cout << "GL renderer: "
          << (const char*)glGetString(GL_RENDERER) << std::endl;




    /* ---------- FBO com depth 32F ---------- */
    m_fbo = new OpenGl_FrameBuffer();

    Standard_Integer w = this->w();
    Standard_Integer h = this->h();
// GL_DEPTH_COMPONENT24
// GL_DEPTH32F_STENCIL8
// GL_DEPTH24_STENCIL8
    if (!m_fbo->Init(glCtx,
                   w,
                   h,
                   GL_RGBA8,
                   GL_DEPTH32F_STENCIL8))
    {
        std::cerr << "Failed to create 32F depth FBO\n";
        return;
    }

    /* ---------- Associar FBO à view ---------- */
    // Handle(OpenGl_View) glView =
    glView =
        Handle(OpenGl_View)::DownCast(m_view->View());

    if (!glView.IsNull())
    {
        glView->SetFBO(m_fbo);
    }


    if (glCtx.IsNull() || !glCtx->IsValid())
    {
        std::cerr << "Invalid GL context\n";
        return;
    }

    if (m_fbo.IsNull() || !m_fbo->IsValid())
    {
        std::cerr << "FBO not valid\n";
        return;
    }


m_fbo->BindBuffer(glCtx);
GLenum err = glewInit();
if (err != GLEW_OK) {
    std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
}
	GLint depthSize = 0;
glGetFramebufferAttachmentParameteriv(
    GL_FRAMEBUFFER,
    GL_DEPTH_ATTACHMENT,
    GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
    &depthSize);

std::cout << "FBO depth bits = " << depthSize << std::endl;
    m_view->Redraw();


}
		// m_context->SetDeviationCoefficient(0.5);
		// m_context->DefaultDrawer()->SetHiddenLineRemoval(Standard_True);

// m_viewer->SetZClippingType(V3d_ZClippingType_UserDefined);


		m_view->SetImmediateUpdate(Standard_False);


		// m_context->SetAutomaticHilight(true);

		// m_context->Activate(AIS_Shape::SelectionMode(TopAbs_WIRE  )); // 4 =
		// Face selection mode
		// m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE )); // 4 =
		// Face selection mode
		// m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX )); // 4 =
		// vertex selection mode

		// m_context->SetMode(TopAbs_VERTEX, Standard_True); // Enable vertex
		// selection as the active mode Standard_True for the second arg makes
		// it persistent for this mode

		// You can also adjust the sensitivity here if points are hard to pick
		// m_context->SetSelectionSensitivity(0.05);

		SetupHighlightLineType(m_context);

		// SetupDashedHighlight(m_context);

		aCtx = m_graphic_driver->GetSharedContext();

		m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_BLACK, 0.08);

		// Create and display a trihedron 0,0,0
		gp_Ax2 axes(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
		Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(axes);
		trihedron0_0_0 = new AIS_Trihedron(placement);
		trihedron0_0_0->SetSize(25.0);
		m_context->Display(trihedron0_0_0, Standard_False);


		m_context->MainSelector()->AllowOverlapDetection(0);
		m_context->SetPixelTolerance(2);
		// m_context->AddFilter(new StdSelect_FaceFilter(StdSelect_Reject));

		// BRepMesh_IncrementalMesh::SetParallel(Standard_True);

		// float lwidth=5;
		// m_context->DefaultDrawer()->LineAspect()->SetWidth(lwidth);
		// m_context->DefaultDrawer()->SeenLineAspect()->SetWidth(lwidth);
		// m_context->DefaultDrawer()->FaceBoundaryAspect()->SetWidth(lwidth);
		// m_context->DefaultDrawer()->WireAspect()->SetWidth(lwidth);

		m_context->DefaultDrawer()->SetLineAspect(edgeAspect);
		m_context->DefaultDrawer()->SetSeenLineAspect(edgeAspect);
		m_context->DefaultDrawer()->SetFaceBoundaryAspect(edgeAspect);
		m_context->DefaultDrawer()->SetWireAspect(edgeAspect);
		m_context->DefaultDrawer()->SetUnFreeBoundaryAspect(edgeAspect);
		m_context->DefaultDrawer()->SetFreeBoundaryAspect(edgeAspect);
		m_context->DefaultDrawer()->SetFaceBoundaryAspect(edgeAspect);






		m_view->SetBackgroundColor(Quantity_NOC_WHITE);
		// m_view->SetBackgroundColor(Quantity_NOC_GRAY90);
		setbar5per();


		m_view->MustBeResized();
		m_view->FitAll();

		SetupHighlightLineType(m_context);
		// m_initialized = true;




m_view->ChangeRenderingParams().IsTransparentShadowEnabled = Standard_False;
m_view->ChangeRenderingParams().ToEnableDepthPrepass = Standard_True;
// m_view->ChangeRenderingParams().UseDepthTestForLines = Standard_True;

				// Handle(Graphic3d_Camera) cam = m_view->Camera();
// 				cam->SetProjectionType(Graphic3d_Camera::Projection_Perspective); // switch to perspective
// cam->SetFOVy(45.0 * M_PI / 180.0); // optional: vertical FOV (radians)

		m_view->SetAutoZFitMode(Standard_True, 1);
		// m_view->SetAutoZFitMode(Standard_False);
//  glDepthFunc(GL_GREATER); 
// cam->SetZRange( 1000,10000);
// glDepthRange(1000,10000);

		// ApplyReverseZ(m_view);
// cam->InvalidateProjection();
// m_view->SetCamera(cam);
// m_view->Redraw();

// glEnable(GL_DEPTH_TEST);
// glDepthFunc(GL_GREATER);     // reversed-Z
// glClearDepth(0.0);           // far = 0
// glDepthRange(1.0, 0.0);      // reversed mapping

// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);

// setupProjection(w(), h());
		redraw();
		// m_view->Redraw();
		// perf("occvldoi");

		{
			const GLubyte* renderer = glGetString(GL_RENDERER);
			const GLubyte* vendor = glGetString(GL_VENDOR);
			const GLubyte* version = glGetString(GL_VERSION);

			if (renderer && vendor && version) {
				std::cout << "OpenGL Vendor:   " << vendor << std::endl;
				std::cout << "OpenGL Renderer: " << renderer << std::endl;
				std::cout << "OpenGL Version:  " << version << std::endl;
			} else {
				std::cout << "glGetString() failed — no OpenGL context active!" << std::endl;
			}
			    // Report depth buffer precision of the current window
			GLint depthBits = 0;
			glGetIntegerv(GL_DEPTH_BITS, &depthBits);
			std::cout << "Window depth buffer precision: " << depthBits << " bits" << std::endl;
			// GLint maxDepthBits = 0;
			// glGetIntegerv(GL_MAX_DEPTH_BITS, &maxDepthBits);
			// printf("Maximum supported depth bits: %d\n", maxDepthBits);

		}

		// zghost();
		toggle_shaded_transp(currentMode);
 
// setupProjection(w(), h());
		Fl::add_timeout(
			1.4,
			[](void* d) {
				auto l = (Fl_Help_View*)d;
				l->value("");
			},
			helpv); 
			m_initialized = true;
	}
	static void idle_refresh_cb(void*) {
		// clear gpu usage each 10 secs
		glFlush();
		glFinish();
		Fl::repeat_timeout(10, idle_refresh_cb, 0);
	}
	void zghost(){
		Handle(Graphic3d_GraphicDriver) aDriver = m_viewer->Driver();
// Handle(Graphic3d_GraphicDriver) aDriver = myViewer->Driver();

// Default layer (main geometry)
Graphic3d_ZLayerSettings defSettings = aDriver->ZLayerSettings(Graphic3d_ZLayerId_Default);
defSettings.SetEnableDepthTest(true);
defSettings.SetEnableDepthWrite(true);
defSettings.SetClearDepth(true);
defSettings.SetRenderInDepthPrepass(true);

Graphic3d_PolygonOffset defOffset;
defOffset.Mode   = Aspect_POM_Fill;
defOffset.Factor = 1.0f;
defOffset.Units  = 0.0f;
defSettings.SetPolygonOffset(defOffset);

aDriver->SetZLayerSettings(Graphic3d_ZLayerId_Default, defSettings);

// Top layer (geometry overlays, highlights)
Graphic3d_ZLayerSettings topSettings = aDriver->ZLayerSettings(Graphic3d_ZLayerId_Top);
topSettings.SetEnableDepthTest(true);
topSettings.SetEnableDepthWrite(true);
topSettings.SetRenderInDepthPrepass(true);
aDriver->SetZLayerSettings(Graphic3d_ZLayerId_Top, topSettings);

// TopOSD layer (UI overlays, dimensions)
Graphic3d_ZLayerSettings topOSDSettings = aDriver->ZLayerSettings(Graphic3d_ZLayerId_TopOSD);
topOSDSettings.SetEnableDepthTest(true);
topOSDSettings.SetEnableDepthWrite(true);

Graphic3d_PolygonOffset topOSDOffset;
topOSDOffset.Mode   = Aspect_POM_Fill;
topOSDOffset.Factor = 2.0f;
topOSDOffset.Units  = 2.0f;
topOSDSettings.SetPolygonOffset(topOSDOffset);

aDriver->SetZLayerSettings(Graphic3d_ZLayerId_TopOSD, topOSDSettings);

// BotOSD layer (backgrounds, HUD)
Graphic3d_ZLayerSettings botOSDSettings = aDriver->ZLayerSettings(Graphic3d_ZLayerId_BotOSD);
botOSDSettings.SetEnableDepthTest(true);
botOSDSettings.SetEnableDepthWrite(true);
aDriver->SetZLayerSettings(Graphic3d_ZLayerId_BotOSD, botOSDSettings);

	}
// 	void draw() override {
// 		if (!m_initialized) return;
// 		// m_context->UpdateCurrentViewer();
// 		glEnable(GL_DEPTH_TEST);
// 		glDepthFunc(GL_LESS);
// 		// glDepthFunc(GL_GREATER);
// 		glPolygonOffset(0,0);
// 		glDepthRange(0,1);

// 		glEnable(GL_POLYGON_OFFSET_LINE);
// glPolygonOffset(-1.0f, -1.0f); 
// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);

// 		m_view->Update();

// glDisable(GL_POLYGON_OFFSET_LINE);

// 		// m_view->Redraw(); //new
// 		// flush();
// 	}
void setupProjection(int w, int h)
{
    float aspect = float(w) / float(h);
    float fovY   = 45.0f * M_PI / 180.0f;
    float zNear  = 1.0f;   // push near plane far

    float f = 1.0f / tanf(fovY * 0.5f);

    float proj[16] = {
        f / aspect, 0,  0,  0,
        0,           f,  0,  0,
        0,           0,  0, -1,
        0,           0,  zNear, 0
    };

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);
}

// void draw() override
// {
//     if (!m_initialized)
//         return;

//     /* ---------------- clear ---------------- */

// setupProjection(w(), h());
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     /* ---------------- faces ---------------- */

//     glDisable(GL_POLYGON_OFFSET_LINE);
//     glDisable(GL_POLYGON_OFFSET_FILL);

//     m_view->Redraw();   // OCCT draws faces here
//     // m_view->Update();   // OCCT draws faces here

//     /* ---------------- edges ---------------- */

//     glEnable(GL_POLYGON_OFFSET_LINE);
//     glPolygonOffset(-1.0f, -1.0f);

//     m_view->Update();   // OCCT draws edges

//     glDisable(GL_POLYGON_OFFSET_LINE);
// }
// void show() override {
//     Fl_Gl_Window::show();
//     if (!m_initialized) {
//         initialize_opencascade();
//     }
// }
void draw () 
{
	// m_view->Redraw();return;
	// if (!m_initialized && !valid()) {
	// 	valid(1);
	if (!m_initialized) {
		// valid(1);
            glViewport(0, 0, w(), h());
        initialize_opencascade();

            // glEnable(GL_DEPTH_TEST);
    }
    if (!m_initialized) return;
	m_view->Redraw();return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthRange(0.0, 1.0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ----------------- Compute scene bounding box -----------------
    // Bnd_Box sceneBox;
    // AIS_ListOfInteractive displayed;
    // m_context->DisplayedObjects(displayed);

    // Standard_Real sceneDiag = 1.0; // fallback

    // for (AIS_ListOfInteractive::Iterator it(displayed); it.More(); it.Next())
    // {
    //     Bnd_Box b;
    //     it.Value()->BoundingBox(b);
    //     sceneBox.Add(b);
    // }

    // if (!sceneBox.IsVoid())
    // {
    //     Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    //     sceneBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    //     gp_Pnt pmin(xmin, ymin, zmin);
    //     gp_Pnt pmax(xmax, ymax, zmax);
    //     sceneDiag = pmin.Distance(pmax);

    //     if (sceneDiag <= Precision::Confusion())
    //         sceneDiag = 1000.0; // fallback

    //     // ----------------- Set optimal camera Z range -----------------
    //     Handle(Graphic3d_Camera) cam = m_view->Camera();
    //     // m_view->SetAutoZFitMode(Standard_False);

    //     Standard_Real zNear = sceneDiag * 0.02;  // 2% of scene size
    //     Standard_Real zFar  = sceneDiag * 2.0;   // 2× scene diagonal
	// 	cotm2(zNear, zFar);
    //     cam->SetZRange(zNear, zFar);
    //     cam->InvalidateProjection();
    // }

    // // ----------------- Dynamic line offset based on scene size -----------------
    // // Use 0.1% of the diagonal for the polygon offset factor
    // Standard_Real offsetFactor = sceneDiag * 0.001;
    glEnable(GL_POLYGON_OFFSET_LINE);
    // glPolygonOffset(-offsetFactor, -offsetFactor);
    glPolygonOffset(-3, -3);


    // ----------------- Draw the scene -----------------

// m_view->SetZoom(30.0);
    m_view->Redraw();

    glDisable(GL_POLYGON_OFFSET_LINE);
}
void draw_() {
    make_current();

    if (!m_initialized || m_glContext.IsNull()) {
        return;
    }

    int winW = w();
    int winH = h();

    if (!m_fbo.IsNull()) {
        // Render into custom FBO
        m_fbo->BindBuffer(m_glContext);
        glViewport(0, 0, winW, winH);

        // Let OCCT handle clearing (background color is already set)
        m_view->Redraw();

        m_fbo->UnbindBuffer(m_glContext);

        // Blit to window - correct orientation (no manual Y-flip needed)
        m_fbo->BindReadBuffer(m_glContext);
        glBlitFramebuffer(0, 0, winW, winH,
                          0, 0, winW, winH,
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    } else {
        // Direct rendering fallback
		cotm(3)
        // m_view->Redraw();
    }
}
// void draw() override
// {
//     if (!m_initialized) return;

//     make_current(); // garante contexto FLTK

//     Handle(OpenGl_Context) ctx = m_graphic_driver->GetSharedContext();
//     if (ctx.IsNull() || !ctx->IsValid()) return;

//     // ---------- Bind do FBO ----------
//     if (!m_fbo.IsNull() && m_fbo->IsValid())
//         m_fbo->BindBuffer(ctx);

//     // ---------- Configuração OpenGL ----------
//     glEnable(GL_DEPTH_TEST);
//     glDepthFunc(GL_LESS);
//     glDepthRange(0.0, 1.0);

//     glEnable(GL_CULL_FACE);
//     glCullFace(GL_BACK);

//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     glEnable(GL_POLYGON_OFFSET_LINE);
//     glPolygonOffset(-3, -3);

//     // ---------- Redraw da view OCCT ----------
//     m_view->Redraw(); // renderiza a cena no FBO

//     glDisable(GL_POLYGON_OFFSET_LINE);

//     // ---------- Unbind do FBO ----------
//     if (!m_fbo.IsNull() && m_fbo->IsValid())
//         m_fbo->UnbindBuffer(ctx);

//     // ---------- Blit do FBO para a janela FLTK ----------
//     if (!m_fbo.IsNull() && m_fbo->IsValid())
//     {
//         // Se FboId() for público
//         // GLuint fboId = m_fbo->FboId();
// 	GLint fboId = 0;
// glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fboId);

//         glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
//         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // framebuffer da janela

//         glBlitFramebuffer(
//             0, 0, this->w(), this->h(),   // origem
//             0, 0, this->w(), this->h(),   // destino
//             GL_COLOR_BUFFER_BIT, GL_NEAREST
//         );

//         glBindFramebuffer(GL_FRAMEBUFFER, 0);
//     }
// }


bool createOrResizeFBO(int width, int height)
{
    if (width <= 0 || height <= 0) return false;

    make_current();  // Critical!

	// if(!m_fbo->IsNull())m_fbo.Nullify()
	if (m_glContext.IsNull()) return 0;

    // Temporarily detach FBO to prevent OCCT rendering into broken state
    if (!m_fbo.IsNull()) {
        m_glView->SetFBO(Handle(OpenGl_FrameBuffer)());  // null handle = default window FB
    };

	// Now safely re-create FBO with new size
    // if (!m_fbo.IsNull()) {
    //     bool success = createOrResizeFBO(width, height);

    //     if (success) {
    //         m_glView->SetFBO(m_fbo);
    //     } else {
    //         m_fbo.Nullify();
    //     }
    // }
zghost();
const Graphic3d_Vec2i size(width, height);
// m_fbo->Init(m_glContext, size, GL_RGBA8, GL_DEPTH_COMPONENT32, 0);zghost();
// return 1;
// // Preferred retry with InitLazy
// if (!m_fbo->Init(m_glContext, size, GL_RGBA8, GL_DEPTH_COMPONENT32F, 0)) {
//     m_fbo->Release(m_glContext.get());

//     // Try InitLazy with 32F
//     if (!m_fbo->InitLazy(m_glContext, size, GL_RGBA8, GL_DEPTH_COMPONENT32F, 0)) {
//         // Fresh object for a clean state
//         m_fbo = new OpenGl_FrameBuffer();

//         // Try 32-bit integer depth
//         if (!m_fbo->Init(m_glContext, size, GL_RGBA8, GL_DEPTH_COMPONENT32, 0)) {
//             // Final fallback to 24-bit
//             m_fbo->Release(m_glContext.get());
//             m_fbo = new OpenGl_FrameBuffer();
//             m_fbo->Init(m_glContext, size, GL_RGBA8, GL_DEPTH_COMPONENT24, 0);
//         }
//     }
// }
// return 1;
    // Fallback 1: RGBA8 + 32F depth/stencil
    if (m_fbo->InitLazy(m_glContext, Graphic3d_Vec2i(width, height),
            GL_RGBA8, GL_DEPTH_COMPONENT32, 0))
    {
        std::cout << "FBO created: RGBA8 + GL_DEPTH_COMPONENT32F" << std::endl;
        return true;
    }
	m_fbo->Release(m_glContext.get());
	m_fbo = new OpenGl_FrameBuffer();
	m_glView->SetFBO(Handle(OpenGl_FrameBuffer)());
	//  m_fbo = new OpenGl_FrameBuffer();
    // Fallback 1: RGBA8 + 32F depth/stencil

    if (m_fbo->InitLazy(m_glContext,
                    Graphic3d_Vec2i(width, height),
                    GL_RGBA8,
                    GL_DEPTH32F_STENCIL8,
                    0))
    {
        std::cout << "FBO created: RGBA8 + DEPTH32F_STENCIL8" << std::endl;
		zghost();
        return true;
    }


    // First try the best format: sRGB + 32F depth/stencil
    if (m_fbo->Init(m_glContext,
                    Graphic3d_Vec2i(width, height),
                    GL_SRGB8_ALPHA8,
                    GL_DEPTH32F_STENCIL8,
                    0))
    {
        std::cout << "FBO created: sRGB8_ALPHA8 + DEPTH32F_STENCIL8" << std::endl;
        return true;
    }



    // Fallback 2: sRGB + 32F depth only (no stencil)
    if (m_fbo->Init(m_glContext,
                    Graphic3d_Vec2i(width, height),
                    GL_SRGB8_ALPHA8,
                    GL_DEPTH_COMPONENT32F,
                    0))
    {
        std::cout << "FBO created: sRGB8_ALPHA8 + DEPTH_COMPONENT32F" << std::endl;
        return true;
    }

    std::cerr << "All FBO formats failed!" << std::endl;
    return false;
}


	void resize(int X, int Y, int W, int H) override {
		static bool in_resize=0;
		if (in_resize) return; // Prevent recursion
        in_resize = true;

		// cotm(W)
		Fl_Window::resize(X, Y, W, window()->h()-22-25);
		// Fl_Window::resize(X, Y, W, H);
		// cotm(W)
        	// woccbtn->resize(woccbtn->x(), woccbtn->y(), W, 24*0.7);
		if (m_initialized) {

			// lop(i, 0, sbt.size()) {	sbt[i].occbtn->size(sbt[i].occbtn->w(),24);}
			
	// woccbtn->size(W-40, 24);

// if (m_glContext.IsNull()) return;

//     // Detach FBO first — prevents rendering into potentially broken state
//     m_glView->SetFBO(Handle(OpenGl_FrameBuffer)());

//     bool success = createOrResizeFBO(W, H);

//     if (success) {
//         m_glView->SetFBO(m_fbo);
//     } else {
// 		cotm2("nofbo")
//         // m_fbo.Nullify();  // Force fallback to on-screen
//     }


			m_view->MustBeResized();
			setbar5per();
			// redraw(); 
			// init_sizes();
		}
		in_resize=false;
	}

	void setbar5per() {
		Standard_Integer width, height;
		m_view->Window()->Size(width, height);
		Standard_Real barWidth = width * 0.05;

		static Handle(Graphic3d_Structure) barStruct;
		if (!barStruct.IsNull()) {
			barStruct->Erase();
			barStruct->Clear();
		} else {
			barStruct = new Graphic3d_Structure(m_view->Viewer()->StructureManager());
			barStruct->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_2d));
			barStruct->SetZLayer(Graphic3d_ZLayerId_BotOSD);
		}

		Handle(Graphic3d_ArrayOfTriangles) tri = new Graphic3d_ArrayOfTriangles(6);

		Standard_Real x0 = width - barWidth;
		Standard_Real x1 = width;
		Standard_Real y0 = 0.0;
		Standard_Real y1 = height;

		tri->AddVertex(gp_Pnt(x0, y0, 0.0));
		tri->AddVertex(gp_Pnt(x1, y0, 0.0));
		tri->AddVertex(gp_Pnt(x1, y1, 0.0));
		tri->AddVertex(gp_Pnt(x0, y0, 0.0));
		tri->AddVertex(gp_Pnt(x1, y1, 0.0));
		tri->AddVertex(gp_Pnt(x0, y1, 0.0));

		Handle(Graphic3d_Group) group = barStruct->NewGroup();

		// Create fill area aspect
		Handle(Graphic3d_AspectFillArea3d) aspect = new Graphic3d_AspectFillArea3d();
		aspect->SetInteriorStyle(Aspect_IS_SOLID);
		aspect->SetInteriorColor(Quantity_NOC_RED);

		// Configure material for transparency
		Graphic3d_MaterialAspect material;
		material.SetMaterialType(Graphic3d_MATERIAL_ASPECT);
		material.SetAmbientColor(Quantity_NOC_RED);
		material.SetDiffuseColor(Quantity_NOC_RED);
		// material.SetSpecularColor(Quantity_NOC_WHITE);
		material.SetTransparency(0.5);	// 50% transparency

		aspect->SetFrontMaterial(material);
		aspect->SetBackMaterial(material);

		// For proper transparency rendering
		aspect->SetSuppressBackFaces(false);

		group->SetGroupPrimitivesAspect(aspect);
		group->AddPrimitiveArray(tri);

		barStruct->Display();
		// m_view->Redraw();
		//  redraw(); //  redraw(); // m_view->Update ();
	}

	/// Configure dashed highlight lines without conversion errors
	void SetupHighlightLineType(const Handle(AIS_InteractiveContext) & ctx) {


		{
// 1. Create the Drawer object
Handle(Prs3d_Drawer) hl = new Prs3d_Drawer();

// 2. Set the highlight color (which you said is working)
hl->SetColor(Quantity_NOC_RED);

// 3. Create a Prs3d_LineAspect specifically for the *width* and *style*
//    Make sure the color here is also the highlight color for consistency.
Handle(Prs3d_LineAspect) thickGreenAspect =
    new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 5.0);

// 4. ***Crucially, set the Line Aspect for the highlight presentation***
//    This tells the Drawer what line properties to use when drawing the highlighted shape.
hl->SetLineAspect(thickGreenAspect);

// 5. Ensure the shape is highlighted in a *Wireframe* mode, 
//    as shaded highlighting (the default) might only show the face color.
hl->SetDisplayMode(0); // Display mode 0 typically refers to Wireframe/Edges

// 6. Apply the style
ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Dynamic, hl);
ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Selected, hl);
ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalSelected, hl);

// 7. Update the viewer
ctx->UpdateCurrentViewer();

			return;
		}

Handle(Prs3d_Drawer) selDrawer = new Prs3d_Drawer();

// Explicitly set edge/wire aspects
Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(
    Quantity_NOC_RED,   // highlight color
    Aspect_TOL_SOLID,   // line type
    5.0                 // width in pixels
);

selDrawer->SetWireAspect(edgeAspect);
selDrawer->SetLineAspect(edgeAspect);       // fallback
selDrawer->SetSeenLineAspect(edgeAspect);   // for visible edges
selDrawer->SetColor(Quantity_NOC_GREEN);
// Apply to context
ctx->SetSelectionStyle(selDrawer);

		ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalSelected, selDrawer);
		// cotm(5)
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Dynamic, selDrawer);
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Selected, selDrawer);

// Force viewer update
ctx->UpdateCurrentViewer();
return;




		// 1. Create a drawer for highlights
		// Handle(Prs3d_Drawer) customDrawer = new Prs3d_Drawer();
		// customDrawer->SetDisplayMode(1);  // wireframe only

		// // 2. Build the raw Graphic3d aspect
		// Handle(Graphic3d_AspectLine3d) rawAspect =
		//     new Graphic3d_AspectLine3d(Quantity_NOC_RED,
		//                                Aspect_TOL_DASH,
		//                                5.0);

		// // 3. Wrap it in a Prs3d_LineAspect
		// Handle(Prs3d_LineAspect) lineAspect =
		//     new Prs3d_LineAspect(rawAspect);

		// // 4. Apply color, transparency, and the wrapped aspect
		// customDrawer->SetColor(Quantity_NOC_RED);
		// customDrawer->SetTransparency(0.3f);
		// customDrawer->SetLineAspect(lineAspect);
		// // customDrawer->Setwi

		// customDrawer->SetLineAspect(lineAspect);
		// customDrawer->SetSeenLineAspect(lineAspect);
		// customDrawer->SetWireAspect(lineAspect);
		// customDrawer->SetUnFreeBoundaryAspect(lineAspect);
		// customDrawer->SetFreeBoundaryAspect(lineAspect);
		// customDrawer->SetFaceBoundaryAspect(lineAspect);

		Handle(AIS_ColoredDrawer) customDrawer=new AIS_ColoredDrawer(customDrawerp);

		// customDrawer->SetLineAspect(highlightaspect);
		// customDrawer->SetSeenLineAspect(highlightaspect);
		// customDrawer->SetWireAspect(highlightaspect);
		// customDrawer->SetUnFreeBoundaryAspect(highlightaspect);
		// customDrawer->SetFreeBoundaryAspect(highlightaspect);
		// customDrawer->SetFaceBoundaryAspect(highlightaspect);


Handle(Prs3d_LineAspect) highlightAsp =
    new Prs3d_LineAspect(Quantity_Color(Quantity_NOC_GREEN),
                         Aspect_TOL_SOLID,
                         3.0);

// assign it to various roles
customDrawer->SetLineAspect(highlightAsp);
customDrawer->SetWireAspect(highlightAsp);
customDrawer->SetFreeBoundaryAspect(highlightAsp);
customDrawer->SetUnFreeBoundaryAspect(highlightAsp);
customDrawer->SetFaceBoundaryAspect(highlightAsp);
customDrawer->SetHiddenLineAspect(highlightAsp);
customDrawer->SetVectorAspect(highlightAsp);
customDrawer->SetSectionAspect(highlightAsp);
customDrawer->SetSeenLineAspect(highlightAsp);

// customDrawer->SetDimensionAspect(highlightAsp);
// customDrawer->SetArrowAspect(highlightAsp);
// customDrawer->SetDatumAspect(highlightAsp);

// 2. Make sure those edges actually are drawn
customDrawer->SetFreeBoundaryDraw(true);
customDrawer->SetUnFreeBoundaryDraw(true);
customDrawer->SetFaceBoundaryDraw(true);

// 3. Use better tessellation / curve smoothing if needed
customDrawer->SetTypeOfDeflection(Aspect_TOD_RELATIVE);
customDrawer->SetMaximalChordialDeviation(0.25);  // e.g. 0.25 units tolerance

// 4. Hidden-line, HLR settings
// customDrawer->SetTypeOfHLR(Prs3d_TOH_PREFERRED);  // or whichever you prefer

// 5. Draw highlight on top
customDrawer->SetZLayer(Graphic3d_ZLayerId_Topmost);











		// ctx->SetSelectionStyle(highlightaspect);

		// customDrawer->SetColor(Quantity_NOC_RED);
		// customDrawer->SetOwnWidth(8);
		// customDrawer->SetTransparency(0.3f);

		// 5. Assign to all highlight modes
		// ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalDynamic,
		// customDrawer);
		ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalSelected, customDrawer);
		// cotm(5)
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Dynamic, customDrawer);
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Selected, customDrawer);


// 				Handle(Prs3d_Drawer) selDrawer = ctx->SelectionStyle();
// selDrawer->SetColor(Quantity_NOC_GREEN);    // green color
// // selDrawer->SetWidth(3.0);                   // 3 px thick
// ctx->SetSelectionStyle(selDrawer);
	}

#pragma endregion initialization

#pragma region luastruct
	struct luadraw;
	// vector<luadraw*> vlua;
	// unordered_map<string, OCC_Viewer::luadraw*> ulua;
	vector<luadraw*> vlua;
	unordered_map<string,luadraw*> ulua;
	template <typename T>
	struct ManagedPtrWrapper : public Standard_Transient {
		T* ptr;
		ManagedPtrWrapper(T* p) : ptr(p) {}
		~ManagedPtrWrapper() override {
			if(ptr)delete ptr; // delete when wrapper is destroyed
		}
	};

	struct luadraw {
		TopLoc_Location Origin;
		int type=0;
		// vector<luadraw*> vnl;
		bool editing=0; 
		bool ttfillet=0;

		// build it using BRep_Builder
		BRep_Builder builder; 
		string name = "";
		bool visible_hardcoded = 1;
		// TopoDS_Shape cshape;
		TopoDS_Compound cshape;
		TopoDS_Shape shape;
		TopoDS_Shape fshape;
		// std::shared_ptr<TopoDS_Shape> shape;
		Handle(AIS_Shape) ashape;
		// TopoDS_Face face;
		gp_Pnt origin = gp_Pnt(0, 0, 0);
		gp_Dir normal = gp_Dir(0, 0, 1);
		gp_Dir xdir = gp_Dir(1, 0, 0);
		gp_Trsf trsf;
		vector<gp_Trsf> vtrsf;
		gp_Trsf trsftmp;
		bool needsplacementupdate = 1;
		OCC_Viewer* occv;
		// RTTI declaration
		// DEFINE_STANDARD_RTTIEXT(OCC_Viewer::luadraw, Standard_Transient)
		luadraw(string _name = "test", OCC_Viewer* p = 0) : occv(p), name(_name) {
			// regen
			 if(occv->vaShape.size()>0){
			 	OCC_Viewer::luadraw*
			 ld=occv->getluadraw_from_ashape(occv->vaShape.back());
			 	// occv
			 	ld->redisplay();
			 }

			builder.MakeCompound(TopoDS::Compound( cshape) );

			auto it = occv->ulua.find(name);
			int counter = 1;
			std::string new_name = name;
			while (it != occv->ulua.end()) {
				new_name = name + std::to_string(counter);
				it = occv->ulua.find(new_name);
				counter++;
			}
			name = new_name;
			// cotm("new",name)

			gp_Ax2 ax3(origin, normal, xdir);
			trsf.SetTransformation(ax3);
			trsf.Invert();
			ashape = new AIS_Shape(cshape);
			// shape = cshape;

			// allocate something for the application and hand ownership to the
			// wrapper
			Handle(ManagedPtrWrapper<luadraw>) wrapper = new ManagedPtrWrapper<luadraw>(this);

			// store the wrapper in the AIS object via SetOwner()
			ashape->SetOwner(wrapper);

			// ashape->SetUserData(new ManagedPtrWrapper<luadraw>(this));
			occv->vaShape.push_back(ashape);
			occv->m_context->Display(ashape, 0);
			occv->ulua[name] = this;
			occv->vlua.push_back(this);

				Origin=occv->Origin; 
				// Origin = TopLoc_Location(occv->Origin.Transformation());

				ashape->SetLocalTransformation(Origin.Transformation());
		}
 

		TopoDS_Shape resetShapePlacement(const TopoDS_Shape& s) {
			if (s.IsNull()) return s;

			TopoDS_Shape working = s;
			working.Location(TopLoc_Location()); // clear top-level

			gp_Trsf trsf = working.Location().Transformation();
			if (trsf.Form() != gp_Identity) {
				working = BRepBuilderAPI_Transform(working, trsf, true).Shape();
				working.Location(TopLoc_Location());
			}

			if (working.ShapeType() == TopAbs_COMPOUND) {
				TopoDS_Compound newCompound;
				BRep_Builder builder;
				builder.MakeCompound(newCompound);
				for (TopExp_Explorer exp(working, TopAbs_SHAPE); exp.More(); exp.Next()) {
					builder.Add(newCompound, resetShapePlacement(exp.Current()));
				}
				return newCompound;
			}

			return working;
		}
		bool orupd=0;
		void update_placement() { 
			if (needsplacementupdate == 0) return; 
			// FuseAndRefineWithAPI(cshape); 
			shape=FuseAndRefineWithAPI(cshape);

			// Origin=
			// occv->Origintrsf.SetTranslation(gp_Vec(200, 100, -200));
 			// occv->Origin=TopLoc_Location(occv->Origintrsf);

			// TopLoc_Location Lcur = shape.Location();  // current location
			// TopLoc_Location Lnew = Lcur * occv->Origin;   // compose
			// shape.Location(Lnew); 
			// {
			// TopLoc_Location Lcur = cshape.Location();  // current location
			// TopLoc_Location Lnew = Lcur * occv->Origin;   // compose
			// cshape.Location(Lnew); 
			// }

			// shape=cshape; 
			ashape->Set(cshape);

			
			if(orupd==0 ){//&& !occv->Origin.IsIdentity()){
				// orupd=1;
				// Origin=occv->Origin; 
				// ashape->SetLocalTransformation(Origin.Transformation());
			}


			// ashape->Set(Extract_Solids(cshape));
			needsplacementupdate = 0; 
		}
	void redisplay() {
		// needsplacementupdate=1;
			update_placement();

				// cotm("notvisible",name,visible_hardcoded)
			// If visible, redisplay (only if it's displayed already, otherwise
			// Display)
			if (visible_hardcoded) {
				if (occv->m_context->IsDisplayed(ashape)) {
					occv->m_context->Redisplay(ashape, false);
				} else {
					occv->m_context->Display(ashape, false);
				}
			} else {
				// Only erase if it is displayed
				if (occv->m_context->IsDisplayed(ashape)) {
				cotm1("notvisible",name)
					occv->m_context->Erase(ashape, Standard_False);
				}
			}
		}

		void display() { occv->m_context->Display(ashape, 0); }
		// void rotate(int angle){
		// void rotate(int angle,gp_Dir normal={0,0,1}){
		// 	trsftmp = gp_Trsf();
		// 	// gp_Dir normal=gp_Dir(0,1,0);
		// 	trsftmp.SetRotation(gp_Ax1(origin, normal), angle*(M_PI/180) );
		// 	trsf  *= trsftmp;
		// }

		void rotate(int angle, float x = 0, float y = 0, float z = 0) {
			// void rotate(int angle,gp_Dir normal={0,0,1}){
			needsplacementupdate = 1;
			trsftmp = gp_Trsf();
			gp_Dir normal = gp_Dir(x, y, z);
			trsftmp.SetRotation(gp_Ax1(origin, normal), angle * (M_PI / 180));
			trsf = trsftmp * trsf;
			// trsf  *= trsftmp;
			// update_placement();
		}
		bool hasAnyTriangulation(const TopoDS_Shape& shape) {
			for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next()) {
				const TopoDS_Face& F = TopoDS::Face(ex.Current());
				TopLoc_Location L;
				Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(F, L);
				if (!tri.IsNull() && tri->NbTriangles() > 0) {
					return true;
				}
			}
			return false;
		}
		void translate(float x = 0, float y = 0, float z = 0, bool world = 1, float fromwx = 0, float fromwy = 0,
					   float fromwz = 0) {
			needsplacementupdate = 1;
			if (world) {
				gp_Trsf moveToOrigin;
				moveToOrigin.SetTranslation(gp_Vec(-fromwx, -fromwy, -fromwz));

				gp_Trsf moveToTarget;
				moveToTarget.SetTranslation(gp_Vec(x, y, z));

				// Apply: first to origin, then to target
				trsf = moveToTarget * moveToOrigin * trsf;
			}
			if (!world) {
				trsftmp = gp_Trsf();
				trsftmp.SetTranslation(gp_Vec(x, y, z));
				// trsf = trsftmp * trsf; //world
				trsf *= trsftmp;
			}
		}

  

TopoDS_Shape ExtractNonSolids(const TopoDS_Shape& compound)
{
    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);

    // Iterate only over immediate children of the compound
    for (TopoDS_Iterator it(compound); it.More(); it.Next())
    {
        const TopoDS_Shape& child = it.Value();

        if (child.ShapeType() != TopAbs_SOLID)
        {
            builder.Add(result, child);
        }
    }

    return result;
}

TopoDS_Compound Extract_Solids(const TopoDS_Compound& source) {
    TopoDS_Compound result;
    TopoDS_Builder builder;
    builder.MakeCompound(result);

    for (TopExp_Explorer ex(source, TopAbs_SOLID); ex.More(); ex.Next()) {
        const TopoDS_Solid& solid = TopoDS::Solid(ex.Current());
        builder.Add(result, solid);
    }

    return result;
}

std::vector<TopoDS_Solid> ExtractSolids(const TopoDS_Shape& shape) {
    std::vector<TopoDS_Solid> solids;
    for (TopExp_Explorer exp(shape, TopAbs_SOLID); exp.More(); exp.Next()) {
        solids.push_back(TopoDS::Solid(exp.Current()));
    }
    return solids;
}


TopoDS_Shape FuseAllSolids(const TopoDS_Shape& shape) {
    std::vector<TopoDS_Solid> solids = ExtractSolids(shape);
    if (solids.empty()) return TopoDS_Shape();
    if (solids.size() == 1) return solids[0]; // only one solid, nothing to fuse

    BOPAlgo_BOP bop;
    bop.SetOperation(BOPAlgo_FUSE);
    for (const auto& s : solids) {
        bop.AddArgument(s);
    }
    bop.Perform();

    // Check for errors instead of IsDone()
    // if (bop.HasErrors()) {
    //     throw std::runtime_error("BOPAlgo_BOP fuse failed (HasErrors).");
    // }

    return bop.Shape();
}
// #include <ShapeUpgrade_UnifySameDomain.hxx>
static TopoDS_Shape CompoundInstead(const std::vector<TopoDS_Solid>& solids) {
    BRep_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);
    for (auto& s : solids) builder.Add(comp, s);
    return comp;
}
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_BuilderAlgo.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <vector>
#include <BOPAlgo_BOP.hxx>
#include <BOPAlgo_Options.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

TopoDS_Shape FuseAndRefineWithAPI_v2gpt(const TopoDS_Shape& inputShape) {
    auto solids = ExtractSolids(inputShape);
    if (solids.empty()) {
        lua_error_with_line(L, "No solids found in input shape\n");
        return shape;
    }

    if (solids.size() == 1) {
        ShapeUpgrade_UnifySameDomain unify(solids[0], true, true, true);
        unify.Build();
        return unify.Shape();
    }

    // ✅ Use BOPAlgo_BOP for parallel multi-fuse
    BOPAlgo_BOP bop;
    for (const auto& s : solids)
        bop.AddArgument(s);

    bop.SetOperation(BOPAlgo_FUSE);
    bop.SetRunParallel(Standard_True);
    bop.SetNonDestructive(Standard_True);
    bop.SetFuzzyValue(1.0e-6);
    bop.Perform();

    if (bop.HasErrors()) {
        std::cerr << "❌ BOPAlgo_BOP fusion failed with errors:\n";
        bop.DumpErrors(std::cerr);
        return inputShape; // fallback
    }

    TopoDS_Shape fused = bop.Shape();
return fused;
    // ✅ Optional refinement
    ShapeUpgrade_UnifySameDomain unify(fused, true, true, true);
    unify.Build();
    return unify.Shape();
}

TopoDS_Shape FuseAndRefineWithAPI_v3(const TopoDS_Shape& inputShape) {
    // Early validation of input shape
    if (inputShape.IsNull()) {
        cotm(name, "⚠️ Input shape is null");
        lua_error_with_line(L, "Input shape is null\n");
        return shape;
    }
	// return inputShape;

    // Extract solids efficiently
    std::vector<TopoDS_Shape> solids;
    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next()) {
        solids.push_back(exp.Current());
    }

    if (solids.empty()) {
        cotm(name, "⚠️ No solids found in input shape");
        lua_error_with_line(L, "No solids found in input shape\n");
        return shape; // Return input as-is or empty shape
    }

    if (solids.size() == 1) {
        // Single solid: refine only
        ShapeUpgrade_UnifySameDomain unify(solids[0], /*UnifyFaces*/ true, /*UnifyEdges*/ true, /*ConcatBSplines*/ true);
        unify.Build();
        return unify.Shape();
    }

    // Batch fusion using BRepAlgoAPI_BuilderAlgo for efficiency
    BRepAlgoAPI_BuilderAlgo fuseAlgo;
    TopTools_ListOfShape arguments;
    for (const auto& solid : solids) {
        arguments.Append(solid);
    }
    fuseAlgo.SetArguments(arguments);
    fuseAlgo.SetRunParallel(true); // Enable parallel processing if safe
    fuseAlgo.Build();

    if (!fuseAlgo.IsDone()) {
        cotm(name, "❌ Fusion failed, returning compound instead");
        // Optionally return a compound of solids (not shown)
        return inputShape; // Or create compound from solids
    }

	return fuseAlgo;
    // Refine result: merge faces, edges, vertices
    ShapeUpgrade_UnifySameDomain unify(fuseAlgo, true, true, true);
    unify.Build();
    return unify.Shape();


    // Refine the fused shape
    // TopoDS_Shape fused = fuseAlgo.Shape();
    // ShapeUpgrade_UnifySameDomain unify(fused, /*UnifyFaces*/ true, /*UnifyEdges*/ true, /*ConcatBSplines*/ true);
    // unify.Build();
    // return unify.Shape();
}
TopoDS_Shape FuseAndRefineWithAPI_v1(const TopoDS_Shape& inputShape) {
    auto solids = ExtractSolids(inputShape);
    if (solids.empty()) {
        // std::cerr << "⚠️ No solids found in input shape\n";
		cotm(name,"⚠️ No solids found in input shape");
		lua_error_with_line(L, "No solids found in input shape\n");
        return shape;
        // return TopoDS_Shape();
    }
    if (solids.size() == 1) {
        // Only one solid → just refine it
        ShapeUpgrade_UnifySameDomain unify(solids[0], true, true, true);
        unify.Build();
        return unify.Shape();
    }

    // Iteratively fuse solids
    TopoDS_Shape fused = solids[0];
    for (size_t i = 1; i < solids.size(); ++i) {
        BRepAlgoAPI_Fuse fuse(fused, solids[i]);
        fuse.Build();
        if (!fuse.IsDone()) {
            std::cerr << "❌ Fuse failed at step " << i << ", returning compound instead.\n";
            // return CompoundInstead(solids);
        }
        fused = fuse.Shape();
    }

    // Refine result: merge faces, edges, vertices
    ShapeUpgrade_UnifySameDomain unify(fused, true, true, true);
    unify.Build();
    TopoDS_Shape refined = unify.Shape();

    return refined;
}
 
 TopoDS_Shape FuseAndRefineWithAPI(const TopoDS_Shape& inputShape) {
    auto solids = ExtractSolids(inputShape);
    if (solids.empty()) {
        // std::cerr << "⚠️ No solids found in input shape\n";
		cotm(name,"⚠️ No solids found in input shape");
		lua_error_with_line(L, "No solids found in input shape\n");
        return shape;
        // return TopoDS_Shape();
    }
    if (solids.size() == 1) {
		return solids[0];
        // Only one solid → just refine it
        ShapeUpgrade_UnifySameDomain unify(solids[0], true, true, true);
        unify.Build();
        return unify.Shape();
    }
	return cshape;

}
 
		void copy_placement(luadraw* tocopy) { trsf = tocopy->trsf; }
		void exit() { throw std::runtime_error(lua_error_with_line(L, "exit")); }
 


void clone(luadraw* toclone, bool copy_placement = false) {
    if (!toclone) {
        throw std::runtime_error(lua_error_with_line(L, "Something went wrong"));
    }

    this->vpoints = toclone->vpoints;

    if (!toclone->shape.IsNull()) {
		toclone->needsplacementupdate=1; //verify better why this is needed
		toclone->redisplay(); //verify better why this is needed

		// auto solids = ExtractSolids(toclone->cshape);
		// if (solids.size() >0) {
		// this->shape = solids[0];}
		// else  this->shape = toclone->cshape;
        this->shape = toclone->shape;

        if (!copy_placement) { 
			// shape.Location(TopLoc_Location());
            shape = resetShapePlacement(shape);
        }

        mergeShape(cshape, shape);
    }
}

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <V3d_View.hxx>
#include <vector>

void MoveAssembly(std::vector<Handle(AIS_Shape)>& parts,
                  Handle(AIS_InteractiveContext)& context,
                  const gp_Trsf& move)
{
    for (auto& part : parts)
    {
        // Keep each part’s local transform, but add the new offset
        gp_Trsf current = part->LocalTransformation();
        part->SetLocalTransformation(current.Multiplied(move));
        context->Redisplay(part, Standard_False);
    }
    context->UpdateCurrentViewer();
}

void ResetAssembly(std::vector<Handle(AIS_Shape)>& parts,
                   Handle(AIS_InteractiveContext)& context)
{
    for (auto& part : parts)
    {
        part->ResetTransformation();
        context->Redisplay(part, Standard_False);
    }
    context->UpdateCurrentViewer();
}

// Example usage:
void Example_MoveAssembly(Handle(AIS_InteractiveContext)& context)
{
    std::vector<Handle(AIS_Shape)> parts;

    // make two boxes offset in space
    TopoDS_Shape box1 = BRepPrimAPI_MakeBox(100, 100, 100);
    TopoDS_Shape box2 = BRepPrimAPI_MakeBox(100, 100, 100).Shape();

    Handle(AIS_Shape) ais1 = new AIS_Shape(box1);
    Handle(AIS_Shape) ais2 = new AIS_Shape(box2);

    gp_Trsf offset;
    offset.SetTranslation(gp_Vec(150, 0, 0));
    ais2->SetLocalTransformation(offset);

    parts.push_back(ais1);
    parts.push_back(ais2);

    for (auto& p : parts)
        context->Display(p, Standard_False);

    context->UpdateCurrentViewer();

    // Now move the entire group (shift world origin)
    gp_Trsf moveAll;
    moveAll.SetTranslation(gp_Vec(-75, -50, 0));
    MoveAssembly(parts, context, moveAll);

    // Later, you can reset them:
    // ResetAssembly(parts, context);
}

TopoDS_Shape GetLastType(const TopoDS_Compound& cshape, TopAbs_ShapeEnum type = TopAbs_WIRE) {
    TopoDS_Shape last;

    // Only explore direct subshapes of the compound
    for (TopExp_Explorer ex(cshape, type); ex.More(); ex.Next()) {
        last = ex.Current();
    }
// for (TopExp_Explorer ex(cshape, TopAbs_SHAPE); ex.More(); ex.Next()) {
//     std::cout << "Shape type: " << ex.Current().ShapeType() << std::endl;
// }
    return last; // may be null if none found
}




		bool solidify_wire_to_face() { 
			TopoDS_Shape shape=GetLastType(cshape,TopAbs_WIRE);
			if (shape.IsNull() || shape.ShapeType() != TopAbs_WIRE) {
				return false;  // not a wire — nothing to do
			}

			TopoDS_Wire wire = TopoDS::Wire(shape);

			// Attempt to make a face directly from the wire
			BRepBuilderAPI_MakeFace mkFace(wire);
			if (!mkFace.IsDone()) {
				return false;  // face creation failed (open or non‑planar wire,
							   // etc.)
			}

			shape = mkFace.Face();
    // Merge the new face into the compound
    mergeShape(cshape, shape);
			return true;
		}
		void extrude_v1(float qtd = 0) {
			cotm("Extrude", name);
			bool sw=solidify_wire_to_face();
			cout<<"sw"<<sw<<endl;
			// 			gp_Dir dir = [](gp_Trsf trsf) {
			// 	gp_Ax3 ax3;
			// 	ax3.Transform(trsf);
			// 	return ax3.Direction();
			// }(vtrsf);
			// gp_Vec extrusionVec(dir);
			gp_Vec extrusionVec(normal);
			extrusionVec *= qtd;
			TopoDS_Shape shape=GetLastType(cshape,TopAbs_FACE);
			if (shape.IsNull()){
				return;
			}
			TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(shape, extrusionVec).Shape();  // shape from last
				printf("extrude\n");
			mergeShape(cshape, extrudedShape);
			printf("extrude2\n");
			// cshape=extrudedShape;
			// throw std::runtime_error(lua_error_with_line(L, "Something went
			// wrong")); throw std::runtime_error("Something went wrong");
		}
void fuse(luadraw* tofuse1, luadraw* tofuse2) {
			if (!tofuse1 || !tofuse2) {
				throw std::runtime_error(lua_error_with_line(L, "Invalid luadraw pointer in fuse"));
			}
			tofuse1->update_placement();
			tofuse2->update_placement();
			Handle(AIS_Shape) af1 = tofuse1->ashape;
			Handle(AIS_Shape) af2 = tofuse2->ashape;
			TopoDS_Shape ts1 = tofuse1->shape;
			TopoDS_Shape ts2 = tofuse2->shape;

			BRepAlgoAPI_Fuse fuse(ts1, ts2);
			fuse.Build();
			if (!fuse.IsDone()) {
				throw std::runtime_error(lua_error_with_line(L, "Fuse operation failed"));
			}

			tofuse1->visible_hardcoded = 0;
			tofuse2->visible_hardcoded = 0;

			TopoDS_Shape fusedShape = fuse.Shape();

			// Refine the result
			ShapeUpgrade_UnifySameDomain unify(fusedShape, true, true, true);  // merge faces, edges, vertices
			unify.Build();
			this->shape = unify.Shape();
		}
		void fuse_v1(luadraw* tofuse1, luadraw* tofuse2) {
			if (!tofuse1 || !tofuse2) {
				throw std::runtime_error(lua_error_with_line(L, "Invalid luadraw pointer in fuse"));
			}
			tofuse1->update_placement();
			tofuse2->update_placement();
			Handle(AIS_Shape) af1 = tofuse1->ashape;
			Handle(AIS_Shape) af2 = tofuse2->ashape;
			TopoDS_Shape ts1 = tofuse1->shape;
			TopoDS_Shape ts2 = tofuse2->shape;

			BRepAlgoAPI_Fuse fuse(ts1, ts2);
			fuse.Build();
			if (!fuse.IsDone()) {
				throw std::runtime_error(lua_error_with_line(L, "Fuse operation failed"));
			}

			tofuse1->visible_hardcoded = 0;
			tofuse2->visible_hardcoded = 0;

			TopoDS_Shape fusedShape = fuse.Shape();

			// Refine the result
			ShapeUpgrade_UnifySameDomain unify(fusedShape, true, true, true);  // merge faces, edges, vertices
			unify.Build();
			this->shape = unify.Shape();
		}
void mergeShape(TopoDS_Compound& target, const TopoDS_Shape& toAdd) {
    shape = toAdd; // last added 
	cotm1(ShapeTypeName(shape));
    if (toAdd.IsNull()) {
        std::cerr << "Warning: toAdd is null." << std::endl;
        return;
    }
    if (toAdd.IsSame(target)) {
        std::cerr << "Warning: attempted to merge compound into itself." << std::endl;
        return;
    }

    if (target.IsNull()) {
        builder.MakeCompound(target);
    }

    builder.Add(target, toAdd);

    // Track transform
    gp_Ax2 ax3(origin, normal, xdir);
    trsf.SetTransformation(ax3);
    trsf.Invert();
    vtrsf.push_back(trsf);

    // Debug: count shapes
    int count = 0;
    for (TopExp_Explorer exp(target, TopAbs_SHAPE); exp.More(); exp.Next()) {
        ++count;
    }
	 count = 0; 
// Use TopAbs_SOLID to count the high-level components added to the compound
for (TopExp_Explorer exp(target, TopAbs_SOLID); exp.More(); exp.Next()) {
    ++count;
}
    cotm1(count);

    // shape = toAdd; // last added
}
const char* ShapeTypeName(const TopoDS_Shape& s) {
    switch (s.ShapeType()) {
        case TopAbs_COMPOUND: return "COMPOUND";
        case TopAbs_COMPSOLID: return "COMPSOLID";
        case TopAbs_SOLID: return "SOLID";
        case TopAbs_SHELL: return "SHELL";
        case TopAbs_FACE: return "FACE";
        case TopAbs_WIRE: return "WIRE";
        case TopAbs_EDGE: return "EDGE";
        case TopAbs_VERTEX: return "VERTEX";
        case TopAbs_SHAPE: return "SHAPE";
        default: return "UNKNOWN";
    }
}
bool replaceLastWireWithFace(TopoDS_Compound& cshape) {
    // Collect all shapes in order
    std::vector<TopoDS_Shape> ordered;
    for (TopExp_Explorer ex(cshape, TopAbs_SHAPE); ex.More(); ex.Next()) {
        ordered.push_back(ex.Current());
    }

    // Find last wire index
    int lastWireIdx = -1;
    for (int i = static_cast<int>(ordered.size()) - 1; i >= 0; --i) {
        if (ordered[i].ShapeType() == TopAbs_WIRE) {
            lastWireIdx = i;
            break;
        }
    }

    if (lastWireIdx == -1) {
        std::cerr << "No wire found in compound." << std::endl;
        return false;
    }

    // Try to build a face from that wire
    TopoDS_Wire wire = TopoDS::Wire(ordered[lastWireIdx]);
    BRepBuilderAPI_MakeFace mkFace(wire);
    if (!mkFace.IsDone()) {
        std::cerr << "Failed to create face from wire." << std::endl;
        return false;
    }

    TopoDS_Face face = mkFace.Face();

    // Replace the wire with the face
    ordered[lastWireIdx] = face;

    // Rebuild compound in the same order
    TopoDS_Compound result;
    TopoDS_Builder builder;
    builder.MakeCompound(result);
    for (const auto& s : ordered) {
        builder.Add(result, s);
    }

    cshape = result; // overwrite with rebuilt compound
    return true;
}
void extrude(float qtd = 0) {
    cotm1("Extrude", name);

	cotm2("onex",ShapeTypeName(shape));
    // bool replaced = replaceLastWireWithFace(cshape);
    // std::cout << "replaceLastWireWithFace: " << replaced << std::endl;

    gp_Vec extrusionVec(normal);
    extrusionVec *= qtd;

    // TopoDS_Shape face = GetLastType(cshape, TopAbs_FACE);
    if (shape.IsNull()) {
        std::cerr << "No face found to extrude." << std::endl;
        return;
    }

    // TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(shape, extrusionVec).Shape();
	TopoDS_Shape extrudedShape;

try {

	cotm2("extr",ShapeTypeName(shape));
    BRepPrimAPI_MakePrism mkPrism(shape, extrusionVec);

    if (!mkPrism.IsDone()) {
        throw std::runtime_error("Extrusion failed: prism builder not done.");
    }
    extrudedShape = mkPrism.Shape();
}
catch (Standard_Failure& e) {
    std::cerr << "OpenCascade error: " << e.GetMessageString() << std::endl;
    return; // or handle gracefully
}
catch (...) {
    std::cerr << "Unknown error during extrusion." << std::endl;
    return;
}
    mergeShape(cshape, extrudedShape);
	cotm2("extr2")
}

		// void mergeShape(TopoDS_Shape& target, const TopoDS_Shape &toAdd) {
		void mergeShape_v1(TopoDS_Compound& target, const TopoDS_Shape& toAdd) {
			if (toAdd.IsNull()) {
				std::cerr << "Warning: toAdd is null." << std::endl;
				return;
			}
			builder.Add(target, toAdd);
			gp_Ax2 ax3(origin, normal, xdir);
			trsf.SetTransformation(ax3);
			trsf.Invert();
			vtrsf.push_back(trsf);
			    int count = 0;
    for (TopExp_Explorer exp(target, TopAbs_SHAPE); exp.More(); exp.Next()) {
        ++count;
    }
	cotm(count);
			shape = toAdd;
			return;
			// try
			// {
			//     if (toAdd.IsNull())
			//     {
			//         std::cerr << "Warning: toAdd is null." << std::endl;
			//         return;
			//     }

			//     BRepCheck_Analyzer checker(toAdd);
			//     if (!checker.IsValid())
			//     {
			//         std::cerr << "Warning: toAdd is invalid." << std::endl;
			//         return;
			//     }

			//     BRep_Builder builder;
			//     TopoDS_Compound comp;

			//     if (target.IsNull())
			//     {
			//         target = BRepBuilderAPI_Copy(toAdd).Shape();
			//         return;
			//     }

			//     BRepCheck_Analyzer targetChecker(target);
			//     if (!targetChecker.IsValid())
			//     {
			//         std::cerr << "Warning: target is invalid." << std::endl;
			//         return;
			//     }

			//     if (target.ShapeType() != TopAbs_COMPOUND)
			//     {
			//         builder.MakeCompound(comp);
			//         builder.Add(comp, target);
			//         target = comp;
			//     }
			//     else
			//     {
			//         comp = TopoDS::Compound(target);
			//     }

			//     // Copy toAdd to ensure it's standalone
			//     TopoDS_Shape toAddCopy = BRepBuilderAPI_Copy(toAdd).Shape();
			//     builder.Add(comp, toAddCopy);
			//     target = comp;
			// }
			// catch (Standard_Failure& e)
			// {
			//     std::cerr << "Open CASCADE error: " << e.GetMessageString()
			//     << std::endl; throw;
			// }
		}

		void ConvertVec2dToPnt2d(const std::vector<gp_Vec2d>& vpoints, std::vector<gp_Pnt2d>& ppoints) {
			ppoints.clear();
			ppoints.reserve(vpoints.size());
			for (const auto& vec : vpoints) {
				ppoints.emplace_back(vec.X(), vec.Y());
			}
		}

		void ConvertPnt2dToPnt(const std::vector<gp_Pnt2d>& pnts2d, std::vector<gp_Pnt>& pnts3d) {
			pnts3d.clear();
			pnts3d.reserve(pnts2d.size());
			for (const auto& p : pnts2d) {
				pnts3d.emplace_back(p.X(), p.Y(), 0.0);
			}
		}

		// 2D cross‐product (scalar)
		static double VCross(const gp_Vec2d& a, const gp_Vec2d& b) { return a.X() * b.Y() - a.Y() * b.X(); }

		// Intersect two infinite 2d lines: P1 + t1*d1  and  P2 + t2*d2
		// Returns false if they’re (nearly) parallel.
		static bool IntersectLines(const gp_Pnt2d& P1, const gp_Vec2d& d1, const gp_Pnt2d& P2, const gp_Vec2d& d2,
								   gp_Pnt2d& Pint) {
			double denom = VCross(d1, d2);
			if (std::abs(denom) < 1e-9) return false;

			gp_Vec2d diff(P2.X() - P1.X(), P2.Y() - P1.Y());
			double t1 = VCross(diff, d2) / denom;
			Pint = P1.Translated(d1 * t1);
			return true;
		}

		static constexpr double EPS = 1e-12;

		// 2D cross-product helper
		static double cross2d(const gp_Vec2d& u, const gp_Vec2d& v) { return u.X() * v.Y() - u.Y() * v.X(); }

		// Intersect two infinite lines P1+t*d1 and P2+s*d2
		static bool intersectLines(const gp_Pnt2d& P1, const gp_Vec2d& d1, const gp_Pnt2d& P2, const gp_Vec2d& d2,
								   gp_Pnt2d& out) {
			double denom = cross2d(d1, d2);
			if (std::abs(denom) < EPS) return false;
			gp_Vec2d w(P2.X() - P1.X(), P2.Y() - P1.Y());
			double t = cross2d(w, d2) / denom;
			out = P1.Translated(d1 * t);
			return true;
		}

		// Compare two 2D points
		static bool equal2d(const gp_Pnt2d& A, const gp_Pnt2d& B) {
			return (std::abs(A.X() - B.X()) < EPS) && (std::abs(A.Y() - B.Y()) < EPS);
		}

		// Convert 2D->3D
		static gp_Pnt to3d(const gp_Pnt2d& P) { return gp_Pnt(P.X(), P.Y(), 0.0); }

		// Main: one function to build a ring‐shaped face offset by dist
		// pts: must form a closed loop (first==last) or will close
		// automatically dist: positive→expand outward; negative→shrink inward
		TopoDS_Face MakeOffsetRingFace(const std::vector<gp_Pnt2d>& pts, double dist) {
			// need at least 3 distinct points
			if (pts.size() < 3) return TopoDS_Face();

			// 1) build original closed wire
			BRepBuilderAPI_MakePolygon poly;
			for (auto& p : pts) poly.Add(to3d(p));
			if (!equal2d(pts.front(), pts.back())) poly.Close();
			TopoDS_Wire origWire = poly.Wire();

			// 2) extract 2D points (drop duplicate last if present)
			std::vector<gp_Pnt2d> v = pts;
			if (equal2d(v.front(), v.back())) v.pop_back();
			const size_t N = v.size();
			if (N < 3) return TopoDS_Face();

			// 3) compute unit‐edge directions and left‐hand normals
			std::vector<gp_Vec2d> dirs(N), norms(N);
			for (size_t i = 0; i < N; ++i) {
				auto& A = v[i];
				auto& B = v[(i + 1) % N];
				gp_Vec2d d(B.X() - A.X(), B.Y() - A.Y());
				double len = d.Magnitude();
				if (len < EPS) return TopoDS_Face();  // degenerate
				d /= len;
				dirs[i] = d;
				norms[i] = gp_Vec2d(-d.Y(), d.X());	 // left‐hand normal
			}

			// 4) offset each vertex by signed dist along its two adjacent
			// normals
			//    (positive dist → inward for CCW; negative → outward)
			std::vector<gp_Pnt2d> off(N);
			for (size_t i = 0; i < N; ++i) {
				size_t ip = (i + N - 1) % N;  // prev edge
				size_t in = i;				  // next edge

				gp_Pnt2d Pp = v[i].Translated(norms[ip] * dist);
				gp_Pnt2d Pn = v[i].Translated(norms[in] * dist);
				gp_Pnt2d X;
				if (intersectLines(Pp, dirs[ip], Pn, dirs[in], X))
					off[i] = X;
				else
					off[i] = Pn;  // fallback on parallel
			}

			// 5) build the offset wire
			BRepBuilderAPI_MakeWire mkOff;
			for (size_t i = 0; i < N; ++i) {
				mkOff.Add(BRepBuilderAPI_MakeEdge(to3d(off[i]), to3d(off[(i + 1) % N])));
			}
			TopoDS_Wire offsetWire = mkOff.Wire();

			// 6) assign outer vs. hole based on sign of dist
			TopoDS_Wire outerLoop, holeLoop;
			if (dist < 0) {
				// negative dist → outward offset is the outer boundary
				outerLoop = offsetWire;
				holeLoop = TopoDS::Wire(origWire.Reversed());
			} else {
				// positive dist → inward offset is the hole
				outerLoop = origWire;
				holeLoop = TopoDS::Wire(offsetWire.Reversed());
			}

			// 7) build and return the ring‐shaped face
			BRepBuilderAPI_MakeFace faceMaker(outerLoop);
			faceMaker.Add(holeLoop);
			return faceMaker.Face();
		}

		// Builds a closed, one‐sided offset wire around the input polyline.
		// vpoints: the “spine” as 2D points.
		// dist:    offset distance.
		// outward: true→offset on the left side of each segment..(negative
		// number does the same, so no need)
		TopoDS_Wire MakeOneSidedOffsetWire(const std::vector<gp_Pnt2d>& vpoints, double dist) {
			// bool closed = ((vpoints[0].X() == vpoints.back().X()) &&
			// 			   (vpoints[0].Y() == vpoints.back().Y()));
			// cotm(closed);
			// bool outward = dist>0?-1:1;
			bool outward = 1;
			const size_t N = vpoints.size();
			if (N < 2) return TopoDS_Wire();

			// 1) Compute segment tangents and outward normals
			std::vector<gp_Vec2d> dirs(N - 1), norms(N - 1);
			for (size_t i = 0; i < N - 1; ++i) {
				gp_Vec2d d(vpoints[i + 1].X() - vpoints[i].X(), vpoints[i + 1].Y() - vpoints[i].Y());
				d.Normalize();
				dirs[i] = d;
				// left‐normal = ( -dy, dx ); right‐normal = ( dy, -dx )
				gp_Vec2d n(outward ? -d.Y() : d.Y(), outward ? d.X() : -d.X());
				norms[i] = n;
			}

			// 2) Compute offset points at each vertex, trimmed with
			// perpendicular caps
			std::vector<gp_Pnt2d> offp(N);

			// 2a) start cap: intersect offset‐line with a perpendicular at the
			// start
			{
				gp_Pnt2d Poff = vpoints[0].Translated(norms[0] * dist);
				IntersectLines(Poff, dirs[0], vpoints[0], norms[0], offp[0]);
			}

			// 2b) internal joints: intersect successive offset‐lines
			for (size_t i = 1; i < N - 1; ++i) {
				gp_Pnt2d P1 = vpoints[i].Translated(norms[i - 1] * dist);
				gp_Pnt2d P2 = vpoints[i].Translated(norms[i] * dist);

				if (!IntersectLines(P1, dirs[i - 1], P2, dirs[i], offp[i])) {
					// fallback if parallel: just take the later
					offp[i] = P2;
				}
			}

			// 2c) end cap: intersect offset‐line with perpendicular at the end
			{
				gp_Pnt2d PendOff = vpoints[N - 1].Translated(norms[N - 2] * dist);
				IntersectLines(PendOff, dirs[N - 2], vpoints[N - 1], norms[N - 2], offp[N - 1]);
			}

			// 3) Build the planar wire: original spine + end‐cap + offset side
			// + start‐cap
			BRepBuilderAPI_MakeWire wireMaker;

			// 3a) original spine
			for (size_t i = 0; i < N - 1; ++i) {
				gp_Pnt A(vpoints[i].X(), vpoints[i].Y(), 0.0);
				gp_Pnt B(vpoints[i + 1].X(), vpoints[i + 1].Y(), 0.0);
				wireMaker.Add(BRepBuilderAPI_MakeEdge(A, B));
			}

			// 3b) cap at far end
			{
				gp_Pnt A(vpoints[N - 1].X(), vpoints[N - 1].Y(), 0.0);
				gp_Pnt B(offp[N - 1].X(), offp[N - 1].Y(), 0.0);
				wireMaker.Add(BRepBuilderAPI_MakeEdge(A, B));
			}

			// 3c) offset side (reverse direction)
			for (size_t i = N - 1; i > 0; --i) {
				gp_Pnt A(offp[i].X(), offp[i].Y(), 0.0);
				gp_Pnt B(offp[i - 1].X(), offp[i - 1].Y(), 0.0);
				wireMaker.Add(BRepBuilderAPI_MakeEdge(A, B));
			}

			// 3d) cap at start
			{
				gp_Pnt A(offp[0].X(), offp[0].Y(), 0.0);
				gp_Pnt B(vpoints[0].X(), vpoints[0].Y(), 0.0);
				wireMaker.Add(BRepBuilderAPI_MakeEdge(A, B));
			}

			return wireMaker.Wire();
		}

		// Helper: compute perpendicular vector
		gp_Vec2d Perpendicular(const gp_Vec2d& v) { return gp_Vec2d(-v.Y(), v.X()); }

		void createOffset(double distance) {
			vector<gp_Pnt2d> ppoints;
			ConvertVec2dToPnt2d(vpoints.back(), ppoints);

			bool closed = ((vpoints.back()[0].X() == vpoints.back().back().X()) && (vpoints.back()[0].Y() == vpoints.back().back().Y()));
			TopoDS_Face f;
			if (closed) {
				f = TopoDS::Face(MakeOffsetRingFace(ppoints, -distance));  // well righ
			} else {
				TopoDS_Wire wOff = TopoDS::Wire(MakeOneSidedOffsetWire(ppoints, distance));	 // well profile
				f = BRepBuilderAPI_MakeFace(wOff);
			}
			BRepMesh_IncrementalMesh mesher(f, 0.5, true, 0.5, true);  // adjust deflection/angle
			mergeShape(cshape, f);
		}

		vector<std::vector<gp_Vec2d>> vpoints;

		void CreateWire(const std::vector<gp_Vec2d>& points, bool closed = false) {
			vpoints.push_back(points);
			// vpoints = points;
			BRepBuilderAPI_MakePolygon poly;
			for (auto& v : points) {
				poly.Add(gp_Pnt(v.X(), v.Y(), 0));
			}
			if (closed && points.size() > 2) poly.Close();
			// printf("s1\n");

				TopoDS_Wire wire = poly.Wire();
			if (points.size() > 2) {
				// cotm("s2")
					// Make a planar face from that wire
					TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);
				// cotm("s3")
					// Mesh the face so it gets Poly_Triangulation
					// if(points.size()>2)
					BRepMesh_IncrementalMesh mesher(face, 0.5, true, 0.5, true);
				// printf("s4\n");
				mergeShape(cshape, face);
				shape=face;
	cotm1("bextr",ShapeTypeName(shape));
				// cotm("s5")
			} else {
				mergeShape(cshape, wire);
			}
			
		}

		#include <gp_Vec2d.hxx>
#include <vector>
#include <cmath>

// Gera 6 pontos que formam um hexágono regular.
// flatDistance = distância entre as faces paralelas horizontais.
// O canto inferior-esquerdo do bounding-box fica exatamente em (0,0).
// Retorna os 6 pontos em sentido CCW (começando pelo canto inferior-esquerdo).
std::vector<gp_Vec2d> MakeHexagonPoints(double flatDistance)
{
    if (flatDistance <= 0)
        flatDistance = 1.0;

    const double sqrt3 = std::sqrt(3.0);

    // R = distância do centro aos vértices
    double R = flatDistance / sqrt3;

    // metades pré-calculadas
    double hx   = R;                 // R
    double hx2  = R * 0.5;           // R/2
    double hy2  = (sqrt3 * 0.5) * R; // sqrt(3)/2 * R

    // pontos centrados no (0,0) (centro do hexágono)
    gp_Vec2d v1(  hx,    0.0 ); // 0°
    gp_Vec2d v2(  hx2,   hy2 ); // 60°
    gp_Vec2d v3( -hx2,   hy2 ); // 120°
    gp_Vec2d v4( -hx,    0.0 ); // 180°
    gp_Vec2d v5( -hx2,  -hy2 ); // 240°
    gp_Vec2d v6(  hx2,  -hy2 ); // 300°

    // bounding-box centrado:
    // minX = -R
    // minY = -hy2
    // para colocar no (0,0):
    double tx = R;
    double ty = hy2;

    // ordenar CCW começando no canto inferior-esquerdo
    std::vector<gp_Vec2d> pts = {
        gp_Vec2d(v5.X() + tx, v5.Y() + ty), // inferior-esquerdo
        gp_Vec2d(v6.X() + tx, v6.Y() + ty), // inferior-direito
        gp_Vec2d(v1.X() + tx, v1.Y() + ty),
        gp_Vec2d(v2.X() + tx, v2.Y() + ty),
        gp_Vec2d(v3.X() + tx, v3.Y() + ty),
        gp_Vec2d(v4.X() + tx, v4.Y() + ty)
    };

    return pts;
}

		// GeomAbs_Intersection

		// Creates a simple wire from points
		TopoDS_Wire CreateWireFromPoints(const std::vector<gp_Pnt>& points, bool closed) {
			BRepBuilderAPI_MakePolygon polyBuilder;
			for (const auto& pnt : points) {
				polyBuilder.Add(pnt);
			}
			if (closed) polyBuilder.Close();
			return polyBuilder.Wire();
		}

		void dofromstart(int x = 0) {
			gp_Pnt p1(0, 0, 0);
			gp_Pnt p2(100 + x, 0, 0);
			gp_Pnt p3(100 + x, 50, 0);
			gp_Pnt p4(0, 50, 0);

			// Criar arestas
			TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
			TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
			TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
			TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p1);

			// Fazer o wire
			BRepBuilderAPI_MakeWire wireBuilder;
			wireBuilder.Add(e1);
			wireBuilder.Add(e2);
			wireBuilder.Add(e3);
			wireBuilder.Add(e4);
			TopoDS_Wire wire = wireBuilder.Wire();

			TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

			shape = face;
			// final
			//  BRepBuilderAPI_Transform transformer(face, trsf);
			//  shape= transformer.Shape();
		}

		TopoDS_Shape GetLastFromCompound(const TopoDS_Compound& comp) {
			TopoDS_Shape last;
			for (TopoDS_Iterator it(comp); it.More(); it.Next()) {
				last = it.Value();	// will end up holding the final element
			}
			return last;
		}

void connect(OCC_Viewer::luadraw* target, int faceIndex,
                   const gp_Dir& nSerialized, const gp_Pnt& c, double /*area*/)
{
    TopTools_IndexedMapOfShape M; TopExp::MapShapes(target->shape, TopAbs_FACE, M);
    if (faceIndex < 1 || faceIndex > M.Extent()) Standard_Failure::Raise("Face index out of range");
    TopoDS_Face F = TopoDS::Face(M.FindKey(faceIndex));

    Handle(Geom_Plane) pl = Handle(Geom_Plane)::DownCast(BRep_Tool::Surface(F));
    if (pl.IsNull()) Standard_Failure::Raise("Target face is not planar");

    // Face frame and robust normal aligned with face orientation
    gp_Ax3 faceCS = pl->Position();
    gp_Dir nFace = faceCS.Direction();
    if (F.Orientation() == TopAbs_REVERSED) nFace.Reverse();
    if (nFace.Dot(nSerialized) < 0) nFace.Reverse(); // keep same side as stored

    // Map from world XY -> face frame at centroid
    gp_Ax3 srcCS(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0)); // world XY
    gp_Ax3 dstCS(c, nFace, faceCS.XDirection());               // face at centroid

    gp_Trsf xf; xf.SetTransformation(srcCS, dstCS);

    BRepBuilderAPI_Transform tr(shape, xf, true);
    shape = tr.Shape(); 
}




	};

	luadraw* getluadraw_from_ashape(const Handle(AIS_Shape) & ashape) {
		Handle(Standard_Transient) owner = ashape->GetOwner();
		if (!owner.IsNull()) {
			Handle(ManagedPtrWrapper<luadraw>) wrapper = Handle(ManagedPtrWrapper<luadraw>)::DownCast(owner);

			if (!wrapper.IsNull() && wrapper->ptr) {
				return wrapper->ptr;
			}
		}
		return nullptr;
	}

#pragma endregion luastruct
 
#pragma region events

	// Safe clear of AIS objects with compound-first order and selection-safe
	// guards
	// - Clears selection & closes local contexts first
	// - Deactivates each object from selection manager before Remove()
	// - Removes compounds first, then the rest
	// - Dedupes handles (no double Remove())
	// - If Remove() still throws, falls back to Erase() to avoid tolerance-map
	// crashes

	// Hash + equality for OCC Handle types (hash by underlying pointer)
	struct HandleAISHasher {
		std::size_t operator()(const Handle(AIS_InteractiveObject) & h) const noexcept {
			return std::hash<const void*>()(h.get());
		}
	};
	struct HandleAISEqual {
		bool operator()(const Handle(AIS_InteractiveObject) & a,
						const Handle(AIS_InteractiveObject) & b) const noexcept {
			return a.get() == b.get();
		}
	};

	void SafeClearShapes() {
		// ---- 0) Selection hygiene: clear selection & close local contexts
		// Avoids SelectMgr_ToleranceMap underflows when removing many objects
		m_context->ClearSelected(false);
		m_context->ClearCurrents(false);
		// if (m_context->HasOpenedLocalContext())
		//     m_context->CloseLocalContext();

		// ---- 1) Collect unique handles (skip nulls)
		std::unordered_set<Handle(AIS_InteractiveObject), HandleAISHasher, HandleAISEqual> unique;
		unique.reserve(vaShape.size());
		for (const auto& h : vaShape)
			if (!h.IsNull()) unique.insert(h);

		auto safeRemove = [&](const Handle(AIS_InteractiveObject) & obj, const char* tag) {
			if (obj.IsNull()) return;

			// Deactivate all selection modes for this object (mode=0 means all)
			// This detaches it from the selection manager before Remove().
			try {
				m_context->Deactivate(obj, 0);
			} catch (const Standard_Failure& e) {
				std::cerr << "Deactivate failed (" << tag << "): " << e.GetMessageString() << "\n";
			}

			// If it’s displayed, try Remove(); on failure, fall back to Erase()
			if (m_context->IsDisplayed(obj)) {
				try {
					m_context->Remove(obj,
									  false);  // don’t update per object; update once at end
				} catch (const Standard_Failure& e) {
					std::cerr << "Remove failed (" << tag << "): " << e.GetMessageString()
							  << " -> falling back to Erase()\n";
					try {
						m_context->Erase(obj, Standard_False);
					} catch (const Standard_Failure& e2) {
						std::cerr << "Erase also failed (" << tag << "): " << e2.GetMessageString() << "\n";
					}
				}
			} else {
				// Not displayed: still deactivate & try Erase to be thorough
				try {
					m_context->Erase(obj, Standard_False);
				} catch (const Standard_Failure& e) {
					std::cerr << "Erase (not displayed) failed (" << tag << "): " << e.GetMessageString() << "\n";
				}
			}
		};

		// ---- 2) Remove compounds first
		for (auto it = unique.begin(); it != unique.end();) {
			Handle(AIS_Shape) ais = Handle(AIS_Shape)::DownCast(*it);
			if (!ais.IsNull()) {
				const TopoDS_Shape& s = ais->Shape();
				if (!s.IsNull() && s.ShapeType() == TopAbs_COMPOUND) {
					safeRemove(*it, "compound");
					it = unique.erase(it);	// ensure we don’t process twice
					continue;
				}
			}
			++it;
		}

		// ---- 3) Remove everything else
		for (const auto& h : unique) {
			safeRemove(h, "regular");
		}

		// ---- 4) Drop Lua-side TopoDS handles (no Free(); just Nullify)
		for (auto& luaObj : vlua) {
			if (luaObj) luaObj->shape.Nullify();
			delete luaObj;
		}

		// ---- 5) Clear containers & update once
		vaShape.clear();
		vlua.clear();
		m_context->UpdateCurrentViewer();
	}

	Handle(AIS_Shape) myHighlightedPointAIS;  // To store the highlighting sphere
	TopoDS_Vertex myLastHighlightedVertex;	  // To store the last highlighted vertex
	void clearHighlight() {
		if (!myHighlightedPointAIS.IsNull()) {
			m_context->Remove(myHighlightedPointAIS, Standard_True);
			myHighlightedPointAIS.Nullify();
		}
		myLastHighlightedVertex.Nullify();
	}
	///@return vector 0=windowToWorldX 1=windowToWorldY 2=worldToWindowX
	/// 3=worldToWindowY 4=viewportHeight 5=viewportWidth
	vfloat GetViewportAspectRatio() {
		const Handle(Graphic3d_Camera) & camera = m_view->Camera();

		// Obtém a altura e largura do mundo visível na viewport
		float viewportHeight = camera->ViewDimensions().Y();	  // world
		float viewportWidth = camera->Aspect() * viewportHeight;  // Largura = ratio * altura

		Standard_Integer winWidth, winHeight;
		m_view->Window()->Size(winWidth, winHeight);

		//     // Mundo -> Window (quantos pixels por unidade de mundo)
		float worldToWindowX = winWidth / viewportWidth;
		float worldToWindowY = winHeight / viewportHeight;

		// cotm(worldToWindowX,worldToWindowY)

		// Window -> Mundo (quantas unidades de mundo por pixel)
		float windowToWorldX = viewportWidth / winWidth;
		float windowToWorldY = viewportHeight / winHeight;
		// cotm(camera->Aspect(),viewportWidth ,viewportHeight);
		return {windowToWorldX, windowToWorldY, worldToWindowX, worldToWindowY, viewportHeight, viewportWidth};
	}

	#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Image_PixMap.hxx>

bool IsShapeVisible(const Handle(AIS_Shape)& aisShape,
                           const Handle(V3d_View)& view,
                           const Handle(AIS_InteractiveContext)& context)
{
    if (aisShape.IsNull() || aisShape->Shape().IsNull())
        return false;

    // 1. Cena completa
    view->Redraw();
    Image_PixMap depthAll;
    if (!view->ToPixMap(depthAll, 256, 256, Graphic3d_BT_Depth, true, V3d_SDO_MONO))
        return false;

    // Guardar lista de objetos
    NCollection_Sequence<Handle(AIS_InteractiveObject)> objs;
    for (context->InitCurrent(); context->MoreCurrent(); context->NextCurrent())
    {
        Handle(AIS_InteractiveObject) o = context->Current();
        if (!o.IsNull() && context->IsDisplayed(o))
            objs.Append(o);
    }

    // 2. Cena sem o shape
    context->Erase(aisShape, false);
    view->Redraw();
    Image_PixMap depthWithout;
    if (!view->ToPixMap(depthWithout, 256, 256, Graphic3d_BT_Depth, true, V3d_SDO_MONO))
        return false;

    // 3. Só o shape
    for (auto it = objs.cbegin(); it != objs.cend(); ++it)
        if (*it != aisShape)
            context->Erase(*it, false);
    context->Display(aisShape, false);
    view->Redraw();
    Image_PixMap depthShape;
    if (!view->ToPixMap(depthShape, 256, 256, Graphic3d_BT_Depth, true, V3d_SDO_MONO))
        return false;

    // 4. Comparação pixel a pixel
    bool visible = false;
    for (Standard_Size y = 0; y < depthShape.SizeY() && !visible; ++y)
    {
        const float* rowShape   = (const float*)depthShape.Row(y);
        const float* rowAll     = (const float*)depthAll.Row(y);
        const float* rowWithout = (const float*)depthWithout.Row(y);

        for (Standard_Size x = 0; x < depthShape.SizeX(); ++x)
        {
            float zS = rowShape[x];
            if (zS < 1.0f) // pixel do shape
            {
                float zA = rowAll[x];
                float zW = rowWithout[x];

                if (fabs(zS - zA) < 1e-5f && zS <= zW) // está à frente
                {
                    visible = true;
                    break;
                }
            }
        }
    }

    // Restaurar cena
    for (auto it = objs.cbegin(); it != objs.cend(); ++it)
        context->Display(*it, false);
    view->Redraw();

    return visible;
}

bool IsShapeVisible_v1(const Handle(AIS_Shape)& aisShape, const Handle(V3d_View)& view, const Handle(AIS_InteractiveContext)& context)
{
    if (aisShape.IsNull() || view.IsNull() || context.IsNull())
        return false;

    AIS_ListOfInteractive visibleObjects;
    context->ObjectsForView(visibleObjects, view, Standard_True);

    AIS_ListIteratorOfListOfInteractive it(visibleObjects);
    for (; it.More(); it.Next())
    {
        if (it.Value() == aisShape)
            return true;
    }
    return false;
}
 

bool IsVertexVisible(
    const gp_Pnt& vertex,
    const Handle(V3d_View)& view,
    const Handle(AIS_InteractiveContext)& context)
{
    if (view.IsNull())
        return false;

    const Handle(Graphic3d_Camera)& camera = view->Camera();
    if (camera.IsNull())
        return false;

    // 1. Project the vertex to screen space
    Standard_Real Xv, Yv, Zv;
    view->Project(vertex.X(), vertex.Y(), vertex.Z(), Xv, Yv, Zv);

    // 2. Get viewport dimensions
    Standard_Integer w, h;
    view->Window()->Size(w, h);

    // 3. Convert to integer pixel coordinates
    Standard_Integer px = static_cast<Standard_Integer>(Xv + 0.5);
    Standard_Integer py = static_cast<Standard_Integer>(Yv + 0.5);

    // 4. Check if the projected vertex is inside the viewport
    if (px < 0 || py < 0 || px >= w || py >= h)
    {
        return false;
    }

    // 5. Grab the depth buffer
    Image_PixMap zMap;
    if (!view->ToPixMap(zMap, Graphic3d_BT_Depth, Image_Format_GrayF))
    {
        return false;
    }

    // 6. Convert depth to normalized [0, 1] range
    float shapeDepth = 0.0f;
    if (camera->IsOrthographic())
    {
        // For orthographic: Zv is in NDC [-1, 1], convert to [0, 1]
        shapeDepth = static_cast<float>((Zv + 1.0) * 0.5);
    }
    else
    {
        // For perspective: linearize depth
        float zNear = static_cast<float>(camera->ZNear());
        float zFar = static_cast<float>(camera->ZFar());
        shapeDepth = static_cast<float>((Zv - zNear) / (zFar - zNear));
    }
    shapeDepth = std::clamp(shapeDepth, 0.0f, 1.0f);

    // 7. Get depth from depth buffer (flip Y coordinate for OpenGL)
    Standard_Integer bufferY = h - 1 - py;
    const float* depthRow = reinterpret_cast<const float*>(zMap.Row(bufferY));
    float depthAtPixel = depthRow[px];

    // 8. INVERTED DEPTH COMPARISON
    // In OpenGL: lower depth values = closer to camera
    // So if shapeDepth is LESS than depthAtPixel, vertex is closer = visible
    static const float DEPTH_TOLERANCE = 1e-4f;
    
    // TRY THIS INVERTED VERSION:
    bool isVisible = (shapeDepth < depthAtPixel - DEPTH_TOLERANCE);
    
    // If that doesn't work, try the original:
    // bool isVisible = (shapeDepth <= depthAtPixel + DEPTH_TOLERANCE);

    return isVisible;
}

#include <V3d_View.hxx>
#include <Image_PixMap.hxx>
#include <Image_Format.hxx>
#include <V3d_View.hxx>
#include <Image_PixMap.hxx>
#include <Image_Format.hxx>
#include <V3d_View.hxx>
#include <Image_PixMap.hxx>
#include <Image_Format.hxx>
#include <gp_Pnt.hxx>
#include <V3d_View.hxx>
#include <Image_PixMap.hxx>
#include <Image_Format.hxx>
#include <gp_Pnt.hxx>

bool IsWorldPointGreen(const Handle(V3d_View)& view,
                           const gp_Pnt&            point,
                           int                      radius       = 5,
                           unsigned char            minGreen     = 200,
                           unsigned char            maxRedBlue   =  64)
{
    // 1. Get view size
    Standard_Integer w = 0, h = 0;
    view->Window()->Size(w, h);

    // 2. Project world point → view coords → pixel coords
    Standard_Real Xv = 0., Yv = 0., Zv = 0.;
    view->Project(point.X(), point.Y(), point.Z(), Xv, Yv, Zv);

    Standard_Integer px = 0, py = 0;
    view->Convert(Xv, Yv, px, py);

    // 3. Bail out if point is outside view
    if (px < 0 || px >= w || py < 0 || py >= h)
        return false;

	// cotm("w1")
	// make_current();
	// if (view->IsInvalidated())
    // m_view->Redraw();
    // 4. Capture full-view pixmap 
    Image_PixMap pix;
    if (!view->ToPixMap(pix, w, h) || pix.IsEmpty())
        return false;


    // Image_PixMap pix;
    // if (!view->ToPixMap(pix, w, h,Graphic3d_BT_RGB,1,V3d_StereoDumpOptions::V3d_SDO_BLENDED) || pix.IsEmpty())
    //     return false;
// 	Image_AlienPixMap pix;
// if (!view->ToPixMap(pix, w, h, Graphic3d_BT_RGB) || pix.IsEmpty())
//     return false;
// cotm("w2")
    // 5. Determine channel count and stride
    const Image_Format fmt      = pix.Format();
    const size_t      channels = (fmt == Image_Format_RGBA ? 4 : 3);
    const size_t      rowBytes = pix.Width() * channels;
    const Standard_Byte* data   = pix.Data();
// cotm("w3")
    // 6. Scan a circular neighborhood for “greenish” pixels
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx*dx + dy*dy > radius*radius)
                continue;                      // outside circle

            int sx = px + dx;
            int sy = py + dy;
            if (sx < 0 || sx >= w || sy < 0 || sy >= h)
                continue;                      // out of bounds

            // Flip Y: mouse top-left vs pixmap bottom-left
            int yPix = h - 1 - sy;
            size_t idx = size_t(yPix) * rowBytes + size_t(sx) * channels;

            unsigned char r = data[idx + 0];
            unsigned char g = data[idx + 1];
            unsigned char b = data[idx + 2];

            if (g >= minGreen && r <= maxRedBlue && b <= maxRedBlue)
                return true;                   // found greenish pixel
        }
    }
// cotm("w7")
    return false;  // no green near the projected point
}

// Scans a circular neighborhood around the mouse for “greenish” pixels.
//working but not great
bool IsPixelQuantityGreen(const Handle(V3d_View)& view,
                      int mouseX,
                      int mouseY,
                      int radius       = 4,    // search radius in pixels
                      unsigned char minGreen    = 200,
                      unsigned char maxRedBlue  =  64)
{
    // 1. Get view dimensions
    Standard_Integer w = 0, h = 0;
    view->Window()->Size(w, h);

    // 2. Capture the full-view pixmap
    Image_PixMap pix;
    if (!view->ToPixMap(pix, w, h) || pix.IsEmpty())
        return false;

    // 3. Determine channel count (RGB or RGBA)
    const Image_Format fmt      = pix.Format();
    const size_t      channels = (fmt == Image_Format_RGBA ? 4 : 3);
    const size_t      width    = pix.Width();
    const size_t      rowBytes = width * channels;
    const Standard_Byte* data   = pix.Data();
    if (!data)
        return false;

    // 4. Scan in a circle of given radius
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            // Skip outside circular region
            if (dx*dx + dy*dy > radius*radius)
                continue;

            int x = mouseX + dx;
            int y = mouseY + dy;
            // Skip out-of-bounds
            if (x < 0 || x >= w || y < 0 || y >= h)
                continue;

            // Flip Y to OCCT pixmap origin (bottom-left)
            int yPix = h - 1 - y;
            size_t idx = size_t(yPix) * rowBytes + size_t(x) * channels;

            unsigned char r = data[idx + 0];
            unsigned char g = data[idx + 1];
            unsigned char b = data[idx + 2];

            // If this pixel is “green enough,” return true immediately
            if (g >= minGreen && r <= maxRedBlue && b <= maxRedBlue)
                return true;
        }
    }

    // No green pixel found in the neighborhood
    return false;
}

bool IsPixelQuantityGreen_v1(const Handle(V3d_View)& view,
                                int mouseX,
                                int mouseY,
                                unsigned char minGreen = 200,
                                unsigned char maxRedBlue = 64)
{
    // 1. Get view size
    Standard_Integer w = 0, h = 0;
    view->Window()->Size(w, h);

    // 2. Capture full-view pixmap
    Image_PixMap pix;
    if (!view->ToPixMap(pix, w, h) || pix.IsEmpty())
        return false;

    // 3. Determine channel count (RGB or RGBA)
    Image_Format fmt     = pix.Format();
    int          chan    = (fmt == Image_Format_RGBA ? 4 : 3);

    // 4. Flip Y to go from top-left mouse to bottom-left pixmap origin
    int yPix = h - 1 - mouseY;

    // 5. Compute byte offset into raw buffer
    const Standard_Byte* data     = pix.Data();
    size_t               rowBytes = size_t(pix.Width()) * chan;
    size_t               idx      = size_t(yPix) * rowBytes
                                      + size_t(mouseX) * chan;

    unsigned char r = data[idx + 0];
    unsigned char g = data[idx + 1];
    unsigned char b = data[idx + 2];

    // 6. Tolerance test: strong green, low red and blue
    return (g >= minGreen
         && r <= maxRedBlue
         && b <= maxRedBlue);
}


	void highlightVertex(const TopoDS_Vertex& aVertex,luadraw* ldd=0) {
		clearHighlight();  // Clear any existing highlight first

		// perf();
		// sleepms(1000);
		float ratio = GetViewportAspectRatio()[0];
		// perf("GetViewportAspectRatio");
		// double ratio=theProjector->Aspect();
		// cotm(ratio);

		

		gp_Pnt vertexPntL = BRep_Tool::Pnt(aVertex);
		gp_Pnt vertexPnt=vertexPntL;
		if(ldd)
			vertexPnt=vertexPntL.Transformed(ldd->Origin);

		//check here
		// if(!IsVertexVisible(vertexPnt,m_view,m_context))return;


		// Create a small red sphere at the vertex location
		Standard_Real sphereRadius = 5 * ratio;	 // Small radius for the highlight ball
		TopoDS_Shape sphereShape = BRepPrimAPI_MakeSphere(vertexPnt, sphereRadius).Shape();
		myHighlightedPointAIS = new AIS_Shape(sphereShape);
		myHighlightedPointAIS->SetColor(Quantity_NOC_GREEN);
		myHighlightedPointAIS->SetDisplayMode(AIS_Shaded);
		// myHighlightedPointAIS->SetTransparency(0.2f);			   // Slightly transparent
		myHighlightedPointAIS->SetZLayer(Graphic3d_ZLayerId_Top);  // Ensure it's drawn on top

		m_context->Display(myHighlightedPointAIS, Standard_True);
		myLastHighlightedVertex = aVertex;
		redraw();
		// Fl::wait();
		if(!IsWorldPointGreen(m_view,vertexPnt))return;
		// if(!IsPixelQuantityGreen(m_view,mousex,mousey))return;


		help.point ="";help.upd();
		// if(!IsVisible(myHighlightedPointAIS,m_view))return;
		// if(!IsShapeVisible(myHighlightedPointAIS,m_view,m_context))return;

		// cotm("Highlighted Vertex:", vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z());
		help.point = "Point: "
    + fmt(vertexPnt.X()) + ","
    + fmt(vertexPnt.Y()) + ","
    + fmt(vertexPnt.Z());
	if(ldd && !ldd->Origin.IsIdentity()){
		help.point += " Pointl: " 
    + fmt(vertexPntL.X()) + ","
    + fmt(vertexPntL.Y()) + ","
    + fmt(vertexPntL.Z());
	}
		help.upd();
		// 	if(!projector.Perspective()){
		// 		gp_Pnt clickedPoint(mousex, mousey, 0);
		// 		// Passo 1: converter mouse para coordenadas projetadas reais
		// (no plano do HLR) Standard_Real x, y, z; m_view->Convert(mousex,
		// mousey, x, y, z);

		// // Como HLR só trabalha em XY, podes descartar z:
		// gp_Pnt screenPoint(x, y, 0);
		// // BRep_Tool::Pnt(vertex);
		// 		gp_Pnt vertexhlrPnt
		// =getAccurateVertexPosition(vertexPnt,projector);
		// 		// gp_Pnt vertexhlrPnt
		// =getAccurateVertexPosition(m_context,mousex,mousey);
		// 		// gp_Pnt vertexhlrPnt =getAccurateVertexPosition(clickedPoint);
		// 		// gp_Pnt vertexhlrPnt =convertHLRVertexToWorld(vertexPnt);
		// 		// gp_Pnt vertexhlrPnt =convertHLRToWorld(vertexPnt);
		// 		cotm("Highlighted hlr Vertex:", vertexhlrPnt.X(),
		// vertexhlrPnt.Y(), vertexhlrPnt.Z());
		// 	}
		//     // printf("Highlighted Vertex: X=%.3f, Y=%.3f, Z=%.3f\n",
		//     vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z());

		// if (!projector.Perspective()) {
		//     // 1. Converte coordenadas de ecrã para mundo 2D projetado
		//     Standard_Real projX, projY, projZ;
		//     m_view->Convert(mousex, mousey, projX, projY, projZ);
		//     gp_Pnt projectedHLRPoint(projX, projY, 0); // plano HLR é Z=0

		//     // 2. Converte o ponto projetado para o espaço 3D real
		//     gp_Pnt worldPoint = convertHLRToWorld(projectedHLRPoint);

		//     cotm("Convertido de HLR para 3D:", worldPoint.X(),
		//     worldPoint.Y(), worldPoint.Z());
		// }
	}

	int mousex = 0;
	int mousey = 0;

	void getvertex() {
		m_view->SetComputedMode(Standard_False);
		clearHighlight();

		// 2. Activate ONLY vertex selection mode for this specific picking
		// operation. This uses the AIS_Shape::SelectionMode utility, which
		// correctly returns 0 for TopAbs_VERTEX.
		m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));

		// 3. Perform the picking operations
		m_context->MoveTo(mousex, mousey, m_view, Standard_False);

		m_context->SelectDetected(AIS_SelectionScheme_Replace);

		Handle(SelectMgr_EntityOwner) foundOwner;

		for (m_context->InitDetected(); m_context->MoreDetected(); m_context->NextDetected()) {
			Handle(SelectMgr_EntityOwner) owner = m_context->DetectedOwner();

			if (owner.IsNull()) continue;

			Handle(AIS_InteractiveObject) obj = Handle(AIS_InteractiveObject)::DownCast(owner->Selectable());

			// Skip the HLR shape
			if (obj == visible_){
				cotm("Skipped");
				continue;
			} 
			// if (obj == hidden_) continue;

			foundOwner = owner;	 // First valid non-HLR owner
			break;
		}

		if (!foundOwner.IsNull()) {
			Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(foundOwner);
			if (!brepOwner.IsNull()) {
				TopoDS_Shape detectedTopoShape = brepOwner->Shape();

				printf(
					"Detected TopoDS_ShapeType: %d (0=Vertex, 1=Edge, 2=Wire, "
					"3=Face, etc.)\n",
					detectedTopoShape.ShapeType());
				printf("Value of TopAbs_VERTEX: %d\n", TopAbs_VERTEX);

				if (detectedTopoShape.ShapeType() == TopAbs_VERTEX) {
					TopoDS_Vertex currentVertex = TopoDS::Vertex(detectedTopoShape);
					if (!myLastHighlightedVertex.IsEqual(currentVertex)) {
						highlightVertex(currentVertex);
					} else {
						gp_Pnt pt = BRep_Tool::Pnt(currentVertex);
						printf(
							"Hovering over same vertex: X=%.3f, Y=%.3f, "
							"Z=%.3f\n",
							pt.X(), pt.Y(), pt.Z());
					}
				} else {
					printf("Detected shape is not a vertex (type: %d)\n", detectedTopoShape.ShapeType());
					clearHighlight();
				}
			} else {
				printf("Owner is not a StdSelect_BRepOwner.\n");
				clearHighlight();
			}
		} else {
			// cotmupset
			// cotm("Nothing detected under the mouse.");
			// cotmup
			clearHighlight();
		}

		m_context->UpdateCurrentViewer();
	}

	// retorna ponteiro luadraw* (ou nullptr)
	luadraw* lua_detected(Handle(SelectMgr_EntityOwner) entOwner) {
		if (!entOwner.IsNull()) {
			// 1) tenta obter o Selectable associado ao entOwner
			if (entOwner->HasSelectable()) {
				Handle(SelectMgr_SelectableObject) selObj = entOwner->Selectable();
				// SelectableObject é a base de AIS_InteractiveObject, faz
				// downcast
				Handle(AIS_InteractiveObject) ao = Handle(AIS_InteractiveObject)::DownCast(selObj);
				if (!ao.IsNull() && ao->HasOwner()) {
					Handle(Standard_Transient) owner = ao->GetOwner();
					Handle(ManagedPtrWrapper<luadraw>) w = Handle(ManagedPtrWrapper<luadraw>)::DownCast(owner);
					if (!w.IsNull()) return w->ptr;	 // devolve o ponteiro armazenado
				}
			}
		}

		// Alternativa: tenta obter directamente o interactive object detectado
		// pelo contexto if (!m_context.IsNull()) {
		//     Handle(AIS_InteractiveObject) detected =
		//     m_context->DetectedInteractive(); if (!detected.IsNull() &&
		//     detected->HasOwner()) {
		//         Handle(Standard_Transient) owner = detected->GetOwner();
		//         Handle(ManagedPtrWrapper<luadraw>) w =
		//         Handle(ManagedPtrWrapper<luadraw>)::DownCast(owner); if
		//         (!w.IsNull()) return w->ptr;
		//     }
		// }
		return nullptr;
	}

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <V3d_View.hxx>

bool IsPointVisible_Shaded(const Handle(AIS_InteractiveContext)& ctx,
                           const Handle(V3d_View)& view,
                           const gp_Pnt& point3d)
{
    // Converter ponto para coordenadas de ecrã
    Standard_Integer x, y;
    view->Convert(point3d.X(), point3d.Y(), point3d.Z(), x, y);

    // Limpar seleção e fazer pick
    ctx->MoveTo(x, y, view, false);
    ctx->Select(false);

    // Verificar se o primeiro objeto selecionado contém o ponto
    if (ctx->NbSelected() > 0) {
        // Aqui podes verificar se é o shape certo
        return true;
    }
    return false;
}

bool IsPointVisibleOnShape(const TopoDS_Shape& shape,
                                   const Handle(V3d_View)& view,
                                   const gp_Pnt& point3d)
{
    if (shape.IsNull() || view.IsNull())
        return false;

    const Handle(Graphic3d_Camera)& cam = view->Camera();
    gp_Pnt camPos = cam->Eye();
    gp_Dir dir(point3d.XYZ() - camPos.XYZ());

    // Criar uma linha infinita da câmara na direção do ponto
    gp_Lin ray(camPos, dir);

    // Interseção linha-shape
    BRepIntCurveSurface_Inter inter;
    inter.Init(shape, ray, Precision::Confusion());

    double distToPoint = camPos.Distance(point3d);
    bool visible = true;

    while (inter.More()) {
        gp_Pnt ip = inter.Pnt();
        double distHit = camPos.Distance(ip);

        // Se a interseção for antes do ponto (com tolerância), está oculto
        if (distHit < distToPoint - Precision::Confusion()) {
            visible = false;
            break;
        }
        inter.Next();
    }

    return visible;
}
bool IsPointVisible_Picking(const Handle(AIS_InteractiveContext)& ctx,
                            const Handle(V3d_View)& view,
                            const gp_Pnt& point3d)
{
    Standard_Integer px, py;
    view->Convert(point3d.X(), point3d.Y(), point3d.Z(), px, py);

    ctx->MoveTo(px, py, view, false);
    ctx->Select(false);

    return (ctx->NbSelected() > 0);
}
// Returns closest subshape (edge/vertex/face) and its owning AIS_Shape at mouse (x,y).
// Returns false if nothing hit.
// static bool PickSubShapeAtCursor(
//     int mousex,
//     int mousey,
//     const Handle(V3d_View)& theView,
//     const std::vector<Handle(AIS_Shape)>& theShapes, // your list of real shapes
//     TopoDS_Shape& outSubShape,
//     Handle(AIS_Shape)& outAisShape)
// {
//     outSubShape.Nullify();
//     outAisShape.Nullify();

//     // 1. Build pick ray from eye through mouse (OCCT 7.9.3)
//     gp_Pnt eyePnt;
//     gp_Vec dirVec;
//     theView->Convert(mousex, mousey, eyePnt, dirVec); // this overload exists in 7.9.3
//     if (dirVec.Magnitude() <= gp::Resolution())
//         return false;

//     gp_Dir rayDir(dirVec);
//     gp_Lin ray(eyePnt, rayDir);

//     // Convert ray to an edge (required by BRepExtrema_DistShapeShape in OCCT 7.9.3)
//     TopoDS_Edge rayEdge = BRepBuilderAPI_MakeEdge(ray);

//     Standard_Real bestDist = RealLast();
//     TopoDS_Shape bestSub;
//     Handle(AIS_Shape) bestAis;

//     // 2. Test against all shapes you consider pickable
//     for (const Handle(AIS_Shape)& ais : theShapes)
//     {
//         if (ais.IsNull())
//             continue;

//         const TopoDS_Shape& S = ais->Shape();
//         if (S.IsNull())
//             continue;

//         BRepExtrema_DistShapeShape dist(rayEdge, S);
//         dist.Perform();
//         if (!dist.IsDone())
//             continue;

//         Standard_Real d = dist.Value();
//         if (d < bestDist && dist.NbSolution() > 0)
//         {
//             bestDist = d;
//             bestSub = dist.SupportOnShape2(1); // closest subshape on S
//             bestAis = ais;
//         }
//     }

//     if (bestSub.IsNull() || bestAis.IsNull())
//         return false;

//     outSubShape = bestSub;
//     outAisShape = bestAis;
//     return true;
// }

void ev_highlight() {
	if(!m_initialized)return;
// m_context->SetViewAffinity((Standard_False);
// Handle(SelectMgr_ViewerSelector) viewerSel = m_context->MainSelector();
// viewerSel->GetManager().AllowOverlapDetection(0);

static Handle(AIS_Shape) highlight;

if (!highlight.IsNull())m_context->Remove(highlight, 1);

    clearHighlight();

    // // Activate only what you need once per hover
    m_context->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
    m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
    if (!hlr_on)
        m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
    else{
        m_context->Deactivate(visible_);
		if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Default );
        m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_FACE));
	}

    // Move cursor in context
    m_context->MoveTo(mousex, mousey, m_view, Standard_False);
    m_context->SelectDetected(AIS_SelectionScheme_Replace);

    Handle(SelectMgr_EntityOwner) anOwner = m_context->DetectedOwner();
    if (anOwner.IsNull()) {
        clearHighlight();
		if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
        redraw();
        return;
    }

// // ------------------------------
// // HLR‑aware picking (drop‑in)
// // ------------------------------

// Handle(SelectMgr_EntityOwner) anOwner;

// // Case 1: HLR OFF → normal AIS selection (unchanged)
// if (!hlr_on)
// {
//     m_context->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
//     m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
//     m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));

//     m_context->MoveTo(mousex, mousey, m_view, Standard_False);
//     m_context->SelectDetected(AIS_SelectionScheme_Replace);

//     anOwner = m_context->DetectedOwner();
// }
// else
// {
//     // Case 2: HLR ON → geometric picking on real shapes

//     m_context->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
//     m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
//     m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_FACE));

//     // Do NOT rely on AIS for the “behind” edge here
//     TopoDS_Shape pickedSub;
//     Handle(AIS_Shape) pickedAis;

//     // You must provide this: all AIS_Shapes that represent the real solids/parts.
//     // Example: std::vector<Handle(AIS_Shape)> m_shapes;
//     if (!PickSubShapeAtCursor(mousex, mousey, m_view, m_shapes, pickedSub, pickedAis))
//     {
//         anOwner.Nullify();
//     }
//     else
//     {
//         // Wrap subshape into an owner, OCCT 7.9.3 constructor
//         Handle(StdSelect_BRepOwner) owner = new StdSelect_BRepOwner(pickedSub, 0);
//         anOwner = owner;

//         // Optionally, you can still use AIS highlight:
//         // m_context->ClearSelected();
//         // m_context->AddOrRemoveSelected(pickedAis, Standard_False);
//     }
// }

// // ------------------------------
// // Unified post‑selection logic
// // ------------------------------
// if (anOwner.IsNull())
// {
//     clearHighlight();
//     redraw();
//     return;
// }

// Continue with your existing code using `anOwner`


	


    luadraw* ldd = lua_detected(anOwner);
    if (ldd) {
        std::string pname =   ldd->name;
        // if (pname != help.pname) //to optimize speed
		{
			help.pname = pname; 
			GProp_GProps systemProps;
			// Density is not directly handled here — mass is proportional to volume.
			// If you want real mass, multiply by material density later.
			BRepGProp::VolumeProperties(ldd->shape, systemProps);

			// Get the mass (volume if density=1)
			Standard_Real mass = systemProps.Mass();
			double massa=mass/1000;
			help.mass=" Mass:"+ fmt(massa,0)+"cm³"+ " Petg50%:~"+fmt(massa*1.27*0.75,0)+"g"+ " Steel:~"+fmt(massa*0.007850,2)+"kg";
			// std::cout << "Mass (assuming density=1): " << mass << std::endl;
			help.upd(); 
		}
    }else{
		if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
		return;
	} 



    Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(anOwner);
    if (brepOwner.IsNull()) {
        clearHighlight();
		if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
        redraw();
        return;
    }


    TopoDS_Shape detected = brepOwner->Shape();
    if (detected.IsNull()) {
        clearHighlight();
		if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
        redraw();
        return;
    }
	if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
					// Skip the HLR shape
// 			if (detected == visible_->Shape()){
// 				cotm("Skipped");
// 				return;
// 			} 
// return;
	// if(detected.ShapeType()==TopAbs_SOLID){
	// 	cotm("TopAbs_SOLID")
	// }


    switch (detected.ShapeType()) {
    case TopAbs_VERTEX: {


if(0){
    Standard_Real f, l;
	try{
	TopoDS_Edge edge = TopoDS::Edge(detected);
        cotm2("✅ This IS a circle fe")
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

    if (!curve.IsNull() &&
        curve->IsKind(STANDARD_TYPE(Geom_Circle)))    {
		Handle(Geom_Circle) circ = Handle(Geom_Circle)::DownCast(curve);

		if (!circ.IsNull()){
			gp_Pnt center = circ->Location();   // ✅ CENTER HERE
			double radius = circ->Radius();
			TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(center);
			highlightVertex(v, ldd);
		}
		return;
    }
}catch(...){}
}






        TopoDS_Vertex v = TopoDS::Vertex(detected);

		gp_Pnt vertexPnt = BRep_Tool::Pnt(v);
		// if(!IsPointVisible_Picking(m_context,m_view,vertexPnt))break;
		// if(!IsPointVisible_Shaded(m_context,m_view,vertexPnt))break;
		// if(!IsPointVisibleOnShape(detected,m_view,vertexPnt))break;

        if (!myLastHighlightedVertex.IsEqual(v))
            highlightVertex(v,ldd);
        break;
    }
// break;
    case TopAbs_EDGE: {

		

// 	static	Handle(AIS_Shape) highlight = new AIS_Shape(edge);
// 	if(m_context->IsDisplayed(highlight))m_context->Erase(highlight,1);
// 	highlight->Set(edge);
// // wireframe mode
// highlight->SetDisplayMode(AIS_WireFrame);

// // line style (color + width)
// Handle(Prs3d_LineAspect) la =
//     new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 3.0);
// highlight->Attributes()->SetLineAspect(la);
// // customDrawer->SetLineAspect(highlightAsp);
// highlight->Attributes()->SetWireAspect(la);
// highlight->Attributes()->SetFreeBoundaryAspect(la);
// highlight->Attributes()->SetUnFreeBoundaryAspect(la);
// highlight->Attributes()->SetFaceBoundaryAspect(la);
// highlight->Attributes()->SetHiddenLineAspect(la);
// highlight->Attributes()->SetVectorAspect(la);
// highlight->Attributes()->SetSectionAspect(la);
// highlight->Attributes()->SetSeenLineAspect(la);

// // display
// highlight->SetZLayer(Graphic3d_ZLayerId_Topmost);
// if(m_context->IsDisplayed(highlight))m_context->Redisplay(highlight, 1);else m_context->Display(highlight, 1);

// // force topmost overlay
// // m_context->SetDisplayPriority(highlight, 10);

// // m_context->Redisplay(highlight, 1);


TopoDS_Edge edge = TopoDS::Edge(detected);

// if(0)
{
    Standard_Real f, l;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

    if (!curve.IsNull() &&
        curve->IsKind(STANDARD_TYPE(Geom_Circle)))    {
		Handle(Geom_Circle) circ = Handle(Geom_Circle)::DownCast(curve);

		if (!circ.IsNull()){
			gp_Pnt center = circ->Location();   // ✅ CENTER HERE
			bool ctrlDown = (Fl::event_state() & FL_CTRL) != 0;
			if(ctrlDown){
				pickcenterforstudyrotation=center;
				pickcenteraxisDir = circ->Circ().Axis().Direction(); // circle normal
				studyRotation=1;
        		cotm2(studyRotation)
			}
			double radius = circ->Radius();
			TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(center);
			highlightVertex(v, ldd);
		}
        cotm2("✅ This IS a circle")
    }
}






GProp_GProps props;
BRepGProp::LinearProperties(edge, props);
double length = props.Mass();

string pname = "Edge length: " + to_string_trim(length);
if (pname != help.edge)
{
    help.edge = pname;
    help.upd();
}

if (highlight.IsNull())
	    highlight = new AIS_Shape(edge);
// {
//     TopoDS_Shape moved = edge;
//     moved.Location(ldd->Origin);   // apply TopLoc_Location

//     highlight->SetShape(moved);

	// highlight = new AIS_Shape(edge);
	// if(ldd && !ldd->Origin.IsIdentity())
	{
    	highlight->SetLocalTransformation(ldd->Origin.Transformation());
		// cotm2("ldd->Origin.Transformation()")
		// PrintLocation(ldd->Origin);
}

if(hlr_on){

if (m_context->IsDisplayed(highlight))
    m_context->Remove(highlight, Standard_False);


highlight->Set(edge);
m_context->Deactivate(highlight);
highlight->SetDisplayMode(AIS_WireFrame);

Handle(Prs3d_LineAspect) la =
    new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 3.0);

highlight->Attributes()->SetLineAspect(la);
highlight->Attributes()->SetWireAspect(la);
highlight->Attributes()->SetFreeBoundaryAspect(la);
highlight->Attributes()->SetUnFreeBoundaryAspect(la);
highlight->Attributes()->SetFaceBoundaryAspect(la);
highlight->Attributes()->SetHiddenLineAspect(la);
highlight->Attributes()->SetVectorAspect(la);
highlight->Attributes()->SetSectionAspect(la);
highlight->Attributes()->SetSeenLineAspect(la);

highlight->SetZLayer(Graphic3d_ZLayerId_TopOSD);

if (m_context->IsDisplayed(highlight))
    m_context->Redisplay(highlight, Standard_True);
else
    m_context->Display(highlight, Standard_True);
m_context->Deactivate(highlight);
redraw();


		// redraw();
		return;
}

	if(!hlr_on){

if (m_context->IsDisplayed(highlight))
    m_context->Remove(highlight, Standard_False);

m_context->Deactivate(highlight);

        TopoDS_Edge edge = TopoDS::Edge(detected);
		GProp_GProps props;
		BRepGProp::LinearProperties(edge, props);
		double length = props.Mass();
		string pname="Edge length: "+to_string_trim(length);
		if(pname!=help.edge){help.edge=pname;help.upd();}
		//here want highlight is visible
		// if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
		// highlight modes must ALSO be moved to topmost
// m_context->SetZLayer(visible_, Graphic3d_ZLayerId_Topmost,
//                      Prs3d_TypeOfHighlight_LocalDynamic);
// m_context->SetZLayer(visible_, Graphic3d_ZLayerId_Topmost,
//                      Prs3d_TypeOfHighlight_Selected);
		// m_context->SetDisplayPriority(visible_, -1);
		//here want that highlight keep visible


// m_context->SetDisplayPriority(visible_, 1);
redraw();
return;
		}
		break;
	}
    case TopAbs_FACE: {
        TopoDS_Face picked = TopoDS::Face(detected);
        TopLoc_Location origLoc = picked.Location();
        picked.Location(TopLoc_Location()); // normalize

        // stable face index
        int faceIndex = -1;
        {
            Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(brepOwner->Selectable());
            if (!aisShape.IsNull()) {
                TopoDS_Shape full = aisShape->Shape();
                full.Location(TopLoc_Location());
                static TopTools_IndexedMapOfShape faceMap;
                faceMap.Clear();
                TopExp::MapShapes(full, TopAbs_FACE, faceMap);
                faceIndex = faceMap.FindIndex(picked) - 1;
            }
        }

        // centroid & area
        GProp_GProps props;
        BRepGProp::SurfaceProperties(picked, props);
        gp_Pnt centroid = props.CentreOfMass();
        double area     = props.Mass();

        // normal
        Standard_Real u1,u2,v1,v2;
        BRepTools::UVBounds(picked, u1,u2, v1,v2);
        Handle(Geom_Surface) surf = BRep_Tool::Surface(picked);
        GeomLProp_SLProps slp(surf, (u1+u2)*0.5, (v1+v2)*0.5, 1, Precision::Confusion());
        gp_Dir normal = slp.IsNormalDefined() ? slp.Normal() : gp_Dir(0,0,1);
        if (picked.Orientation() == TopAbs_REVERSED)
            normal.Reverse();

        std::string serialized = SerializeCompact(
            ldd ? ldd->name : "part", faceIndex, normal, centroid, area);
        cotm("serialized", serialized);

        picked.Location(origLoc); // restore
        break;
    }

    default:
		// if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
        clearHighlight();
        break;
    }
// if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
    redraw();
}

	void ev_highlight_v1() {
		// Start with a clean slate for the custom highlight
		clearHighlight();

		// --- Strict Selection Mode Control for Hover ---
		// 1. Deactivate ALL active modes first to ensure a clean slate for
		// picking. This loops through common topological modes. for
		// (Standard_Integer mode = TopAbs_VERTEX; mode <= TopAbs_COMPSOLID;
		// ++mode) {
		//     m_context->Deactivate(mode);
		// }
		// You might also need: m_context->Deactivate(0); // If 0 means "all" or
		// a special mode.

		// 2. Activate ONLY vertex selection mode for this specific picking
		// operation. This uses the AIS_Shape::SelectionMode utility, which
		// correctly returns 0 for TopAbs_VERTEX.
		m_context->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
		m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
		if (!hlr_on) {
			m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
		} else {
			m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_FACE));
		}
		// 3. Perform the picking operations
		m_context->MoveTo(mousex, mousey, m_view, Standard_False);

		m_context->SelectDetected(AIS_SelectionScheme_Replace);

		// 4. Get the detected owner
		Handle(SelectMgr_EntityOwner) anOwner = m_context->DetectedOwner();

		luadraw* ldd=0;
		if (!anOwner.IsNull()) {
			ldd = lua_detected(anOwner);
			if (ldd) {
				string pname=ldd->name;
				if(pname!=help.pname){help.pname=pname;help.upd();}
				// cotm(ldd->name);
			}
		}

		// 5. Deactivate vertex mode immediately after picking
		// This is crucial if you only want vertex picking *during* hover,
		// and want other selection behaviors (e.g., selecting faces on click)
		// at other times.
		// m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
		// --- End Strict Selection Mode Control ---

		// --- Debugging and Highlighting Logic ---
		if (!anOwner.IsNull()) {
			Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(anOwner);
			if (!brepOwner.IsNull()) {
				TopoDS_Shape detectedTopoShape = brepOwner->Shape();

				// printf("Detected TopoDS_ShapeType: %d (0=Vertex, 1=Edge,
				// 2=Wire, 3=Face, etc.)\n", detectedTopoShape.ShapeType());
				// printf("Value of TopAbs_VERTEX: %d\n", TopAbs_VERTEX); //
				// Confirms the actual value of TopAbs_VERTEX

				if (detectedTopoShape.ShapeType() == TopAbs_VERTEX) {
					// printf("--- CONDITION: detectedTopoShape.ShapeType() ==
					// TopAbs_VERTEX is TRUE ---\n");
					TopoDS_Vertex currentVertex = TopoDS::Vertex(detectedTopoShape);
					if (!myLastHighlightedVertex.IsEqual(currentVertex)) {
						highlightVertex(currentVertex);
					} else {
						// Highlighted same vertex, no need to re-print or
						// re-draw
						printf(
							"Hovering over same vertex: X=%.3f, Y=%.3f, "
							"Z=%.3f\n",
							BRep_Tool::Pnt(currentVertex).X(), BRep_Tool::Pnt(currentVertex).Y(),
							BRep_Tool::Pnt(currentVertex).Z());
	// 						help.point = "Point: " 
    // + std::to_string( BRep_Tool::Pnt(currentVertex).X() ) + ","
    // + std::to_string( BRep_Tool::Pnt(currentVertex).Y() ) + ","
    // + std::to_string( BRep_Tool::Pnt(currentVertex).Z() );
	// 						help.upd();
					}
				} else if(detectedTopoShape.ShapeType() == TopAbs_FACE){


TopoDS_Face pickedFace = TopoDS::Face(detectedTopoShape);

// Get original location
TopLoc_Location origLoc = pickedFace.Location();

// Reset to identity for geometry calculations
TopoDS_Face pickedRaw = pickedFace;
pickedRaw.Location(TopLoc_Location());

// --- stable face index ---
// Build map from the full owner shape with identity location
int faceIndex = -1;
if (!brepOwner.IsNull()) {
    Handle(SelectMgr_SelectableObject) selObj = brepOwner->Selectable();
    Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(selObj);
    if (!aisShape.IsNull()) {
        TopoDS_Shape fullRaw = aisShape->Shape();
        fullRaw.Location(TopLoc_Location());
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(fullRaw, TopAbs_FACE, faceMap);
        faceIndex = faceMap.FindIndex(pickedRaw) - 1; // Subtract 1 to convert to 0-based index
    }
}

// --- centroid & area ---
GProp_GProps props;
BRepGProp::SurfaceProperties(pickedRaw, props);
gp_Pnt centroid = props.CentreOfMass();
double area     = props.Mass();

// --- normal ---
Standard_Real u1,u2,v1,v2;
BRepTools::UVBounds(pickedRaw, u1,u2, v1,v2);
Handle(Geom_Surface) surf = BRep_Tool::Surface(pickedRaw);
GeomLProp_SLProps slp(surf, (u1+u2)*0.5, (v1+v2)*0.5, 1, Precision::Confusion());
gp_Dir normal = slp.IsNormalDefined() ? slp.Normal() : gp_Dir(0,0,1);
if (pickedFace.Orientation() == TopAbs_REVERSED) normal.Reverse();

// --- serialize ---
std::string serialized = SerializeCompact(
    ldd ? ldd->name : std::string("part"),
    faceIndex,
    normal,
    centroid,
    area
);
cotm("serialized", serialized);

// --- restore original placement ---
pickedFace.Location(origLoc);


#if 0					
	TopoDS_Face pickedFace = TopoDS::Face(detectedTopoShape);

    // Get original location
    TopLoc_Location origLoc = pickedFace.Location();

    // Reset to identity
    pickedFace.Location(TopLoc_Location());

    // --- all geometry now in "raw" model space ---
    // Stable face index
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(pickedFace, TopAbs_FACE, faceMap);
    int faceIndex = faceMap.FindIndex(pickedFace);

    // Centroid & area
    GProp_GProps props;
    BRepGProp::SurfaceProperties(pickedFace, props);
    gp_Pnt centroid = props.CentreOfMass();
    double area     = props.Mass();

    // Normal
    Standard_Real u1,u2,v1,v2;
    BRepTools::UVBounds(pickedFace, u1,u2, v1,v2);
    Handle(Geom_Surface) surf = BRep_Tool::Surface(pickedFace);
    GeomLProp_SLProps slp(surf, (u1+u2)*0.5, (v1+v2)*0.5, 1, Precision::Confusion());
    gp_Dir normal = slp.IsNormalDefined() ? slp.Normal() : gp_Dir(0,0,1);
    if (pickedFace.Orientation() == TopAbs_REVERSED) normal.Reverse();

    // Serialize
    std::string serialized = SerializeCompact(
        ldd ? ldd->name : std::string("part"),
        faceIndex,
        normal,
        centroid,
        area
    );
    cotm("serialized", serialized);

    // --- restore original placement ---
    pickedFace.Location(origLoc);
	#endif

					// string ser=SerializeFaceInvariant(TopoDS::Face(detectedTopoShape));
					// cotm(ser)

					    // Step 3: Get the selected shape
    // if (context->HasSelectedShape()) {
    //     TopoDS_Shape selectedShape = context->SelectedShape();

    //     // Step 4: Explore faces in the selected shape
    //     for (TopExp_Explorer exp(selectedShape, TopAbs_FACE); exp.More(); exp.Next()) {
    //         TopoDS_Face face = TopoDS::Face(exp.Current());

    //         // You now have the face!
    //         std::cout << "Picked face found!" << std::endl;

    //         // You can now use this face for transformation
    //         break; // Just take the first face for simplicity
    //     }
    // }



				}		
				else {
					// printf("--- CONDITION: detectedTopoShape.ShapeType() ==
					// TopAbs_VERTEX is FALSE (Type %d) ---\n",
					// detectedTopoShape.ShapeType());
					clearHighlight();  // Detected a BRepOwner, but not a vertex
				}
			} else {
				// printf("Owner is not a StdSelect_BRepOwner.\n");
				clearHighlight();  // Owner is not a BRepOwner (e.g., detected
								   // an AIS_Text)
			}
		} else {
			// go_up(1);
			// cotm("test")
			// printf("%s",cotmlastoutput.c_str());
			// go_up;
			// cotmupset
			// cotm("Nothing detected under the mouse.");
			// cotmup

			clearHighlight();  // Nothing detected
		}

		m_context->UpdateCurrentViewer();  // Update the viewer to show/hide
										   // highlight
	}
	
	
	
	int handle(int event) override {
		if(!m_initialized)return Fl_Window::handle(event);
static auto last_event = std::chrono::steady_clock::now();
auto now = std::chrono::steady_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event);

constexpr auto frame_interval = std::chrono::milliseconds( 1000/30);
// constexpr auto frame_interval = std::chrono::milliseconds(((int)(1000 / 8.0)));

if (event == FL_DRAG && isRotating && m_initialized) {
	// cotm(elapsed);
    if (elapsed > frame_interval) {
        // perf(); 
        m_view->Rotation(Fl::event_x(), Fl::event_y()); 
        projectAndDisplayWithHLR(vshapes, 1);
        // perf("vnormalr"); 
        colorisebtn(); 
        redraw();	
        // cotm("in");
        last_event = std::chrono::steady_clock::now();
        return 0;
    }
    // cotm("out");
    return 0;
}

bool ctrlDown = (Fl::event_state() & FL_CTRL) != 0;
if(ctrlDown && studyRotation==1 ){
	
	cotm2(vaShape.size())
	rotateShapeByMouse(ulua[help.pname]->ashape, m_context, pickcenterforstudyrotation, pickcenteraxisDir, Fl::event_x(), Fl::event_y());
	study_trg=ulua[help.pname]->ashape;
}
// if(studyRotation && !ctrlDown){
// initLoc = study_trg->LocalTransformation(); // save AIS location
// case FL_RELEASE: if (dragging) { toRotate->SetLocalTransformation(initLoc);   context->Redisplay(toRotate, true); dragging = false; } return 1;
// }
// case FL_DRAG:
//     if (dragging) {
//         float dx = Fl::event_x() - lastX;

//         double angle = dx * 0.002;  // slow rotation

//         gp_Ax1 axis(center, axisDir);

//         gp_Trsf rot;
//         rot.SetRotation(axis, angle);

//         // Combine: original location + new rotation
//         gp_Trsf newLoc = initLoc * rot;

//         toRotate->SetLocalTransformation(newLoc);
//         context->Redisplay(toRotate, true);

//         lastX = Fl::event_x();
//         lastY = Fl::event_y();
//     }
//     return 1;






if(event== FL_PUSH && (Fl::event_state() & FL_CTRL)){
	if(help.pname!=""){
		cotm(help.pname)
		gopart(help.pname);
		// ld->occv->FitViewToShape(ld->occv->m_view, ld->shape);
		return 1;
	}
}



		static int start_y;
		const int edge_zone = this->w() * 0.05;	 // 5% right edge zone

		// #include <SelectMgr_EntityOwner.hxx>
		// #include <StdSelect_BRepOwner.hxx>
		// #include <TopAbs_ShapeEnum.hxx> // Ensure this is included for
		// TopAbs_VERTEX etc.

		// ... (your existing OCCViewerWindow class methods) ...

		// In your initializeOCC() method:
		// Ensure SetSelectionSensitivity is set appropriately for small
		// vertices. For a sphere of radius 10, 0.02 or 0.05 is a good starting
		// point. m_context->SetSelectionSensitivity(0.02);

		// In your createSampleShape() method:
		// Remove any AIS_InteractiveContext::SetMode() calls here, as we will
		// control it directly in FL_MOVE The default selection behavior on the
		// AIS_Shape itself is sufficient for this approach.

		// In your handle(int event) method:

static std::chrono::steady_clock::time_point lastTime;

if (event == FL_MOVE) {
    int x = Fl::event_x();
    int y = Fl::event_y();





    auto now = std::chrono::steady_clock::now();
    double elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();



    if ((abs(x - mousex) > 5 || abs(y - mousey) > 5) && elapsedMs > 50) {
        mousex = x;
        mousey = y;
        // getvertex();
        ev_highlight();
        lastTime = now;
    }
	// return 1;
}


		if (0 && event == FL_MOVE) {
			int x = Fl::event_x();
			int y = Fl::event_y();
			
			mousex = x;
			mousey = y;
			// getvertex();
			// return 1;

			// {// 1) guarda mousex/mousey (já fazes isso)
			// gp_Pnt screenHLR = screenPointFromMouse(mousex, mousey);

			// // 2) converte para 3D
			// gp_Pnt worldApprox = convertHLRToWorld(screenHLR);

			// // 3) (opcional) vertex exato mais próximo
			// gp_Pnt trueVertex = getAccurateVertexPosition(screenHLR);

			// // Imprime para debug
			// printf("Mouse → HLR2D: (%.3f,%.3f)\n", screenHLR.X(),
			// screenHLR.Y()); printf("Approx world 3D: (%.3f,%.3f,%.3f)\n",
			//        worldApprox.X(), worldApprox.Y(), worldApprox.Z());
			// printf("Best match vertex: (%.3f,%.3f,%.3f)\n",
			//        trueVertex.X(), trueVertex.Y(), trueVertex.Z());}

			ev_highlight();

			// return 1;
		}
		if(event==FL_PUSH){
			// cotm("")
			OnMouseClick(mousex,mousey,m_context,m_view);
		}

		




		switch (event) {
			case FL_PUSH:
				if (Fl::event_button() == FL_LEFT_MOUSE) {
					// Check if click is in right 5% zone
					if (Fl::event_x() > (this->w() - edge_zone)) {
						isRotatingz=1;
						start_y = Fl::event_y();
						return 1;  // Capture mouse
					}
				}
				break;

			// bar5per
			case FL_DRAG:
				if (Fl::event_state(FL_BUTTON1)) {
					// Only rotate if drag started in right edge zone
					if (isRotatingz) {
					// if (Fl::event_x() > (this->w() - edge_zone)) {
						int dy = Fl::event_y() - start_y;
						start_y = Fl::event_y();

						// Rotate view (OpenCascade)
						if (dy != 0) {
							double angle = dy * 0.005;	// Rotation sensitivity factor
							perf();
							// Fl::wait(0.01);
							m_view->Rotate(0, 0,
										   angle);	// Rotate around Z-axis
							perf("vnormal");
							// projectAndDisplayWithHLR(vshapes);
							// recomputeComputed () ;
							redraw();  //  redraw(); // m_view->Update ();

							// redraw();//
							// m_view->Redraw();
						}
						return 1;
					}
				}
				break;
		}

		switch (event) {
			case FL_PUSH:
				if (Fl::event_button() == FL_LEFT_MOUSE) {
					// Fl::add_timeout(0.05, timeout_cb, this);
					isRotating = true;
					lastX = Fl::event_x();
					lastY = Fl::event_y();
					if (m_initialized) {
						m_view->StartRotation(lastX, lastY);
					}
					return 1;
				} else if (Fl::event_button() == FL_RIGHT_MOUSE) {
					isPanning = true;
					lastX = Fl::event_x();
					lastY = Fl::event_y();
					return 1;
				}
				break;

			case FL_DRAG:
				// if (isRotating && m_initialized) {
				// 	func_fps(12, perf(); m_view->Rotation(Fl::event_x(), Fl::event_y());
				// 			// projectAndDisplayWithHLR(vaShape,1);
				// 			projectAndDisplayWithHLR(vshapes, 1);
				// 			redraw();	 //  redraw(); // m_view->Update ();
				// 			perf("vnormalr");

				// 			// colorisebtn();
				// 			// redraw();
				// 	);
				// 	return 1;
				// } else 
				if (isPanning && m_initialized && Fl::event_button() == FL_RIGHT_MOUSE) {
					int dx = Fl::event_x() - lastX;
					int dy = Fl::event_y() - lastY;
					// m_view->Pan(dx, -dy); // Note: -dy to match typical
					// screen coordinates
					// func_fps(25, 
						m_view->Pan(dx, -dy); redraw();	 //  redraw(); // m_view->Update ();

							lastX = Fl::event_x(); lastY = Fl::event_y();
						// );
					return 1;
				}
				break;

			case FL_RELEASE:
				if (Fl::event_button() == FL_LEFT_MOUSE) {
					isRotating = false;
					isRotatingz=0;
					isPanning = false;
					return 1;
				}
				break;
			case FL_MOUSEWHEEL:
				if (m_initialized) {
					// func_fps(25, 
						int mouseX = Fl::event_x(); int mouseY = Fl::event_y();
							m_view->StartZoomAtPoint(mouseX, mouseY);
							// Get wheel delta (normalized)
							int wheelDelta = Fl::event_dy();

							// According to OpenCASCADE docs, ZoomAtPoint needs:
							// 1. The start point (where mouse is)
							// 2. The end point (calculated based on wheel movement)

							// Calculate end point - this determines zoom direction
							// and magnitude
							int endX = mouseX;
							int endY = mouseY - (wheelDelta * 5 * 5);  // Vertical movement controls zoom

							// Call ZoomAtPoint with both points
							m_view->ZoomAtPoint(mouseX, mouseY, endX, endY);
							
		// 							lop(i,0,vaShape.size()){
		// FixZPrecisionAndGhostLines_793(m_context,m_view,vaShape[i]);}

							// redraw();  //  redraw(); // m_view->Update ();
							// Handle(Graphic3d_Camera) cam = m_view->Camera();
							// cam->InvalidateProjection();
// // cam->SetCenter (gp_Pnt(0,0,0));
// m_view->SetAutoZFitMode(Standard_False);
// // Near / Far explícitos
// std::cout
//     << "ZNear = " << cam->ZNear()
//     << " ZFar = "  << cam->ZFar()
//     << std::endl;
// cam->SetZRange(0.1, 10000);

// // Aplicar
// m_view->Invalidate();  std::cout
//     << "ZNear = " << cam->ZNear()
//     << " ZFar = "  << cam->ZFar()
//     << std::endl;
// 	m_view->Redraw();std::cout
//     << "ZNear = " << cam->ZNear()
//     << " ZFar = "  << cam->ZFar()
//     << std::endl;

	redraw();

					// );
					return 1;
				}
				break;
		}
		return Fl_Window::handle(event);
	}

	int lastX, lastY;
	bool isRotatingz = false;
	bool isRotating = false;
	bool isPanning = false;




// Assume: context is your AIS_InteractiveContext
//         view is your V3d_View
//         x, y are mouse click coordinates

void OnMouseClick(Standard_Integer x, Standard_Integer y,
                  const Handle(AIS_InteractiveContext)& context,
                  const Handle(V3d_View)& view)
{
	context->Activate(TopAbs_FACE);
    // Step 1: Move the selection to the clicked point
    context->MoveTo(x, y, view,0);

    // Step 2: Select the clicked object
    context->Select(true); // true = update viewer

    // Step 3: Get the selected shape
    if (context->HasSelectedShape()) {
        TopoDS_Shape selectedShape = context->SelectedShape();

        // Step 4: Explore faces in the selected shape
        for (TopExp_Explorer exp(selectedShape, TopAbs_FACE); exp.More(); exp.Next()) {
            TopoDS_Face face = TopoDS::Face(exp.Current());

            // You now have the face!
            std::cout << "Picked face found!" << std::endl;

            // You can now use this face for transformation
            break; // Just take the first face for simplicity
        }
    }
}

#pragma endregion events

	struct pashape : public AIS_Shape {
		// expõe o drawer protegido da AIS_Shape
		using AIS_Shape::myDrawer;

		pashape(const TopoDS_Shape& shape) : AIS_Shape(shape) {
			// 1) Cria um novo drawer
			Handle(Prs3d_Drawer) dr = new Prs3d_Drawer();

			// 2) Linha tracejada vermelha, espessura 2
			Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect(Quantity_NOC_GRAY,	// cor vermelha
																	Aspect_TOL_DASH,	// tipo: dash
																	0.3					// espessura da linha
			);

			// dr->SetWireAspect(wireAsp);
			// dr->SetWireDraw(true);

			// 3) (Opcional) Desliga faces, só wireframe

			// 4) Substitui o drawer interno
			// SetAttributes(dr);
			// setColor(dr,Quantity_NOC_RED);
			// replaceWithNewOwnAspects();
			Attributes()->SetLineAspect(wireAsp);
			Attributes()->SetSeenLineAspect(wireAsp);
			Attributes()->SetFaceBoundaryAspect(wireAsp);
			Attributes()->SetWireAspect(wireAsp);
			Attributes()->SetUnFreeBoundaryAspect(wireAsp);
			Attributes()->SetFreeBoundaryAspect(wireAsp);
			Attributes()->SetFaceBoundaryAspect(wireAsp);
		}
	};

	void draw_objs() {
		perf1("");
		for (int i = 0; i < vshapes.size(); i++) {
			Handle(AIS_Shape) aShape = new AIS_Shape(vshapes[i]);
			vaShape.push_back(aShape);
			m_context->Display(aShape, 0);
		}
		perf1("draw_objs");
		toggle_shaded_transp(currentMode);
	}
	TopoDS_Shape pl() {
		int x = -150;
		int y = -150;
		int z = -150;
		// Define points for the polyline
		gp_Pnt p1(x + 0.0, y + 0.0, z + 0.0);
		gp_Pnt p2(x + 100.0, y + 0.0, z + 0.0);
		gp_Pnt p3(x + 100.0, y + 10.0, z + 0.0);
		gp_Pnt p4(x + 0.0, y + 100.0, z + 0.0);

		// Create edges between points
		TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
		TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
		TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p4);
		TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1);

		// Combine edges into a wire
		BRepBuilderAPI_MakeWire wireBuilder;
		wireBuilder.Add(edge1);
		wireBuilder.Add(edge2);
		wireBuilder.Add(edge3);
		wireBuilder.Add(edge4);

		TopoDS_Wire polyline = wireBuilder.Wire();

		// Create a face from the wire
		TopoDS_Face face = BRepBuilderAPI_MakeFace(polyline);

		// Extrude the face into a 3D solid
		gp_Vec extrusionVector(0.0, 0.0, -10.0);  // Extrude along the Z-axis
		TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(face, extrusionVector).Shape();

		return extrudedShape;
	}
	// Display trihedron at origin with no axis labels
	void ShowTriedronWithoutLabels(gp_Trsf trsf, const Handle(AIS_InteractiveContext) & ctx) {
		// 1) create a trihedron at world origin
		gp_Ax2 ax2(gp_Pnt(0, 0, 0).Transformed(trsf), gp_Dir(0, 0, 1).Transformed(trsf),
				   gp_Dir(1, 0, 0).Transformed(trsf));
		Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(ax2);
		Handle(AIS_Trihedron) tri = new AIS_Trihedron(placement);

		// 2) build a fresh DatumAspect and attach to the AIS_Trihedron
		Handle(Prs3d_DatumAspect) da = new Prs3d_DatumAspect();
		tri->Attributes()->SetDatumAspect(da);

		// 3) hide the “X Y Z” labels
		da->SetDrawLabels(false);  // :contentReference[oaicite:0]{index=0}

		// 4) style each axis line (width = 2.0 px)
		da->LineAspect(Prs3d_DatumParts_XAxis)->SetWidth(4.0);	// :contentReference[oaicite:1]{index=1}
		da->LineAspect(Prs3d_DatumParts_YAxis)->SetWidth(4.0);
		da->LineAspect(Prs3d_DatumParts_ZAxis)->SetWidth(4.0);

		// 5) color each axis
		da->LineAspect(Prs3d_DatumParts_XAxis)->SetColor(Quantity_NOC_RED);	 // :contentReference[oaicite:2]{index=2}
		da->LineAspect(Prs3d_DatumParts_YAxis)->SetColor(Quantity_NOC_GREEN);
		da->LineAspect(Prs3d_DatumParts_ZAxis)->SetColor(Quantity_NOC_BLUE);

		// 5) color each arrow
		// set line‐width of the arrow stems (you already did this for the axes)
		da->LineAspect(Prs3d_DatumParts_XArrow)->SetWidth(2.0);
		da->LineAspect(Prs3d_DatumParts_YArrow)->SetWidth(2.0);
		da->LineAspect(Prs3d_DatumParts_ZArrow)->SetWidth(2.0);

		// now set each arrow’s colour
		da->LineAspect(Prs3d_DatumParts_XArrow)->SetColor(Quantity_NOC_RED);  // :contentReference[oaicite:0]{index=0}
		da->LineAspect(Prs3d_DatumParts_YArrow)->SetColor(Quantity_NOC_GREEN);
		da->LineAspect(Prs3d_DatumParts_ZArrow)->SetColor(Quantity_NOC_BLUE);

		// 6) (optional) hide arrowheads if you don’t want them:
		//    da->SetDrawArrows(false);

		// 7) display it
		ctx->Display(tri, Standard_False);
	}

#pragma region projection

	HLRAlgo_Projector projector;

	Standard_Integer currentMode = AIS_WireFrame;
	void toggle_shaded_transp(Standard_Integer fromcurrentMode = AIS_WireFrame) {
		perf1();
		// m_context->SetDisplayMode(AIS_HLRMode, Standard_True);

		// cotm(vaShape.size()) 
		for (std::size_t i = 0; i < vaShape.size(); ++i) {
			Handle(AIS_Shape) aShape = vaShape[i];
			if (aShape.IsNull()) continue;

			if (fromcurrentMode == AIS_Shaded) {
				hlr_on = 1;
				// Mudar para modo wireframe
				// aShape->UnsetColor();
				aShape->UnsetAttributes();	// limpa materiais, cor, largura, etc.
				m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_False);

				aShape->Attributes()->SetFaceBoundaryDraw(Standard_False);
				aShape->Attributes()->SetLineAspect(wireAsp);
				aShape->Attributes()->SetSeenLineAspect(wireAsp);
				aShape->Attributes()->SetWireAspect(wireAsp);
				aShape->Attributes()->SetUnFreeBoundaryAspect(wireAsp);
				aShape->Attributes()->SetFreeBoundaryAspect(wireAsp);
				aShape->Attributes()->SetFaceBoundaryAspect(wireAsp);
			} else {
				hlr_on = 0;
	// 			// Mudar para modo sombreado
				aShape->UnsetAttributes();	// limpa materiais, cor, largura, etc.
if(0){
				m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);

				aShape->SetColor(Quantity_NOC_GRAY70); 
				aShape->Attributes()->SetFaceBoundaryDraw(1);//////////////////
				// aShape->Attributes()->SetFaceBoundaryDraw(!vlua[i]->ttfillet);//////////////////
				aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);
				aShape->Attributes()->SetSeenLineAspect(edgeAspect); //
}
	// 			// opcional
	// 		// m_context->Redisplay(aShape, AIS_Shaded, 0);
	// 		    m_context->SetPolygonOffsets(
    //     aShape,
    //     Aspect_POM_Fill,   // mode: fill + edges
    //     0.0f,             // factor
    //     0,             // units
    //     0     // update viewer immediately
    // );
// m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);
// aShape->SetColor(Quantity_NOC_GRAY70);
// 				aShape->Attributes()->SetFaceBoundaryDraw(!vlua[i]->ttfillet);//////////////////
// 				aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);
// 				aShape->Attributes()->SetFaceBoundaryUpperContinuity(GeomAbs_C2);
// 				m_context->SetPolygonOffsets(
//         aShape,
//         Aspect_POM_Fill,   // mode: fill + edges
//         0,             // factor
//         0,             // units
//         0     // update viewer immediately
//     );

if(1){
 
				// aShape->UnsetAttributes();	// limpa materiais, cor, largura, etc.
				if(vlua[i]->name=="framev"  ){
aShape->SetZLayer(Graphic3d_ZLayerId_Top);
				}else{
					// aShape->SetZLayer(Graphic3d_ZLayerId_Bot);
				}

				m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);
// aShape->SetColor(Quantity_NOC_GRAY70);
aShape->SetColor(Quantity_NOC_WHITE);
				// aShape->Attributes()->SetFaceBoundaryDraw(!vlua[i]->ttfillet);//////////////////
				// aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);

// Drawer global (ou shaded->Attributes() se quiseres só para este objeto)
Handle(Prs3d_Drawer) drawer = m_context->DefaultDrawer();

// Ativa o desenho das face boundaries (as edges reais entre faces)
drawer->SetFaceBoundaryDraw(Standard_True);

// Configura cor e espessura das boundaries (ficam pretas e nítidas)
Handle(Prs3d_Drawer) objDrawer = aShape->Attributes();
objDrawer->SetFaceBoundaryDraw(Standard_True);
objDrawer->SetFaceBoundaryAspect(new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.8)); // 1.0 a 1.5 costuma ficar bom

// Crucial: suprime as boundaries com continuidade alta (seams suaves, como em cilindros ou fillets)
drawer->SetFaceBoundaryUpperContinuity(GeomAbs_C2);  // ou GeomAbs_C2 para esconder ainda mais seams suaves
// GeomAbs_C0 = mostra tudo (arestas "duras" só)
// GeomAbs_C1 = esconde seams tangentes (bom para cilindros)
// GeomAbs_C2 = esconde mais (fillets suaves)

// Desativa isoparamétricas (linhas U/V da triangulação)
drawer->SetIsoOnTriangulation(Standard_False);

// m_view->SetZClippingDepth(1.0);
// m_view->SetZClippingWidth(2000.0);

// m_context->SetPolygonOffsets(
//         aShape,
//         Aspect_POM_Fill,   // mode: fill + edges
//         0.1,             // factor
//         0,             // units
//         0     // update viewer immediately
//     );

// Display
// m_context->Display(shaded, Standard_True);
if(0 &&vlua[i]->name=="framev")
{
// 	Graphic3d_ZLayerId edgeLayer;  // ou int edgeLayer; (Graphic3d_ZLayerId é um enum int)

// // Cria uma nova ZLayer custom (inserida antes da Graphic3d_ZLayerId_Top)
// m_viewer->AddZLayer(edgeLayer);  // edgeLayer recebe o novo ID

// // Atribui esta layer ao teu AIS_Shape (todo o objeto vai para esta layer)
// m_context->SetZLayer(aShape, edgeLayer);

	// cotm2("framev")
// 			    m_context->SetPolygonOffsets(
//         aShape,
//         Aspect_POM_Fill,   // mode: fill + edges
//         0.5,             // factor
//         2,             // units
//         0     // update viewer immediately
//     );
// }else{
// m_context->SetPolygonOffsets(
//         aShape,
//         Aspect_POM_Fill,   // mode: fill + edges
//         1,             // factor
//         -1,             // units
//         0     // update viewer immediately
//     );


// #include <Graphic3d_ZLayerSettings.hxx>
// #include <Graphic3d_PolygonOffset.hxx>

// Cria ID para nova layer (inserida antes da Top por defeito)
Graphic3d_ZLayerId myEdgeLayer;
m_viewer->AddZLayer(myEdgeLayer);  // retorna o ID

// Obtém as settings da layer para modificar
Graphic3d_ZLayerSettings layerSettings = m_viewer->ZLayerSettings(myEdgeLayer);

// Configurações úteis para edges/overlay sem z-fighting
// layerSettings.SetEnableDepthTest(Standard_True);   // testa profundidade (normal)
// layerSettings.SetEnableDepthWrite(Standard_True); // NÃO escreve profundidade → objetos por cima não ocultam os de trás (bom para linhas por cima)
// layerSettings.SetClearDepth(Standard_False);       // não limpa depth buffer (se quiseres overlay puro, podes por True)

// Polygon offset na layer inteira (empurra ligeiramente as linhas para a frente)
// Graphic3d_PolygonOffset offset;
// offset.Factor = 1.0;   // experimenta 0.5 a 2.0
// offset.Units  = 0.0;   // ajuda mais em zoom out extremo
// layerSettings.SetPolygonOffset(offset);
// Ou conveniência: layerSettings.SetDepthOffsetPositive(); // offset mínimo positivo

// Opcional: torna immediate para desenhar por último
// layerSettings.SetImmediate(Standard_True);

// Aplica as settings
m_viewer->SetZLayerSettings(myEdgeLayer, layerSettings);

// Atribui ao teu objeto (ou só às edges se separares)
m_context->SetZLayer(aShape, myEdgeLayer);
// m_context->Redisplay(aShape, Standard_True);

}
}
if(0){
m_context->Remove(aShape,0);
// Handle(AIS_Shape) &obj = aShape;
// m_context->Remove(obj,0);
// m_context->Erase(obj,0);

// Garantir triangulação (obrigatório para PolyAlgo)
BRepMesh_IncrementalMesh(aShape->Shape(), 0.5);

Handle(AIS_Shape) obj = new AIS_Shape(aShape->Shape());

// HLR exige Shaded
obj->SetDisplayMode(AIS_Shaded);

Handle(Prs3d_Drawer) dr = obj->Attributes();

// 1) HLR rápido
dr->SetTypeOfHLR(Prs3d_TOH_PolyAlgo);

// 2) NÃO desenhar faces
dr->SetFaceBoundaryDraw(Standard_False);
dr->SetWireDraw(Standard_True);

// 3) Wire aspect explícito (CRUCIAL)
Handle(Prs3d_LineAspect) wire =
    new Prs3d_LineAspect(
        Quantity_NOC_BLACK,
        Aspect_TOL_SOLID,
        1.0
    );

dr->SetWireAspect(wire);

// NÃO definir HiddenLineAspect → linhas escondidas não aparecem

m_context->Display(obj, Standard_True);
}
if(0){
m_context->Remove(aShape,0);
// Handle(AIS_Shape) visible = new AIS_Shape(aShape);  // ou AIS_Shape se quiseres cores custom

// aShape->SetTransparency(1.0);        // remove fill completamente
aShape->SetDisplayMode(AIS_Shaded);  // mantém shaded para boundaries funcionarem bem

Handle(Prs3d_Drawer) drawer = m_context->DefaultDrawer();
drawer->SetFaceBoundaryDraw(Standard_True);

Handle(Prs3d_LineAspect) boundaryAspect = drawer->FaceBoundaryAspect();
boundaryAspect->SetColor(Quantity_NOC_BLACK);
boundaryAspect->SetWidth(1.5);  // ou 3 como tinhas

drawer->SetIsoOnTriangulation(Standard_False);
drawer->SetFreeBoundaryDraw(Standard_False);
drawer->SetUnFreeBoundaryDraw(Standard_False);

// Para prismas/extrusões poligonais, normalmente não precisas de continuity filter (já ficam clean)

m_context->Display(aShape, 0);
}




			}
			m_context->Redisplay(aShape, 0);
			// FixZPrecisionAndGhostLines_793(m_context,m_view,aShape);
		}

		perf1("toggle_shaded_transp");
		if (hlr_on == 1) {
			cotm("hlr1")
			// cotm2(vshapes.size())
				// projectAndDisplayWithHLR(vaShape);
				projectAndDisplayWithHLR(vshapes);
				// if(m_context->IsDisplayed(visible_))visible_->SetZLayer(Graphic3d_ZLayerId_Top);
				// projectAndDisplayWithHLR(vshapes);
		} else {
			cotm("hlr0") if (!visible_.IsNull()) {
				m_context->Remove(visible_, 0);
				visible_.Nullify();
			}
			// zghost();
		}
		currentMode = fromcurrentMode;
		// redraw();
	}

	TopoDS_Shape vEdges;
	TopoDS_Shape hEdges;
	Handle(HLRBRep_PolyAlgo) hlrAlgo;

	void fillvectopo() {
		vshapes.clear();
		//vshapes is not in sync vector
		// cotm(vaShape.size(), vshapes.size());
		for (int i = 0; i < vaShape.size(); i++) {
			if (!m_context->IsDisplayed(vaShape[i]) || !vlua[i]->visible_hardcoded) continue;

			TopoDS_Shape s=vaShape[i]->Shape();
			if (!vlua[i]->Origin.IsIdentity()) {

				s = s.Located(TopLoc_Location(
					vlua[i]->Origin.Transformation()
				));
			}

			vshapes.push_back(s); //not in sync

			// vshapes.push_back(vaShape[i]->Shape());
		}
		// cotm(vaShape.size(), vshapes.size());
	}
	void projectAndDisplayWithHLR(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false){
		if (!hlr_on)return;
		perf2();
		// projectAndDisplayWithHLR_lp_prev(shapes); 
		// projectAndDisplayWithHLR_P(shapes,isDragonly); 
		projectAndDisplayWithHLR_lp(shapes,isDragonly);
		// projectAndDisplayWithHLR_SilhouetteFast(shapes);
		// projectAndDisplayWithHLR_ntw(shapes,isDragonly);
		perf2("p2 hlr");
	}


// Fastest stable GPU-based version: only visible sharp edges + silhouettes (thick black lines)
// Real-time interactive, no hidden lines, no face fill
void projectAndDisplayWithHLR_lp_notwrking(const std::vector<TopoDS_Shape>& shapes, bool isDragonfly = false) {
    if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;
static Handle(AIS_InteractiveObject) visible_v;

    // White background for classic technical drawing look
    m_view->SetBackgroundColor(Quantity_NOC_WHITE);

    // Build compound for best performance and correct silhouette detection
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);

    bool hasShapes = false;
    for (const auto& s : shapes) {
        if (!s.IsNull()) {
            builder.Add(compound, s);
            hasShapes = true;
        }
    }

    if (!hasShapes) {
        if (!visible_v.IsNull()) {
            m_context->Erase(visible_v, Standard_True);
            visible_v.Nullify();
        }
        return;
    }

    Handle(AIS_InteractiveObject) aisObject;

    // Always create a new AIS_Shape - safest way to update shape and avoid issues
    aisObject = new AIS_Shape(compound);

    // Remove old one if exists (prevents duplication and memory issues)
    if (!visible_v.IsNull()) {
        m_context->Remove(visible_v, Standard_True);  // True = update viewer immediately
        visible_v.Nullify();
    }

    // Key settings for visible edges/silhouettes only
    Handle(Prs3d_Drawer) drawer = aisObject->Attributes();

    drawer->SetShadingModel(Graphic3d_TOSM_NONE);          // Disable shading completely

    drawer->SetFaceBoundaryDraw(Standard_True);            // Enable face boundaries (sharp edges + GPU silhouettes)

    // Thick black lines for visible outlines
    // drawer->FaceBoundaryAspect()->SetColor(Quantity_NOC_BLACK);
	// pausa
    // drawer->FaceBoundaryAspect()->SetWidth(3);

    // Optional: also draw free boundaries (open edges) if needed
    // drawer->SetFreeBoundaryDraw(Standard_True);
	
    aisObject->SetInfiniteState(Standard_True);

    // Display the new object
    m_context->Display(aisObject, Standard_False);  // False = no automatic fit

    visible_v = aisObject;  // Store for next call

}

void projectAndDisplayWithHLR_Fastest(
    const std::vector<TopoDS_Shape>& shapes,
    bool forceRebuild = false)
{
    if (!hlr_on || m_context.IsNull() || m_view.IsNull())
        return;

    static TopoDS_Shape cachedEdges;     // camera-space
    static bool built = false;

    const Handle(Graphic3d_Camera)& cam = m_view->Camera();

    // =========================================================
    // 1) EXACT HLR — RUN ONCE (or when geometry changes)
    // =========================================================
    if (!built || forceRebuild)
    {
        gp_Dir viewDir = -cam->Direction();
        gp_Dir viewRight = cam->Up().Crossed(viewDir);

        gp_Ax3 camAx(cam->Center(), viewDir, viewRight);
        gp_Trsf camTrsf;
        camTrsf.SetTransformation(camAx);

        HLRAlgo_Projector projector(
            camTrsf,
            !cam->IsOrthographic(),
            cam->Scale()
        );

        Handle(HLRBRep_Algo) algo = new HLRBRep_Algo();
        for (const auto& s : shapes)
            if (!s.IsNull())
                algo->Add(s, 0);

        algo->Projector(projector);
        algo->Update();
        algo->Hide();

        HLRBRep_HLRToShape toShape(algo);
        cachedEdges = toShape.VCompound();  // stored in camera space

        built = true;
    }

    // =========================================================
    // 2) ULTRA-FAST REDRAW — ONLY TRANSFORM
    // =========================================================
    gp_Dir viewDir = -cam->Direction();
    gp_Dir viewRight = cam->Up().Crossed(viewDir);

    gp_Ax3 camAx(cam->Center(), viewDir, viewRight);
    gp_Trsf camTrsf;
    camTrsf.SetTransformation(camAx);

    gp_Trsf inv = camTrsf.Inverted();

    TopoDS_Shape result =
        BRepBuilderAPI_Transform(cachedEdges, inv).Shape();

    // =========================================================
    // 3) DISPLAY
    // =========================================================
    if (visible_)
        m_context->Remove(visible_, Standard_False);

    visible_ = new AIS_NonSelectableShape(result);
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(2.2);

    m_context->Display(visible_, Standard_False);
    visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
    m_context->Deactivate(visible_);
}
void projectAndDisplayWithHLR_SilhouetteFast(
    const std::vector<TopoDS_Shape>& shapes)
{
    if (!hlr_on || m_context.IsNull() || m_view.IsNull())
        return;

    static TopoDS_Shape cachedResult;
    static gp_Dir lastDir(0, 0, 1);
    static bool initialized = false;

    const Handle(Graphic3d_Camera)& cam = m_view->Camera();

    // ---------------------------------------------------------
    // 1) Detect meaningful rotation (silhouette change)
    // ---------------------------------------------------------
    gp_Dir viewDir = -cam->Direction();
    Standard_Real ang =
        initialized ? viewDir.Angle(lastDir) : M_PI;

    // threshold ~ 0.3°  (tune: 0.2–1.0)
    const Standard_Real ROT_EPS = 0.005;

    const bool needRebuild = !initialized || ang > ROT_EPS;

    // ---------------------------------------------------------
    // 2) Rebuild HLR only when needed
    // ---------------------------------------------------------
    if (needRebuild)
    {
        // copy + clean + mesh (safe)
        Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();

        for (const auto& s : shapes)
        {
            if (s.IsNull()) continue;

            TopoDS_Shape copy =
                BRepBuilderAPI_Copy(s).Shape();

            BRepTools::Clean(copy);

            BRepMesh_IncrementalMesh(
                copy,
                0.1,      // deflection
                Standard_False,
                0.5,
                Standard_True
            );

            algo->Load(copy);
        }

        // camera-aligned projection
        gp_Dir right = cam->Up().Crossed(viewDir);
        gp_Ax3 ax(cam->Center(), viewDir, right);

        gp_Trsf viewTrsf;
        viewTrsf.SetTransformation(ax);

        HLRAlgo_Projector projector(
            viewTrsf,
            !cam->IsOrthographic(),
            cam->Scale()
        );

        algo->Projector(projector);
        algo->Update();

        HLRBRep_PolyHLRToShape toShape;
        toShape.Update(algo);

        cachedResult =
            BRepBuilderAPI_Transform(
                toShape.VCompound(),
                viewTrsf.Inverted()).Shape();

        lastDir = viewDir;
        initialized = true;
    }

    // ---------------------------------------------------------
    // 3) Ultra-fast display (no recompute)
    // ---------------------------------------------------------
    if (visible_)
        m_context->Remove(visible_, Standard_False);

    visible_ = new AIS_NonSelectableShape(cachedResult);
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(2.2);

    m_context->Display(visible_, Standard_False);
    visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
    m_context->Deactivate(visible_);
}


	// Fastest stable version for OCCT 7.9.x (PolyHLR with max internal parallelism)
	bool is_first=0;
void projectAndDisplayWithHLR_lp(const std::vector<TopoDS_Shape>& shapes, bool isDragonfly = false) {
    if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;

	if (visible_) m_context->Remove(visible_, 0);
    
	// 1. Camera transformation setup (kept your working version)
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    gp_Dir viewDir = -camera->Direction();
    gp_Dir viewUp = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);
    gp_Ax3 viewAxes(gp_Pnt(0, 0, 0), viewDir, viewRight);

    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);
    gp_Trsf invTrsf = viewTrsf.Inverted();

    // 2. Projector
    HLRAlgo_Projector projector(viewTrsf, !camera->IsOrthographic(), camera->Scale());

    // 3. Meshing - use OCCT's built-in parallel mode (fastest & thread-safe)
    // Tune deflection higher (e.g. 0.01 - 0.1) for much faster meshing + HLR Update()
    // Lower values = more precision, many more polygons → slower Update()
    Standard_Real deflection = 0.2;  // Adjust this for your speed/quality tradeoff

    Standard_Real angularDeflection = 0.5;  // radians, controls curve discretization

// std::vector<TopoDS_Shape> cpyshapes;
// cpyshapes.reserve(shapes.size());

// for (const auto& s : shapes)
// {
//     if (s.IsNull()) continue;
//     cpyshapes.push_back(
//         BRepBuilderAPI_Copy(s).Shape()
//     );
// }

// perf();

	// int ailoci=0;
	// if(!is_first){is_first=1;
    for (  auto& s : shapes) {
        if (!s.IsNull()) {
			BRepTools::Clean(s);






            // Optional: skip if already meshed sufficiently (your commented check)
            // For maximum speed, you can force remesh or skip check
            BRepMesh_IncrementalMesh(s, deflection, Standard_False, angularDeflection, Standard_True);
        }
    }
// }
// perf("brepclean");

    // 4. HLR computation (sequential - no parallelism available in OCCT HLR)
    Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();

    for (const auto& s : shapes) {
        if (!s.IsNull()) {
            algo->Load(s);
        }
    }

    algo->Projector(projector);
    algo->Update();  // This is the main bottleneck - coarser mesh = much faster here

    // 5. Extract visible edges and transform back
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(algo);
	algo.Nullify();

    TopoDS_Shape vEdges = hlrToShape.VCompound();  // Visible sharp edges (adjust for other types if needed)
    BRepBuilderAPI_Transform visT(vEdges, invTrsf);
    TopoDS_Shape result = visT.Shape();


		visible_ = new AIS_NonSelectableShape(result); 
		visible_->SetColor(Quantity_NOC_BLACK);
		visible_->SetWidth(2.2);
		m_context->Display(visible_, false);
		visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
        m_context->Deactivate(visible_);
		return;





    // 6. Display / update AIS_Shape
    if (!visible_.IsNull()) {
        if (!visible_->Shape().IsEqual(result)) {
            m_context->Deactivate(visible_);
            visible_->SetShape(result);
            visible_->SetColor(Quantity_NOC_BLACK);
            visible_->SetWidth(3);
            visible_->SetInfiniteState(true);
            m_context->Redisplay(visible_, false);
        }
    } else {
        visible_ = new AIS_NonSelectableShape(result);
		m_context->Deactivate(visible_);
        visible_->SetColor(Quantity_NOC_BLACK);
        visible_->SetWidth(3);
        visible_->SetInfiniteState(true);
        m_context->Display(visible_, false);
    }
	visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
	m_context->Deactivate(visible_);
}
	// #define ENABLE_PARALLEL 1
	void projectAndDisplayWithHLR_ntw(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false) {
		if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;
		// perf1();

		// 1. Preparar transformação da câmara
		const Handle(Graphic3d_Camera) & camera = m_view->Camera();
		gp_Dir viewDir = -camera->Direction();
		gp_Dir viewUp = camera->Up();
		gp_Dir viewRight = viewUp.Crossed(viewDir);
		gp_Ax3 viewAxes(gp_Pnt(0, 0, 0), viewDir, viewRight);
		// gp_Ax3 viewAxes(gp_Pnt(0, 0, 0), viewDir, viewRight);

		gp_Trsf viewTrsf;
		viewTrsf.SetTransformation(viewAxes);
		gp_Trsf invTrsf = viewTrsf.Inverted();

		// const Handle(Graphic3d_Camera)& camera = m_view->Camera();
		// gp_Dir viewDir = -camera->Direction();
		// gp_Dir viewUp = camera->Up();
		// gp_Dir viewRight = viewUp.Crossed(viewDir);
		// gp_Ax3 viewAxes(camera->Center(), viewDir, viewRight);

		// gp_Trsf viewTrsf;
		// viewTrsf.SetTransformation(viewAxes);
		// gp_Trsf invTrsf = viewTrsf.Inverted();

		// const Handle(Graphic3d_Camera)& theProjector = m_view->Camera();
		// gp_Dir aBackDir = -theProjector->Direction();
		// gp_Dir aXpers   = theProjector->Up().Crossed (aBackDir);
		// gp_Ax3 anAx3 (theProjector->Center(), aBackDir, aXpers);
		// gp_Trsf viewTrsf;
		// viewTrsf.SetTransformation (anAx3);
		// gp_Trsf invTrsf = viewTrsf.Inverted();

		// 2. Criar projetor HLR
		// projector = HLRAlgo_Projector(viewTrsf,
		// !theProjector->IsOrthographic(), theProjector->Scale());
		projector = HLRAlgo_Projector(viewTrsf, !camera->IsOrthographic(), camera->Scale());

		// 3. Meshing (somente se ainda não estiver meshed)
		Standard_Real deflection = 0.001;

// perf();
#ifdef ENABLE_PARALLEL
		std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const TopoDS_Shape& s) {
#else
		for (const auto& s : shapes) {
#endif
			if (s.IsNull()) return;

			bool needsMesh = false;
			// for (TopExp_Explorer exp(s, TopAbs_FACE);
			// exp.More(); exp.Next()) {
			//     TopLoc_Location loc;
			//     const TopoDS_Face& face =
			//     TopoDS::Face(exp.Current()); if
			//     (BRep_Tool::Triangulation(face, loc).IsNull())
			//     {
			//         needsMesh = true;
			//         break;
			//     }
			// }
			needsMesh = true;
			if (needsMesh) {
				BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
			}
#ifdef ENABLE_PARALLEL
		});
#else
		}
#endif
		// 4. Projeção HLR
		Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();
#ifdef ENABLE_PARALLEL
		std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const TopoDS_Shape& s) {
#else
		for (const auto& s : shapes) {
#endif
			if (!s.IsNull()) algo->Load(s);
#ifdef ENABLE_PARALLEL
		});
#else
		}
#endif
		// perf("hlrload");

		algo->Projector(projector);
		algo->Update();
		// perf("hlrupdate");

		// 5. Converter para shape visível
		HLRBRep_PolyHLRToShape hlrToShape;
		perf();
		hlrToShape.Update(algo);
		perf("hlrupdate");
		TopoDS_Shape vEdges = hlrToShape.VCompound();
		BRepBuilderAPI_Transform visT(vEdges, invTrsf);
		TopoDS_Shape result = visT.Shape();

		// 6. Mostrar ou atualizar AIS_Shape
		if (!visible_.IsNull()) {
			if (!visible_->Shape().IsEqual(result)) {
				visible_->SetShape(result);
				visible_->SetColor(Quantity_NOC_BLACK);
				visible_->SetWidth(3);
				visible_->SetInfiniteState(true);  // opcional
				m_context->Redisplay(visible_, false);
			}
		} else {
			visible_ = new AIS_NonSelectableShape(result);
			visible_->SetColor(Quantity_NOC_BLACK);
			visible_->SetWidth(3);
			visible_->SetInfiniteState(true);  // opcional
			m_context->Display(visible_, false);
		}
		// perf1("elapsed hlr1");
	}

	// #define ENABLE_PARALLEL 1
	// less precision
	void projectAndDisplayWithHLR_lp_prev(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false) {
		if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;
		// perf1();

		// 1. Preparar transformação da câmara
		const Handle(Graphic3d_Camera) & camera = m_view->Camera();
		gp_Dir viewDir = -camera->Direction();
		gp_Dir viewUp = camera->Up();
		gp_Dir viewRight = viewUp.Crossed(viewDir);
		gp_Ax3 viewAxes(gp_Pnt(0, 0, 0), viewDir, viewRight);
		// gp_Ax3 viewAxes(gp_Pnt(0, 0, 0), viewDir, viewRight);

		gp_Trsf viewTrsf;
		viewTrsf.SetTransformation(viewAxes);
		gp_Trsf invTrsf = viewTrsf.Inverted();

		// const Handle(Graphic3d_Camera)& camera = m_view->Camera();
		// gp_Dir viewDir = -camera->Direction();
		// gp_Dir viewUp = camera->Up();
		// gp_Dir viewRight = viewUp.Crossed(viewDir);
		// gp_Ax3 viewAxes(camera->Center(), viewDir, viewRight);

		// gp_Trsf viewTrsf;
		// viewTrsf.SetTransformation(viewAxes);
		// gp_Trsf invTrsf = viewTrsf.Inverted();

		// const Handle(Graphic3d_Camera)& theProjector = m_view->Camera();
		// gp_Dir aBackDir = -theProjector->Direction();
		// gp_Dir aXpers   = theProjector->Up().Crossed (aBackDir);
		// gp_Ax3 anAx3 (theProjector->Center(), aBackDir, aXpers);
		// gp_Trsf viewTrsf;
		// viewTrsf.SetTransformation (anAx3);
		// gp_Trsf invTrsf = viewTrsf.Inverted();

		// 2. Criar projetor HLR
		// projector = HLRAlgo_Projector(viewTrsf,
		// !theProjector->IsOrthographic(), theProjector->Scale());
		projector = HLRAlgo_Projector(viewTrsf, !camera->IsOrthographic(), camera->Scale());

		// 3. Meshing (somente se ainda não estiver meshed)
		Standard_Real deflection = 0.001;

// perf();
#ifdef ENABLE_PARALLEL
		std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const TopoDS_Shape& s) {
#else
		for (const auto& s : shapes) {
#endif
			if (s.IsNull()) return;

			bool needsMesh = false;
			// for (TopExp_Explorer exp(s, TopAbs_FACE);
			// exp.More(); exp.Next()) {
			//     TopLoc_Location loc;
			//     const TopoDS_Face& face =
			//     TopoDS::Face(exp.Current()); if
			//     (BRep_Tool::Triangulation(face, loc).IsNull())
			//     {
			//         needsMesh = true;
			//         break;
			//     }
			// }
			needsMesh = true;
			if (needsMesh) {
				BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
			}
#ifdef ENABLE_PARALLEL
		});
#else
		}
#endif
		// 4. Projeção HLR
		Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();
#ifdef ENABLE_PARALLEL
		std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const TopoDS_Shape& s) {
#else
		for (const auto& s : shapes) {
#endif
			if (!s.IsNull()) algo->Load(s);
#ifdef ENABLE_PARALLEL
		});
#else
		}
#endif
		// perf("hlrload");

		algo->Projector(projector);
		// perf();
		algo->Update();
		// perf("hlrupdate");

		// 5. Converter para shape visível
		HLRBRep_PolyHLRToShape hlrToShape;
		hlrToShape.Update(algo);
		TopoDS_Shape vEdges = hlrToShape.VCompound();
		BRepBuilderAPI_Transform visT(vEdges, invTrsf);
		TopoDS_Shape result = visT.Shape();

		// 6. Mostrar ou atualizar AIS_Shape
		if (!visible_.IsNull()) {
			if (!visible_->Shape().IsEqual(result)) {
				m_context->Deactivate(visible_);//
				visible_->SetShape(result);
				visible_->SetColor(Quantity_NOC_BLACK);
				visible_->SetWidth(3);
				visible_->SetInfiniteState(true);  // opcional
				m_context->Redisplay(visible_, false);
			}
		} else {
			visible_ = new AIS_NonSelectableShape(result);
			m_context->Deactivate(visible_);//
			visible_->SetColor(Quantity_NOC_BLACK);
			visible_->SetWidth(3);
			visible_->SetInfiniteState(true);  // opcional
			m_context->Display(visible_, false);
		}
		// perf1("elapsed hlr1");
	}

	// precision
	void projectAndDisplayWithHLR_P(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false) {
		if (!hlr_on) return;
		// perf();
		// cotm("hlr")
		// m_view->SetImmediateUpdate(Standard_False);
		if (m_context.IsNull() || m_view.IsNull()) {
			std::cerr << "Error: m_context or m_view is null." << std::endl;
			return;
		}
		// m_view->SetComputedMode(Standard_True);

		{
			// m_context->RemoveAll(false);
			// lop(i,0,vaShape.size()){
			//     m_context->Remove(vaShape[i],0);
			// }

			if (visible_) m_context->Remove(visible_, 0);
			// if (hidden_) m_context->Remove(hidden_, 0);
		}

		const Handle(Graphic3d_Camera) & theProjector = m_view->Camera();

		gp_Dir aBackDir = -theProjector->Direction();
		gp_Dir aXpers = theProjector->Up().Crossed(aBackDir);
		gp_Ax3 anAx3(theProjector->Center(), aBackDir, aXpers);
		gp_Trsf aTrsf;
		aTrsf.SetTransformation(anAx3);

		HLRAlgo_Projector projector(aTrsf, !theProjector->IsOrthographic(), theProjector->Scale());

		Handle(HLRBRep_Algo) hlrAlgo = new HLRBRep_Algo();
		for (const auto& shp : shapes) {
			if (!shp.IsNull()) hlrAlgo->Add(shp, 0);
		}

		hlrAlgo->Projector(projector);
		// hlrAlgo-> ShowAll();
		// perf();
		hlrAlgo->Update();
		// perf("hlrAlgo->Update()");
		static bool tes = 0;
		// if(!tes)
		hlrAlgo->Hide();
		// perf("hide");
		tes = 1;

		HLRBRep_HLRToShape hlrToShape_(hlrAlgo);
		// perf("hlrToShape");
		TopoDS_Shape vEdges_ = hlrToShape_.VCompound();
		// TopoDS_Shape hEdges_ = hlrToShape_.HCompound();
		// perf("hlrAlgo->HCompound()");

		gp_Trsf invTrsf = aTrsf.Inverted();
		BRepBuilderAPI_Transform transformer(vEdges_, invTrsf);
		TopoDS_Shape transformedShape = transformer.Shape();

		visible_ = new AIS_NonSelectableShape(transformedShape);

		// cotm2(90)
        // m_context->Deactivate(visible_);
		// visible_ = new AIS_Shape(transformedShape);
		visible_->SetColor(Quantity_NOC_BLACK);
		visible_->SetWidth(2.2);
		m_context->Display(visible_, false);
		visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
        m_context->Deactivate(visible_);

		// Handle(Prs3d_LineAspect) lineAspect = new
		// Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 0.5);
		// // Handle(Prs3d_Drawer) aDrawer = new Prs3d_Drawer();
		// // hEdges_.SetAttributes(aDrawer);

		// BRepBuilderAPI_Transform transformerh(hEdges_, invTrsf);
		// TopoDS_Shape transformedShapeh = transformerh.Shape();

		// hidden_ = new AIS_Shape(transformedShapeh);

		// Handle(Prs3d_LineAspect) dashedAspect = new
		// Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0);

		// hidden_->Attributes()->SetSeenLineAspect(dashedAspect);

		// m_context->Display(hidden_, 0);

		//  setbar5per();
		// perf("hlr slower");
	}

#pragma endregion projection
#pragma region tests
	void test2() {
#if 0
		perf();
		//test
		luadraw* test=new luadraw("consegui",this);
		// test->visible_hardcoded=0;
		// test->translate(10);
		// test.translate(0,10);
		// test.rotate(90);
		test->dofromstart();
		test->redisplay();
 
		
		luadraw* test1=new luadraw("test1",this);
		test1->visible_hardcoded=0;
		test1->shape=test->shape;
		test1->extrude(20);		
		test1->translate(100.0);
		test1->redisplay();
		
		luadraw* test2=new luadraw("test2",this);
		test2->visible_hardcoded=0;
		test2->shape=test->shape;
		test2->extrude(20);
		test2->translate(100,10);	
		test2->rotate(40,0,0,1);	
		test2->redisplay();

		
		luadraw* test3=new luadraw("test3",this);
		test3->fuse(*test1,*test2);
		test3->visible_hardcoded=0;
		// test3->shape
		// test2->extrude(-20);
		// test2->translate(100,10);	
		// test2->rotate(40);	
		test3->redisplay();

		luadraw* test4=new luadraw("test4",this);
		test4->shape=test3->shape;
		test4->visible_hardcoded=0;
		// test3->fuse(*test1,*test2);
		// test3->shape
		// test2->extrude(-20);
		test4->translate(100,10);	
		// test2->rotate(40);	
		test4->redisplay();
		
		
// perf(); 
		luadraw* test5=new luadraw("test5",this);
		test5->fuse(*test3,*test4);
		test5->redisplay();
		

		test1->display();
		test1->translate(0,0,100);
		test1->visible_hardcoded=1;
		test1->redisplay();
		// lop(i,0,140){
		// luadraw* test6=new luadraw("test5"+to_string(i),this);
		// test6->shape=test5->shape;
		// test6->rotate(i);
		// test6->redisplay();
		// }


		// vshapes.push_back(test.shape);
// m_context->UpdateCurrentViewer();

perf();
fillvectopo();
perf("bench");

toggle_shaded_transp(currentMode);

		// OCC_Viewer::luadraw* tt= ulua["test5"];
		// m_context->Remove(tt->ashape,0);
		// delete tt;

#endif
	}

	void test(int rot = 45) {
		TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape();
		TopoDS_Shape pl1 = pl();

		gp_Trsf trsf;
		trsf.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)),
						 rot * (M_PI / 180));  // rotate 30 degrees around Z-axis
		BRepBuilderAPI_Transform transformer(box, trsf);
		TopoDS_Shape rotatedShape = transformer.Shape();

		// TopoDS_Shape fusedShape = BRepAlgoAPI_Fuse(rotatedShape, box);

		// BRepAlgoAPI_Common   fuse(rotatedShape, box);
		// BRepAlgoAPI_Cut  fuse(rotatedShape, box);
		BRepAlgoAPI_Fuse fuse(rotatedShape, box);
		fuse.Build();
		if (!fuse.IsDone()) {
			// handle error
		}
		TopoDS_Shape fusedShape = fuse.Shape();

		// Refine the result
		ShapeUpgrade_UnifySameDomain unify(fusedShape, true, true, true);  // merge faces, edges, vertices
		unify.Build();
		TopoDS_Shape refinedShape = unify.Shape();

		if(1){
			// gp_Pnt origin=gp_Pnt(50, 86.6025, 0);
			// gp_Dir normal=gp_Dir(0.5, 0.866025, 0);
			gp_Pnt origin = gp_Pnt(0, 0, 200);//
			gp_Dir normal = gp_Dir(0, 0, 1);
			gp_Dir xdir = gp_Dir(0, 1, 0);
			gp_Dir ydir = gp_Dir(0.1, -0.5, 0);
			// gp_Dir ydir =   gp_Dir(0.866025, -0.5, 0);
			// gp_Ax2 ax3(origin, normal);
			gp_Ax2 ax3(origin, normal, xdir);
			// ax3.SetYDirection(ydir);
			gp_Trsf trsf;
			trsf.SetTransformation(ax3);
			trsf.Invert();

			gp_Trsf trsftmp;
			trsftmp.SetRotation(gp_Ax1(origin, normal), -45 * (M_PI / 180));
			trsf *= trsftmp;

			trsftmp = gp_Trsf();
			trsftmp.SetTranslation(gp_Vec(20, 0, 0));
			trsf *= trsftmp;

			ShowTriedronWithoutLabels(trsf, m_context);	 // debug

			gp_Pnt p1(0, 0, 0);
			gp_Pnt p2(100, 0, 0);
			gp_Pnt p3(100, 50, 0);
			gp_Pnt p4(0, 50, 0);

			// p1 = origin.Translated(gp_Vec(xdir) * (-0) + gp_Vec(ydir) *
			// (-0)); p2 = origin.Translated(gp_Vec(xdir) * (p2.X()) +
			// gp_Vec(ydir) * (p2.Y())); p3 = origin.Translated(gp_Vec(xdir) *
			// (p3.X()) + gp_Vec(ydir) * (p3.Y())); p4 =
			// origin.Translated(gp_Vec(xdir) * (p4.X()) + gp_Vec(ydir) *
			// (p4.Y()));

			// p1.Transform(trsf);
			// p2.Transform(trsf);
			// p3.Transform(trsf);
			// p4.Transform(trsf);

			// p1.Transform(trsf.Inverted());
			// p2.Transform(trsf.Inverted());
			// p3.Transform(trsf.Inverted());
			// p4.Transform(trsf.Inverted());

			// p1=point_on_plane(p1,ploc,pdir);
			// p2=point_on_plane(p2,ploc,pdir);
			// p3=point_on_plane(p3,ploc,pdir);
			// p4=point_on_plane(p4,ploc,pdir);

			// Criar arestas
			TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
			TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
			TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
			TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p1);

			// Fazer o wire
			BRepBuilderAPI_MakeWire wireBuilder;
			wireBuilder.Add(e1);
			wireBuilder.Add(e2);
			wireBuilder.Add(e3);
			wireBuilder.Add(e4);
			TopoDS_Wire wire = wireBuilder.Wire();

			// Criar a face sobre o plano com o wire
			// gp_Dir xdir =   gp_Dir(0, 0, 1); // produto vetorial —
			// perpendicular a pdir gp_Dir xdir = pdir ^ gp_Dir(0, 0, 1); //
			// produto vetorial — perpendicular a pdir

			// gp_Ax3 ax3(ploc,pdir,xdir);
			// Handle(Geom_Plane) plane = new Geom_Plane(ploc,pdir);
			// Handle(Geom_Plane) plane = new Geom_Plane(ax3);
			// gp_Pnt origin = plane->Location();
			// printf("origin: x=%.3f y=%.3f
			// z=%.3f\n",origin.X(),origin.Y(),origin.Z());
			TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);
			// TopoDS_Face face = BRepBuilderAPI_MakeFace(plane, wire);

			BRepBuilderAPI_Transform transformer(face, trsf);
			TopoDS_Shape rotatedShape = transformer.Shape();

			gp_Ax3 ax3_;
			ax3_.Transform(trsf);  // aplica trsf no sistema de coordenadas padrão
			// gp_Dir dir = ax3.Direction();
			gp_Dir dir = [](gp_Trsf trsf) {
				gp_Ax3 ax3;
				ax3.Transform(trsf);
				return ax3.Direction();
			}(trsf);

			gp_Vec extrusionVec(dir);
			// gp_Vec extrusionVec(normal);
			extrusionVec *= 10.0;
			TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(rotatedShape, extrusionVec).Shape();
			// TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(face,
			// extrusionVec).Shape();

			vshapes.push_back(extrudedShape);
			// vshapes.push_back(rotatedShape);
			// vshapes.push_back(face);
		}

		// vshapes.push_back(pl1);
		vshapes.push_back(refinedShape);
		// vshapes=std::vector<TopoDS_Shape>{pl1,face};
		// vshapes=std::vector<TopoDS_Shape>{refinedShape,pl1,face};

		// {
		// draw_objs();
		// m_view->FitAll();
		// redraw();
		// }
	}

#pragma endregion tests

#pragma region view_rotate

void FitViewToShape(const Handle(V3d_View)& aView,
                    const TopoDS_Shape& aShape,
                    double margin = 1.0,
                    double zoomFactor = 1.0){
    if (aShape.IsNull() || aView.IsNull()) return;

    // Compute bounding box
    Bnd_Box bbox;
    BRepBndLib::Add(aShape, bbox);
    bbox.SetGap(margin);

    if (bbox.IsVoid()) return;

    // Fit to bounding box
    aView->FitAll(bbox, 0.01, false);

    // Apply zoom scaling
    if (zoomFactor != 1.0)
    {
        aView->SetZoom(zoomFactor, false);  // no "Start" toggle; just set
    }

    aView->Redraw();
}


	void colorisebtn(int idx = -1) {
		int idx2 = -1;
		if (idx == -1) {
			vint vidx = check_nearest_btn_idx();
			if (vidx.size() == 0) return;
			idx = vidx[0];
			if (vidx.size() > 1) idx2 = vidx[1];
			if (idx < 0) return;
		}
		lop(i, 0, sbt.size()) {
			if (i == idx) {
				sbt[i].occbtn->color(FL_RED);
				sbt[i].occbtn->redraw();
			} else if (idx2 >= 0 && idx2 == i) {
				sbt[i].occbtn->color(fl_rgb_color(255, 165, 0));
				sbt[i].occbtn->redraw();
			} else {
				sbt[i].occbtn->color(FL_BACKGROUND_COLOR);
				sbt[i].occbtn->redraw();
			}
		}
	}

	struct sbts {
		string label;
		std::function<void()> func;
		bool is_setview = 0;
		struct vs {
			Standard_Real dx = 0, dy = 0, dz = 0, ux = 0, uy = 0, uz = 0;
		} v;
		int idx;
		Fl_Button* occbtn;
		OCC_Viewer* occv;

		static void call(Fl_Widget*, void* data) {
			auto* wrapper = static_cast<sbts*>(data);
			auto& occv = wrapper->occv;
			auto& m_view = wrapper->occv->m_view;
			cotm2("an1")
			if (wrapper->is_setview) {
				// cotm(1);
				// m_view->SetProj(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
				// cotm(2)
				// m_view->SetUp(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz);

				occv->end_proj_global = gp_Vec(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
				occv->end_up_global = gp_Vec(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz);
			cotm2("an2")
				occv->colorisebtn(wrapper->idx);
				occv->start_animation(occv);
			cotm2("an3")
				// occv->animate_flip_view(occv);
			}
			if (wrapper && wrapper->func) wrapper->func();
			// cotm(wrapper->label);

			// // Later, retrieve them
			// Standard_Real dx, dy, dz, ux, uy, uz;
			// m_view->Proj(dx, dy, dz);
			// m_view->Up(ux, uy, uz);
			cotm(dx, dy, dz, ux, uy, uz);
		}

		// 🔍 Find a pointer to sbts by label: sbts* found =
		// sbts::find_by_label(sbt, "T");
		static sbts* find_by_label(vector<sbts>& vec, const string& target) {
			for (auto& item : vec) {
				if (item.label == target) return &item;
			}
			return nullptr;
		}
	};

	vector<sbts> sbt;
	void sbtset(bool zdirup=0){
		sbt.clear();

		vector<sbts> sbty = {
			// Name    {}  ID   {  Dir X, Y, Z,   Up X, Y, Z  }
			sbts{"Front",     {}, 1, {  0,  0,  1,   0, 1,  0 }},
			sbts{"Back",      {}, 1, {  0,  0, -1,   0, 1,  0 }},
			sbts{"Top",       {}, 1, {  0,  1,  0,   1, 0,  0 }},
			sbts{"Bottom",    {}, 1, {  0, -1,  0,  -1, 0,  0 }},
			sbts{"Left",      {}, 1, { -1,  0,  0,   0, 1,  0 }},
			sbts{"Right",     {}, 1, {  1,  0,  0,   0, 1,  0 }},
			sbts{"Iso",       {}, 1, { -1,  1,  1,   0, 1,  0 }},
			sbts{"Isor",      {}, 1, {  1,  1, -1,   0, 1,  0 }},
		};

		vector<sbts> sbtz = {
			// Name     {}  ID   {  Dir X, Y, Z,   Up X, Y, Z  }
			sbts{"Front",    {}, 1, {  0,  1,  0,   0, 0,  1 }},
			sbts{"Back",     {}, 1, {  0, -1,  0,   0, 0,  1 }},
			sbts{"Top",      {}, 1, {  0,  0,  1,   0, 1,  0 }},
			sbts{"Bottom",   {}, 1, {  0,  0, -1,   0, 1,  0 }},
			sbts{"Left",     {}, 1, { -1,  0,  0,   0, 0,  1 }},
			sbts{"Right",    {}, 1, {  1,  0,  0,   0, 0,  1 }},
			sbts{"Iso",      {}, 1, { -1,  1,  1,   0, 0,  1 }},
			sbts{"Isor",     {}, 1, {  1, -1,  1,   0, 0,  1 }},
		};




		if(zdirup==0)
			sbt=sbty;
		if(zdirup==1)
			sbt=sbtz;

		vector<sbts> sbtstd = {
			sbts{"Invert d",
				 [this] {
					 Standard_Real dx, dy, dz, ux, uy, uz;
					 m_view->Proj(dx, dy, dz);
					 m_view->Up(ux, uy, uz);
					 // Reverse the projection direction
					 end_proj_global = gp_Vec(-dx, -dy, -dz);
					 end_up_global = gp_Vec(ux, uy, uz);
					 start_animation(this);
				 }},

			sbts{"Align",
				 [this] { 
					 GetAlignedCameraVectors(m_view,end_proj_global,end_up_global); 
					//  cotm2(end_proj_global,end_up_global);
					 start_animation(this);
				 }}
		};

		sbt.insert(sbt.end(), sbtstd.begin(), sbtstd.end());
	}
	void drawbuttons(float w, int hc1) {
		// auto& sbt = occv->sbt;
		// auto& sbts = occv->sbts;

		std::function<void()> func = [this, &sbt = this->sbt] { cotm(m_initialized, sbt[0].label) };

		sbtset(0);

		// sbt = {
		// 	sbts{"Front", {}, 1, {0, -1, 0, 0, 0, 1}}, 
		// 	sbts{"Top", {}, 1, {0, 0, 1, 0, 1, 0}},
		// 	sbts{"Left", {}, 1, {-1, 0, 0, 0, 0, 1}}, 
		// 	sbts{"Right", {}, 1, {1, 0, 0, 0, 0, 1}},
		// 	sbts{"Back", {}, 1, {0, 1, 0, 0, 0, 1}}, 
		// 	sbts{"Bottom", {}, 1, {0, 0, -1, 0, 1, 0}},
		// 	// sbts{"Bottom", {}, 1, {0, 0, -1, 0, -1, 0}}, //iso but dont like it
		// 	sbts{"Iso", {}, 1, {0.57735, -0.57735, 0.57735, -0.408248, 0.408248, 0.816497}},

		// 	// sbts{"Iso",{}, 1, { 1,  1,  1,   0,  0,  1 }},

		// 	// old standard
		// 	//  sbts{"Front",     {}, 1, {  0,  0,  1,   0, 1,  0 }},
		// 	//  sbts{"Back",      {}, 1, {  0,  0, -1,   0, 1,  0 }},
		// 	//  sbts{"Top",       {}, 1, {  0, -1,  0,   0, 0, -1 }},
		// 	//  sbts{"Bottom",    {}, 1, {  0,  1,  0,   0, 0,  1 }},
		// 	//  sbts{"Left",      {}, 1, {  1,  0,  0,   0, 1,  0 }},
		// 	//  sbts{"Right",     {}, 1, { -1,  0,  0,   0, 1,  0 }},
		// 	//  sbts{"Isometric", {}, 1, { -1,  1,  1,   0, 1,  0 }},
		// 	sbts{"Iso z", {}, 1, {-1, 1, 1, 0, 1, 0}},
		// 	sbts{"Iso zr", {}, 1, {1, 1, -1, 0, 1, 0}},

		// 	// sbts{"T",[this,&sbt = this->sbt]{ cotm(sbt[0].label)   }},

		// 	// sbts{"Invert",
		// 	// 	 [this] {
		// 	// 		 Standard_Real dx, dy, dz, ux, uy, uz;
		// 	// 		 m_view->Proj(dx, dy, dz);
		// 	// 		 m_view->Up(ux, uy, uz);
		// 	// 		 // Reverse the projection direction
		// 	// 		 end_proj_global = gp_Vec(-dx, -dy, -dz);
		// 	// 		 end_up_global = gp_Vec(-ux, -uy, -uz);
		// 	// 		 start_animation(this);
		// 	// 	 }},

		// 	sbts{"Invert d",
		// 		 [this] {
		// 			 Standard_Real dx, dy, dz, ux, uy, uz;
		// 			 m_view->Proj(dx, dy, dz);
		// 			 m_view->Up(ux, uy, uz);
		// 			 // Reverse the projection direction
		// 			 end_proj_global = gp_Vec(-dx, -dy, -dz);
		// 			 end_up_global = gp_Vec(ux, uy, uz);
		// 			 start_animation(this);
		// 		 }},

		// 	sbts{"Align",
		// 		 [this] { 
		// 			 GetAlignedCameraVectors(m_view,end_proj_global,end_up_global); 
		// 			 start_animation(this);
		// 		 }},

		// 	// sbts{"Invertan",[this]{
		// 	//     animate_flip_view(this);
		// 	// }},
		// 	// sbts{"Ti",func }
		// 	// add more if needed
		// };

		float w1 = ceil(w / sbt.size()) + 0.0;

		lop(i, 0, sbt.size()) {
			sbt[i].occv = this;
			sbt[i].idx = i;
			sbt[i].occbtn = new Fl_Button(w1 * i, 0, w1, hc1, sbt[i].label.c_str());
			sbt[i].occbtn->callback(sbts::call, &sbt[i]);  // ✅ fixed here
		}
	}


// rotate_around_visible_shapes.cpp
// Rotate around ONLY the shapes currently visible in the viewport.
// Uses OCCT rectangular picking on the full viewport to find visible AIS_Shapes,
// computes their bbox center, and pivots around that. Also keeps eye-to-center
// distance consistent to avoid "flying away".

#include <FL/Fl.H>

#include <AIS_AnimationCamera.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

#include <Aspect_Window.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include <Graphic3d_Camera.hxx>

#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_SelectableObject.hxx>
#include <SelectMgr_ViewerSelector.hxx>

#include <V3d_View.hxx>

#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

// ---- your app types (assumed) ----
// OCC_Viewer must expose at least:
//   Handle(V3d_View) m_view;
//   Handle(AIS_InteractiveContext) m_context;
//   std::vector<TopoDS_Shape> vshapes;  // optional – not strictly required here
//   gp_Vec end_proj_global, end_up_global;
//   Handle(AIS_AnimationCamera) CurrentAnimation;
//   void colorisebtn();
//   void projectAndDisplayWithHLR(const std::vector<TopoDS_Shape>&);
//   void redraw();
// struct OCC_Viewer;

// static void animation_update(void* userdata);

// ---- Compute center of shapes actually visible in the viewport (without changing selection) ----
#include <AIS_Shape.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <AIS_DisplayStatus.hxx>
#include <SelectMgr_ViewerSelector.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepBndLib.hxx>
#include <V3d_View.hxx>

static gp_Pnt shapeCentroidWorld(const TopoDS_Shape& shp)
{
    GProp_GProps props;
    BRepGProp::VolumeProperties(shp, props);
    if (props.Mass() > 0.0) return props.CentreOfMass();

    BRepGProp::SurfaceProperties(shp, props);
    if (props.Mass() > 0.0) return props.CentreOfMass();

    BRepGProp::LinearProperties(shp, props);
    if (props.Mass() > 0.0) return props.CentreOfMass();

    Bnd_Box b;
    BRepBndLib::Add(shp, b);
    gp_Pnt pmin = b.CornerMin(), pmax = b.CornerMax();
    return gp_Pnt((pmin.X()+pmax.X())*0.5, (pmin.Y()+pmax.Y())*0.5, (pmin.Z()+pmax.Z())*0.5);
}


//was crashing and its not better
static gp_Pnt computeVisibleCenter(OCC_Viewer* occv)
{
    Handle(V3d_View) view = occv->m_view;
    Handle(AIS_InteractiveContext) ctx = occv->m_context;
    // if (view.IsNull() || ctx.IsNull())
        return view->Camera()->Center();

    // Tamanho da viewport
    Standard_Integer winW = 0, winH = 0;
    if (!view->Window().IsNull()) {
        view->Window()->Size(winW, winH);
    } else {
        winW = 1920; winH = 1080;
        winW = 1920; winH = 1080;
    }

    const Standard_Integer cx = winW / 2, cy = winH / 2;

    // Pick no centro
    Handle(SelectMgr_ViewerSelector) selector = ctx->MainSelector();
    if (!selector.IsNull()) {
        auto tryPickRect = [&](int half) -> Handle(SelectMgr_EntityOwner) {
            const int x0 = std::max(0, cx - half);
            const int y0 = std::max(0, cy - half);
            const int x1 = std::min(winW - 1, cx + half);
            const int y1 = std::min(winH - 1, cy + half);
            selector->Pick(x0, y0, x1, y1, view);
            if (selector->NbPicked() > 0) return selector->Picked(1);
            return Handle(SelectMgr_EntityOwner)();
        };

        Handle(SelectMgr_EntityOwner) owner = tryPickRect(0);
        if (owner.IsNull()) owner = tryPickRect(4);
        if (owner.IsNull()) owner = tryPickRect(8);
        if (owner.IsNull()) owner = tryPickRect(16);

        if (!owner.IsNull()) {
            Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(owner);
            TopoDS_Shape shp;
            if (!brepOwner.IsNull()) {
                shp = brepOwner->Shape();
            } else {
                Handle(SelectMgr_SelectableObject) so = owner->Selectable();
                Handle(AIS_Shape) ais = Handle(AIS_Shape)::DownCast(so);
                if (!ais.IsNull()) shp = ais->Shape();
            }
            selector->Clear();
	cotm2(winW,winH)

            if (!shp.IsNull())
                return shapeCentroidWorld(shp);
        }
    }

    // Fallback: objeto exibido mais próximo do centro por projeção
    AIS_ListOfInteractive disp;
#if defined(OCC_VERSION_HEX) && (OCC_VERSION_HEX >= 0x070600)
    ctx->DisplayedObjects(disp);
#else
    ctx->ObjectsByDisplayStatus(AIS_DS_Displayed, disp);
#endif

    double bestD2 = std::numeric_limits<double>::infinity();
    gp_Pnt bestC;

    for (AIS_ListOfInteractive::Iterator it(disp); it.More(); it.Next()) {
        const Handle(AIS_InteractiveObject)& obj = it.Value();
        Handle(AIS_Shape) ais = Handle(AIS_Shape)::DownCast(obj);
        if (ais.IsNull()) continue;

        const TopoDS_Shape& shp = ais->Shape();
        if (shp.IsNull()) continue;

        gp_Pnt c = shapeCentroidWorld(shp);

        Standard_Real sx = 0, sy = 0;
        view->Project(c.X(), c.Y(), c.Z(), sx, sy);

        // Se a sua integração usa Y top-down, pode precisar: sy = winH - 1 - sy;
        if (sx < 0 || sx >= winW || sy < 0 || sy >= winH) continue;

        const double dx = sx - cx, dy = sy - cy;
        const double d2 = dx*dx + dy*dy;
        if (d2 < bestD2) {
            bestD2 = d2;
            bestC = c;
        }
    }

    if (bestD2 < std::numeric_limits<double>::infinity())
        return bestC;

    return view->Camera()->Center();
}


gp_Vec end_proj_global;
gp_Vec end_up_global;
Handle(AIS_AnimationCamera) CurrentAnimation;
 
// Rotate around ONLY the shapes currently visible in the viewport.
// Uses OCCT rectangular picking on the full viewport to find visible AIS_Shapes,
// computes their bbox center, and pivots around that. Also keeps eye-to-center
// distance consistent to avoid "flying away".

// ... (other functions remain the same)

void start_animation(void* userdata)
{
    auto* occv = static_cast<OCC_Viewer*>(userdata);
	// auto& occv = userdata->occv;
    Handle(V3d_View) m_view = occv->m_view;

    // Stop any existing animation
    if (!occv->CurrentAnimation.IsNull())
    {
        occv->CurrentAnimation->Stop();
        occv->CurrentAnimation.Nullify();
    }

    // Current camera
    Handle(Graphic3d_Camera) currentCamera = m_view->Camera();

			cotm2("an11")
    // Pivot = center of currently visible shapes in viewport (robust)
    gp_Pnt center = computeVisibleCenter(occv);
cotm2(center.X());
// gp_Pnt center(2400,0,0);
    // Keep same eye-to-center distance to avoid "flying away"
    const gp_Pnt oldEye = currentCamera->Eye();
    const double distance = oldEye.Distance(center);

    // Start/End cameras
    Handle(Graphic3d_Camera) cameraStart = new Graphic3d_Camera();
    cameraStart->Copy(currentCamera);

    Handle(Graphic3d_Camera) cameraEnd = new Graphic3d_Camera();
    cameraEnd->Copy(currentCamera);

    // Target direction (OpenCASCADE expects eye->center)
    const gp_Dir targetDir(-occv->end_proj_global.X(),
                           -occv->end_proj_global.Y(),
                           -occv->end_proj_global.Z());

    const gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;

    cameraEnd->SetEye(targetEye);
    cameraEnd->SetCenter(center);
    cameraEnd->SetDirection(targetDir);
    cameraEnd->SetUp(gp_Dir(occv->end_up_global.X(),
                            occv->end_up_global.Y(),
                            occv->end_up_global.Z()));

    // Animate
    occv->CurrentAnimation = new AIS_AnimationCamera("ViewAnimation", m_view);
    occv->CurrentAnimation->SetCameraStart(cameraStart);
    occv->CurrentAnimation->SetCameraEnd(cameraEnd);
    occv->CurrentAnimation->SetOwnDuration(0.6);

    occv->CurrentAnimation->StartTimer(0.0, 1.0, Standard_True, Standard_False);

    // Kick the update loop
    Fl::add_timeout(1.0 / 12.0, animation_update, userdata);
}

static void animation_update(void* userdata)
{
    auto* occv = static_cast<OCC_Viewer*>(userdata);
    Handle(V3d_View) m_view = occv->m_view;

    if (occv->CurrentAnimation.IsNull())
    {
        Fl::remove_timeout(animation_update, userdata);
        return;
    }

    if (occv->CurrentAnimation->IsStopped())
    {
        // Recompute pivot from *visible* shapes again (final snap)
        // gp_Pnt center = computeVisibleCenter(occv);

        // // Preserve distance
        // const gp_Pnt oldEye = m_view->Camera()->Eye();
        // const double distance = oldEye.Distance(center);

        // const gp_Dir targetDir(-occv->end_proj_global.X(),
        //                        -occv->end_proj_global.Y(),
        //                        -occv->end_proj_global.Z());

        // const gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;

        // m_view->Camera()->SetEye(targetEye);
        // m_view->Camera()->SetCenter(center);
        // m_view->Camera()->SetDirection(targetDir);
        // m_view->Camera()->SetUp(gp_Dir(occv->end_up_global.X(),
        //                                occv->end_up_global.Y(),
        //                                occv->end_up_global.Z()));

        occv->colorisebtn();
        occv->projectAndDisplayWithHLR(occv->vshapes);
        occv->redraw();

        Fl::remove_timeout(animation_update, userdata);
        return;
    }

    // Advance animation
    occv->CurrentAnimation->UpdateTimer();
    occv->projectAndDisplayWithHLR(occv->vshapes);
    occv->redraw();

    // 30 FPS
    Fl::repeat_timeout(1.0 / 30.0, animation_update, userdata);
}


	
	// gp_Vec end_proj_global;
	// gp_Vec end_up_global;
	// Handle(AIS_AnimationCamera) CurrentAnimation;


	void start_animation_v1(void* userdata) {
		auto* occv = static_cast<OCC_Viewer*>(userdata);
		auto& m_view = occv->m_view;

		// Clear any existing animation
		if (!occv->CurrentAnimation.IsNull()) {
			occv->CurrentAnimation->Stop();
			occv->CurrentAnimation.Nullify();
		}

		// Get current camera state
		Handle(Graphic3d_Camera) currentCamera = m_view->Camera();
		gp_Pnt center = currentCamera->Center();
		double distance = currentCamera->Distance();

		// Create start camera (current state)
		Handle(Graphic3d_Camera) cameraStart = new Graphic3d_Camera();
		cameraStart->Copy(currentCamera);

		// Create end camera (target state)
		Handle(Graphic3d_Camera) cameraEnd = new Graphic3d_Camera();
		cameraEnd->Copy(currentCamera);

		// Invert the direction for OpenCASCADE (eye to center)
		gp_Dir targetDir(-occv->end_proj_global.X(),   // Invert X
						 -occv->end_proj_global.Y(),   // Invert Y
						 -occv->end_proj_global.Z());  // Invert Z

		gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;

		// Set target camera position
		cameraEnd->SetEye(targetEye);
		cameraEnd->SetCenter(center);
		cameraEnd->SetDirection(targetDir);
		cameraEnd->SetUp(gp_Dir(occv->end_up_global.X(), occv->end_up_global.Y(), occv->end_up_global.Z()));

		// Create and configure animation
		occv->CurrentAnimation = new AIS_AnimationCamera("ViewAnimation", m_view);
		occv->CurrentAnimation->SetCameraStart(cameraStart);
		occv->CurrentAnimation->SetCameraEnd(cameraEnd);
		occv->CurrentAnimation->SetOwnDuration(0.6);  // Duration in seconds

		// Start animation immediately
		occv->CurrentAnimation->StartTimer(0.0,			   // Start time
										   1.0,			   // Playback speed (normal)
										   Standard_True,  // Update to start position
										   Standard_False  // Don't stop timer at start
		);

		// Start the update timer
		// Fl::add_timeout(0.001, animation_update, userdata);
		Fl::add_timeout(1.0 / 12, animation_update, userdata);
	}

	static void animation_update_v1(void* userdata) {
		auto* occv = static_cast<OCC_Viewer*>(userdata);
		auto& m_view = occv->m_view;

		if (occv->CurrentAnimation.IsNull()) {
			Fl::remove_timeout(animation_update, userdata);
			return;
		}

		if (occv->CurrentAnimation->IsStopped()) {
			// Ensure perfect final position with inverted direction
			gp_Dir targetDir(-occv->end_proj_global.X(), -occv->end_proj_global.Y(), -occv->end_proj_global.Z());

			gp_Pnt center = m_view->Camera()->Center();
			double distance = m_view->Camera()->Distance();
			gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;

			m_view->Camera()->SetEye(targetEye);
			m_view->Camera()->SetDirection(targetDir);
			m_view->Camera()->SetUp(gp_Dir(occv->end_up_global.X(), occv->end_up_global.Y(), occv->end_up_global.Z()));

			occv->colorisebtn();
			occv->projectAndDisplayWithHLR(occv->vshapes);
			occv->redraw();	 //  redraw(); // m_view->Update ();

			Fl::remove_timeout(animation_update, userdata);
			return;
		}

		// Update animation with current time
		occv->CurrentAnimation->UpdateTimer();
		occv->projectAndDisplayWithHLR(occv->vshapes);
		occv->redraw();	 //  redraw(); // m_view->Update ();

		// Continue updating at 30fps
		Fl::repeat_timeout(1.0 / 30.0, animation_update, userdata);
	}

	vector<int> check_nearest_btn_idx() {
		// Get current view orientation
		Standard_Real dx, dy, dz, ux, uy, uz;
		m_view->Proj(dx, dy, dz);
		m_view->Up(ux, uy, uz);

		gp_Dir current_proj(dx, dy, dz);
		gp_Dir current_up(ux, uy, uz);

		vector<pair<double, int>> view_scores;	// Stores angle sums with their indices

		for (int i = 0; i < 6; i++) {
			// for (int i = 0; i < sbt.size(); i++) {
			if (!sbt[i].is_setview) continue;

			// Get predefined view orientation
			gp_Dir predefined_proj(sbt[i].v.dx, sbt[i].v.dy, sbt[i].v.dz);
			gp_Dir predefined_up(sbt[i].v.ux, sbt[i].v.uy, sbt[i].v.uz);

			// Calculate angular differences
			double proj_angle = current_proj.Angle(predefined_proj);
			double up_angle = current_up.Angle(predefined_up);
			double angle_sum = proj_angle + up_angle;

			view_scores.emplace_back(angle_sum, i);
		}

		// Sort by angle sum (ascending order)
		sort(view_scores.begin(), view_scores.end());

		// Prepare result vector
		vector<int> result;

		// Add nearest (if exists)
		if (!view_scores.empty()) {
			result.push_back(view_scores[0].second);
		} else {
			result.push_back(-1);
		}

		// Add second nearest (if exists)
		if (view_scores.size() > 1) {
			result.push_back(view_scores[1].second);
		} else {
			result.push_back(-1);
		}

		return result;
	}
#pragma endregion view_rotate
};
OCC_Viewer* occv = 0;

#pragma region browser
unordered_map<string,bool> mhide;
unordered_map<string,bool> msolo; 
bool anysolo() {
	bool any = 0;
	for (const auto& [key, value] : msolo) {
		if (value) { 
			any = 1;
			break;	// no need to check further
		}
	}
	if (!any) {
		msolo.clear();
		return 0;
	}
	return 1;
}
void fillbrowser(void*) {
	perf();
	static int binit = 0;
	// if (fbm->size()==0){
	if (!binit) {
		binit = 1;
		fbm->setCallback([](void* data, int code, void* fbm_) {
			OCC_Viewer::luadraw* ld = (OCC_Viewer::luadraw*)data;
			std::cout << "Callback data_ptr=" << ld->name << ", code=" << code << std::endl;

			if (code == 2 && !fbm->isrightclick) {
				gopart(ld->name);
			}
			if (code == 2 && fbm->isrightclick) {
				ld->occv->FitViewToShape(ld->occv->m_view, ld->shape);
			}

			// hide
			if (code == 0) {
				if (mhide[ld->name] == 0)
					mhide[ld->name] = 1;
				else if (mhide[ld->name] == 1)
					mhide[ld->name] = 0;
				fillbrowser(0);
				// return;

				// // if(fbm->on_off[i][code]==1){
				// // 	msolo[ld->name]=1;

				// // }else{
				// // 	msolo[ld->name]=0;
				// // }
				// ld->visible_hardcoded=!ld->visible_hardcoded;
				// mhide[ld->name]=!ld->visible_hardcoded;
				// // if(ld->visible_hardcoded)msolo[ld->name]=1; else msolo[ld->name]=0;
				// ld->redisplay();
				// ld->occv->redraw();
				// return;
			}
			if (code == 1) {
				if (msolo[ld->name] == 0)
					msolo[ld->name] = 1;
				else if (msolo[ld->name] == 1)
					msolo[ld->name] = 0;
				// cotm1(ld->name,anysolo());
				fillbrowser(0);
				// cotm1(ld->name,anysolo());
				// return;

				// 	if(anysolo()==0)return;
				// 	lop(i,0,occv->vlua.size()){
				// 		OCC_Viewer::luadraw* ldi=occv->vlua[i];
				// 		ldi->visible_hardcoded=0;
				// 		if(msolo[ldi->name]==1){
				// 			ldi->visible_hardcoded=1;
				// 		}
				// 		ldi->redisplay();
				// 	}
				// ld->redisplay();
				// ld->occv->redraw();
				// return;
			}
		});
		// lop(i,0,occv->vlua.size()){
		// 	OCC_Viewer::luadraw* ld = occv->vlua[i];
		// 	msolo[ld->name]=0;
		// }
	}

	for (const auto& [key, value] : msolo) {
		std::cout << key << " => " << std::boolalpha << value << '\n';
	}

	bool are_anysolo = anysolo();
	// cotm1(are_anysolo)

		std::vector<std::vector<bool>>
			on_off = fbm->on_off;

	fbm->clear_all();
	fbm->vcols = {{18, "@B31@C64", "@B64@C31"}, {18, "@B29@C64", "@B64@C29"}, {18, "", ""}};
	// fbm->vcols={{18,"@B31@C64","@B64@C31"},{18,"@B29@C64","@B64@C29"},{18,"","@B12@C7"}};
	// fbm->color(fl_rgb_color(220, 235, 255));
	fbm->init();
	// }
	// if(occv->vlua.size()>0)occv->vlua.back()->redisplay(); //regen
	lop(ij, 0, occv->vaShape.size()) {
		OCC_Viewer::luadraw* ld = occv->vlua[ij];
		// OCC_Viewer::luadraw* ld = occv->getluadraw_from_ashape(occv->vaShape[i]);
		// cotm(ld->visible_hardcoded)
		// OCC_Viewer::luadraw* ld=occv->vlua[i];

		// OCC_Viewer::luadraw* ldi=0;
		// 		if(fbm->cache.size()>0){
		// 			ldi=(OCC_Viewer::luadraw*)fbm->data(i+1);
		// 		}
		// 		if(ld==ldi)continue;
		// 		// Remove the old item
		// fbm->remove(i+1);

		// // Insert the new item at the same position
		// fbm->insert_at(i+1, {"H","S",ld->name});
		// fbm->data(i+1, (void*)ld);

		// 		if(fbm->on_off[i][1]==1 || binit)
		// if(on_off.size()>0 && on_off[i][1])

		fbm->addnew({"H", "S", ld->name});
		fbm->data(fbm->size(), (void*)ld);
		int line = fbm->size();

		// fillbuttonhs(ld,line);

		ld->visible_hardcoded = 1;

		if (mhide[ld->name] == 1) {
			ld->visible_hardcoded = 0;
			fbm->toggleon(line, 0, 1, 0);
		}  // else fbm->toggleon(line, 0, 0,0);

		// if (mhide[ld->name] == 1) {
		// 	ld->visible_hardcoded = 0;
		// 	fbm->toggleon(line, 0, 1);
		// }
		// // ld->visible_hardcoded=0;
		// // fbm->on_off[i][1]=0;
		// // fbm->toggleon(fbm->size(),1,1);
		// if (msolo.size() > 0 && msolo[ld->name] == 1) {
		// 	ld->visible_hardcoded = 0;
		// 	// int line=fbm->size();
		// 	// fbm->toggleon(line,1,0);
		// 	// fbm->on_off[i][1]=1;
		// }

		// if(msolo.size()>0 && msolo[ld->name]==0){
		// 	// ld->visible_hardcoded=1;
		// 	if(msolo.size()>0 )fbm->toggleon(line,1,1);

		// }

		// else fbm->toggleon(fbm->size(),1,1);
		//  else{
		//  	ld->visible_hardcoded=1;
		//  	fbm->on_off[i][1]=1;
		//  	cotm(ld->name,ld->visible_hardcoded)
		//  }

		// fbm->addnew({"H","S",ld->name});
		// fbm->data(fbm->size(), (void*)ld);

		// 		if(msolo[ld->name]==1){
		// 			ld->visible_hardcoded=1;
		// 			fbm->on_off[i][1]=0;
		// 			cotm(ld->name,ld->visible_hardcoded)
		// 		}else{
		// ld->visible_hardcoded=0;
		// 			fbm->on_off[i][1]=1;
		// 		}

		// if(ij==occv->vaShape.size()-1)
		ld->redisplay();
	}

	if (are_anysolo) {
		lop(i, 0, occv->vlua.size()) {
			OCC_Viewer::luadraw* ldi = occv->vlua[i];
			ldi->visible_hardcoded = 0;
			if (msolo[ldi->name] == 1) {
				ldi->visible_hardcoded = 1;
				fbm->toggleon(i + 1, 1, 1, 0);
			}
			ldi->redisplay();
		}
	}

	occv->fillvectopo();
	// cotm("vshapes", occv->vshapes.size());
	// perf1("bench fillvectopo");
	occv->toggle_shaded_transp(occv->currentMode);
	occv->redraw();
	perf("fillbrowser");
}

#pragma endregion browser

#pragma region lua
// #include <sol/sol.hpp>

void deletelua(string name) {
	auto it = occv->ulua.find(name);
	if (it != occv->ulua.end()) {
		// 2. Get the pointer before erasure
		OCC_Viewer::luadraw* tt = it->second;
		// cotm(2)
		// 3. Remove from OpenCASCADE context first
		if (!tt->ashape.IsNull() && !occv->m_context.IsNull()) {
			occv->m_context->Remove(tt->ashape, 0);
		}
		// cotm(3)
		// 4. Remove from vector if present
		auto& va = occv->vaShape;
		va.erase(std::remove(va.begin(), va.end(), tt->ashape), va.end());
		// cotm(4)
		// 5. Delete the object
		// delete tt;
		// it->second=0;
		// 6. Finally erase from map
		// cotm(5)
		occv->ulua.erase(it);
		// cotm(6)
	}
}

int close(lua_State* L) {
	luaL_error(L, "close() called!");
	return 0;
}
int Part(lua_State* L) {
	cotm("luatest") cotm(lua_gettop(L));

	OCC_Viewer::luadraw* test = new OCC_Viewer::luadraw("consegui", occv);
	// test->visible_hardcoded=0;
	// test->translate(10);
	// test.translate(0,10);
	// test.rotate(90);
	test->dofromstart();
	test->redisplay();

	perf();
	occv->fillvectopo();
	perf("bench");

	occv->toggle_shaded_transp(occv->currentMode);

	// occv->m_view->FitAll();
	occv->redraw();	 // for win

	return 1;
}

// static std::vector<gp_Vec2d> to_vec2d_vector(const sol::table& t) {
// 	std::vector<gp_Vec2d> out;
// 	cotm("gp_Vec2d0");
// 	out.reserve(t.size());

// 	for (std::size_t i = 1; i <= t.size(); ++i) {
// 		sol::optional<gp_Vec2d> maybe_vec = t[i];
// 		if (maybe_vec) {
// 			out.push_back(*maybe_vec);
// 			cotm(i);
// 		} else {
// 			cotm("Invalid gp_Vec2d at index " + std::to_string(i));
// 		}
// 	}

// 	cotm("gp_Vec2d1");
// 	return out;
// }
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <vector>

// Helper: move points so that refPoint goes to (0,0,0)
// Returns the transform and fills localPts
gp_Trsf MakeRelativeCoords(const std::vector<gp_Pnt>& worldPts,
                           std::vector<gp_Pnt>& localPts,
                           gp_Pnt& refPoint)
{
    if (worldPts.empty()) {
        localPts.clear();
        refPoint = gp_Pnt(0,0,0);
        gp_Trsf trsf;
        trsf.SetTranslation(gp_Vec(0,0,0));
        return trsf;
    }

    // Choose reference point (for simplicity: first point)
    refPoint = worldPts.front();

    // Build translation that moves refPoint → origin
    gp_Trsf trsf;
    trsf.SetTranslation(gp_Vec(refPoint, gp_Pnt(0,0,0)));

    // Apply to all points → local coordinates
    localPts.clear();
    localPts.reserve(worldPts.size());
    for (const gp_Pnt& p : worldPts) {
        localPts.push_back(p.Transformed(trsf));
    }

    return trsf; // This is the "placement" you store separately
}






// Track current part inside C++
OCC_Viewer::luadraw* current_part = nullptr;
void bind_luadraw(sol::state& lua, OCC_Viewer* occv) {
	// Helper to parse "x,y x1,y1 ..." into vector<gp_Vec2d>
	auto parse_coords = [](const std::string& coords) {
		std::vector<gp_Vec2d> out;
		double lastX = 0.0;
		double lastY = 0.0;
		bool have_last = false;

		std::istringstream ss(coords);
		std::string token;
		while (ss >> token) {
			bool relative = false;

			// Detect relative form: "@dx,dy"
			if (!token.empty() && token.front() == '@') {
				relative = true;
				token.erase(0, 1);
			}

			// Split "x,y"
			const auto commaPos = token.find(',');
			if (commaPos == std::string::npos) {
				continue;  // skip malformed token
			}

			// Parse numbers
			double x, y;
			try {
				x = std::stod(token.substr(0, commaPos));
				y = std::stod(token.substr(commaPos + 1));
			} catch (...) {
				continue;  // skip malformed numbers
			}

			if (relative) {
				if (have_last) {
					lastX += x;
					lastY += y;
				} else {
					// No previous point: treat as relative to (0,0)
					lastX = x;
					lastY = y;
					have_last = true;
				}
			} else {
				lastX = x;
				lastY = y;
				have_last = true;
			}

			out.emplace_back(lastX, lastY);
		}

		return out;
	};

	// Vec2 usertype
	// lua.new_usertype<gp_Vec2d>(
	// 	"Vec2", sol::constructors<gp_Vec2d(), gp_Vec2d(double, double)>(), "x",
	// 	sol::property([](const gp_Vec2d& v) { return v.X(); }, [](gp_Vec2d& v, double x) { v.SetX(x); }), "y",
	// 	sol::property([](const gp_Vec2d& v) { return v.Y(); }, [](gp_Vec2d& v, double y) { v.SetY(y); }));

	// luadraw usertype with methods (self is guaranteed to be the same instance)
	lua.new_usertype<OCC_Viewer::luadraw>(
		"luadraw", sol::no_constructor,

		// properties
		"name", &OCC_Viewer::luadraw::name, "visible_hardcoded", &OCC_Viewer::luadraw::visible_hardcoded,

		// methods
		"redisplay", &OCC_Viewer::luadraw::redisplay, 
		"display", &OCC_Viewer::luadraw::display, 
		"rotate", &OCC_Viewer::luadraw::rotate, 
		"rotatex",	[](OCC_Viewer::luadraw& self, int angle) { self.rotate(angle, 1.f, 0.f, 0.f); }, "rotatey",
		[](OCC_Viewer::luadraw& self, int angle) { self.rotate(angle, 0.f, 1.f, 0.f); }, "rotatez",
		[](OCC_Viewer::luadraw& self, int angle) { self.rotate(angle, 0.f, 0.f, 1.f); }, "translate",
		&OCC_Viewer::luadraw::translate, "extrude", &OCC_Viewer::luadraw::extrude, "clone", &OCC_Viewer::luadraw::clone,
		
		"offset", &OCC_Viewer::luadraw::createOffset, 
		
		"copy_placement", &OCC_Viewer::luadraw::copy_placement, 
		// "add_placement", &OCC_Viewer::luadraw::add_placement, 
		
		
		"exit",
		&OCC_Viewer::luadraw::exit, "fuse", &OCC_Viewer::luadraw::fuse, "dofromstart",
		&OCC_Viewer::luadraw::dofromstart,

		// create_wire: accept table OR string
		// "create_wire",
		// sol::overload(
		// 	// existing table overloads
		// 	[](OCC_Viewer::luadraw& self, sol::table pts, bool closed) {
		// 		self.CreateWire(to_vec2d_vector(pts), closed);
		// 	},
		// 	[](OCC_Viewer::luadraw& self, sol::table pts) { self.CreateWire(to_vec2d_vector(pts), false); },
		// 	// new string overloads
		// 	[parse_coords](OCC_Viewer::luadraw& self, const std::string& coords, bool closed) {
		// 		self.CreateWire(parse_coords(coords), closed);
		// 	},
		// 	[parse_coords](OCC_Viewer::luadraw& self, const std::string& coords) {
		// 		self.CreateWire(parse_coords(coords), false);
		// 	}),

		// create_pl: method form, string "x,y x1,y1 ..."
		"pl",
		[parse_coords](OCC_Viewer::luadraw& self, const std::string& coords) {
			auto pts = parse_coords(coords);
			// Replace with the actual polyline builder you want to call:
			self.CreateWire(pts, false);
		},

		// Optional: expose address for sanity checks
		"addr", [](OCC_Viewer::luadraw& self) -> std::uintptr_t { return reinterpret_cast<std::uintptr_t>(&self); });

	// Factory returns a shared_ptr so the same instance is kept alive in Lua
	// lua.set_function("luadraw_new", [occv](const std::string& name) {
	// 	auto* luaDrawPtr = new OCC_Viewer::luadraw(name, occv);
	// 	// Create with the OCC_Viewer* automatically injected
	// 	return luaDrawPtr;
	// });

	lua.set_function("Part", [occv, &lua](const std::string& name) {
		auto* obj = new OCC_Viewer::luadraw(name, occv);
		lua[name] = obj;  // same as: name = obj in Lua
		current_part = obj;
		return obj;
	});

	// Helper: call method on current_part
	// auto call_on_current = [&](const std::string& method, sol::variadic_args args) {
	// 	if (!current_part) return;
	// 	sol::function f = lua["luadraw"][method];
	// 	if (f.valid()) {
	// 		f(current_part, args);
	// 	}
	// };


#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Solid.hxx>
#include <stdexcept>

lua.set_function("Origin", [occv](Standard_Real x=0,Standard_Real y=0,Standard_Real z=0) { 
	if (current_part)current_part->redisplay();
		// cotm2("origin",x)
		// occv->Origintrsf.
		if(x==0 && y==0 && z==0){occv->Origin=TopLoc_Location(); return;}
		gp_Trsf tr;
		tr.SetTranslation(gp_Vec(x, y, z));
		occv->Origin=TopLoc_Location(tr);

	// sol::protected_function Rotatel = lua["Rotatel"];
	cout<<"Origin "<<x<<","<<y<<","<<z<<"\n";
});
lua.set_function("Fuse", [&]() { 
		// cotm2("fuse fail")
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");
    
    const TopoDS_Compound& c = current_part->cshape;

    // Collect all solids
    std::vector<TopoDS_Solid> solids;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }

    if (solids.size() < 2) {

		throw std::runtime_error(lua_error_with_line(L, "Need at least two solids to fuse..."));
		// lua_error_with_line(L,"Need at least two solids to fuse.");
        // throw std::runtime_error("Need at least two solids to fuse..");
    }

    // Fuse the last two solids
    BRepAlgoAPI_Fuse fuseOp(solids[solids.size()-2], solids.back());
    fuseOp.Build();
    if (!fuseOp.IsDone()) {
        throw std::runtime_error("Fuse operation failed.");
    }

    TopoDS_Shape fused = fuseOp.Shape();

    // 🔧 Refine geometry (merge coplanar faces, edges, vertices)
    ShapeUpgrade_UnifySameDomain refine(fused, true, true, true);
    refine.Build();
    TopoDS_Shape refinedShape = refine.Shape();
    if (!refinedShape.IsNull()) {
        fused = refinedShape;
    }

    // Build a new compound with all shapes preserved
    TopoDS_Compound result;
    TopoDS_Builder builder;
    builder.MakeCompound(result);

    // Add all original shapes except the fused ones
    for (TopExp_Explorer ex(c, TopAbs_SHAPE); ex.More(); ex.Next()) {
        const TopoDS_Shape& s = ex.Current();
        if (s.ShapeType() == TopAbs_SOLID) {
            const TopoDS_Solid solid = TopoDS::Solid(s);
            if (solid.IsSame(solids.back()) || solid.IsSame(solids[solids.size()-2]))
                continue;
        }
        builder.Add(result, s);
    }

    // Add refined fused result
    builder.Add(result, fused);

    // Update current part
    current_part->cshape = result;
    current_part->shape = fused;
});

#include <BRepAlgoAPI_Cut.hxx>

lua.set_function("Subtract", [&]() {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");
    const TopoDS_Compound& c = current_part->cshape;

    std::vector<TopoDS_Solid> solids;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }
    if (solids.size() < 2) {
        throw std::runtime_error("Need at least two solids to subtract.");
    }

    // Subtract: before‑last minus last
    TopoDS_Shape cut = BRepAlgoAPI_Cut(solids[solids.size()-2], solids.back());

    // Build new compound
    TopoDS_Compound result;
    TopoDS_Builder builder;
    builder.MakeCompound(result);

    // Add all except the last two
    for (size_t i = 0; i + 2 < solids.size(); ++i) {
        builder.Add(result, solids[i]);
    }
    // Add cut result
    builder.Add(result, cut);

    current_part->cshape = result;
});
lua.set_function("Circle_v1", [&](float radius, float x, float y) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

    if (radius <= 0)
        luaL_error(lua.lua_state(), "Circle radius must be > 0.");

    // Center
    gp_Pnt center(x, y, 0.0);

    // Circle in XY plane
    gp_Ax2 axis(center, gp::DZ());
    gp_Circ circ(axis, radius);

    // Edge → wire → face
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circ);
    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

    // // Merge face
    // current_part->mergeShape(current_part->cshape, face);

    // -------------------------------
    // Add helper vertices (snap points)
    // -------------------------------
    // auto addVertex = [&](double vx, double vy, double vz)
    // {
    //     TopoDS_Vertex v =
    //         BRepBuilderAPI_MakeVertex(gp_Pnt(vx, vy, vz));
    //     current_part->mergeShape(current_part->cshape, v);
    // };

    // const double z = 0.0;

    // // Center
    // addVertex(x,         y,         z);

    // // Quadrants
    // addVertex(x + radius, y,          z); // +X
    // addVertex(x - radius, y,          z); // -X
    // addVertex(x,          y + radius, z); // +Y
    // addVertex(x,          y - radius, z); // -Y


    // Merge face
    current_part->mergeShape(current_part->cshape, face);
});

lua.set_function("Circle", [&](float radius, float x, float y) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

    if (radius <= 0)
        luaL_error(lua.lua_state(), "Circle radius must be > 0.");

    // Define center point
    gp_Pnt center(x, y, 0.0);

    // Define circle in XY plane
    gp_Ax2 axis(center, gp::DZ());  // Normal = Z axis

    // Build OCC circle
    gp_Circ circ(axis, radius);

// Build edge 
TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circ); 
// Build wire 
TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge); 
// Build face (planar) 
TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

	// TopoDS_Shape res=current_part->solidify_wire_to_face();

    // Add to current part
    current_part->mergeShape(current_part->cshape,face);
});


lua.set_function("Pl", [&, parse_coords](const std::string& coords) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");
    current_part->CreateWire(parse_coords(coords), false);

	// current_part->redisplay();
	// current_part->needsplacementupdate=1;

});


lua.set_function("Plhex", [&](double val) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");
    current_part->CreateWire(current_part->MakeHexagonPoints(val), true);

	// current_part->redisplay();
	// current_part->needsplacementupdate=1;

});

lua.set_function("Offset", [&](double val) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
    current_part->createOffset(val);
});
#include <BRepTools.hxx>
#include <ShapeFix_Shape.hxx>



lua.set_function("FilletToAllEdges", [&](double val) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
	// current_part->update_placement();
	// current_part->shape=ApplyFilletToAllEdges(current_part->cshape,val);
	current_part->ttfillet=1;
ReplaceShapeInCompound(
    current_part->cshape,
    current_part->shape,
    [&](const TopoDS_Shape& s) {
        auto clean = CleanShape(s);
        BRepFilletAPI_MakeFillet fillet(clean);
        for (TopExp_Explorer exp(clean, TopAbs_EDGE); exp.More(); exp.Next())
            fillet.Add(TopoDS::Edge(exp.Current()));
       

for (int ic = 1; ic <= fillet.NbContours(); ++ic) {
    int nbEdges = fillet.NbEdges(ic);
    for (int ie = 1; ie <= nbEdges; ++ie) {
        fillet.SetRadius(val, ic, ie);
    }
}



        fillet.Build();
        if (!fillet.IsDone()) return s;
        return FixShape(fillet.Shape());
    }
);
// current_part->ashape->Attributes()->SetFaceBoundaryDraw(false);
});
lua.set_function("DebugShape", [&](const std::string& label){
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Shape& s = current_part->shape;

    gp_Trsf tr = s.Location().Transformation();
    gp_XYZ t = tr.TranslationPart();
    gp_Mat m = tr.VectorialPart();

    std::cout << "=== " << label << " ===\n";
    std::cout << "Translation: ("
              << t.X() << ", " << t.Y() << ", " << t.Z() << ")\n";

    std::cout << "Rotation matrix:\n";
    std::cout << "  [" << m.Value(1,1) << " " << m.Value(1,2) << " " << m.Value(1,3) << "]\n";
    std::cout << "  [" << m.Value(2,1) << " " << m.Value(2,2) << " " << m.Value(2,3) << "]\n";
    std::cout << "  [" << m.Value(3,1) << " " << m.Value(3,2) << " " << m.Value(3,3) << "]\n";
});

lua.set_function("Extrude", [&](float val){
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Shape baseShape = current_part->shape;
    if (baseShape.IsNull())
        luaL_error(lua.lua_state(), "Current part has no shape.");

    // 1) Save current transform (Movel + Rotatel)
    TopLoc_Location savedLoc = baseShape.Location();

    // 2) Work on pure local geometry (no Location)
    TopoDS_Shape localShape = baseShape;
    localShape.Location(TopLoc_Location());

    // 3) Extrude along local Z (perpendicular to the sketch plane)
    gp_Vec extrusionVec(0.0, 0.0, val);
    BRepPrimAPI_MakePrism prism(localShape, extrusionVec, Standard_False);
    prism.Build();
    if (!prism.IsDone())
        luaL_error(lua.lua_state(), "Extrusion failed.");

    TopoDS_Shape extrudedLocal = prism.Shape();

    // 4) Restore the original transform chain
    TopoDS_Shape extruded = extrudedLocal;
    extruded.Location(savedLoc);

    // 5) Add and make it the current shape
    current_part->mergeShape(current_part->cshape, extruded);
    current_part->shape = extruded;
});


lua.set_function("Dup", [&](){
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Shape baseShape = current_part->shape;
    if (baseShape.IsNull())
        luaL_error(lua.lua_state(), "Current part has no shape.");
	current_part->mergeShape(current_part->cshape, baseShape);
});
lua.set_function("Extrude_vw", [&](float val){
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Shape baseShape = current_part->shape;
    if (baseShape.IsNull())
        luaL_error(lua.lua_state(), "Current part has no shape.");

    // 1) Get transformation
    TopLoc_Location loc = baseShape.Location();
    gp_Trsf trsf = loc.Transformation();

    // 2) Extract rotation ONLY (ignore translation completely)
    gp_Mat rot = trsf.VectorialPart();   // pure rotation matrix

    // 3) Rotate local Z using rotation matrix only
    gp_Dir localZ(0.0, 0.0, 1.0);
    gp_Dir worldZ(
        rot.Value(1,1)*localZ.X() + rot.Value(1,2)*localZ.Y() + rot.Value(1,3)*localZ.Z(),
        rot.Value(2,1)*localZ.X() + rot.Value(2,2)*localZ.Y() + rot.Value(2,3)*localZ.Z(),
        rot.Value(3,1)*localZ.X() + rot.Value(3,2)*localZ.Y() + rot.Value(3,3)*localZ.Z()
    );

    // 4) Build extrusion vector
    gp_Vec extrusionVec(worldZ);
    extrusionVec *= val;

    // 5) Extrude
    BRepPrimAPI_MakePrism prism(baseShape, extrusionVec, Standard_False);
    prism.Build();

    if (!prism.IsDone())
        luaL_error(lua.lua_state(), "Extrusion failed.");

    TopoDS_Shape extruded = prism.Shape();

    // 6) DO NOT reapply location (angle already respected, position irrelevant)
    current_part->mergeShape(current_part->cshape, extruded);
});


lua.set_function("Extrude_v1", [&](float val) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
    current_part->extrude(val);
});
lua.set_function("Clone", [&](OCC_Viewer::luadraw* val,int copy_placement=0) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
    current_part->clone(val,copy_placement);
});
lua.set_function("Visible", [&](int val=1) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
    current_part->visible_hardcoded=val;
});

lua.set_function("Copy_placement", [&](OCC_Viewer::luadraw* val) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToSet = current_part->shape;

    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToSet)) {
            // Copy Location directly from val
            TopLoc_Location newLoc = val->shape.Location();
            TopoDS_Shape placedShape = currentShape;
            placedShape.Location(newLoc);

            builder.Add(newCompound, placedShape);
            shapeToSet = placedShape;
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToSet;
});

// COPY_PLACEMENT (replace location exactly with val->shape.Location())
lua.set_function("Copy_placement_v1new", [&](OCC_Viewer::luadraw* val) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToSet = current_part->shape;

    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    TopLoc_Location sourceLoc = val->shape.Location();

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToSet)) {
            // Replace placement exactly (no composition)
            TopoDS_Shape placedShape = currentShape;
            placedShape.Location(sourceLoc);

            builder.Add(newCompound, placedShape);
            shapeToSet = placedShape;
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToSet;
});


lua.set_function("Copy_placement_baked", [&](OCC_Viewer::luadraw* val) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    // Obter transformação absoluta do objeto de origem
    gp_Trsf sourceTrsf = val->shape.Location().Transformation();

    // Obter transformação atual do destino
    gp_Trsf targetTrsf = current_part->shape.Location().Transformation();

    // Queremos substituir o placement, não multiplicar cumulativamente
    TopoDS_Shape baseShape = current_part->shape;
    baseShape.Location(TopLoc_Location()); // remove transformação existente

    // Aplicar a transformação do objeto de origem
    BRepBuilderAPI_Transform transformer(baseShape, sourceTrsf, true);
    current_part->shape = transformer.Shape();

    // Atualizar localização para refletir corretamente no modelo
    current_part->shape.Location(TopLoc_Location(sourceTrsf));
});

lua.set_function("Copy_placement_v1nw", [&](OCC_Viewer::luadraw* val) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
    // current_part->trsf*=val->trsf;//.Inverted();
    // current_part->trsf*=val->trsf;
	gp_Trsf trsf = val->shape.Location().Transformation();
	BRepBuilderAPI_Transform transformer(current_part->shape, trsf, true);
	current_part->shape = transformer.Shape();

});
lua.set_function("Placeg", [&](OCC_Viewer::luadraw* val) {
    if (!current_part) luaL_error(lua.lua_state(), "No current part.");
    // current_part->trsf*=val->trsf;//.Inverted();
    current_part->trsf=val->trsf*current_part->trsf;
});

// to_local: move current_part into the local space of 'val'
lua.set_function("to_local", [&](OCC_Viewer::luadraw* val) {
    if (!current_part) 
        luaL_error(lua.lua_state(), "No current part.");

    // Apply inverse of val's transform to bring current_part into val's local space
    val->trsf = val->trsf.Inverted() ;
    // current_part->trsf = val->trsf.Inverted() * current_part->trsf;
});

// to_world: move current_part back to world coordinates from 'val'
lua.set_function("to_world", [&](OCC_Viewer::luadraw* val) {
    if (!current_part) 
        luaL_error(lua.lua_state(), "No current part.");

    // Apply val's transform to bring current_part back into world space
    current_part->trsf = val->trsf * current_part->trsf;
    // current_part->trsf = val->trsf * current_part->trsf;
});



// Lua binding
//keeper
lua.set_function("ConnectAtCenter", [&](OCC_Viewer::luadraw* targetShape, int targetFaceIndex) {
    TopoDS_Shape target = targetShape->shape;
    gp_Trsf targetTrsf = targetShape->trsf;

    // --- Find the requested face in the target
    TopExp_Explorer faceExplorer(target, TopAbs_FACE);
    TopoDS_Face targetFace;
    int currentIndex = 0;
    for (; faceExplorer.More(); faceExplorer.Next(), currentIndex++) {
        if (currentIndex == targetFaceIndex) {
            targetFace = TopoDS::Face(faceExplorer.Current());
            break;
        }
    }
    if (targetFace.IsNull()) {
        std::cerr << "Error: Invalid face index or face not found." << std::endl;
        return;
    }

    // --- Ensure target face is planar
    BRepAdaptor_Surface targetAdaptor(targetFace);
    if (targetAdaptor.GetType() != GeomAbs_Plane) {
        std::cerr << "Error: Only planar faces are supported." << std::endl;
        return;
    }

    // --- Target face system in world coords
    gp_Pln targetPlane = targetAdaptor.Plane();
    gp_Ax3 targetAx3 = targetPlane.Position();
    targetAx3.Transform(targetTrsf);

    // --- Target face center (world coords)
    GProp_GProps targetProps;
    BRepGProp::SurfaceProperties(targetFace, targetProps);
    gp_Pnt targetCenter = targetProps.CentreOfMass();
    targetCenter.Transform(targetTrsf);

    // --- Current part: pick its "main face" (e.g. index 0 for now)
    TopExp_Explorer curExplorer(current_part->shape, TopAbs_FACE);
    if (!curExplorer.More()) {
        std::cerr << "Error: current_part has no faces." << std::endl;
        return;
    }
    TopoDS_Face currentFace = TopoDS::Face(curExplorer.Current());

    // --- Ensure current face is planar
    BRepAdaptor_Surface curAdaptor(currentFace);
    if (curAdaptor.GetType() != GeomAbs_Plane) {
        std::cerr << "Error: current_part face not planar." << std::endl;
        return;
    }

    // --- Current face system in local coords
    gp_Pln curPlane = curAdaptor.Plane();
    gp_Ax3 curAx3 = curPlane.Position();

    // --- Current face center (local coords → then into world with part’s trsf)
    GProp_GProps curProps;
    BRepGProp::SurfaceProperties(currentFace, curProps);
    gp_Pnt curCenter = curProps.CentreOfMass();
    curCenter.Transform(current_part->trsf);

    curAx3.Transform(current_part->trsf);

    // --- Build frames using centers as locations
    gp_Ax3 sourceFrame(curCenter, curAx3.Direction(), curAx3.XDirection());
    gp_Ax3 targetFrame(targetCenter, targetAx3.Direction(), targetAx3.XDirection());

    // --- Build displacement (maps source face → target face)
    gp_Trsf alignmentTrsf;
    alignmentTrsf.SetDisplacement(sourceFrame, targetFrame);

    // --- Apply to current part
    current_part->trsf = alignmentTrsf;
});


lua.set_function("Connectwl", [&](OCC_Viewer::luadraw* targetShape, int targetFaceIndex, int currface=0) {
    TopoDS_Shape target = targetShape->shape;
    gp_Trsf targetTrsf = targetShape->trsf;

    // Find the requested face
    TopExp_Explorer faceExplorer(target, TopAbs_FACE);
    TopoDS_Face targetFace;
    int currentIndex = 0;
    for (; faceExplorer.More(); faceExplorer.Next(), currentIndex++) {
        if (currentIndex == targetFaceIndex) {
            targetFace = TopoDS::Face(faceExplorer.Current());
            break;
        }
    }
    if (targetFace.IsNull()) {
        std::cerr << "Error: Invalid face index or face not found." << std::endl;
        return;
    }

    // Ensure it’s planar
    BRepAdaptor_Surface faceAdaptor(targetFace);
    if (faceAdaptor.GetType() != GeomAbs_Plane) {
        std::cerr << "Error: Only planar faces are supported." << std::endl;
        return;
    }

    // Get plane in world coordinates
    gp_Pln plane = faceAdaptor.Plane();
    gp_Ax3 faceAx3 = plane.Position();
    faceAx3.Transform(targetTrsf); // <--- important!

    // Define the part’s local reference system you want to map
    gp_Ax3 partAx3(gp_Pnt(0,0,0), gp::DZ(), gp::DX()); 

    // Build transformation to glue part to face
    gp_Trsf alignmentTrsf;
    alignmentTrsf.SetDisplacement(partAx3, faceAx3);

    // Apply to current part
    current_part->trsf = alignmentTrsf;
});

//keeper working well tested
lua.set_function("Connect", [&](OCC_Viewer::luadraw* targetShape, int targetFaceIndex) {
    TopoDS_Shape target = targetShape->shape;
    gp_Trsf targetTrsf = targetShape->trsf;

    // Find the requested face
    TopExp_Explorer faceExplorer(target, TopAbs_FACE);
    TopoDS_Face targetFace;
    int currentIndex = 0;
    for (; faceExplorer.More(); faceExplorer.Next(), currentIndex++) {
        if (currentIndex == targetFaceIndex) {
            targetFace = TopoDS::Face(faceExplorer.Current());
            break;
        }
    }
    if (targetFace.IsNull()) {
        std::cerr << "Error: Invalid face index or face not found." << std::endl;
        return;
    }

    // Ensure it’s planar
    BRepAdaptor_Surface faceAdaptor(targetFace);
    if (faceAdaptor.GetType() != GeomAbs_Plane) {
        std::cerr << "Error: Only planar faces are supported." << std::endl;
        return;
    }

    // Get the face's plane and its coordinate system in world space
    gp_Pln plane = faceAdaptor.Plane();
    gp_Ax3 faceAx3 = plane.Position();
    faceAx3.Transform(targetTrsf); // Apply the target's transformation

    // Define the part's coordinate system to match the face's axes
    gp_Ax3 partAx3(
        gp_Pnt(0, 0, 0),  // Origin of the part
        faceAx3.Direction(),  // Z-axis (normal)
        faceAx3.XDirection()   // X-axis (defines rotation in the plane)
    );

    // Invert the face's coordinate system to align the part correctly
    gp_Ax3 invertedFaceAx3(
        faceAx3.Location(),
        -faceAx3.Direction(),  // Invert Z-axis
        faceAx3.XDirection()    // Keep X-axis
    );

    // Build the transformation to align the part to the inverted face coordinate system
    gp_Trsf alignmentTrsf;
    alignmentTrsf.SetDisplacement(gp_Ax3(gp_Pnt(0, 0, 0), gp::DZ(), gp::DX()), invertedFaceAx3);

    // Apply the transformation to the current part
    current_part->trsf = alignmentTrsf;
});

lua.set_function("Movel_v1", [&](float x = 0, float y = 0, float z = 0) {
	gp_Trsf trsftmp = gp_Trsf();
	trsftmp.SetTranslation(gp_Vec(x, y, z));
	// trsf = trsftmp * trsf; //world
	current_part->vtrsf.back() *= trsftmp;

	// TopoDS_Shape target = current_part->shape;
	// gp_Trsf targetTrsf = current_part->vtrsf.back();

	BRepBuilderAPI_Transform transformer(current_part->shape, current_part->vtrsf.back());	// if part only last
	// cshape = TopoDS::Compound(transformer.Shape());
	current_part->shape = transformer.Shape();
});

// MOVE (location-only, pre-multiply -> translate in global coords)
lua.set_function("Movel", [&](float x = 0, float y = 0, float z = 0) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToMove = current_part->shape;

    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(x, y, z));
    TopLoc_Location transLoc(translation);

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToMove)) {
            cotm("Movel");
            // PRE-multiply: translate in global coords
            TopLoc_Location newLoc = transLoc * currentShape.Location();
            TopoDS_Shape movedShape = currentShape;
            movedShape.Location(newLoc);

            builder.Add(newCompound, movedShape);
            shapeToMove = movedShape;
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToMove;
});


lua.set_function("Movel_brep", [&](float x = 0, float y = 0, float z = 0) {
    // Get reference to the current part's compound
    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToMove = current_part->shape;
    
    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    // Define the translation
    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(x, y, z));

    // Iterate over all shapes in the original compound
    for (TopoDS_Iterator it(compound); it.More(); it.Next())
    {
        const TopoDS_Shape& currentShape = it.Value();

        // Check if this is the exact same shape object
        if (currentShape.IsSame(shapeToMove))
        {
            cotm("Movel")
            // Translate the shape
            BRepBuilderAPI_Transform transformer(currentShape, translation, false);
            if (transformer.IsDone())
            {
                TopoDS_Shape movedShape = transformer.Shape();
                builder.Add(newCompound, movedShape);
                
                // Update the reference shape as well
                shapeToMove = movedShape;
            }
            else
            {
                // Fallback: add the original shape if translation fails
                builder.Add(newCompound, currentShape);
            }
        }
        else
        {
            // Add the unmodified shape
            builder.Add(newCompound, currentShape);
        }
    }

    // Replace the original compound

    current_part->cshape = newCompound; 
});

// guarda orientação acumulada em quaternion dentro da tua struct
 // inicializar como identidade na criação da peça

lua.set_function("Rotatex", [&](float angle = 0) {
    // Get the current transformation of the part
    gp_Trsf currentTrsf = current_part->trsf;
    // gp_Trsf currentTrsf = current_part->vtrsf.back();

    // Invert the current transformation to work in local space
    // gp_Trsf invCurrentTrsf = currentTrsf.Inverted();

    // Define the X-axis at the origin
    gp_Ax1 xAxis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));

    // Create a rotation transformation
    gp_Trsf rotation;
    rotation.SetRotation(xAxis, angle * (M_PI / 180));

    // Apply the rotation in local space:
    // new_transform = current_transform * inverse_current * rotation * current_transform
    current_part->trsf = current_part->trsf * rotation ;

    // Apply the transformation to the shape
    BRepBuilderAPI_Transform transformer(current_part->cshape,  current_part->trsf );
    // current_part->cshape = transformer.Shape();               ///////////////////////////////

    // // Update the part's transformation stack if needed
    // current_part->vtrsf.push_back(newTrsf);
});


#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopLoc_Location.hxx>

// … other necessary OCCT headers …
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopLoc_Location.hxx>

#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopLoc_Location.hxx>
#include <AIS_InteractiveContext.hxx>

// … your other OCCT includes …

// Assume you have a valid Handle(AIS_InteractiveContext) named 'aisContext'
// extern Handle(AIS_InteractiveContext) aisContext;

lua.set_function("Rotatexl_v1notw", [&](float angleDegrees = 0.0f) {
TopoDS_Compound compound=current_part->cshape;
TopoDS_Shape shapeToRotate=current_part->shape;


	 BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    // Define the rotation
    gp_Ax1 xAxis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    gp_Trsf rotation;
    rotation.SetRotation(xAxis, angleDegrees * (M_PI / 180.0));

    // Iterate over all shapes in the original compound
    for (TopoDS_Iterator it(compound); it.More(); it.Next())
    {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToRotate))
        {
			cotm("Rotatexl")
            // Rotate the shape
            BRepBuilderAPI_Transform transformer(currentShape, rotation);
            if (transformer.IsDone())
            {
                TopoDS_Shape rotatedShape = transformer.Shape();
                builder.Add(newCompound, rotatedShape);
            }
            else
            {
                // Fallback: add the original shape if rotation fails
                builder.Add(newCompound, currentShape);
            }
        }
        else
        {
            // Add the unmodified shape
            builder.Add(newCompound, currentShape);
        }
    }

    // Replace the original compound
    compound = newCompound;
	current_part->needsplacementupdate=0;
});

lua.set_function("Rotatexl_almost", [&](float angleDegrees = 0.0f) {
    // Create rotation transformation
    gp_Ax1 xAxis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    gp_Trsf rotation;
    rotation.SetRotation(xAxis, angleDegrees * (M_PI / 180.0));
    
    // Apply rotation to the specific shape
    BRepBuilderAPI_Transform transformer(current_part->shape, rotation, false);
    if (transformer.IsDone()) {
        current_part->shape = transformer.Shape();
        
        // Rebuild the compound with the rotated shape
        BRep_Builder builder;
        TopoDS_Compound newCompound;
        builder.MakeCompound(newCompound);
        builder.Add(newCompound, current_part->shape);
        
        current_part->cshape = newCompound;
    }
    
    // current_part->needsplacementupdate = 0;
});
lua.set_function("Rotatel_baked_new", [&](float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 0) {
    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToRotate = current_part->shape;

    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    // Get the current location (transformation) of the part
    gp_Trsf partTrsf = shapeToRotate.Location().Transformation();

    // Compute the local origin (0,0,0) of the part in global coordinates
    gp_Pnt localOrigin = gp_Pnt(0, 0, 0).Transformed(partTrsf);

    // Define the rotation axis around that local origin
    gp_Ax1 axis(localOrigin, gp_Dir(x, y, z));

    // Define the rotation transformation
    gp_Trsf rotation;
    rotation.SetRotation(axis, angleDegrees * (M_PI / 180.0));

    TopoDS_Shape rshape;

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToRotate)) {
            cotm("Rotatexl");
            // Apply rotation relative to local origin
            BRepBuilderAPI_Transform transformer(currentShape, rotation, true); // true = copy location context
            if (transformer.IsDone()) {
                TopoDS_Shape rotatedShape = transformer.Shape();
                builder.Add(newCompound, rotatedShape);
                shapeToRotate = rotatedShape; // Update reference
            } else {
                builder.Add(newCompound, currentShape);
            }
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToRotate;
});

lua.set_function("Rotatel_v1working", [&](float angleDegrees = 0.0f,int x=0,int y=0,int z=0) {
    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToRotate = current_part->shape;
    
    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    // Define the rotation -> make the gp_Pnt(0, 0, 0) be relative to original  0,0,0 of the part
    gp_Ax1 xAxis(gp_Pnt(0, 0, 0), gp_Dir(x, y, z));
    gp_Trsf rotation;
    rotation.SetRotation(xAxis, angleDegrees * (M_PI / 180.0));

	TopoDS_Shape rshape;

    for (TopoDS_Iterator it(compound); it.More(); it.Next())
    {
        const TopoDS_Shape& currentShape = it.Value();

        // Check if this is the exact same shape object
        if (currentShape.IsSame(shapeToRotate))
        {
            cotm("Rotatexl")
            BRepBuilderAPI_Transform transformer(currentShape, rotation, false);
            if (transformer.IsDone())
            {
                TopoDS_Shape rotatedShape = transformer.Shape();
                builder.Add(newCompound, rotatedShape);
                shapeToRotate = rotatedShape; // Update reference
            }
            else
            {
                builder.Add(newCompound, currentShape);
            }
        }
        else
        {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToRotate;
    // current_part->needsplacementupdate = 0;
});
// ROTATE (location-only, pre-multiply -> same effect as BRepBuilderAPI_Transform)
lua.set_function("Rotatew", [&](float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 0) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToRotate = current_part->shape;

    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(x, y, z));
    gp_Trsf rotation;
    rotation.SetRotation(axis, angleDegrees * (M_PI / 180.0));
    TopLoc_Location rotLoc(rotation);

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToRotate)) {
            cotm("Rotatexl");
            // PRE-multiply: apply rotation in global coordinates
            TopLoc_Location newLoc = rotLoc * currentShape.Location();
            TopoDS_Shape rotatedShape = currentShape;
            rotatedShape.Location(newLoc);

            builder.Add(newCompound, rotatedShape);
            shapeToRotate = rotatedShape;
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToRotate;
});
lua.set_function("Rotatel", [&](float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 0) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToRotate = current_part->shape;

    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    // --- Make rotation relative to the part’s local origin ---
    // Get the current transformation of the part
    gp_Trsf partTrsf = shapeToRotate.Location().Transformation();

    // Compute the global position of the part's local (0,0,0)
    gp_Pnt localOrigin = gp_Pnt(0, 0, 0).Transformed(partTrsf);

    // Define the axis using that origin
    gp_Ax1 axis(localOrigin, gp_Dir(x, y, z));

    // Create the rotation transformation
    gp_Trsf rotation;
    rotation.SetRotation(axis, angleDegrees * (M_PI / 180.0));
    TopLoc_Location rotLoc(rotation);

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(shapeToRotate)) {
            // cotm("Rotatexl");

            // Apply rotation relative to the *local origin* (not world)
            TopLoc_Location newLoc = rotLoc * currentShape.Location();

            TopoDS_Shape rotatedShape = currentShape;
            rotatedShape.Location(newLoc);

            builder.Add(newCompound, rotatedShape);
            shapeToRotate = rotatedShape;
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    current_part->cshape = newCompound;
    current_part->shape = shapeToRotate;
});


lua.set_function("Rotatelx", [&](float angleDegrees = 0.0f) {
  // 1. Get the function from the global Lua table (using the sol::state object 'lua')
  sol::protected_function Rotatel = lua["Rotatel"];

  // 2. Call the function with the desired arguments
  // sol::protected_function::operator() returns a sol::protected_function_result
  Rotatel(angleDegrees, 1);
});
lua.set_function("Rotately", [&](float angleDegrees = 0.0f) { 
  sol::protected_function Rotatel = lua["Rotatel"]; 
  Rotatel(angleDegrees, 0,1);
});
lua.set_function("Rotatelz", [&](float angleDegrees = 0.0f) { 
  sol::protected_function Rotatel = lua["Rotatel"]; 
  Rotatel(angleDegrees, 0,0,1);
});
// Lua binding
lua.set_function("Connect_v1", [&](const std::string& s) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    // Parse serialized line (world/model-space centroid + normal)
    std::istringstream iss(s);
    std::string name;
    int idx;
    double nx, ny, nz, cx, cy, cz, areaIn;
    char comma;
    if (!(iss >> name >> idx
              >> nx >> comma >> ny >> comma >> nz
              >> cx >> comma >> cy >> comma >> cz
              >> areaIn))
    {
        luaL_error(lua.lua_state(),
                   "Expected: \"partName index nx,ny,nz cx,cy,cz area\"");
    }

    Handle(AIS_Shape) aisShape = current_part->ashape;
    if (aisShape.IsNull())
        luaL_error(lua.lua_state(), "Current part has no AIS_Shape");

    // Full owner shape in raw model space
    TopoDS_Shape fullRaw = aisShape->Shape();
    fullRaw.Location(TopLoc_Location());

    // Map all faces
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(fullRaw, TopAbs_FACE, faceMap);

    // Incoming signature (world/model space)
    gp_Dir nIn(nx, ny, nz);
    gp_Pnt cIn(cx, cy, cz);

    // Matching tolerances/weights
    const Standard_Real tolLen  = 1.0e-4;     // length (model units)
    const Standard_Real tolAng  = 1.0e-3;     // radians
    const Standard_Real tolRelA = 5.0e-3;     // 0.5%
    const Standard_Real wAng    = 10.0;
    const Standard_Real wLen    = 1.0;
    const Standard_Real wRelA   = 0.25;

    auto faceNormalW = [](const TopoDS_Face& F) -> gp_Dir {
        Standard_Real u1,u2,v1,v2;
        BRepTools::UVBounds(F, u1,u2, v1,v2);
        Handle(Geom_Surface) surf = BRep_Tool::Surface(F);
        GeomLProp_SLProps slp(surf, (u1+u2)*0.5, (v1+v2)*0.5, 1, Precision::Confusion());
        gp_Dir n = slp.IsNormalDefined() ? slp.Normal() : gp_Dir(0,0,1);
        if (F.Orientation() == TopAbs_REVERSED) n.Reverse();
        return n;
    };

    auto faceCentroidAreaW = [](const TopoDS_Face& F, gp_Pnt& cW, double& a) {
        GProp_GProps props;
        BRepGProp::SurfaceProperties(F, props);
        cW = props.CentreOfMass();
        a  = props.Mass();
    };

    // Cost in world space (use unsigned normal angle)
    auto costFace = [&](const gp_Dir& nW, const gp_Pnt& cW, double aW) -> double {
        const double cosang = std::abs(nW.Dot(nIn));
        const double ang = std::acos(std::clamp(cosang, -1.0, 1.0));
        const double cDist = cW.Distance(cIn);
        const double relA  = std::abs(aW - areaIn) / std::max(aW, areaIn);
        return wAng * ang + wLen * cDist + wRelA * relA;
    };

    // Try to resolve by signature
    int resolvedIdx = -1;
    double bestCost = std::numeric_limits<double>::infinity();

    for (int i = 1; i <= faceMap.Extent(); ++i) {
        const TopoDS_Face F = TopoDS::Face(faceMap(i));
        gp_Pnt cW; double aW = 0.0; faceCentroidAreaW(F, cW, aW);
        gp_Dir nW = faceNormalW(F);

        const double cosang = std::abs(nW.Dot(nIn));
        const double ang = std::acos(std::clamp(cosang, -1.0, 1.0));
        const bool okAng  = (ang <= tolAng);
        const bool okCent = (cW.Distance(cIn) <= tolLen);
        const bool okArea = (std::abs(aW - areaIn) <= tolRelA * std::max(aW, areaIn));

        const double cost = costFace(nW, cW, aW);

        if ((okAng && okCent && okArea && cost < bestCost) || cost < bestCost) {
            bestCost = cost;
            resolvedIdx = i;
        }
    }

    const int idxToUse = (resolvedIdx > 0) ? resolvedIdx : idx;
    const TopoDS_Face targetFace = TopoDS::Face(faceMap(idxToUse));

    // Build a stable in-plane X from topology; get Z from the actual face normal
    auto intrinsicFrame = [&](const TopoDS_Face& F, gp_Pnt& anchorW, gp_Dir& zW, gp_Dir& xW) -> bool {
        zW = faceNormalW(F);

        TopoDS_Wire w = BRepTools::OuterWire(F);
        if (!w.IsNull()) {
            BRepTools_WireExplorer wexp(w);
            if (wexp.More()) {
                TopoDS_Edge e1 = wexp.Current();
                TopoDS_Vertex vAnchor = wexp.CurrentVertex();
                TopoDS_Vertex vA, vB; TopExp::Vertices(e1, vA, vB);
                TopoDS_Vertex vNext = vAnchor.IsSame(vA) ? vB : vA;

                anchorW = BRep_Tool::Pnt(vAnchor);
                gp_Pnt nextW = BRep_Tool::Pnt(vNext);

                gp_Vec edgeVec(anchorW, nextW);
                gp_Vec z(zW.X(), zW.Y(), zW.Z());
                gp_Vec xProj = edgeVec - (edgeVec.Dot(z)) * z;

                if (xProj.SquareMagnitude() > Precision::SquareConfusion()) {
                    xW = gp_Dir(xProj);
                    return true;
                }
            }
        }

        // Fallback: project global X (then Y) into the plane
        anchorW = BRep_Tool::Pnt(TopExp::FirstVertex(TopoDS::Edge(TopExp_Explorer(F, TopAbs_EDGE).Current())));
        gp_Vec z(zW.X(), zW.Y(), zW.Z());

        gp_Vec alt(1,0,0); gp_Vec xProj = alt - (alt.Dot(z)) * z;
        if (xProj.SquareMagnitude() < Precision::SquareConfusion()) {
            alt = gp_Vec(0,1,0);
            xProj = alt - (alt.Dot(z)) * z;
        }
        xW = gp_Dir(xProj);
        return true;
    };

// Rebuild destination CS using world-space centroid directly
gp_Pnt anchorW; gp_Dir zW; gp_Dir xW;
if (!intrinsicFrame(targetFace, anchorW, zW, xW))
    Standard_Failure::Raise("Failed to build face frame");

// Flip Z if needed
if (zW.Dot(nIn) < 0) zW.Reverse();

// Destination frame origin = stored centroid (world/model space)
gp_Ax3 dstCS(cIn, zW, xW);

// Source frame = part's "XY face"
gp_Ax3 srcCS(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));

gp_Trsf place;
place.SetTransformation(srcCS, dstCS);

current_part->trsf = place;
current_part->needsplacementupdate = 1;
});

//keeper
lua.set_function("Fuse1", [&](OCC_Viewer::luadraw* tofuse2){
		OCC_Viewer::luadraw* tofuse1=current_part;
		if (!tofuse1 || !tofuse2) {
			throw std::runtime_error(lua_error_with_line(L, "Invalid luadraw pointer in fuse"));
		}
		// tofuse1->update_placement();
		// tofuse2->update_placement();
		// Handle(AIS_Shape) af1 = tofuse1->ashape;
		// Handle(AIS_Shape) af2 = tofuse2->ashape;
		TopoDS_Shape ts1 = tofuse1->shape;
		TopoDS_Shape ts2 = tofuse2->shape;

		BRepAlgoAPI_Fuse fuse(ts1, ts2);
		fuse.Build();
		if (!fuse.IsDone()) {
			throw std::runtime_error(lua_error_with_line(L, "Fuse operation failed"));
		}

		// tofuse1->visible_hardcoded = 0;
		tofuse2->visible_hardcoded = 0;

		TopoDS_Shape fusedShape = fuse.Shape();

		// Refine the result
		ShapeUpgrade_UnifySameDomain unify(fusedShape, true, true, true);  // merge faces, edges, vertices
		unify.Build();
		tofuse1->shape = unify.Shape();
});

lua.set_function("compound", [&](){
    TopoDS_Shape shape = current_part->cshape;
    std::function<void(const TopoDS_Shape&, int)> dump = [&](const TopoDS_Shape& s, int indent) {
        if (s.IsNull()) return;
        std::string prefix(indent * 2, ' ');
        std::string typeName;
        switch (s.ShapeType()) {
            case TopAbs_COMPOUND:   typeName = "COMPOUND"; break;
            case TopAbs_COMPSOLID:  typeName = "COMPSOLID"; break;
            case TopAbs_SOLID:      typeName = "SOLID"; break;
            case TopAbs_SHELL:      typeName = "SHELL"; break;
            case TopAbs_FACE:       typeName = "FACE"; break;
            case TopAbs_WIRE:       typeName = "WIRE"; break;
            case TopAbs_EDGE:       typeName = "EDGE"; break;
            case TopAbs_VERTEX:     typeName = "VERTEX"; break;
            default: return;
        }
        std::cout << prefix << typeName << std::endl;

        // Only explore children for shapes that can logically contain them
        TopAbs_ShapeEnum childType;
        bool shouldExplore = true;
        switch (s.ShapeType()) {
            case TopAbs_COMPOUND:
            case TopAbs_COMPSOLID:
                // Compounds and Compsolids can contain any type
                for (int i = TopAbs_COMPOUND; i <= TopAbs_VERTEX; ++i) {
                    TopExp_Explorer exp(s, static_cast<TopAbs_ShapeEnum>(i));
                    if (exp.More()) { // Only proceed if there are children
                        for (; exp.More(); exp.Next()) {
                            dump(exp.Current(), indent + 1);
                        }
                    }
                }
                break;
            case TopAbs_SOLID:
                childType = TopAbs_SHELL;
                break;
            case TopAbs_SHELL:
                childType = TopAbs_FACE;
                break;
            case TopAbs_FACE:
                childType = TopAbs_WIRE;
                break;
            case TopAbs_WIRE:
                childType = TopAbs_EDGE;
                break;
            case TopAbs_EDGE:
                childType = TopAbs_VERTEX;
                break;
            default:
                shouldExplore = false;
                break;
        }

        if (shouldExplore && s.ShapeType() != TopAbs_VERTEX) {
            TopExp_Explorer exp(s, childType);
            if (exp.More()) { // Only proceed if there are children
                for (; exp.More(); exp.Next()) {
                    dump(exp.Current(), indent + 1);
                }
            }
        }
    };
    dump(shape, 0);
});






	// std::vector<std::string> methods = {"pl", "offset", "extrude"};
	// for (auto& m : methods) {
	// 	lua.set_function(m, [&, m](sol::variadic_args va) { call_on_current(m, va); });
	// }
	// Optional free-function variant, if you like: create_pl(d, "1,2 3,4")
	// lua.set_function("create_pl",
	//     [parse_coords](OCC_Viewer::luadraw& self, const std::string& coords) {
	//         self.CreateWire(parse_coords(coords), false);
	//     }
	// );
}

 
// void extractMethodNames(const FunctionDecl* bindFunc) {
//     std::vector<std::string> methodNames;

//     // Find all method binding statements
//     for (auto stmt : bindFunc->body()) {
//         if (isMethodBinding(stmt)) {
//             methodNames.push_back(getMethodName(stmt));
//         }
//     }

//     return methodNames;
// }

#define setluafunc(val, desc)                               \
	{                                                       \
		lua_pushcfunction(L, val);                          \
		lua_setglobal(L, #val);                             \
		if (luafuncs.count(val) == 0) luafuncs[val] = desc; \
	}

unordered_map<lua_CFunction, string> luafuncs;

string getfunctionhelp() {
	string val;
	for (const auto& pair : luafuncs) {
		// lua_CFunction func = pair.first;
		val += (pair.second) + "\n\n";
	}
	return val;
}
void lua_hook(lua_State* L, lua_Debug* ar) {
	int cl = (ar->currentline);
	// if (lua_getstack(L, 0, ar)) {
	lua_getinfo(L, "nSl", ar);
	cotm(cl, ar->source, ar->name)
	// }
}


//
void luainit() {
    if (G){
		G.reset();
	} 
    // if (G) return;
    if (!occv) cotm("occv not init");

    G = std::make_unique<sol::state>();
    auto& lua = *G;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua["package"]["path"] = std::string("./lua/?.lua;") + std::string(lua["package"]["path"]);
    L = lua.lua_state();

    bind_luadraw(lua, occv);
    setluafunc(close, "close()Exit script.");
    install_shorthand_searcher(lua.lua_state());
}

nmutex lua_mtx("lua_mtx", 1);

void clearAll(OCC_Viewer* occv){
	// cotm2("luarun", occv->vaShape.size());
	for (int i = static_cast<int>(occv->vaShape.size()) - 1; i >= 0; --i) {
		occv->m_context->Deactivate(occv->vaShape[i]);
		if (occv->m_context->IsDisplayed(occv->vaShape[i]))
			occv->m_context->Remove(occv->vaShape[i], Standard_False);
		else
			occv->m_context->Erase(occv->vaShape[i], Standard_False);
		occv->vaShape[i].Nullify();
		occv->vlua[i]->cshape.Nullify();
		occv->vlua[i]->shape.Nullify();
		occv->vlua[i]->Origin=TopLoc_Location();
	}
	current_part=nullptr;
	// for (int i = static_cast<int>(occv->vlua.size()) - 1; i >= 0; --i) {
	// 	// delete occv->vlua[i];
	// }



	occv->vshapes.clear();
	occv->vaShape.clear();
	occv->ulua.clear();
	occv->vlua.clear();

	// gp_Trsf tr;
	// tr.SetTranslation(gp_Vec(0, 0, 0));  // no move
	// occv->Origin=TopLoc_Location(tr);
	occv->Origin = TopLoc_Location(); //reset 
}

// #include <AIS_Shape.hxx>
// #include <BRepMesh_IncrementalMesh.hxx>
// #include <TopoDS_Shape.hxx>

// Refine tessellation for all AIS_Shapes in a vector
void RefineAISShapes(std::vector<Handle(AIS_Shape)>& shapes,
                     double deflection = 0.000001,
                     bool isRelative = 1)
{
    for (auto& aisShape : shapes)
    {
        if (aisShape.IsNull()) continue;

        // Get the underlying TopoDS_Shape
        TopoDS_Shape topo = aisShape->Shape();

        // Apply remeshing (triangulation)
        BRepMesh_IncrementalMesh(topo, deflection, isRelative);

        // Update AIS_Shape with the remeshed geometry
        aisShape->Set(topo);

        // Optional: recompute presentation
        aisShape->Redisplay(Standard_True);
    }
}


void lua_str(const string &str, bool isfile) {
    // thread([](string str, bool isfile, OCC_Viewer* occv) {
        lua_mtx.lock();
		// std::lock_guard<std::mutex> lock(lua_mtx);
		perf();
        luainit();

		clearAll(occv);
        // cotm("luarun", occv->vaShape.size());
        // for (int i = static_cast<int>(occv->vaShape.size()) - 1; i >= 0; --i) {
        //     occv->m_context->Deactivate(occv->vaShape[i]);
        //     if (occv->m_context->IsDisplayed(occv->vaShape[i]))
        //         occv->m_context->Remove(occv->vaShape[i], Standard_False);
        //     else
        //         occv->m_context->Erase(occv->vaShape[i], Standard_False);
        //     occv->vaShape[i].Nullify();
        //     occv->vlua[i]->cshape.Nullify();
        // }
        // occv->vshapes.clear();
        // occv->vaShape.clear();
        // occv->ulua.clear();
        // occv->vlua.clear();
		
        int status;
        std::string code;
        if (isfile) {
            std::ifstream f(str, std::ios::binary);
            if (!f) {
                std::cerr << "Load error: cannot open " << str << std::endl;
                lua_mtx.unlock();
                return;
            }
            std::string src((std::istreambuf_iterator<char>(f)), {});
            code = translate_shorthand(src);
			// code+="\nrobot1()";
			// cotm2(str)
			// cotm2(code.data())
            status = luaL_loadbuffer(L, code.data(), code.size(), str.c_str());
        } else {
            code = translate_shorthand(str);
			// code+="\nrobot1()";
            status = luaL_loadbuffer(L, code.data(), code.size(), "chunk");
        }

        if (status == LUA_OK) {
            if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
                std::cerr << "runtimer error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
				// lua_mtx.unlock(); return;
            }
			perf("lua");

            Fl::awake(fillbrowser);

			cotm2("awake(fillbrowser)")



// gp_Trsf scaleTrsf;
// scaleTrsf.SetScale(gp_Pnt(0,0,0), 10.0); // scale 10x around origin

// for (auto& aisShape : occv->vaShape) {
//     // Get the underlying TopoDS_Shape
//     TopoDS_Shape original = aisShape->Shape();

//     // Apply scaling to geometry
//     BRepBuilderAPI_Transform geomTrsf(original, scaleTrsf);
//     TopoDS_Shape scaledGeom = geomTrsf.Shape();

//     // Replace geometry in AIS_Shape
//     aisShape->Set(scaledGeom);

//     // Apply same scaling to AIS local transformation
//     aisShape->SetLocalTransformation(scaleTrsf * aisShape->LocalTransformation());
// }

			//error?
			// RefineAISShapes(occv->vaShape,0.1,1);



        } else {
            std::cerr << "Load error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
        lua_mtx.unlock();
    // }, str, isfile, occv).detach();
}
void lua_str_realtime(const string &str){
	
    // luainit();
	// cotm(str);
	// clearAll(occv);
	// int status;
	// std::string code;
	// code = translate_shorthand(str);
	// status = luaL_loadbuffer(L, code.data(), code.size(), "chunk");


}
#if 0
void luainit() {
	if (G) return;
	if (!occv)
		cotm("occv not init")

			G = std::make_unique<sol::state>();
	auto& lua = *G;

	lua.open_libraries(sol::lib::base, sol::lib::package);

	// Adjust package.path
	lua["package"]["path"] = std::string("./lua/?.lua;") + std::string(lua["package"]["path"]);

	// Example: if you need the raw lua_State* for C APIs
	L = lua.lua_state();
	// lua_sethook(L, my_hook, LUA_MASKLINE, 0);

	bind_luadraw(lua, occv);

	// Bind types
	// bind_occt_value_types(lua);
	// bind_occ_viewer_type(lua);
	// bind_luadraw(lua);

	// Make your OCC_Viewer* available to Lua (as a usertype instance)
	// lua["occv"] = occv;

	// // Optional: a convenience factory if you prefer function-call style
	// lua.set_function("new_luadraw", [occv](const std::string& name) ->
	// OCC_Viewer::luadraw* {
	//     return new OCC_Viewer::luadraw(name, occv);
	// });

	// setluafunc(Part,
	// 		   "Part(title,axle2,...)\
	// 		Sets the rotation angle of the servos of the arm.");

	setluafunc(close,
			   "close()\
			Exit script.");
}

nmutex lua_mtx("lua_mtx", 1);

void lua_str(string str, bool isfile) {
	thread(
		[](string str, bool isfile, OCC_Viewer* occv) {
			lua_mtx.lock();
			luainit();
			// perf1();
			// init_luas.init();
			// cotm(lua_mtx.waiting_threads.load())
			// if(all_cancel && lua_mtx.waiting_threads.load()<=1){
			//     all_cancel=0;
			//     lua_mtx.unlock();
			//     return;
			// }
			// if(all_cancel){lua_mtx.unlock(); return;}

			// temporario
			//  {
			//  OCC_Viewer::luadraw* tt= ulua["test5"];
			//  m_context->Remove(tt->ashape,0);
			//  delete tt;
			//  occv->ulua.clear();
			//  occv->m_context->RemoveAll(0);
			//  occv->vaShape.clear();
			//  Delete all pointers and clear the map
			//  deletelua("test5");
			// auto it = occv->ulua.begin();
			// while (it != occv->ulua.end()) {
			//     deletelua(it->second->name);  // Your deletion function
			//     // OR do it directly:
			//     /*
			//     OCC_Viewer::luadraw* ptr = it->second;
			//     if (!ptr->ashape.IsNull() && !occv->m_context.IsNull()) {
			//         occv->m_context->Remove(ptr->ashape, 0);
			//     }
			//     delete ptr;
			//     */
			//     // it--;  // Returns next valid iterator
			// }
			// // occv->ulua.clear();  // Clear the map entries
			// cotm("All elements removed from ulua");
			// }

			// lop(i,0,occv->vaShape.size()){
			// 	// OCC_Viewer::luadraw*ld=
			// occv->getluadraw_from_ashape(occv->vaShape[i]);
			// 	// if(ld){
			// 	// 	cotm(1)
			// 	// 	delete ld;
			// 	// 	ld=nullptr;
			// 	// }
			// 	occv->m_context->Remove(occv->vaShape[i],0);
			// }
			// cotm(2)
			// occv->vaShape.clear();
			// occv->ulua.clear();

			// 		for (auto& ashape : occv->vaShape) {
			//     occv->m_context->Remove(ashape, 0);
			// }
			// auto& va = occv->vaShape;
			// va.erase(std::unique(va.begin(), va.end()), va.end());
			// occv->vaShape.clear();
			// occv->ulua.clear();

			cotm("luarun", occv->vaShape.size())
				// temporary
				// for (auto& ashape : occv->vaShape) {
				//     if (!ashape.IsNull()) {
				//         // occv->m_context->Erase(ashape, 0);
				//         // occv->m_context->Remove(ashape, 0);
				//     }
				// }
				// lop(i,0,occv->vaShape.size()){
				// 	// occv->vlua[i]->shape.Free(1);
				// 	auto& ashape=occv->vaShape[i];
				// 	occv->m_context->Remove(ashape, 0);
				// }
				// occv->m_context->RemoveAll(0);

				// Remove in reverse order (in case of dependencies)

				//     for (int i = static_cast<int>(occv->vaShape.size()) - 1;
				//     i >= 0; --i)
				//     {
				// 		// occv->vlua[i]->shape.Nullify();
				//         try
				//         {
				//             if (!occv->vaShape[i].IsNull())
				//             {
				//                 // Remove AIS object from the context
				//                 occv->m_context->Remove(occv->vaShape[i],
				//                 /*erase from viewer*/ false);
				//             }

				//             // Also remove from Lua-side if exists
				//             // if (i < static_cast<int>(occv->vlua.size()) &&
				//             occv->vlua[i])
				//             // {
				//             //     // Avoid touching shared TopoDS_Shape here
				//             //     occv->vlua[i]->shape.Nullify();
				//             // }
				//         }
				//         catch (const Standard_Failure& e)
				//         {
				//             std::cerr << "OCC error removing shape index " <<
				//             i << ": "
				//                       << e.GetMessageString() << "\n";
				//         }

				// 		TopoDS_Compound comp =
				// TopoDS::Compound(occv->vlua[i]->shape); comp.Nullify();
				// 		delete occv->vlua[i];
				//     }

				// occv->SafeClearShapes();

				for (int i = static_cast<int>(occv->vaShape.size()) - 1; i >= 0; --i) {
				// occv->vlua[i]->shape.Nullify();

				// delete occv->vlua[i];
			}
			// occv->m_context->RemoveAll(0);
			// occv->m_context->EraseAll(Standard_True);
			// occv->m_context->Clear(Standard_True);

			// 1) Stop any hover/selection first
			occv->m_context->ClearCurrents(Standard_False);
			occv->m_context->ClearSelected(Standard_False);
			for (int i = static_cast<int>(occv->vaShape.size()) - 1; i >= 0; --i) {
				auto& aisShapeHandle = occv->vaShape[i];
				// 2) Deactivate selection modes for this specific AIS object
				occv->m_context->Deactivate(aisShapeHandle);

				// 3) Remove it from the context
				if (occv->m_context->IsDisplayed(aisShapeHandle))
					occv->m_context->Remove(aisShapeHandle, Standard_False);
				else
					occv->m_context->Erase(aisShapeHandle, Standard_False);

				// 4) Update the viewer once you’re done
				// occv->m_context->UpdateCurrentViewer();

				// 5) Release your last handle to allow cleanup
				aisShapeHandle.Nullify();

				// 6) Only after the AIS is gone, release/nullify the
				// TopoDS_Compound compoundShape.Nullify();
				occv->vlua[i]->cshape.Nullify();
			}

			occv->vshapes.clear();
			occv->vaShape.clear();
			occv->ulua.clear();
			occv->vlua.clear();

			int status;
			if (isfile) {
				status = luaL_loadfile(L, str.c_str());
			} else {
				status = luaL_loadstring(L, str.c_str());
			}

			if (status == LUA_OK) {
				if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
					std::cerr << "runtime error: " << lua_tostring(L, -1) << std::endl;
					lua_pop(L, 1);
				}

				// perf1("lua script");
				// lop(i,0,occv->vaShape.size()){
				// 	OCC_Viewer::luadraw*
				// ld=occv->getluadraw_from_ashape(occv->vaShape[i]);
				// 	// ld->redisplay();
				// 	cotm("vashape",occv->vaShape.size());
				// }
				Fl::awake(fillbrowser);
				// occv->fillvectopo();
				// 		cotm("vshapes",occv->vshapes.size());
				// perf1("bench fillvectopo");
				// occv->toggle_shaded_transp(occv->currentMode);
				// occv->redraw();

			} else {
				std::cerr << "Load error: " << lua_tostring(L, -1) << std::endl;
				lua_pop(L, 1);
			}

			// cotm(2)
			// lua_close(L);
			// L=0;
			// 	    if (G) {
			//     // G->collect_garbage(); // optional, run GC before closing
			//     G.reset();
			// }
			lua_mtx.unlock();
		},
		str, isfile, occv)
		.detach();
}
#endif
#pragma endregion lua



static Fl_Menu_Item items[] = {
	// Main menu items
	{"&File", 0, 0, 0, FL_SUBMENU},
	{"&Open", FL_ALT + 'o', ([](Fl_Widget*, void* v) { open_cb(); }), (void*)menu},

	// {"Pause", 0, ([](Fl_Widget* fw, void* v) {
	// 	 if (!occv) cotm("notok") occv->m_view->SetProj(V3d_XposYposZpos);
	// 	 // occv->animate_flip_view(occv);
	// 	 // Fl_Menu_Item* btpause=
	// 	 // const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)fw)->find_item("&File/Pause"));
	// 	 // if(btpause->value()>0)
	// 	 // // 	lop(i,0,pool.size())
	// 	 // // 		pool[i]->pause=1;
	// 	 // // else
	// 	 // // 	lop(i,0,pool.size())
	// 	 // // 		pool[i]->pause=0;

	// 	 // cot1(btpause->value());
	//  }),
	//  (void*)menu, FL_MENU_TOGGLE},

	// {"&Get view", FL_COMMAND + 'g', ([](Fl_Widget*, void* v) {
	// 	 occv->m_view->SetProj(1, 1, 1);
	// 	 // // ve[0]->rotate(Vec3f(0,0,1),45);
	// 	 // getview();
	// 	 // // Break=1;

	// 	 // 	// ve[0]->rotate( 10);
	// 	 // 	// cot(*ve[1]->axisbegin );
	// 	 // 	// cot(*ve[1]->axisend );
	//  }),
	//  (void*)menu},

	// {"Inverse kinematics", 0, ([](Fl_Widget*, void* v) {
	// 	 Standard_Real dx, dy, dz, ux, uy, uz;
	// 	 occv->m_view->Proj(dx, dy, dz);
	// 	 occv->m_view->Up(ux, uy, uz);
	// 	 cotm("draw", dx, dy, dz, ux, uy, uz);
	// 	 // // ve[ve.size()-1]->rotate_posk(10);
	// 	 // // dbg_pos(); /////
	// 	 // ve[3]->posik(vec3(150,150,-150));
	//  }),
	//  (void*)menu},

	{"Quit", 0, ([](Fl_Widget*, void* v) {
		 cotm("exit");
		 exit(0);
	 })},
	{0},

	{"&View", 0, 0, 0, FL_SUBMENU},

	{"&Fit all", FL_ALT + 'f', ([](Fl_Widget*, void* v) { occv->m_view->FitAll(); occv->redraw();}), (void*)menu, FL_MENU_DIVIDER},

	{"&Transparent", FL_ALT + 't', ([](Fl_Widget* mnu, void* v) {
		 // Fl_Menu_Item* btn=
		 // const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)fw)->find_item("&View/Transparent"));
		 Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
		 const Fl_Menu_Item* item = menu->mvalue();	 // This gets the actually clicked item
		//  occv->fillvectopo();
		 if (!item->value()) {
			 occv->toggle_shaded_transp(AIS_WireFrame);
		 } else {
			 occv->toggle_shaded_transp(AIS_Shaded);
		 }
		 occv->redraw();
		 return;

		 if (item->value()) {  // Check if the item is checked
			 cotm(1);
			 occv->hlr_on = 1;
			 occv->projectAndDisplayWithHLR(occv->vshapes);
			 // occv->setbar5per();
		 } else {
			 cotm(0);
			 occv->hlr_on = 0;
			 occv->draw_objs();
		 }
		 // occv-> m_view->Update ();
		//  occv->m_view->Redraw();
		 occv->redraw();  //  redraw(); //
	 }),
	 (void*)menu, FL_MENU_TOGGLE},

	// {"&test", 0, ([](Fl_Widget*, void* v) {
	// 	//  perf();
	// 	//  lop(i, 0, 40) occv->test(i * 5);
	// 	//  perf("test");
	// 	 {
	// 		occv->test(0);
	// 		 occv->draw_objs();
	// 		 perf("draw");
	// 		 // occv->m_view->FitAll();
	// 		 occv->redraw();
	// 		 occv->colorisebtn();

	// 		 // occv->m_view->Redraw();
	// 		 perf("draw");

	// 		 // occv->m_view->Update();
	// 	 }
	//  }),
	//  (void*)menu},


	{0},

	{"&Help", 0, ([](Fl_Widget*, void* v) { 
		// win->begin();
		Fl_Group::current(nullptr);  // clear current group

		static Fl_Window* fhelp = new Fl_Window(
			win->x() + win->w()/4,   // position relative to parent
			win->y() + win->h()/4,
			win->w()/2, win->h()/2,
			"Help"
		);
		// fhelp->set_menu_window();

		// make it a child of 'parent' so no new taskbar icon
		fhelp->set_modal();   // owned by parent, not independent

		static Fl_Help_View* fh = new Fl_Help_View(0, 0, fhelp->w(), fhelp->h());
		fh->textfont(0);
		// if(string(fh->value()).size()<10) fh->value("<html><body>
		if(fh->value()==0) fh->value("<html><body>\
		<h2>Keys:</h2>\
		<p><b>Ctrl + Mouse Click </b> Go to the code of the Part under the mouse cursor\
		<p><p>\ 
		<h2>Commands:</h2>\
		<p><b>Origin(x,y,z) </b> Move all subsequent Parts to the specified (x,y,z) relative to world coordinates until another Origin is defined\
		<p><b>Part [label] </b> Create a new Part with a label of your choice\
		<p><b>Clone ([label],[copy placement=0]) </b> Clone the Part with the given label; [copy placement]: 0 = no, 1 = yes\
		<p><b>Pl [coords] </b> Create polyline with coords Autocad style, dont accept variables yet, e.g. Pl 0,0 10,0 @0,10 @-10,0 0,0  =  a square\
		<p><b>Offset ([thickness]) </b> Creates offset of last polyline, e.g. (closed polyline) Offset -3  = offset 3 inside, Offset 3  = offset 3 outside. e.g. (opened polyline) Offset -3  = offset 3 to right, Offset 3  = offset 3 to left\
		<p><b>Circle (radius,x,y) </b>  \
		<p><b>Extrude ([height]) </b> Extrusion of last polyline or circle from Part \
		<p><b>Dup () </b> Duplicates last shape from Part, so the new be moved or rotated \
		<p><b>Fuse () </b> \ Fuse the 2 last solids from same Part \
		<p><b>Movel (x,y,z) </b> \ Move last solid from Part \
		<p><b>Rotatelx (angle) </b> \ Rotate x on point relative to 0,0,0 of last solid from Part \
		<p><b>Rotately (angle) </b> \ Rotate y on point relative to 0,0,0 of last solid from Part \
		<p><b>Rotatelz (angle) </b> \ Rotate z on point relative to 0,0,0 of last solid from Part \
		</body></html>");


		fhelp->end();
		fhelp->show();

	 }),
	 (void*)menu,FL_MENU_DIVIDER},
	{0},

	{"test", 0, ([](Fl_Widget*, void* v) {
		 cotm("test")
		//  WriteBinarySTL(occv->ulua["corner_clones"]->shape,"test2.stl");
		 // // lua_init();
		 // // getallsqlitefuncs();
		 // string val=getfunctionhelp();
		 // cot1(val);
		 // fl_message(val.c_str());
	 }),
	 (void*)menu,FL_MENU_DIVIDER},
	{0},


	{0}	 // End marker
};

// Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
// // ... use aisShape ...
// aisShape.Nullify();
// BRepTools::Clean();

void test(){
	// 110,250,-20),vec3(110,250,200)
	gp_Pnt axisBegin(110,250,-20);    // Start point
gp_Pnt axisEnd(110,250,200);      // End point (Y-direction)

// and set direction 0,1,0



// Calculate direction vector from begin to end
gp_Vec directionVec(axisBegin, axisEnd);
gp_Dir direction(directionVec);

// Create the axis
gp_Ax1 axis(axisBegin, direction);

// Verify direction is (0,1,0)
std::cout << "Direction: (" << direction.X() << ", " 
          << direction.Y() << ", " << direction.Z() << ")" << std::endl;
}


std::string load_app_font(const std::string& filename);



int checkdepthbit()
{
#ifdef __linux__
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Failed to open X display\n";
        return 1;
    }

    int screen = DefaultScreen(dpy);
    int fbcount = 0;
    GLXFBConfig* fbc = glXGetFBConfigs(dpy, screen, &fbcount);

    if (!fbc) {
        std::cerr << "No FBConfigs found\n";
        return 1;
    }

    std::cout << "Found " << fbcount << " framebuffer configs\n";

    for (int i = 0; i < fbcount; ++i) {
        int depthSize = 0;
        int redSize = 0, greenSize = 0, blueSize = 0;

        glXGetFBConfigAttrib(dpy, fbc[i], GLX_DEPTH_SIZE, &depthSize);
        glXGetFBConfigAttrib(dpy, fbc[i], GLX_RED_SIZE, &redSize);
        glXGetFBConfigAttrib(dpy, fbc[i], GLX_GREEN_SIZE, &greenSize);
        glXGetFBConfigAttrib(dpy, fbc[i], GLX_BLUE_SIZE, &blueSize);

        std::cout << "FBConfig " << i
                  << " Depth=" << depthSize
                  << " R=" << redSize
                  << " G=" << greenSize
                  << " B=" << blueSize
                  << "\n";
    }

    XFree(fbc);
    XCloseDisplay(dpy);
#endif
    return 0;
}
static void ExtractSolids(const TopoDS_Shape& shape, TopoDS_Compound& outComp, BRep_Builder& B)
{
    if (shape.IsNull()) return;

    TopAbs_ShapeEnum t = shape.ShapeType();

    if (t == TopAbs_SOLID)
    {
        B.Add(outComp, shape);
        return;
    }

    if (t == TopAbs_COMPOUND || t == TopAbs_COMPSOLID || t == TopAbs_SHELL)
    {
        for (TopoDS_Iterator it(shape); it.More(); it.Next())
        {
            ExtractSolids(it.Value(), outComp, B);
        }
    }
}
TopoDS_Shape FixFaceOrientation(const TopoDS_Shape& shape)
{
    BRep_Builder B;
    TopoDS_Compound result;
    B.MakeCompound(result);

    for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next())
    {
        TopoDS_Face face = TopoDS::Face(exp.Current());

        // Fix wire orientation
        ShapeFix_Wire fixWire;
        fixWire.Load(BRepTools::OuterWire(face));
        fixWire.Perform();

        // Fix face orientation
        ShapeFix_Face fixFace;
        fixFace.Init(face);
        fixFace.Perform();

        B.Add(result, fixFace.Face());
    }

    return result;
}

void fill_menu(){
// menu->add("File/Export/test", 0,
//     [](Fl_Widget*, void* ud){
//         OCC_Viewer* v = (OCC_Viewer*)(ud);
		
// 		 WriteBinarySTL(occv->ulua["corner"]->shape,"../approbot/stl/test2.stl");
//     },
//     occv, 0);


menu->add("File/Export visible solids/STL Single solid", 0,
    [](Fl_Widget*, void* ud){
        OCC_Viewer* v = (OCC_Viewer*)(ud);

        // Create a single compound
        BRep_Builder B;
        TopoDS_Compound comp;
        B.MakeCompound(comp);

        bool added = false;

        for (int i = 0; i < v->vaShape.size(); i++) {

            if (!v->m_context->IsDisplayed(v->vaShape[i]) ||
                !v->vlua[i]->visible_hardcoded)
                continue;

            TopoDS_Shape s = v->vlua[i]->cshape;
            // TopoDS_Shape s = v->vaShape[i]->Shape();
            if (s.IsNull()) continue;

            if (!v->vlua[i]->Origin.IsIdentity()) {
                s = s.Located(TopLoc_Location(
                    v->vlua[i]->Origin.Transformation()
                ));
            }

            // B.Add(comp, s);
			ExtractSolids(s,comp,B);
            added = true;
        }

        if (!added) {
            fl_alert("No visible shapes to export.");
            return;
        }

        // Force triangulation of the entire compound
        // BRepMesh_IncrementalMesh(comp, 0.05, false);

TopoDS_Shape oriented = comp.Reversed();



// Export STL
// WriteBinarySTL(comp, "../approbot/stl/test2.stl");
WriteBinarySTL(oriented, "../approbot/stl/test2.stl");





		// WriteBinarySTL(fixed, "../approbot/stl/test2.stl");
		// WriteBinarySTL(comp, "../approbot/stl/all_shapes_fused");
        // Now write STL from the triangulated compound
        // StlAPI_Writer writer;
        // writer.ASCIIMode() = false;
        // writer.Write(comp, "../approbot/stl/all_shapes_fused.stl");
    },
    occv, 0);

menu->add("File/Export visible solids/STL Multiple solids", 0,
    [](Fl_Widget*, void* ud){
        OCC_Viewer* v = (OCC_Viewer*)(ud);

        bool exported_any = false;

        for (int i = 0; i < v->vaShape.size(); i++) {

            if (!v->m_context->IsDisplayed(v->vaShape[i]) ||
                !v->vlua[i]->visible_hardcoded)
                continue;

            TopoDS_Shape s = v->vlua[i]->cshape;
            if (s.IsNull()) continue;

            // Apply origin transform if needed
            if (!v->vlua[i]->Origin.IsIdentity()) {
                s = s.Located(TopLoc_Location(
                    v->vlua[i]->Origin.Transformation()
                ));
            }

            // Build a compound for this single part (in case it contains multiple solids)
            BRep_Builder B;
            TopoDS_Compound comp;
            B.MakeCompound(comp);

            ExtractSolids(s, comp, B);

            // Reverse orientation if you want consistent normals
            TopoDS_Shape oriented = comp.Reversed();

            // Build filename
            std::string fname = "../approbot/stl/";
            fname += v->vlua[i]->name;
            fname += ".stl";

            // Export STL
            WriteBinarySTL(oriented, fname.c_str());

            exported_any = true;
        }

        if (!exported_any) {
            fl_alert("No visible shapes to export.");
            return;
        }
    },
    occv, 0);


}
int main(int argc, char** argv) {
	test();
	// pausa
	Fl::set_font(FL_HELVETICA, "DejaVu Sans");
	Fl::use_high_res_GL(1);
	// setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
	std::cout << "FLTK version: " << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION << "." << FL_PATCH_VERSION
			  << std::endl;
	std::cout << "FLTK ABI version: " << FL_ABI_VERSION << std::endl;
	std::cout << "OCCT version: " << OCC_VERSION_COMPLETE << std::endl;
	// OSD_Parallel::SetUseOcctThreads(1);
	std::cout << "Parallel mode: " << OSD_Parallel::ToUseOcctThreads() << std::endl;

	Fl::visual(FL_DOUBLE | FL_INDEX);
	Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
// Fl::gl_visual(64);
	Fl::scheme("oxy");	
	
	Fl_Group::current(content);
 
	// string floaded = load_app_font("Courier Prime Bold.otf");
	// string floaded = load_app_font("DejaVuSansMono.ttf");
	// string floaded = load_app_font("FiraGO-SemiBold.otf");
	// string floaded = load_app_font("Cascadia Mono PL SemiBold 600.otf");
	// Fl::set_font(FL_HELVETICA, floaded.c_str());

	Fl::lock();
	int w = 1024;
	int h = 576;
	int firstblock = w * 0.62;
	int secondblock = w * 0.12;
	int lastblock = w - firstblock - secondblock;

	// Fl_Window* win = new Fl_Window(0, 0, w, h, "simplecad");
	// Fl_Double_Window* win = new Fl_Double_Window(0, 0, w, h, "simplecad"); 
	win = new Fl_Double_Window(0, 0, w, h, "simplecad"); 
	win->color(FL_RED);
	win->callback([](Fl_Widget* widget, void*) {
		if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) return;
		menu->find_item("&File/Quit")->do_callback(menu);
	});
	menu = new Fl_Menu_Bar(0, 0, firstblock, 22);
	menu->menu(items);


	content = new Fl_Group(0, 22, w, h - 22); 
	int hc1 = 24;
	occv = new OCC_Viewer(0, 22, firstblock, h - 22 - hc1-1);
	// content->add(occv);
	Fl_Group::current(content);
	woccbtn = new FixedHeightWindow(0, h - hc1, firstblock, hc1, ""); 
	
	// woccbtn->color(0x7AB0CfFF);
	// woccbtn->show();
	occv->drawbuttons(woccbtn->w(), hc1);
	woccbtn->resizable(woccbtn); 
	int htable=22*3;

	Fl_Group::current(content);
	fbm = new fl_browser_msv(firstblock, 22, secondblock, h - 22-htable);
	fbm->box(FL_UP_BOX);
	fbm->color(FL_WHITE);
	fbm->scrollbar_size(1);
	Fl_Scrollbar &vsb = fbm->scrollbar;  // vertical scrollbar 
	vsb.selection_color(FL_RED); 


	Fl_Group::current(content); 
	scint_init(firstblock + secondblock, 0, lastblock, h - 0 - htable); 

	Fl_Group::current(content); 
	helpv=new Fl_Help_View(firstblock, h-htable, w-firstblock, htable);
	helpv->box(FL_UP_BOX);
	helpv->value(R"(
<html> 
    <font face="Helvetica">
		<br><br>
      <b><font size=5 color="Red">Loading...</font></b>
    </font> 
</html>
)"); 

	
	fill_menu();

	win->resizable(content); 

	// win->position(Fl::w()/2-win->w()/2,10);
	win->position(0, 0);  
	// int x, y, _w, _h;
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22); 
	win->show(); 
	// win->show(argc, argv); 

	// woccbtn->damage(FL_DAMAGE_ALL); 
	// woccbtn->redraw(); 
	Fl::flush();  // make sure everything gets drawn
	win->flush();	 
	woccbtn->flush(); 
	// woccbtn->damage(FL_DAMAGE_VALUE); 

	// win->maximize();
	// occv->initialize_opencascade(); 


		Fl::add_timeout(0.7,[](void* d) {lua_str(currfilename,1);},0);				
		Fl::add_timeout(0.9,[](void* d) {occv->m_view->FitAll();},0);				
		Fl::add_timeout(1.5,[](void* d) {occv->sbt[6].occbtn->do_callback(); },0);				
		Fl::add_timeout(1.8,[](void* d) {occv->m_view->FitAll(); },0);				
 
		// checkdepthbit();
	return Fl::run();
}