#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

#include <FL/Fl.H>
#include <FL/Fl_Window.H> 
#include <FL/Fl_Gl_Window.H>
#include <FL/x.H>
// #include <FL/Fl_Gl_Choice.H>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx> 
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Quantity_Color.hxx>
#include <WNT_Window.hxx>  // Windows-specific
#include <Xw_Window.hxx>   // Linux-specific

#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <AIS_ColoredShape.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <V3d_View.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>  // ou Xw_Window se for Linux
#include <Aspect_DisplayConnection.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <Prs3d_Drawer.hxx>
#include <VrmlConverter_Drawer.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRAlgo_Projector.hxx>
#include <BRepBndLib.hxx> 
#include <StepVisual_CameraModelD3.hxx> 
#include <HLRAlgo_EdgeIterator.hxx> 
#include <AIS_Shape.hxx> 
#include <Prs3d_LineAspect.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Quantity_Color.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <AIS_Shape.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp.hxx>
#include <Geom_Line.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <AIS_Shape.hxx>
#include <Prs3d_LineAspect.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <TopoDS_Shape.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <HLRAlgo_Projector.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <Quantity_Color.hxx>
#include <HLRBRep_Data.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepTools.hxx>
#include <ShapeUpgrade_ShapeDivideContinuity.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Solid.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx> 
#include <Precision.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <Geom_Axis2Placement.hxx>
#include <AIS_Trihedron.hxx>
#include <OpenGl_Context.hxx>
#include <gp_Dir.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Trihedron.hxx>
#include <V3d_View.hxx>
#include <Geom_Axis2Placement.hxx>
#include <gp_Ax2.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <gp_Ax2.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <gp_Ax2.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <gp_Quaternion.hxx>

#include <GL/glx.h>

#include <chrono>

#include "general.hpp"


#include <Aspect_Window.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <FL/Fl_Gl_Window.H>

// class FltkWindow : public Aspect_Window {
// public:
//     FltkWindow(Fl_Gl_Window* flWin) : myFlWindow(flWin) {
//         if (!flWin) {
//             throw Standard_ProgramError("Null FLTK window passed to FltkWindow");
//         }
//     }

//     // 1. Return native window handle (X11/Win32/MacOS)
//     Aspect_Drawable NativeHandle() const override {
//         return (Aspect_Drawable)fl_xid(myFlWindow);
//     }

//     // 2. Return native FB config (optional)
//     Aspect_FBConfig NativeFBConfig() const override {
//         return NULL; // Can be null if not needed
//     }

//     // 3. Get window position and size
//     void Position(int& theX1, int& theY1, int& theX2, int& theY2) const override {
//         theX1 = myFlWindow->x();
//         theY1 = myFlWindow->y();
//         theX2 = theX1 + myFlWindow->w();
//         theY2 = theY1 + myFlWindow->h();
//     }

//     // 4. Check if window is mapped (visible)
//     Standard_Boolean IsMapped() const override {
//         return myFlWindow->visible() ? Standard_True : Standard_False;
//     }

//     // 5. Map (show) the window
//     void Map() const override {
//         myFlWindow->show();
//     }

//     // 6. Unmap (hide) the window
//     void Unmap() const override {
//         myFlWindow->hide();
//     }

//     // 7. Handle resize events - returns Aspect_TypeOfResize
//     Aspect_TypeOfResize DoResize() override {
//         int aWidth = 0, aHeight = 0;
//         Size(aWidth, aHeight);
        
//         // Simple resize detection - can be enhanced as needed
//         static int aPrevWidth = aWidth;
//         static int aPrevHeight = aHeight;
        
//         if (aWidth != aPrevWidth || aHeight != aPrevHeight) {
//             aPrevWidth = aWidth;
//             aPrevHeight = aHeight;
//             return Aspect_TOR_UNKNOWN; // Generic resize
//         }
//         return Aspect_TOR_NO_BORDER;
//     }

//     // 8. Always allow mapping
//     Standard_Boolean DoMapping() const override {
//         return Standard_True;
//     }

//     // 9. Get window aspect ratio
//     Standard_Real Ratio() const override {
//         int aWidth = 0, aHeight = 0;
//         Size(aWidth, aHeight);
//         return (aHeight > 0) ? (Standard_Real)aWidth / aHeight : 1.0;
//     }

//     // 10. Get window dimensions
//     void Size(int& theWidth, int& theHeight) const override {
//         theWidth = myFlWindow->w();
//         theHeight = myFlWindow->h();
//     }

//     // 11. Return true if window is virtual (always false for real windows)
//     Standard_Boolean IsVirtual() {
//         return Standard_False;
//     }

//     // 12. Get virtual window size (same as physical for real windows)
//     void VirtualSize(int& theWidth, int& theHeight)  {
//         Size(theWidth, theHeight);
//     }

// private:
//     Fl_Gl_Window* myFlWindow;
// };

class OCC_Viewer : public Fl_Gl_Window {
public:
    Handle(Aspect_DisplayConnection) m_display_connection;
    Handle(OpenGl_GraphicDriver) m_graphic_driver;
    Handle(V3d_Viewer) m_viewer;
    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_View) m_view;
    bool m_initialized = false;
    std::vector<TopoDS_Shape> vshapes; 
    std::vector<Handle(AIS_Shape)> vaShape;  
Aspect_RenderingContext aContext=0;

public:
    OCC_Viewer(int X, int Y, int W, int H, const char* L = 0)
        : Fl_Gl_Window(X, Y, W, H, L) {
        mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // Fl_Gl_Window::handle(0);

    }

    void initialize_opencascade() { 

        // Handle(FltkWindow) occWindow = new FltkWindow(this);

        // Get native window handle
#ifdef _WIN32
        HWND hwnd = (HWND)fl_xid(this);
        Handle(WNT_Window) wind = new WNT_Window(hwnd);
        m_display_connection = new Aspect_DisplayConnection("");
#else
        Window win = (Window)fl_xid((this));
        // context_valid();
        // make_current();
        Display* display = fl_display;
        
        m_display_connection = new Aspect_DisplayConnection(display);
        Handle(Xw_Window) wind = new Xw_Window(m_display_connection, win);
#endif

        m_graphic_driver = new OpenGl_GraphicDriver(m_display_connection);

        m_viewer = new V3d_Viewer(m_graphic_driver);
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();
        m_context = new AIS_InteractiveContext(m_viewer);
        m_view = m_viewer->CreateView();



        m_view->SetWindow(wind,aContext);
        m_view->SetBackgroundColor(Quantity_NOC_GRAY90);
        m_context->SetAutomaticHilight(true);  

        Handle(OpenGl_Context) aCtx = m_graphic_driver->GetSharedContext();
        
        m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_BLACK, 0.08);

        // Create and display a cube
        TopoDS_Shape cube = BRepPrimAPI_MakeBox(50.0, 50.0, 50.0).Shape();
        Handle(AIS_Shape) aisCube = new AIS_Shape(cube);
        m_context->Display(aisCube, Standard_True);

        // Create and display a trihedron
        gp_Ax2 axes(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
        Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(axes);
        Handle(AIS_Trihedron) trihedron = new AIS_Trihedron(placement);
        trihedron->SetSize(25.0);
        m_context->Display(trihedron, Standard_True);
// m_view->SetComputedMode(0); 
        m_view->MustBeResized();
        m_view->FitAll();
        m_initialized = true;
        m_view->Redraw();

        {const GLubyte* renderer = glGetString(GL_RENDERER);
const GLubyte* vendor = glGetString(GL_VENDOR);
const GLubyte* version = glGetString(GL_VERSION);

if (renderer && vendor && version) {
    std::cout << "OpenGL Vendor:   " << vendor << std::endl;
    std::cout << "OpenGL Renderer: " << renderer << std::endl;
    std::cout << "OpenGL Version:  " << version << std::endl;
} else {
    std::cout << "glGetString() failed — no OpenGL context active!" << std::endl;
}}
    }

    void draw() override { 
        // Get the FLTK OpenGL context
// make_current();
// valid(1);
//         if(!m_initialized && context_valid()){
// aContext = (Aspect_RenderingContext)glXGetCurrentContext();
// cotm("context")
// initialize_opencascade();
// test();
//         }
        if (!m_initialized) return;
    // static bool swapSet = false;
    // if (!swapSet) {
    //     swapSet = true;
    //     Display* dpy = fl_display;
    //     GLXDrawable drawable = glXGetCurrentDrawable();

    //     typedef void (*glXSwapIntervalEXTProc)(Display*, GLXDrawable, int);
    //     glXSwapIntervalEXTProc swapIntervalEXT =
    //         (glXSwapIntervalEXTProc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");

    //     if (swapIntervalEXT && drawable) {
    //         swapIntervalEXT(dpy, drawable, 0); // 0 = vsync off
    //     }
    // }


        // Ensure the OpenGL context is current for this window
        // make_current(); 
//         glFlush();
// glFinish();
        cotm("redr")
        m_view->Redraw();
    }

    void resize(int X, int Y, int W, int H) override {
        Fl_Gl_Window::resize(X, Y, W, H);
        if (m_initialized) {
            m_view->MustBeResized();
            // redraw(); 
        }
    }
    
    int handle(int event) override {
        // if (isRotatingq && event==FL_DRAG && Fl::wait() == 1) return 1;
        if(isRotating) if(event!=FL_DRAG && event!=FL_RELEASE)return 1; 
        
    // static double last_redraw = 0;
    // double now = Fl::event_seconds();
        // cotm(event);
        switch (event) {
            case FL_PUSH :
                if (Fl::event_button() == FL_LEFT_MOUSE) {
                    isRotating = true;
                    lastX = Fl::event_x();
                    lastY = Fl::event_y();
                    if (m_initialized) {
                        // make_current(); // Ensure context is current before starting rotation
                        m_view->StartRotation(lastX, lastY);
                    }
                    return 1;
                }
                break;
    
            case FL_DRAG :
                if (isRotating && m_initialized) { 
                    // Fl_Gl_Window::handle(event);
                    // int x = Fl::event_x();
                    // int y = Fl::event_y();
                    // // m_context->MakeCurrent();
                    // // make_current(); // Make sure context is current for every drag step
                    // // m_view->SetImmediateUpdate(Standard_True);  // prioriza resposta
                    // // m_view->Invalidate();
                    // // remove_fd(0); 
                    static std::chrono::steady_clock::time_point last_event;
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event).count();
                    if(elapsed>50){
                        m_view->Rotation(Fl::event_x(),Fl::event_y());
                        last_event = now;
                    }
                    // if (!Fl::has_timeout(0.1)) { // Throttle redraws
                    // m_view->Rotation(Fl::event_x(),Fl::event_y());
// }
                    // m_view->Rotation(x, y);
                    // Fl::unlock();
                    // cotm("redraw")
                    // redraw();
                    // m_view->Redraw();

                    
                    // Option A: Standard redraw (what we had). Still recommended.
                    // redraw(); 

                    // Option B (Diagnostic/Aggressive): Force immediate buffer swap.
                    // Use this ONLY for testing if Option A doesn't work.
                    // It can cause tearing if VSync is off, but might reduce perceived lag.
                    // If this makes it *instantly* responsive, the issue is FLTK's redraw queueing.
                    // swap_buffers(); 
                    // Fl::flush(); // Potentially useful with swap_buffers() for immediate display

                    return 1;
                }
                break;
    
            case FL_RELEASE :
                if (Fl::event_button() == FL_LEFT_MOUSE) {
                    // m_view->SetAnimationModeOff();
                    // m_view->Rotate(0.0, 0.0);
                    isRotating = false;
                    cotm("stop")
                    // m_view->SetImmediateUpdate(Standard_False); // volta à renderização completa
                    // m_view->Redraw();
                    // if (m_initialized) redraw();  // Ensure a final redraw after rotation ends
                    return 1;
                }
                break;
            
            // ... (rest of the handle method remains the same if you had more event types)
        }
        return Fl_Gl_Window::handle(event);
    }
    int lastX, lastY;
    bool isRotating = false;

    void test(){
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape(); 
        TopoDS_Shape pl1=pl(); 

        gp_Trsf trsf;
trsf.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), 45*(M_PI/180)); // rotate 30 degrees around Z-axis
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
ShapeUpgrade_UnifySameDomain unify(fusedShape, true, true, true); // merge faces, edges, vertices
unify.Build();
TopoDS_Shape refinedShape = unify.Shape();





{

    // gp_Pnt origin=gp_Pnt(50, 86.6025, 0);
    // gp_Dir normal=gp_Dir(0.5, 0.866025, 0);
    gp_Pnt origin=gp_Pnt(0, 0, 100);
    gp_Dir normal=gp_Dir(0,0,1);
gp_Dir xdir =   gp_Dir(0, 1, 0);
gp_Dir ydir =   gp_Dir(0.866025, -0.5, 0);
// gp_Ax2 ax3(origin, normal);
gp_Ax2 ax3(origin, normal, xdir);
// ax3.SetYDirection(ydir);
gp_Trsf trsf;
trsf.SetTransformation(ax3);
trsf.Invert();


gp_Trsf trsftmp; 
trsftmp.SetRotation(gp_Ax1(origin, normal), -45*(M_PI/180) );
trsf  *= trsftmp; 
trsftmp = gp_Trsf();
trsftmp.SetTranslation(gp_Vec(20, 0, 0));
trsf  *= trsftmp; 
 
ShowTriedronWithoutLabels(trsf,m_context);

gp_Pnt p1(0, 0,0);
    gp_Pnt p2(100, 0,0);
    gp_Pnt p3(100, 50,0);
    gp_Pnt p4(0, 50,0);

    
    // p1 = origin.Translated(gp_Vec(xdir) * (-0) + gp_Vec(ydir) * (-0));
    // p2 = origin.Translated(gp_Vec(xdir) * (p2.X()) + gp_Vec(ydir) * (p2.Y()));
    // p3 = origin.Translated(gp_Vec(xdir) * (p3.X()) + gp_Vec(ydir) * (p3.Y()));
    // p4 = origin.Translated(gp_Vec(xdir) * (p4.X()) + gp_Vec(ydir) * (p4.Y()));

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
    // gp_Dir xdir =   gp_Dir(0, 0, 1); // produto vetorial — perpendicular a pdir
    // gp_Dir xdir = pdir ^ gp_Dir(0, 0, 1); // produto vetorial — perpendicular a pdir
    
    // gp_Ax3 ax3(ploc,pdir,xdir);
    // Handle(Geom_Plane) plane = new Geom_Plane(ploc,pdir);
    // Handle(Geom_Plane) plane = new Geom_Plane(ax3);
    // gp_Pnt origin = plane->Location();
    // printf("origin: x=%.3f y=%.3f z=%.3f\n",origin.X(),origin.Y(),origin.Z());
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);
    // TopoDS_Face face = BRepBuilderAPI_MakeFace(plane, wire);

    BRepBuilderAPI_Transform transformer(face, trsf);
TopoDS_Shape rotatedShape = transformer.Shape();
    
gp_Ax3 ax3_;
ax3_.Transform(trsf);  // aplica trsf no sistema de coordenadas padrão
// gp_Dir dir = ax3.Direction();
gp_Dir dir = [](gp_Trsf trsf){ gp_Ax3 ax3; ax3.Transform(trsf); return ax3.Direction(); }(trsf);


gp_Vec extrusionVec(dir);
// gp_Vec extrusionVec(normal);
extrusionVec *= 10.0;
    TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(rotatedShape, extrusionVec).Shape();
    // TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(face, extrusionVec).Shape();

    vshapes.push_back(extrudedShape);
    // vshapes.push_back(rotatedShape);
    // vshapes.push_back(face);

}

vshapes.push_back(pl1);
vshapes.push_back(refinedShape);
// vshapes=std::vector<TopoDS_Shape>{pl1,face};
// vshapes=std::vector<TopoDS_Shape>{refinedShape,pl1,face};
draw_objs();
m_view->FitAll();
}

void draw_objs(){
//     Handle(Prs3d_Drawer) defaultDrawer = m_context->DefaultDrawer();
  
// // Set default line widths
// defaultDrawer->WireAspect()->SetWidth(3);
// defaultDrawer->LineAspect()->SetWidth(3);
    for(int i=0;i<vshapes.size();i++)
    {
        Handle(AIS_Shape) aShape = new AIS_Shape(vshapes[i]);
        vaShape.push_back(aShape);
        m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_True); 

          

        aShape->SetColor(Quantity_NOC_GRAY70);
        aShape->Attributes()->SetFaceBoundaryDraw(Standard_True);  
        Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 3.0);
        aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);


        aShape->Attributes()->SetSeenLineAspect(edgeAspect);
        // aShape->SetTransparency(0.5);
        m_context->Display(aShape, Standard_True); 
    }
}
TopoDS_Shape pl() {
    int x=-150;
    int y=-150;
    int z=-150;
    // Define points for the polyline
    gp_Pnt p1(x+0.0, y+0.0, z+0.0);
    gp_Pnt p2(x+100.0, y+0.0, z+0.0);
    gp_Pnt p3(x+100.0, y+10.0, z+0.0);
    gp_Pnt p4(x+0.0, y+100.0, z+0.0);

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
    gp_Vec extrusionVector(0.0, 0.0, -10.0); // Extrude along the Z-axis
    TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(face, extrusionVector).Shape();

    return extrudedShape;
}
// Display trihedron at origin with no axis labels
void ShowTriedronWithoutLabels(gp_Trsf trsf,const Handle(AIS_InteractiveContext)& ctx)
{
    // 1) create a trihedron at world origin
    gp_Ax2 ax2(gp_Pnt(0, 0, 0).Transformed(trsf), gp_Dir(0, 0, 1).Transformed(trsf), gp_Dir(1, 0, 0).Transformed(trsf));
    Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(ax2);
    Handle(AIS_Trihedron) tri = new AIS_Trihedron(placement);

    // 2) build a fresh DatumAspect and attach to the AIS_Trihedron
    Handle(Prs3d_DatumAspect) da = new Prs3d_DatumAspect();
    tri->Attributes()->SetDatumAspect(da);

    // 3) hide the “X Y Z” labels
    da->SetDrawLabels(false);                                     // :contentReference[oaicite:0]{index=0}

    // 4) style each axis line (width = 2.0 px)
    da->LineAspect(Prs3d_DatumParts_XAxis)->SetWidth(4.0);         // :contentReference[oaicite:1]{index=1}
    da->LineAspect(Prs3d_DatumParts_YAxis)->SetWidth(4.0);
    da->LineAspect(Prs3d_DatumParts_ZAxis)->SetWidth(4.0);

    // 5) color each axis
    da->LineAspect(Prs3d_DatumParts_XAxis)->SetColor(Quantity_NOC_RED);    // :contentReference[oaicite:2]{index=2}
    da->LineAspect(Prs3d_DatumParts_YAxis)->SetColor(Quantity_NOC_GREEN);
    da->LineAspect(Prs3d_DatumParts_ZAxis)->SetColor(Quantity_NOC_BLUE);

    // 5) color each arrow
    // set line‐width of the arrow stems (you already did this for the axes)
da->LineAspect( Prs3d_DatumParts_XArrow )->SetWidth( 2.0 );
da->LineAspect( Prs3d_DatumParts_YArrow )->SetWidth( 2.0 );
da->LineAspect( Prs3d_DatumParts_ZArrow )->SetWidth( 2.0 );

// now set each arrow’s colour
da->LineAspect( Prs3d_DatumParts_XArrow )->SetColor( Quantity_NOC_RED   );  // :contentReference[oaicite:0]{index=0}
da->LineAspect( Prs3d_DatumParts_YArrow )->SetColor( Quantity_NOC_GREEN );
da->LineAspect( Prs3d_DatumParts_ZArrow )->SetColor( Quantity_NOC_BLUE  );

    // 6) (optional) hide arrowheads if you don’t want them:
    //    da->SetDrawArrows(false);

    // 7) display it
    ctx->Display(tri, Standard_True);
}

};
OCC_Viewer* occv;
static void my_callback(Fl_Widget* widget, void* data) {
    cotm(9)
  int x = Fl::event_x();
                    int y = Fl::event_y();
                    // m_context->MakeCurrent();
                    // make_current(); // Make sure context is current for every drag step
                    // m_view->SetImmediateUpdate(Standard_True);  // prioriza resposta
                    // m_view->Invalidate();
                    // remove_fd(0);
                    occv->m_view->Rotation(x, y);
}
int main(int argc, char** argv) {
    Fl::gl_visual(FL_RGB | FL_DOUBLE | FL_DEPTH);
    // Fl::lock();
    Fl_Window* win = new Fl_Window(1000, 600, "FLTK with OpenCASCADE");
    occv = new OCC_Viewer(10, 10, 500, 580);
// //     occv->callback(my_callback);
//   occv->when(FL_DRAG );
    win->end();
    win->show(argc, argv); 
    occv->initialize_opencascade();


    //  Fl::use_high_res_GL(1) ;
    occv->test();
    return Fl::run();
}