#pragma region includes

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

 

#include <FL/Fl.H>
#include <FL/Fl_Window.H> 
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_File_Chooser.H> 
#include <FL/x.H>
#include <GL/gl.h>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>

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

#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Quantity_Color.hxx>
#include <Aspect_TypeOfLine.hxx>

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
#include <BRepBuilderAPI_MakeFace.hxx> 
#include <Graphic3d_ZLayerId.hxx>
#include <Aspect_GradientBackground.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <gp_Quaternion.hxx> // For quaternion-based rotation
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <gp_QuaternionSLerp.hxx> // For proper SLERP interpolation
#include <AIS_AnimationCamera.hxx>
#include <Graphic3d_Camera.hxx>
#include <Message.hxx>
#include <Standard_Version.hxx> 
#include <gp_Dir.hxx>
#include <BRepPrimAPI_MakeSphere.hxx> 
#include <SelectMgr_EntityOwner.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <AIS_Shape.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Quantity_Color.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <Standard_ProgramError.hxx>

#include <chrono>
#include <thread>
#include <functional>
#include <cmath>


#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Graphic3d_Camera.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Transient.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_SelectableObject.hxx>
#include <AIS_InteractiveObject.hxx>

#include <chrono>
#include <execution> // Para C++17 paralelismo

#include "general.hpp"

void scint_init(int x,int y,int w,int h);

#define flwindow Fl_Window  
#ifdef __linux__
#define flwindow Fl_Double_Window
#endif

using namespace std;
#pragma endregion includes

 
Fl_Menu_Bar* menu;
class AIS_NonSelectableShape : public AIS_Shape {
public:
    AIS_NonSelectableShape(const TopoDS_Shape& s) : AIS_Shape(s) {}

    void ComputeSelection(const Handle(SelectMgr_Selection)&,
                          const Standard_Integer) override
    {
        // Do nothing -> no selectable entities created
    }
};

void open_cb() {
    Fl_File_Chooser chooser(".", "*", Fl_File_Chooser::SINGLE, "Escolha um arquivo");
    chooser.show();
    while (chooser.shown()) Fl::wait();
    if (chooser.value()) {
        printf("Arquivo selecionado: %s\n", chooser.value());
    }
}

struct  OCC_Viewer : public flwindow {
#pragma region initialization
    Handle(Aspect_DisplayConnection) m_display_connection;
    Handle(OpenGl_GraphicDriver) m_graphic_driver;
    Handle(V3d_Viewer) m_viewer;
	Handle(OpenGl_Context) aCtx;
    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_View) m_view;
	Handle(AIS_Trihedron) trihedron0_0_0;
    bool m_initialized = false;
    bool hlr_on = false;
    std::vector<TopoDS_Shape> vshapes; 
    std::vector<Handle(AIS_Shape)> vaShape; 
	Handle(AIS_NonSelectableShape) visible_; 
    Handle(AIS_ColoredShape) hidden_;

	Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect( Quantity_NOC_BLUE,  Aspect_TOL_DASH, 0.5 );	  
	Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 3.0);
	Handle(Prs3d_LineAspect) highlightaspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 5.0);
	Handle(Prs3d_Drawer) customDrawer = new Prs3d_Drawer();
 

    OCC_Viewer(int X, int Y, int W, int H, const char* L = 0): flwindow(X, Y, W, H, L) { 
		Fl::add_timeout(10, idle_refresh_cb,0);

    }
   
    void initialize_opencascade() { 
        // Get native window handle
#ifdef _WIN32
		Fl::wait(); 
		make_current();
        HWND hwnd = (HWND)fl_xid(this);
        Handle(WNT_Window) wind = new WNT_Window(hwnd);
        m_display_connection = new Aspect_DisplayConnection("");
#else
        Window win = (Window)fl_xid((this)); 
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

        m_view->SetWindow(wind); 

        m_view->SetImmediateUpdate(Standard_False);  

        // m_context->SetAutomaticHilight(true);  

		
        // m_context->Activate(AIS_Shape::SelectionMode(TopAbs_WIRE  )); // 4 = Face selection mode
        // m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE )); // 4 = Face selection mode
        // m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX )); // 4 = vertex selection mode

    // m_context->SetMode(TopAbs_VERTEX, Standard_True); // Enable vertex selection as the active mode
                                                          // Standard_True for the second arg makes it persistent for this mode

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

		// BRepMesh_IncrementalMesh::SetParallel(Standard_True);

        m_view->SetBackgroundColor(Quantity_NOC_GRAY90);
        setbar5per();

        m_view->MustBeResized();
        m_view->FitAll();
        m_initialized = true; 		
        redraw();  
        m_view->Redraw();  

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
        }
    }
	static void idle_refresh_cb(void*) {
	//clear gpu usage each 10 secs
	glFlush();
	glFinish();
	Fl::repeat_timeout(10, idle_refresh_cb,0);  
}
    void draw() override { 
        if (!m_initialized) return;	 
        m_view->Update();
		// m_view->Redraw(); //new 
		// flush();
    }

    void resize(int X, int Y, int W, int H) override {
        Fl_Window::resize(X, Y, W, H);
        if (m_initialized) {
            m_view->MustBeResized();
            setbar5per();  
        }
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
    material.SetTransparency(0.5); // 50% transparency
    
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
void SetupHighlightLineType(const Handle(AIS_InteractiveContext)& ctx)
{ 
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
 
	customDrawer->SetLineAspect(highlightaspect);
	customDrawer->SetSeenLineAspect(highlightaspect); 
	customDrawer->SetWireAspect(highlightaspect);
	customDrawer->SetUnFreeBoundaryAspect(highlightaspect);
	customDrawer->SetFreeBoundaryAspect(highlightaspect);
	customDrawer->SetFaceBoundaryAspect(highlightaspect);
 
    customDrawer->SetColor(Quantity_NOC_RED);
    customDrawer->SetTransparency(0.3f);
 


    // 5. Assign to all highlight modes
    // ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalDynamic,  customDrawer);
    ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalSelected, customDrawer);
   cotm(5)
	// ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Dynamic,       customDrawer);
    ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Selected,      customDrawer);
}


#pragma endregion initialization
	

#pragma region luastruct
	struct luadraw;
	// vector<luadraw*> vlua;
	unordered_map<string,luadraw*> ulua;
	template <typename T>
	struct ManagedPtrWrapper : public Standard_Transient {
		T* ptr;
		ManagedPtrWrapper(T* p) : ptr(p) {}
		~ManagedPtrWrapper() { delete ptr; } // free automatically
	};
	struct luadraw{
		bool protectedshape=0;

		string command="";

 
		string name="";
		bool visible_hardcoded=1;
		TopoDS_Shape shape; 
		// std::shared_ptr<TopoDS_Shape> shape; 
		Handle(AIS_Shape) ashape; 
		TopoDS_Face face;
		gp_Pnt origin=gp_Pnt(0, 0, 0);
		gp_Dir normal=gp_Dir(0,0,1);
		gp_Dir xdir =  gp_Dir(1, 0, 0);
		gp_Trsf trsf;
		gp_Trsf trsftmp; 
		OCC_Viewer* occv;
		luadraw(string _name,OCC_Viewer* p=0): occv(p) {
			gp_Ax2 ax3(origin, normal, xdir);
			trsf.SetTransformation(ax3);
			trsf.Invert();
			name=_name;
			ashape = new AIS_Shape(shape);

			// allocate something for the application and hand ownership to the wrapper 
			Handle(ManagedPtrWrapper<luadraw>) wrapper = new ManagedPtrWrapper<luadraw>(this);

			// store the wrapper in the AIS object via SetOwner()
			ashape->SetOwner(wrapper);

			// ashape->SetUserData(new ManagedPtrWrapper<luadraw>(this));
        	occv->vaShape.push_back(ashape);
			occv->m_context->Display(ashape, 0); 
			occv->ulua[name]=this;
		}
		void redisplay(){
			BRepBuilderAPI_Transform transformer(shape, trsf);
			shape=transformer.Shape();
			ashape->Set(shape); 
			if(visible_hardcoded){
				occv->m_context->Redisplay(ashape, 0);
			}else{
				occv->m_context->Erase(ashape, Standard_False);
				cotm(8)
			}
			if(!visible_hardcoded && occv->m_context->IsDisplayed(ashape)){
				occv->m_context->Erase(ashape, Standard_False);
			}
		}
		void display(){
			occv->m_context->Display(ashape, 0);
		}
		void rotate(int angle,gp_Dir normal={0,0,1}){
			trsftmp = gp_Trsf();
			// gp_Dir normal=gp_Dir(0,1,0);
			trsftmp.SetRotation(gp_Ax1(origin, normal), angle*(M_PI/180) );
			trsf  *= trsftmp;
		}
		void translate(float x=0,float y=0, float z=0){
			trsftmp = gp_Trsf();
			trsftmp.SetTranslation(gp_Vec(x, y, z));
			trsf  *= trsftmp; 
		}
		void extrude(float qtd=0){
			// gp_Vec extrusionVec(dir);
			gp_Vec extrusionVec(normal);
			extrusionVec *= qtd;
    		TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(shape, extrusionVec).Shape();
			shape=extrudedShape;
		}
		
		void fuse(luadraw &tofuse1,luadraw &tofuse2 ){
			Handle(AIS_Shape) af1=tofuse1.ashape;
			Handle(AIS_Shape) af2=tofuse2.ashape;
			TopoDS_Shape ts1=tofuse1.shape;
			TopoDS_Shape ts2=tofuse2.shape; 

			BRepAlgoAPI_Fuse fuse(ts1, ts2);
			fuse.Build();
			if (!fuse.IsDone()) {
				// handle error
			}
			TopoDS_Shape fusedShape = fuse.Shape();

			// Refine the result
			ShapeUpgrade_UnifySameDomain unify(fusedShape, true, true, true); // merge faces, edges, vertices
			unify.Build();
			this->shape = unify.Shape();
		}
		
		
		void dofromstart(int x=0){
			gp_Pnt p1(0, 0,0);
			gp_Pnt p2(100+x, 0,0);
			gp_Pnt p3(100+x, 50,0);
			gp_Pnt p4(0, 50,0);

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

			face = BRepBuilderAPI_MakeFace(wire);

			shape = face;
			//final
			// BRepBuilderAPI_Transform transformer(face, trsf);
			// shape= transformer.Shape();

		}

	};


 
 
#pragma endregion luastruct
#pragma region lua






#pragma endregion lua
#pragma region scint






#pragma endregion scint
#pragma region events


Handle(AIS_Shape) myHighlightedPointAIS; // To store the highlighting sphere
TopoDS_Vertex myLastHighlightedVertex;   // To store the last highlighted vertex
void clearHighlight() {
    if (!myHighlightedPointAIS.IsNull()) {
        m_context->Remove(myHighlightedPointAIS, Standard_True);
        myHighlightedPointAIS.Nullify();
    }
    myLastHighlightedVertex.Nullify();
}
///@return vector 0=windowToWorldX 1=windowToWorldY 2=worldToWindowX 3=worldToWindowY 4=viewportHeight 5=viewportWidth
vfloat GetViewportAspectRatio(){
	const Handle(Graphic3d_Camera)& camera=m_view->Camera();

    // Obtém a altura e largura do mundo visível na viewport
    float viewportHeight = camera->ViewDimensions().Y(); // world
    float viewportWidth = camera->Aspect() * viewportHeight; // Largura = ratio * altura


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
	return {windowToWorldX,windowToWorldY,worldToWindowX,worldToWindowY,viewportHeight,viewportWidth};
}


void highlightVertex(const TopoDS_Vertex& aVertex) {
    clearHighlight(); // Clear any existing highlight first
 
	perf();
	// sleepms(1000);
	float ratio=GetViewportAspectRatio()[0];
	perf("GetViewportAspectRatio");
	// double ratio=theProjector->Aspect(); 
	cotm(ratio);

    gp_Pnt vertexPnt = BRep_Tool::Pnt(aVertex);

    // Create a small red sphere at the vertex location
    Standard_Real sphereRadius = 7*ratio; // Small radius for the highlight ball
    TopoDS_Shape sphereShape = BRepPrimAPI_MakeSphere(vertexPnt, sphereRadius).Shape();
    myHighlightedPointAIS = new AIS_Shape(sphereShape);
    myHighlightedPointAIS->SetColor(Quantity_NOC_RED);
    myHighlightedPointAIS->SetDisplayMode(AIS_Shaded);
    myHighlightedPointAIS->SetTransparency(0.2f); // Slightly transparent
    myHighlightedPointAIS->SetZLayer(Graphic3d_ZLayerId_Top); // Ensure it's drawn on top

    m_context->Display(myHighlightedPointAIS, Standard_True);
    myLastHighlightedVertex = aVertex;

    cotm("Highlighted Vertex:", vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z());
// 	if(!projector.Perspective()){
// 		gp_Pnt clickedPoint(mousex, mousey, 0);
// 		// Passo 1: converter mouse para coordenadas projetadas reais (no plano do HLR)
// Standard_Real x, y, z;
// m_view->Convert(mousex, mousey, x, y, z);

// // Como HLR só trabalha em XY, podes descartar z:
// gp_Pnt screenPoint(x, y, 0);
// // BRep_Tool::Pnt(vertex);
// 		gp_Pnt vertexhlrPnt =getAccurateVertexPosition(vertexPnt,projector);
// 		// gp_Pnt vertexhlrPnt =getAccurateVertexPosition(m_context,mousex,mousey);
// 		// gp_Pnt vertexhlrPnt =getAccurateVertexPosition(clickedPoint);
// 		// gp_Pnt vertexhlrPnt =convertHLRVertexToWorld(vertexPnt);
// 		// gp_Pnt vertexhlrPnt =convertHLRToWorld(vertexPnt);
// 		cotm("Highlighted hlr Vertex:", vertexhlrPnt.X(), vertexhlrPnt.Y(), vertexhlrPnt.Z());
// 	}
//     // printf("Highlighted Vertex: X=%.3f, Y=%.3f, Z=%.3f\n", vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z());

// if (!projector.Perspective()) {
//     // 1. Converte coordenadas de ecrã para mundo 2D projetado
//     Standard_Real projX, projY, projZ;
//     m_view->Convert(mousex, mousey, projX, projY, projZ);
//     gp_Pnt projectedHLRPoint(projX, projY, 0); // plano HLR é Z=0

//     // 2. Converte o ponto projetado para o espaço 3D real
//     gp_Pnt worldPoint = convertHLRToWorld(projectedHLRPoint);

//     cotm("Convertido de HLR para 3D:", worldPoint.X(), worldPoint.Y(), worldPoint.Z());
// }

}


int mousex=0;
int mousey=0;

void getvertex() {
	m_view->SetComputedMode(Standard_False);
	clearHighlight();
   
    // 2. Activate ONLY vertex selection mode for this specific picking operation.
    // This uses the AIS_Shape::SelectionMode utility, which correctly returns 0 for TopAbs_VERTEX.
    m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));

	 

    // 3. Perform the picking operations
    m_context->MoveTo(mousex, mousey, m_view, Standard_False);


    m_context->SelectDetected(AIS_SelectionScheme_Replace);



    Handle(SelectMgr_EntityOwner) foundOwner;

    for (m_context->InitDetected(); m_context->MoreDetected(); m_context->NextDetected()) {
        Handle(SelectMgr_EntityOwner) owner = m_context->DetectedOwner();

        if (owner.IsNull())
            continue;

        Handle(AIS_InteractiveObject) obj = Handle(AIS_InteractiveObject)::DownCast(owner->Selectable());

        // Skip the HLR shape
        if (obj == hidden_)
            continue;

        foundOwner = owner; // First valid non-HLR owner
        break;
    }

    if (!foundOwner.IsNull()) {
        Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(foundOwner);
        if (!brepOwner.IsNull()) {
            TopoDS_Shape detectedTopoShape = brepOwner->Shape();

            printf("Detected TopoDS_ShapeType: %d (0=Vertex, 1=Edge, 2=Wire, 3=Face, etc.)\n", detectedTopoShape.ShapeType());
            printf("Value of TopAbs_VERTEX: %d\n", TopAbs_VERTEX);

            if (detectedTopoShape.ShapeType() == TopAbs_VERTEX) {
                TopoDS_Vertex currentVertex = TopoDS::Vertex(detectedTopoShape);
                if (!myLastHighlightedVertex.IsEqual(currentVertex)) {
                    highlightVertex(currentVertex);
                } else {
                    gp_Pnt pt = BRep_Tool::Pnt(currentVertex);
                    printf("Hovering over same vertex: X=%.3f, Y=%.3f, Z=%.3f\n", pt.X(), pt.Y(), pt.Z());
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
luadraw* lua_detected(Handle(SelectMgr_EntityOwner) entOwner)
{
    if (!entOwner.IsNull()) {
        // 1) tenta obter o Selectable associado ao entOwner
        if (entOwner->HasSelectable()) {
            Handle(SelectMgr_SelectableObject) selObj = entOwner->Selectable();
            // SelectableObject é a base de AIS_InteractiveObject, faz downcast
            Handle(AIS_InteractiveObject) ao = Handle(AIS_InteractiveObject)::DownCast(selObj);
            if (!ao.IsNull() && ao->HasOwner()) {
                Handle(Standard_Transient) owner = ao->GetOwner();
                Handle(ManagedPtrWrapper<luadraw>) w = Handle(ManagedPtrWrapper<luadraw>)::DownCast(owner);
                if (!w.IsNull()) return w->ptr; // devolve o ponteiro armazenado
            }
        }
    }

    // Alternativa: tenta obter directamente o interactive object detectado pelo contexto
    // if (!m_context.IsNull()) {
    //     Handle(AIS_InteractiveObject) detected = m_context->DetectedInteractive();
    //     if (!detected.IsNull() && detected->HasOwner()) {
    //         Handle(Standard_Transient) owner = detected->GetOwner();
    //         Handle(ManagedPtrWrapper<luadraw>) w = Handle(ManagedPtrWrapper<luadraw>)::DownCast(owner);
    //         if (!w.IsNull()) return w->ptr;
    //     }
    // }
    return nullptr;
}



void ev_highlight(){
	
    // Start with a clean slate for the custom highlight
    clearHighlight();

    // --- Strict Selection Mode Control for Hover ---
    // 1. Deactivate ALL active modes first to ensure a clean slate for picking.
    // This loops through common topological modes.
    // for (Standard_Integer mode = TopAbs_VERTEX; mode <= TopAbs_COMPSOLID; ++mode) {
    //     m_context->Deactivate(mode);
    // }
    // You might also need: m_context->Deactivate(0); // If 0 means "all" or a special mode.
	
    // 2. Activate ONLY vertex selection mode for this specific picking operation.
    // This uses the AIS_Shape::SelectionMode utility, which correctly returns 0 for TopAbs_VERTEX.
    m_context->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
    m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
    if(!hlr_on){
		m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
	}else{
		m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_FACE));
	}
    // 3. Perform the picking operations
    m_context->MoveTo(mousex, mousey, m_view, Standard_False);

	m_context->SelectDetected(AIS_SelectionScheme_Replace);

    // 4. Get the detected owner
    Handle(SelectMgr_EntityOwner) anOwner = m_context->DetectedOwner();

	if (!anOwner.IsNull()  ){
	luadraw* ldd=lua_detected(anOwner);
	if(ldd){
	cotm(ldd->name);
	}
	}

    // 5. Deactivate vertex mode immediately after picking
    // This is crucial if you only want vertex picking *during* hover,
    // and want other selection behaviors (e.g., selecting faces on click) at other times.
    // m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
    // --- End Strict Selection Mode Control ---


    // --- Debugging and Highlighting Logic ---
    if (!anOwner.IsNull()  ) {
        Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(anOwner);
        if (!brepOwner.IsNull()) {
            TopoDS_Shape detectedTopoShape = brepOwner->Shape();

            // printf("Detected TopoDS_ShapeType: %d (0=Vertex, 1=Edge, 2=Wire, 3=Face, etc.)\n", detectedTopoShape.ShapeType());
            // printf("Value of TopAbs_VERTEX: %d\n", TopAbs_VERTEX); // Confirms the actual value of TopAbs_VERTEX

            if (detectedTopoShape.ShapeType() == TopAbs_VERTEX) {
                // printf("--- CONDITION: detectedTopoShape.ShapeType() == TopAbs_VERTEX is TRUE ---\n");
                TopoDS_Vertex currentVertex = TopoDS::Vertex(detectedTopoShape);
                if (!myLastHighlightedVertex.IsEqual(currentVertex)) {
                    highlightVertex(currentVertex);
                } else {
                    // Highlighted same vertex, no need to re-print or re-draw
                    printf("Hovering over same vertex: X=%.3f, Y=%.3f, Z=%.3f\n",
                           BRep_Tool::Pnt(currentVertex).X(),
                           BRep_Tool::Pnt(currentVertex).Y(),
                           BRep_Tool::Pnt(currentVertex).Z());
                }
            } else {
                // printf("--- CONDITION: detectedTopoShape.ShapeType() == TopAbs_VERTEX is FALSE (Type %d) ---\n", detectedTopoShape.ShapeType());
                clearHighlight(); // Detected a BRepOwner, but not a vertex
            }
        } else {
            // printf("Owner is not a StdSelect_BRepOwner.\n");
            clearHighlight(); // Owner is not a BRepOwner (e.g., detected an AIS_Text)
        }
    } else {
		// go_up(1);
		// cotm("test")
		// printf("%s",cotmlastoutput.c_str());
		// go_up;
        // cotmupset
		// cotm("Nothing detected under the mouse.");
		// cotmup

        clearHighlight(); // Nothing detected
    }

    m_context->UpdateCurrentViewer(); // Update the viewer to show/hide highlight
}
int handle(int event) override { 
    static int start_y;
    const int edge_zone = this->w() * 0.05; // 5% right edge zone

// #include <SelectMgr_EntityOwner.hxx>
// #include <StdSelect_BRepOwner.hxx>
// #include <TopAbs_ShapeEnum.hxx> // Ensure this is included for TopAbs_VERTEX etc.

// ... (your existing OCCViewerWindow class methods) ...

// In your initializeOCC() method:
// Ensure SetSelectionSensitivity is set appropriately for small vertices.
// For a sphere of radius 10, 0.02 or 0.05 is a good starting point.
// m_context->SetSelectionSensitivity(0.02);


// In your createSampleShape() method:
// Remove any AIS_InteractiveContext::SetMode() calls here, as we will control it directly in FL_MOVE
// The default selection behavior on the AIS_Shape itself is sufficient for this approach.


// In your handle(int event) method:
if (event == FL_MOVE) {
    int x = Fl::event_x();
    int y = Fl::event_y();
	mousex=x;
	mousey=y;
	// getvertex();
	// return 1;
	
// {// 1) guarda mousex/mousey (já fazes isso)
// gp_Pnt screenHLR = screenPointFromMouse(mousex, mousey);

// // 2) converte para 3D
// gp_Pnt worldApprox = convertHLRToWorld(screenHLR);

// // 3) (opcional) vertex exato mais próximo
// gp_Pnt trueVertex = getAccurateVertexPosition(screenHLR);

// // Imprime para debug
// printf("Mouse → HLR2D: (%.3f,%.3f)\n", screenHLR.X(), screenHLR.Y());
// printf("Approx world 3D: (%.3f,%.3f,%.3f)\n",
//        worldApprox.X(), worldApprox.Y(), worldApprox.Z());
// printf("Best match vertex: (%.3f,%.3f,%.3f)\n",
//        trueVertex.X(), trueVertex.Y(), trueVertex.Z());}


	ev_highlight();



    return 1;
}


    switch (event) {
        case FL_PUSH:
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                // Check if click is in right 5% zone
                if (Fl::event_x() > (this->w() - edge_zone)) {
                    start_y = Fl::event_y();
                    return 1; // Capture mouse
                }
            }
            break;

		//bar5per
        case FL_DRAG:
            if (Fl::event_state(FL_BUTTON1)) {
                // Only rotate if drag started in right edge zone
                if (Fl::event_x() > (this->w() - edge_zone)) {
                    int dy = Fl::event_y() - start_y;
                    start_y = Fl::event_y();
                    
                    // Rotate view (OpenCascade)
                    if (dy != 0) {
                        double angle = dy * 0.005; // Rotation sensitivity factor
						perf();
    	                // Fl::wait(0.01);	
                        m_view->Rotate(0, 0, angle); // Rotate around Z-axis
						perf("vnormal");
                        // projectAndDisplayWithHLR(vshapes);
						// recomputeComputed () ;
                         redraw(); //  redraw(); // m_view->Update ();


                        // redraw();//
                        // myView->Redraw();
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
            }
            else if (Fl::event_button() == FL_RIGHT_MOUSE) {
                isPanning = true;
                lastX = Fl::event_x();
                lastY = Fl::event_y();
                return 1;
            }
            break;

        case FL_DRAG: 
            if (isRotating && m_initialized) { 
                funcfps(12,
					perf();
					m_view->Rotation(Fl::event_x(),Fl::event_y());
                	// projectAndDisplayWithHLR(vaShape,1);
                	projectAndDisplayWithHLR(vshapes,1);
                 	redraw(); //  redraw(); // m_view->Update ();
					perf("vnormalr");


                colorisebtn();    
                // redraw();        
            ); 
                return 1;
            }
            else if (isPanning && m_initialized && Fl::event_button() == FL_RIGHT_MOUSE) {
                int dx = Fl::event_x() - lastX;
                int dy = Fl::event_y() - lastY;
                // m_view->Pan(dx, -dy); // Note: -dy to match typical screen coordinates
                funcfps(25,
                    m_view->Pan(dx, -dy);
                     redraw(); //  redraw(); // m_view->Update ();


                    lastX = Fl::event_x(); 
                    lastY = Fl::event_y();  
                ); 
                return 1;
            }
            break;

        case FL_RELEASE:
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                isRotating = false; 
                return 1;
            }
            else if (Fl::event_button() == FL_RIGHT_MOUSE) {
                isPanning = false;
                return 1;
            }
            break;
case FL_MOUSEWHEEL:
    if (m_initialized) {
        funcfps(25,
            int mouseX = Fl::event_x();
            int mouseY = Fl::event_y();
            m_view->StartZoomAtPoint(mouseX, mouseY);
            // Get wheel delta (normalized)
            int wheelDelta = Fl::event_dy();
            
            // According to OpenCASCADE docs, ZoomAtPoint needs:
            // 1. The start point (where mouse is)
            // 2. The end point (calculated based on wheel movement)
            
            // Calculate end point - this determines zoom direction and magnitude
            int endX = mouseX;
            int endY = mouseY - (wheelDelta*5*5);  // Vertical movement controls zoom
            
            // Call ZoomAtPoint with both points
            m_view->ZoomAtPoint(mouseX, mouseY, endX, endY);
             redraw(); //  redraw(); // m_view->Update ();


        );
        return 1;
    }
    break;
    }
    return Fl_Window::handle(event);
}

int lastX, lastY;
bool isRotating = false;
bool isPanning = false;
#pragma endregion events



struct pashape : public AIS_Shape {
  // expõe o drawer protegido da AIS_Shape
  using AIS_Shape::myDrawer;

  pashape(const TopoDS_Shape& shape)
    : AIS_Shape(shape)
  {
    // 1) Cria um novo drawer
    Handle(Prs3d_Drawer) dr = new Prs3d_Drawer();

    // 2) Linha tracejada vermelha, espessura 2
    Handle(Prs3d_LineAspect) wireAsp =
      new Prs3d_LineAspect(
        Quantity_NOC_BLUE,    // cor vermelha
        Aspect_TOL_DASH,     // tipo: dash
        0.5                  // espessura da linha
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



void draw_objs(){
	perf1("");
	for(int i=0;i<vshapes.size();i++){
        Handle(AIS_Shape) aShape = new AIS_Shape(vshapes[i]);
        vaShape.push_back(aShape);
		m_context->Display(aShape, 0);
	}
	perf1("draw_objs");
	toggle_shaded_transp(currentMode);

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
    ctx->Display(tri, Standard_False);
}



#pragma region projection
 
 
HLRAlgo_Projector projector; 


Standard_Integer currentMode = AIS_WireFrame;
void toggle_shaded_transp(Standard_Integer fromcurrentMode = AIS_WireFrame) {
	perf1();
    for (std::size_t i = 0; i < vaShape.size(); ++i) {
        Handle(AIS_Shape) aShape = vaShape[i];
        if (aShape.IsNull()) continue;

        if (fromcurrentMode == AIS_Shaded) {			
			hlr_on=1;
            // Mudar para modo wireframe
            // aShape->UnsetColor();
            aShape->UnsetAttributes(); // limpa materiais, cor, largura, etc.
            m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_False);


            aShape->Attributes()->SetFaceBoundaryDraw(Standard_False);
            aShape->Attributes()->SetLineAspect(wireAsp);
            aShape->Attributes()->SetSeenLineAspect(wireAsp);
            aShape->Attributes()->SetWireAspect(wireAsp);
            aShape->Attributes()->SetUnFreeBoundaryAspect(wireAsp);
            aShape->Attributes()->SetFreeBoundaryAspect(wireAsp);
            aShape->Attributes()->SetFaceBoundaryAspect(wireAsp);
        } else {
			hlr_on=0;
            // Mudar para modo sombreado
			 aShape->UnsetAttributes(); // limpa materiais, cor, largura, etc.

            m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);

            aShape->SetColor(Quantity_NOC_GRAY70);
            aShape->Attributes()->SetFaceBoundaryDraw(Standard_True);
            aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);
            // aShape->Attributes()->SetSeenLineAspect(edgeAspect); // opcional
        }

        m_context->Redisplay(aShape, 0);
    }
	
	perf1("toggle_shaded_transp");
	if(hlr_on==1){
		// projectAndDisplayWithHLR(vaShape);
		projectAndDisplayWithHLR(vshapes);
	}else{
		if(visible_){
			m_context->Remove(visible_,0);
			visible_.Nullify();
		}
	}
	currentMode=fromcurrentMode;
	redraw();
}



TopoDS_Shape vEdges;
TopoDS_Shape hEdges;
Handle(HLRBRep_PolyAlgo) hlrAlgo;
// gp_Trsf glb_invTrsf;


// #include <BRepMesh_IncrementalMesh.hxx>
// #include <BRep_Tool.hxx>
// #include <BRepBuilderAPI_Transform.hxx>
// #include <Graphic3d_Camera.hxx>
// #include <HLRBRep_PolyHLRToShape.hxx>
// #include <AIS_Shape.hxx>
// #include <TopExp_Explorer.hxx>
// #include <TopoDS_Face.hxx>
// #include <chrono>
// #include <execution> // Para C++17 paralelismo
// #ifndef HAVE_TBB
// #define ENABLE_PARALLEL  // Ative se souber que sua build do OpenCascade é thread-safe
// #endif



void projectAndDisplayWithHLR_vnotwrk( const std::vector<Handle(AIS_Shape)>& vaShape,  bool isDragonly = false){
    if (!hlr_on || m_context.IsNull() || m_view.IsNull())
        return;

    perf1();

    // 1. Prepare camera transform
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    gp_Dir viewDir   = -camera->Direction();
    gp_Dir viewUp    = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);
    gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight);

    gp_Trsf viewTrsf; 
    viewTrsf.SetTransformation(viewAxes);
    gp_Trsf invTrsf = viewTrsf.Inverted();

    // 2. Create HLR projector
    projector = HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(),
        camera->Scale()
    );

    // 3. Meshing (only if not already meshed)
    Standard_Real deflection = 0.001;

#ifdef ENABLE_PARALLEL
    std::for_each(std::execution::par, vaShape.begin(), vaShape.end(),
        [&](const Handle(AIS_Shape)& ash)
        {
            if (ash.IsNull()) return;
            TopoDS_Shape s = ash->Shape();
            bool needsMesh = false;
            for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
                TopLoc_Location loc;
                const TopoDS_Face& face = TopoDS::Face(exp.Current());
                if (BRep_Tool::Triangulation(face, loc).IsNull()) {
                    needsMesh = true;
                    break;
                }
            }
            if (needsMesh) {
                BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
            }
        }
    );
#else
    for (const auto& ash : vaShape) {
        if (ash.IsNull() || !m_context->IsDisplayed(ash)) continue;
        TopoDS_Shape s = ash->Shape();
        bool needsMesh = false;
        for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
            TopLoc_Location loc;
            const TopoDS_Face& face = TopoDS::Face(exp.Current());
            if (BRep_Tool::Triangulation(face, loc).IsNull()) {
                needsMesh = true;
                break;
            }
        }
        if (needsMesh) {
            BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
        }
    }
#endif

    // 4. HLR projection
Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();
#ifdef ENABLE_PARALLEL
std::for_each(std::execution::par, vaShape.begin(), vaShape.end(),
    [&](const Handle(AIS_Shape)& ash)
    {
        if (ash.IsNull()) return;
        algo->Load(ash->Shape());
    }
);
#else
for (const auto& ash : vaShape) {
    if (!ash.IsNull() && m_context->IsDisplayed(ash)) {
        algo->Load(ash->Shape());
    }
}
#endif

algo->Projector(projector);
algo->Update();

// 5. Convert to visible shape
HLRBRep_PolyHLRToShape hlrToShape;
hlrToShape.Update(algo);
TopoDS_Shape vEdges = hlrToShape.VCompound();
TopoDS_Shape result = vEdges;  // Alteração aqui: sem invTrsf

// 6. Display or update AIS_Shape
if (!visible_.IsNull()) {
    if (!visible_->Shape().IsEqual(result)) {
        visible_->SetShape(result);
        visible_->SetColor(Quantity_NOC_BLACK);
        visible_->SetWidth(3);
        visible_->SetInfiniteState(true);
        m_context->Redisplay(visible_, false);
    }
} else {
    visible_ = new AIS_NonSelectableShape(result);
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(3);
    visible_->SetInfiniteState(true);
    m_context->Display(visible_, false);
}
    perf1("elapsed hlra");
}


 

void fillvectopo(){
	vshapes.clear();
	cotm(vaShape.size(),vshapes.size());
	for(int i=0;i<vaShape.size();i++){
		if(!m_context->IsDisplayed(vaShape[i])) continue;
        vshapes.push_back(vaShape[i]->Shape());
	}
	cotm(vaShape.size(),vshapes.size());
}

void projectAndDisplayWithHLR_bug( const std::vector<Handle(AIS_Shape)>& vaShape,  bool isDragonly = false){
    if (!hlr_on || m_context.IsNull() || m_view.IsNull())
        return;

    perf1();

    // 1. Prepare camera transform
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    gp_Dir viewDir   = -camera->Direction();
    gp_Dir viewUp    = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);
    gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight);

    gp_Trsf viewTrsf; 
    viewTrsf.SetTransformation(viewAxes);
    gp_Trsf invTrsf = viewTrsf.Inverted();

    // 2. Create HLR projector
    projector = HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(),
        camera->Scale()
    );

    // 3. Meshing (only if not already meshed)
    Standard_Real deflection = 0.001;
    // deflection = 0.00005;

#ifdef ENABLE_PARALLEL
    std::for_each(std::execution::par, vaShape.begin(), vaShape.end(),
        [&](const Handle(AIS_Shape)& ash)
        {
            if (ash.IsNull()) return;
            TopoDS_Shape s = ash->Shape();
            bool needsMesh = false;
            for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
                TopLoc_Location loc;
                const TopoDS_Face& face = TopoDS::Face(exp.Current());
                if (BRep_Tool::Triangulation(face, loc).IsNull()) {
                    needsMesh = true;
                    break;
                }
            }
            if (needsMesh) {
                BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
            }
        }
    );
#else
    for (const auto& ash : vaShape) {
        if (ash.IsNull() ) continue;
        // if (ash.IsNull() || !m_context->IsDisplayed(ash)) continue;
        TopoDS_Shape s = ash->Shape();
        bool needsMesh = false;
        for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
            TopLoc_Location loc;
            const TopoDS_Face& face = TopoDS::Face(exp.Current());
            if (BRep_Tool::Triangulation(face, loc).IsNull()) {
                needsMesh = true;
                break;
            }
        }
        if (needsMesh) {
            BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
        }
    }
#endif

    // 4. HLR projection
    Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();
#ifdef ENABLE_PARALLEL
    std::for_each(std::execution::par, vaShape.begin(), vaShape.end(),
        [&](const Handle(AIS_Shape)& ash)
        {
            if (ash.IsNull()) return;
            algo->Load(ash->Shape());
        }
    );
#else
    for (const auto& ash : vaShape) {
        if (!ash.IsNull() ) {
        // if (!ash.IsNull() && m_context->IsDisplayed(ash)) {
            algo->Load(ash->Shape());
        }
    }
#endif

    algo->Projector(projector);
    algo->Update();

    // 5. Convert to visible shape
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(algo);
    TopoDS_Shape vEdges   = hlrToShape.VCompound();
    BRepBuilderAPI_Transform visT(vEdges, invTrsf);
    TopoDS_Shape result   = visT.Shape();

    // 6. Display or update AIS_Shape
    if (!visible_.IsNull()) {
        if (!visible_->Shape().IsEqual(result)) {
            visible_->SetShape(result);
            visible_->SetColor(Quantity_NOC_BLACK);
            visible_->SetWidth(3);
            visible_->SetInfiniteState(true);
            m_context->Redisplay(visible_, false);
        }
    } else {
        visible_ = new AIS_NonSelectableShape(result);
        // visible_ = new AIS_Shape(result);
        visible_->SetColor(Quantity_NOC_BLACK);
        visible_->SetWidth(3);
        visible_->SetInfiniteState(true);
        m_context->Display(visible_, false);
    }
	
	// visible_->SetSelectionPriority(0);
// m_context->RemoveSelectionMode(visible_, 0);
// m_context->SetSelectable(visible_, Standard_False);
	// m_context->Deactivate(visible_, 0);

	// 0 = default selection mode
// m_context->Deactivate(visible_, 0);  

// // If you want to be extra sure, deactivate all modes:
// for (int mode = 0; mode <= 9; ++mode) {
//     m_context->Deactivate(visible_, mode);
// }

    perf1("elapsed hlra");
}



void projectAndDisplayWithHLR_nda(const std::vector<TopoDS_Shape>& shapes, bool v = 1) {
    if (m_context.IsNull() || m_view.IsNull()) {
        std::cerr << "Error: Context or View is null\n";
        return;
    }

    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    if (camera.IsNull()) {
        std::cerr << "Error: Camera is null\n";
        return;
    }

    // Ensure triangulation before HLR
    Standard_Real deflection = 0.001;
    for (const auto& s : shapes) {
        if (!s.IsNull()) {
            BRepMesh_IncrementalMesh mesh(s, deflection);
        }
    }

    Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();

    // Load entire shapes, not individual faces
    for (const auto& s : shapes) {
        if (!s.IsNull()) {
            algo->Load(s);
        }
    }

    // Set up view transformation with eye as origin
    gp_Pnt eye = camera->Eye();
    gp_Dir dir = camera->Direction();  // From eye to center
    gp_Dir up = camera->Up();
    gp_Dir right = up.Crossed(dir);  // Right-handed system
    gp_Ax3 viewAxes(eye, dir, right);
    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);

    // Set up projector with appropriate focus
    double focus = camera->IsOrthographic() ? 0.0 : camera->Distance();
    HLRAlgo_Projector projector(viewTrsf, !camera->IsOrthographic(), focus);
    algo->Projector(projector);
    algo->Update();

    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(algo);

    TopoDS_Shape vEdges = hlrToShape.VCompound();
    if (vEdges.IsNull()) {
        std::cerr << "HLR result is empty\n";
        return;
    }

    // Use vEdges directly as final shape (no inverse transformation needed)
    TopoDS_Shape finalShape = vEdges;

    if (!visible_.IsNull()) {
        if (!visible_->Shape().IsEqual(finalShape)) {
            visible_->Set(finalShape);
            m_context->Redisplay(visible_, false);
        }
    } else {
        visible_ = new AIS_NonSelectableShape(finalShape);
        visible_->SetColor(Quantity_NOC_BLACK);
        visible_->SetWidth(2.0);
        visible_->SetInfiniteState(true);
        m_context->Display(visible_, false);
    }
}




void projectAndDisplayWithHLR_notw(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false) {
    if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;
    perf1();

    // --- 0. grab camera
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    if (camera.IsNull()) return;

    // camera direction: OpenCASCADE camera->Direction() points from eye to target
    gp_Dir viewDir = -camera->Direction(); // we want "view ray" from eye into scene
    gp_Dir viewUp  = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);

    // --- 1. Build a pure-rotation Ax3 at the origin (no translation)
    // This is the frame that defines the projector rotation (eye orientation).
    gp_Ax3 rotAx3(gp_Pnt(0.0, 0.0, 0.0), viewDir, viewRight);
    gp_Trsf rotTrsf;
    rotTrsf.SetTransformation(rotAx3); // pure rotation (and maybe scale/reflection if axes are funky)

    // --- 2. Build a translation that moves world so that camera center becomes the origin
    // We'll translate shapes by 'trsl' so that the camera is at (0,0,0) for the projector.
    gp_Trsf trsl;
    trsl.SetTranslation(camera->Center(), gp_Pnt(0.0, 0.0, 0.0));
    gp_Trsf invTrsl = trsl.Inverted();

    // NOTE: We *do not* multiply rotation and translation into one transform for the projector.
    // The projector should see only the rotation; translation is applied to shapes (or inverted after HLR).

    // --- 3. Create projector (rotation-only). Keep perspective flag & scale from camera.
    projector = HLRAlgo_Projector(rotTrsf, !camera->IsOrthographic(), camera->Scale());

    // --- 4. Meshing: ensure shapes are meshed adequately for stable silhouette detection
    // Tweak deflection as needed; smaller values -> finer mesh -> more stable silhouettes.
    Standard_Real deflection = 0.001; // <-- adjust downward (0.0005, 0.0001) for higher quality/stability

#ifdef ENABLE_PARALLEL
    std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const TopoDS_Shape& s) {
#else
    for (const auto& s : shapes) {
#endif
        if (s.IsNull()) continue;

        bool needsMesh = false;
        for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
            TopLoc_Location loc;
            const TopoDS_Face& face = TopoDS::Face(exp.Current());
            if (BRep_Tool::Triangulation(face, loc).IsNull()) {
                needsMesh = true;
                break;
            }
        }
        if (needsMesh) {
            // create mesh in world coordinates — translation doesn't matter for triangulation quality
            BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
        }
#ifdef ENABLE_PARALLEL
    });
#else
    }
#endif

    // --- 5. Prepare HLR algorithm and load shapes (no transform needed here)
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

    // give algo the projector (rotation-only)
    algo->Projector(projector);

    // Optional: if your OpenCASCADE has these methods you can tune them:
    // algo->SetAngularTolerance(myAngularTol); // <-- reduce flicker across silhouette threshold
    // algo->SetOffset(myOffset); // <-- if present, can tweak numeric offset used by classification

    algo->Update();

    // --- 6. Convert HLR result to visible-edge shape
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(algo);
    TopoDS_Shape vEdges = hlrToShape.VCompound();

    // --- 7. Now undo the earlier translation: transform visible edges back into world space
    // We earlier considered shapes as translated so camera sits at origin. Now move edges back.
    BRepBuilderAPI_Transform visT(vEdges, invTrsl);
    // TopoDS_Shape result = visT.Shape();
    TopoDS_Shape result = vEdges;

    // --- 8. Display / update AIS_Shape
    if (!visible_.IsNull()) {
        if (!visible_->Shape().IsEqual(result)) {
            visible_->SetShape(result);
            visible_->SetColor(Quantity_NOC_BLACK);
            visible_->SetWidth(3);
            visible_->SetInfiniteState(true); // optional, but preserves display regardless of view
            m_context->Redisplay(visible_, false);
        }
    } else {
        visible_ = new AIS_NonSelectableShape(result);
        visible_->SetColor(Quantity_NOC_BLACK);
        visible_->SetWidth(3);
        visible_->SetInfiniteState(true); // optional
        m_context->Display(visible_, false);
    }

    perf1("elapsed hlr1");
}


#define ENABLE_PARALLEL
//less precision
void projectAndDisplayWithHLR(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false) {
    if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;
	perf1();

    // 1. Preparar transformação da câmara
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
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
    // projector = HLRAlgo_Projector(viewTrsf, !theProjector->IsOrthographic(), theProjector->Scale());
    projector = HLRAlgo_Projector(viewTrsf, !camera->IsOrthographic(), camera->Scale());

    // 3. Meshing (somente se ainda não estiver meshed)
    Standard_Real deflection = 0.001;

#ifdef ENABLE_PARALLEL
    std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const TopoDS_Shape& s) {
#else
    for (const auto& s : shapes) {
#endif
        if (s.IsNull()) return;

        bool needsMesh = false;
        for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
            TopLoc_Location loc;
            const TopoDS_Face& face = TopoDS::Face(exp.Current());
            if (BRep_Tool::Triangulation(face, loc).IsNull()) {
                needsMesh = true;
                break;
            }
        }
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

    algo->Projector(projector);
    algo->Update();

    // 5. Converter para shape visível
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(algo);
    TopoDS_Shape vEdges = hlrToShape.VCompound();
    BRepBuilderAPI_Transform visT(vEdges, invTrsf);
    TopoDS_Shape result = visT.Shape();

    // 6. Mostrar ou atualizar AIS_Shape
    if (!visible_.IsNull()) {
        if (!visible_->Shape().IsEqual(result)) {
            visible_->SetShape(result);
            visible_->SetColor(Quantity_NOC_BLACK);
            visible_->SetWidth(3);
            visible_->SetInfiniteState(true); // opcional
            m_context->Redisplay(visible_, false);
        }
    } else {
        visible_ = new AIS_NonSelectableShape(result);
        visible_->SetColor(Quantity_NOC_BLACK);
        visible_->SetWidth(3);
        visible_->SetInfiniteState(true); // opcional
        m_context->Display(visible_, false);
    }
	perf1("elapsed hlr1");
}


void projectAndDisplayWithHLRold(const std::vector<TopoDS_Shape>& shapes, bool isDragonly=0) {
    if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;


        if(visible_) m_context->Remove(visible_,0); 

    // 1. Prepara camera e transformação
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    // usa Ax3 completo (posição + orientação)
	gp_Dir viewDir = -camera->Direction(); // direção da visualização
	gp_Dir viewUp = camera->Up();          // vetor para cima
	gp_Dir viewRight = viewUp.Crossed(viewDir); // vetor para a direita

	gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight); // origem, direção Z, direção X

    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);
    gp_Trsf invTrsf = viewTrsf.Inverted();

    // 2. Cria o projector HLR
    projector = HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(),
        camera->Scale()
    );

    // 3. Malha consistente
    Standard_Real deflection = 0.001;
    // Standard_Real deflection = 0.01;
    for (auto& s : shapes) {
        if (!s.IsNull())
            BRepMesh_IncrementalMesh(s, deflection, true, 0.5, false);
    }

    // 4. Computa HLR
    hlrAlgo = new HLRBRep_PolyAlgo();
    for (auto& s : shapes) {
        if (!s.IsNull())
            hlrAlgo->Load(s);
    }
    hlrAlgo->Projector(projector);
    hlrAlgo->Update();

    // 5. Converte de volta para 3D e exibe
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(hlrAlgo);

    // Visíveis
    vEdges = hlrToShape.VCompound();
    BRepBuilderAPI_Transform visT(vEdges, invTrsf);
    visible_ = new AIS_NonSelectableShape(visT.Shape());
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(3);
    m_context->Display(visible_, false);

}
 //precision
void projectAndDisplayWithHLR_P(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false){
    if(!hlr_on)return;
	perf();
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

        if(visible_) m_context->Remove(visible_,0);
        if(hidden_) m_context->Remove(hidden_,0);
    }

    const Handle(Graphic3d_Camera) &theProjector=m_view->Camera();
 
    gp_Dir aBackDir = -theProjector->Direction();
    gp_Dir aXpers   = theProjector->Up().Crossed (aBackDir);
    gp_Ax3 anAx3 (theProjector->Center(), aBackDir, aXpers);
    gp_Trsf aTrsf;
    aTrsf.SetTransformation (anAx3); 
     
    HLRAlgo_Projector projector (aTrsf, !theProjector->IsOrthographic(), theProjector->Scale());

    Handle(HLRBRep_Algo) hlrAlgo = new HLRBRep_Algo();
    for (const auto& shp : shapes) {
        if (!shp.IsNull()) hlrAlgo->Add(shp,0);
    }

    hlrAlgo->Projector(projector );
    // hlrAlgo-> ShowAll();
    // perf();
    hlrAlgo->Update();
    // perf("hlrAlgo->Update()");
    static bool tes=0;
    // if(!tes)
	hlrAlgo->Hide();
    // perf("hide");
    tes=1;

    HLRBRep_HLRToShape hlrToShape_(hlrAlgo);
    // perf("hlrToShape");
    TopoDS_Shape vEdges_ = hlrToShape_.VCompound();
    // TopoDS_Shape hEdges_ = hlrToShape_.HCompound();
    // perf("hlrAlgo->HCompound()");

    gp_Trsf invTrsf = aTrsf.Inverted();
    BRepBuilderAPI_Transform transformer(vEdges_, invTrsf);
    TopoDS_Shape transformedShape = transformer.Shape();


    visible_ = new AIS_NonSelectableShape(transformedShape);
    // visible_ = new AIS_Shape(transformedShape);
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(1.5);
    m_context->Display(visible_, false);

    
    // Handle(Prs3d_LineAspect) lineAspect = new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 0.5);
    // // Handle(Prs3d_Drawer) aDrawer = new Prs3d_Drawer();
    // // hEdges_.SetAttributes(aDrawer);

    // BRepBuilderAPI_Transform transformerh(hEdges_, invTrsf);
    // TopoDS_Shape transformedShapeh = transformerh.Shape();


    // hidden_ = new AIS_ColoredShape(transformedShapeh);

 
    // Handle(Prs3d_LineAspect) dashedAspect = new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0);
  
    // hidden_->Attributes()->SetSeenLineAspect(dashedAspect);
 

    // m_context->Display(hidden_, 0); 
 
//  setbar5per();
	perf("hlr slower");
}



#pragma endregion projection
#pragma region tests
	void test2(){
		perf();
		//test
		luadraw* test=new luadraw("consegui",this);
		test->visible_hardcoded=0;
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
		test2->rotate(40);	
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

	}

    void test(int rot=45){
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 100.0, 100.0).Shape(); 
        TopoDS_Shape pl1=pl(); 

        gp_Trsf trsf;
trsf.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), rot*(M_PI/180)); // rotate 30 degrees around Z-axis
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
gp_Dir ydir =   gp_Dir(0.1, -0.5, 0);
// gp_Dir ydir =   gp_Dir(0.866025, -0.5, 0);
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
 
ShowTriedronWithoutLabels(trsf,m_context); // debug

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

// {
// draw_objs();
// m_view->FitAll();
// redraw();
// }
} 

#pragma endregion tests

#pragma region view_rotate
void colorisebtn(int idx=-1){
    int idx2=-1;
    if(idx==-1){
        vint vidx=check_nearest_btn_idx();
        if(vidx.size()==0)return;
        idx = vidx[0];
        if(vidx.size()>1)idx2=vidx[1];
        if(idx<0)return;
    }
    lop(i,0,sbt.size()){
        if(i==idx){
            sbt[i].occbtn->color(FL_RED);
            sbt[i].occbtn->redraw();  
        }else if(idx2>=0 && idx2==i){
            sbt[i].occbtn->color(fl_rgb_color(255, 165, 0));
            sbt[i].occbtn->redraw();  
        }
        else{            
            sbt[i].occbtn->color(FL_BACKGROUND_COLOR); 
            sbt[i].occbtn->redraw(); 
        }
    }
}
 
struct sbts { 
    string label;
    std::function<void()> func;
    bool is_setview=0;
    struct vs{Standard_Real dx=0, dy=0, dz=0, ux=0, uy=0, uz=0;}v;
    int idx;
    Fl_Button* occbtn; 
    OCC_Viewer* occv;

    static void call(Fl_Widget*, void* data) {
        auto* wrapper = static_cast<sbts*>(data);
        auto& occv = wrapper->occv;
        auto& m_view = wrapper->occv->m_view;
        if(wrapper->is_setview){
            // cotm(1);
            // m_view->SetProj(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz); 
            // cotm(2)
            // m_view->SetUp(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz);

            occv->end_proj_global=gp_Vec(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
            occv->end_up_global=gp_Vec(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz);
            occv->colorisebtn(wrapper->idx);
            occv->start_animation(occv);
            // occv->animate_flip_view(occv);
        }
        if (wrapper && wrapper->func) wrapper->func();
        // cotm(wrapper->label);

        // // Later, retrieve them
        // Standard_Real dx, dy, dz, ux, uy, uz;
        // m_view->Proj(dx, dy, dz);
        // m_view->Up(ux, uy, uz);
        // cotm(dx, dy, dz, ux, uy, uz);
    }

    // 🔍 Find a pointer to sbts by label: sbts* found = sbts::find_by_label(sbt, "T");
    static sbts* find_by_label(vector<sbts>& vec, const string& target) {
        for (auto& item : vec) {
            if (item.label == target)
                return &item;
        }
        return nullptr;
    }
};

vector<sbts> sbt;
void drawbuttons(float w,int hc1 ){
    // auto& sbt = occv->sbt;
    // auto& sbts = occv->sbts;

    std::function<void()> func=[this,&sbt = this->sbt]{ cotm(m_initialized,  sbt[0].label)   };

    sbt = {
        sbts{"Front",    {}, 1, { 0, -1,  0,   0,  0,  1 }},
        sbts{"Top",      {}, 1, { 0,  0,  1,   0,  1,  0 }},
        sbts{"Left",     {}, 1, {-1,  0,  0,   0,  0,  1 }},
        sbts{"Right",    {}, 1, { 1,  0,  0,   0,  0,  1 }},
        sbts{"Back",     {}, 1, { 0,  1,  0,   0,  0,  1 }},
        sbts{"Bottom",   {}, 1, { 0,  0, -1,   0, -1,  0 }},
        sbts{"Iso", {}, 1, { 0.57735, -0.57735, 0.57735 ,  -0.408248 , 0.408248 ,0.816497  }},

        // sbts{"Iso",{}, 1, { 1,  1,  1,   0,  0,  1 }},

        //old standard
        // sbts{"Front",     {}, 1, {  0,  0,  1,   0, 1,  0 }},
        // sbts{"Back",      {}, 1, {  0,  0, -1,   0, 1,  0 }},
        // sbts{"Top",       {}, 1, {  0, -1,  0,   0, 0, -1 }},
        // sbts{"Bottom",    {}, 1, {  0,  1,  0,   0, 0,  1 }},
        // sbts{"Left",      {}, 1, {  1,  0,  0,   0, 1,  0 }},
        // sbts{"Right",     {}, 1, { -1,  0,  0,   0, 1,  0 }},
        // sbts{"Isometric", {}, 1, { -1,  1,  1,   0, 1,  0 }},
        sbts{"Iso z", {}, 1, { -1,  1,  1,   0, 1,  0 }},

         
        // sbts{"T",[this,&sbt = this->sbt]{ cotm(sbt[0].label)   }},

        sbts{"Invert",[this]{
            Standard_Real dx, dy, dz, ux, uy, uz;
            m_view->Proj(dx, dy, dz);
            m_view->Up(ux, uy, uz); 
            // Reverse the projection direction
            end_proj_global=gp_Vec(-dx, -dy, -dz); 
            end_up_global=gp_Vec(-ux, -uy, -uz);
            start_animation(this);   
        }},

        sbts{"Invert p",[this]{
            Standard_Real dx, dy, dz, ux, uy, uz;
            m_view->Proj(dx, dy, dz);
            m_view->Up(ux, uy, uz); 
            // Reverse the projection direction
            end_proj_global=gp_Vec(-dx, -dy, -dz); 
            end_up_global=gp_Vec(ux, uy, uz);
            start_animation(this);   
        }},

        // sbts{"Invertan",[this]{
        //     animate_flip_view(this);
        // }},
        // sbts{"Ti",func }
        // add more if needed
    };

    float w1 = ceil(w/sbt.size())+0.0;

    lop(i, 0, sbt.size()) {
        sbt[i].occv=this;
        sbt[i].idx=i;
        sbt[i].occbtn = new Fl_Button(w1 * i, 0, w1, hc1, sbt[i].label.c_str());
        sbt[i].occbtn->callback(sbts::call, &sbt[i]); // ✅ fixed here
    }
}

gp_Vec end_proj_global;   
gp_Vec end_up_global; 
Handle(AIS_AnimationCamera) CurrentAnimation;

void start_animation(void* userdata) 
{
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
    gp_Dir targetDir(
        -occv->end_proj_global.X(),  // Invert X
        -occv->end_proj_global.Y(),  // Invert Y
        -occv->end_proj_global.Z()); // Invert Z
    
    gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;
    
    // Set target camera position
    cameraEnd->SetEye(targetEye);
    cameraEnd->SetCenter(center);
    cameraEnd->SetDirection(targetDir);
    cameraEnd->SetUp(gp_Dir(
        occv->end_up_global.X(),
        occv->end_up_global.Y(),
        occv->end_up_global.Z()));

    // Create and configure animation
    occv->CurrentAnimation = new AIS_AnimationCamera("ViewAnimation", m_view);
    occv->CurrentAnimation->SetCameraStart(cameraStart);
    occv->CurrentAnimation->SetCameraEnd(cameraEnd);
    occv->CurrentAnimation->SetOwnDuration(0.6);  // Duration in seconds

    // Start animation immediately
    occv->CurrentAnimation->StartTimer(
        0.0,           // Start time
        1.0,           // Playback speed (normal)
        Standard_True,  // Update to start position
        Standard_False  // Don't stop timer at start
    );

    // Start the update timer
    // Fl::add_timeout(0.001, animation_update, userdata);
    Fl::add_timeout(1.0/12, animation_update, userdata);
}

static void animation_update(void* userdata)
{
    auto* occv = static_cast<OCC_Viewer*>(userdata);
    auto& m_view = occv->m_view;

    if (occv->CurrentAnimation.IsNull()) {
        Fl::remove_timeout(animation_update, userdata);
        return;
    }

    if (occv->CurrentAnimation->IsStopped()) {
        // Ensure perfect final position with inverted direction
        gp_Dir targetDir(
            -occv->end_proj_global.X(), 
            -occv->end_proj_global.Y(), 
            -occv->end_proj_global.Z());
        
        gp_Pnt center = m_view->Camera()->Center();
        double distance = m_view->Camera()->Distance();
        gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;
        
        m_view->Camera()->SetEye(targetEye);
        m_view->Camera()->SetDirection(targetDir);
        m_view->Camera()->SetUp(gp_Dir(
            occv->end_up_global.X(),
            occv->end_up_global.Y(),
            occv->end_up_global.Z()));
        
        occv->colorisebtn();
        occv->projectAndDisplayWithHLR(occv->vshapes);
        occv->redraw(); //  redraw(); // m_view->Update ();


        Fl::remove_timeout(animation_update, userdata);
        return;
    }

    // Update animation with current time
    occv->CurrentAnimation->UpdateTimer();
    occv->projectAndDisplayWithHLR(occv->vshapes);
    occv->redraw(); //  redraw(); // m_view->Update ();


    
    // Continue updating at 30fps
    Fl::repeat_timeout(1.0/30.0, animation_update, userdata);
}


vector<int> check_nearest_btn_idx() {
    // Get current view orientation
    Standard_Real dx, dy, dz, ux, uy, uz;
    m_view->Proj(dx, dy, dz);
    m_view->Up(ux, uy, uz);
    
    gp_Dir current_proj(dx, dy, dz);
    gp_Dir current_up(ux, uy, uz);
    
    vector<pair<double, int>> view_scores; // Stores angle sums with their indices
    
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
OCC_Viewer* occv=0;

static Fl_Menu_Item items[] = {
	// Main menu items
	{ "&File", 0, 0, 0, FL_SUBMENU },
		{ "&Open", FL_ALT + 'o', ([](Fl_Widget *, void* v){ 	  
			open_cb();	  
		}), (void*)menu },
		 
		
		{ "Pause", 0, ([](Fl_Widget *fw, void* v){ 	 
            if(!occv)cotm("notok")
            occv->m_view->SetProj(V3d_XposYposZpos);
            // occv->animate_flip_view(occv); 
			// Fl_Menu_Item* btpause= const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)fw)->find_item("&File/Pause"));
			// if(btpause->value()>0)
			// // 	lop(i,0,pool.size())
			// // 		pool[i]->pause=1;
			// // else
			// // 	lop(i,0,pool.size())
			// // 		pool[i]->pause=0;	
			
			// cot1(btpause->value());  
		}), (void*)menu, FL_MENU_TOGGLE },


		{ "&Get view", FL_COMMAND  + 'g', ([](Fl_Widget *, void* v){  
            occv->m_view->SetProj(1,1,1);
			// // ve[0]->rotate(Vec3f(0,0,1),45); 
			// getview();
			// // Break=1;
			
			// 	// ve[0]->rotate( 10); 
			// 	// cot(*ve[1]->axisbegin ); 
			// 	// cot(*ve[1]->axisend );  
		}), (void*)menu },

		{ "Inverse kinematics", 0, ([](Fl_Widget *, void* v){ 	
            
        
        Standard_Real dx, dy, dz, ux, uy, uz;
        occv->m_view->Proj(dx, dy, dz);
        occv->m_view->Up(ux, uy, uz);
        cotm("draw",dx, dy, dz, ux, uy, uz);
			// // ve[ve.size()-1]->rotate_posk(10);	
			// // dbg_pos(); /////
			// ve[3]->posik(vec3(150,150,-150));
		}), (void*)menu },

		{ "Quit", 0, ([](Fl_Widget *, void* v){cotm("exit"); exit(0);}) },
		{ 0 },
	
	{ "&View", 0, 0, 0, FL_SUBMENU },

		{ "&Fit all", FL_ALT + 'f', ([](Fl_Widget *, void* v){ 	 
            occv->m_view->FitAll();
		}), (void*)menu, FL_MENU_DIVIDER },

		{ "&Transparent", FL_ALT + 't', ([](Fl_Widget *mnu, void* v){ 	
			// Fl_Menu_Item* btn= const_cast<Fl_Menu_Item*>(((Fl_Menu_Bar*)fw)->find_item("&View/Transparent"));
            Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
            const Fl_Menu_Item* item = menu->mvalue();  // This gets the actually clicked item
            
				if (!item->value()) { 
				occv->toggle_shaded_transp(AIS_WireFrame);
				}else{
				occv->toggle_shaded_transp(AIS_Shaded);
				}
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
occv-> m_view->Redraw ();	    
            occv-> redraw(); //  redraw(); // 


		}), (void*)menu,FL_MENU_TOGGLE },

		
		{ "&test", 0, ([](Fl_Widget *, void* v){ 	
            perf();
			lop(i,0,40)occv->test(i*5);
            perf("test");
                {
occv->draw_objs();
            perf("draw");
// occv->m_view->FitAll();
occv->redraw();
occv->colorisebtn();



// occv->m_view->Redraw();
            perf("draw");

// occv->m_view->Update();
}
		}), (void*)menu },

		
		{ "Robot visible", 0, ([](Fl_Widget *, void* v){ 	
			glFlush();
glFinish();

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
			// ViewerFLTK* view=osggl;
			// if ( view->getCamera() ){
			// 	// http://ahux.narod.ru/olderfiles/1/OSG3_Cookbook.pdf
			// 	double _distance=-1; float _offsetX=0, _offsetY=0;
			// 	osg::Vec3d eye, center, up;
			// 	view->getCamera()->getViewMatrixAsLookAt( eye, center,
			// 	up );
	
			// 	osg::Vec3d lookDir = center - eye; lookDir.normalize();
			// 	osg::Vec3d side = lookDir ^ up; side.normalize();
	
			// 	const osg::BoundingSphere& bs = view->getSceneData()->getBound();
			// 	if ( _distance<0.0 ) _distance = bs.radius() * 3.0;
			// 	center = bs.center();
			// 	center -= (side * _offsetX + up * _offsetY) * 0.1;
			// 	fix_center_bug=center;
			// 	// up={0,1,0};
			// 	osggl->tmr->setHomePosition( center-lookDir*_distance, center, up );
			// 	osggl->setCameraManipulator(osggl->tmr);
			// 	getview();
			// }	
		}), (void*)menu },
		{ 0 },
	
	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "&About v2", 0, ([](Fl_Widget *, void* v){ 
			// // lua_init();
			// // getallsqlitefuncs();
			// string val=getfunctionhelp();
			// cot1(val);
			// fl_message(val.c_str());
		}), (void*)menu },
		{ 0 },


		
	{ "&test", 0, ([](Fl_Widget *, void* v){ 
		cotm("test")
			// // lua_init();
			// // getallsqlitefuncs();
			// string val=getfunctionhelp();
			// cot1(val);
			// fl_message(val.c_str());
		}), (void*)menu },
		{ 0 },

	{ 0 } // End marker
};



// Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
// // ... use aisShape ...
// aisShape.Nullify();
// BRepTools::Clean();

 


int main(int argc, char** argv) { 
    Fl::use_high_res_GL(1);
    // setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
	std::cout << "FLTK version: "
              << FL_MAJOR_VERSION << "."
              << FL_MINOR_VERSION << "."
              << FL_PATCH_VERSION << std::endl;
    std::cout << "OCCT version: " << OCC_VERSION_COMPLETE << std::endl;

	Fl::visual(FL_DOUBLE|FL_INDEX);
	Fl::gl_visual( FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);

     Fl::scheme("gleam");
    
    Fl::lock();    
    int w=1024;
	int h=576; 
    Fl_Double_Window* win = new Fl_Double_Window(0,0,w, h, "simplecad");
    win->color(FL_RED);
    win->callback([](Fl_Widget *widget, void* ){		
		if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
			return;
		menu->find_item("&File/Quit")->do_callback(menu);
	});
    menu = new Fl_Menu_Bar(0, 0,w,22);  
	menu->menu(items); 
  
    int hc1=24;

    Fl_Group* content = new Fl_Group(0, 22, w, h-22); 

	// scint_init(w*0.62,22,w*0.38,h-22-hc1); 

    occv = new OCC_Viewer(0, 22, w*0.62, h-22-hc1);
    content->add(occv); 

    Fl_Window* woccbtn = new Fl_Window(0,h-hc1,occv->w(),hc1, "");
    content->add(woccbtn); 
    
    Fl_Group* content1 = new Fl_Group(0, 0, woccbtn->w()+0, woccbtn->h());

    
    occv->drawbuttons(woccbtn->w(),hc1);
	// content1->end();
    woccbtn->resizable(content1);
    

	Fl_Group::current(content);
	// content->begin();
	scint_init(w*0.62,22,w*0.38,h-22-hc1); 



	// win->clear_visible_focus(); 	 
	woccbtn->color(0x7AB0CfFF);
	win->resizable(content);	
	// win->position(Fl::w()/2-win->w()/2,10); 
	win->position(0,0);  
    win->show(argc, argv); 
	while (!win->shown()) Fl::wait();
	win->wait_for_expose();     // wait, until displayed
	occv->wait_for_expose();     // wait, until displayed
	Fl::flush();                // make sure everything gets drawn
	win->flush(); 
	// occv->flush(); 
    // win->maximize();
	// int x, y, _w, _h; 
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22);
	// sleepms(200);
    occv->initialize_opencascade();

	// return Fl::run();
// Fl_Group::current(content);
	// content->begin();
// 	scint_init(w*0.62,22,w*0.38,h-22-hc1); 
// content->end();


    occv->test2();
    // occv->test();
    {
// occv->draw_objs();
occv->m_view->FitAll();
occv->redraw(); //for win
// occv->m_view->Update();
}
    return Fl::run();
}