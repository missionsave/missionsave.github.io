#pragma region includes

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

 

#include <FL/Fl.H>
#include <FL/Fl_Window.H> 
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

#include <chrono>
#include <thread>
#include <functional>
#include <cmath>

#include "general.hpp"


using namespace std;
#pragma endregion includes


 

Fl_Window* win;
Fl_Menu_Bar* menu;

void open_cb() {
    Fl_File_Chooser chooser(".", "*", Fl_File_Chooser::SINGLE, "Escolha um arquivo");
    chooser.show();
    while (chooser.shown()) Fl::wait();
    if (chooser.value()) {
        printf("Arquivo selecionado: %s\n", chooser.value());
    }
}

struct OCC_Viewer;
struct  OCC_Viewer : public Fl_Window {
// public:
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
	Handle(AIS_Shape) visible_;
    Handle(AIS_ColoredShape) hidden_;
 

    OCC_Viewer(int X, int Y, int W, int H, const char* L = 0)
        : Fl_Window(X, Y, W, H, L) { 
			// nested;
			
		Fl::add_timeout(10, idle_refresh_cb,0);
    }
     

	
 
    void initialize_opencascade() { 
        // Get native window handle
#ifdef _WIN32
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
    }

    void resize(int X, int Y, int W, int H) override {
        Fl_Window::resize(X, Y, W, H);
        if (m_initialized) {
            m_view->MustBeResized();
            setbar5per();
            m_view->Redraw(); 
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


#pragma endregion initialization
	

#pragma region lua
	struct luadraw;
	vector<luadraw> vluadraw;
	struct luadraw{
		string command="";
		TopoDS_Shape shape; 
		Handle(AIS_Shape) ashape; 
		TopoDS_Face face;
		gp_Pnt origin=gp_Pnt(0, 0, 0);
		gp_Dir normal=gp_Dir(0,0,1);
		gp_Dir xdir =  gp_Dir(1, 0, 0);
		gp_Trsf trsf;
		gp_Trsf trsftmp; 
		luadraw(){
			gp_Ax2 ax3(origin, normal, xdir);
			trsf.SetTransformation(ax3);
			trsf.Invert();
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
		void dofromstart(){
			gp_Pnt p1(0, 0,0);
			gp_Pnt p2(100, 0,0);
			gp_Pnt p3(100, 50,0);
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
			// wireBuilder.Add(e4);
			TopoDS_Wire wire = wireBuilder.Wire();

			face = BRepBuilderAPI_MakeFace(wire);

			//final
			BRepBuilderAPI_Transform transformer(face, trsf);
			shape= transformer.Shape();

		}

	};


 
 
#pragma endregion lua
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
        cotmupset
        cotm("Nothing detected under the mouse.");
        cotmup
        clearHighlight();
    }

    m_context->UpdateCurrentViewer();
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
    m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));


    // 3. Perform the picking operations
    m_context->MoveTo(x, y, m_view, Standard_False);


    m_context->SelectDetected(AIS_SelectionScheme_Replace);

    // 4. Get the detected owner
    Handle(SelectMgr_EntityOwner) anOwner = m_context->DetectedOwner();

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
        cotmupset
		cotm("Nothing detected under the mouse.");
		cotmup

        clearHighlight(); // Nothing detected
    }

    m_context->UpdateCurrentViewer(); // Update the viewer to show/hide highlight
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

        case FL_DRAG:
            if (Fl::event_state(FL_BUTTON1)) {
                // Only rotate if drag started in right edge zone
                if (Fl::event_x() > (this->w() - edge_zone)) {
                    int dy = Fl::event_y() - start_y;
                    start_y = Fl::event_y();
                    
                    // Rotate view (OpenCascade)
                    if (dy != 0) {
                        double angle = dy * 0.005; // Rotation sensitivity factor
    	                // Fl::wait(0.01);	
                        m_view->Rotate(0, 0, angle); // Rotate around Z-axis
                        projectAndDisplayWithHLR(vshapes);
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
                funcfps(12,m_view->Rotation(Fl::event_x(),Fl::event_y());
                projectAndDisplayWithHLR(vshapes);
                 redraw(); //  redraw(); // m_view->Update ();


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
    if(hlr_on)return;
    m_view->SetComputedMode(Standard_False);    
    // m_context->RemoveAll(false); 


        // if(visible_) m_context->Remove(visible_,0);
        // if(hidden_) m_context->Remove(hidden_,0);



    Handle(Prs3d_Drawer) defaultDrawer = m_context->DefaultDrawer();
  
	// Set default line widths
	// defaultDrawer->WireAspect()->SetWidth(3);
	// defaultDrawer->LineAspect()->SetWidth(3);
	// defaultDrawer->SetColor(Quantity_NOC_RED);
    for(int i=0;i<vshapes.size();i++)
    {
        pashape* aShape = new pashape(vshapes[i]);
        // vaShape.push_back(aShape);
        // m_context->SetDisplayMode(aShape, AIS_Shaded, 0); /////////////////////////////
        // m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_True); 

        //   m_context->Activate(aShape, aShape->SelectionMode(TopAbs_VERTEX));
        //   m_context->Activate(aShape, aShape->SelectionMode(TopAbs_EDGE));



    // m_context->SetMode(aShape, TopAbs_VERTEX, Standard_True); // Enable vertex mode for 'aisShape'
    // m_context->SetMode(aShape, TopAbs_EDGE, Standard_True);   // Enable edge mode for 'aisShape' (if you still want to pick edges too)
    // m_context->SetMode(aShape, TopAbs_FACE, Standard_True);   // Enable face mode for 'aisShape'

    // // You can also adjust sensitivity for point picking.
    // // A smaller value means you need to be closer.
    // m_context->SetSelectionSensitivity(0.5);


{

// Define o modo de exibição como wireframe
// m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_True);

// Acessa os atributos de apresentação
// Handle(Prs3d_Drawer) drawer = aShape->Attributes();

// // Define o aspecto de linha tracejada vermelha
// Handle(Prs3d_LineAspect) dashedAspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_DASH, 2.0);
// drawer->SetWireAspect(dashedAspect);

// drawer->(Quantity_NOC_RED);
// aShape->setColor(drawer,Quantity_NOC_RED);

// Exibe a forma com atualização forçada
// 	Handle(Prs3d_Drawer) dr1 = aShape->Attributes();
// if (dr1->WireAspect()->Aspect()->Type() == Aspect_TOL_DASH) {
//   std::cout << "Linha tracejada aplicada!" << std::endl;
// } 
m_context->Display(aShape, Standard_False);
// m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_True);


// m_context->Redisplay(aShape, Standard_True);
continue;
}



m_context->SetDisplayMode(aShape, AIS_WireFrame, false);


    // aShape->Attributes()->SetSeenLineAspect(
    //     new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0)
    // );
Handle(Prs3d_Drawer) drawer = aShape->Attributes();
drawer->SetFaceBoundaryDraw(false); // Don't draw edges on shaded shapes



// Set dashed blue lines
drawer->SetSeenLineAspect(new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_SOLID, 3.0));
// drawer->SetSeenLineAspect(new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0));





        // aShape->SetColor(Quantity_NOC_GRAY70);
        // aShape->Attributes()->SetFaceBoundaryDraw(Standard_True);  
        // Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_DASH, 1.0);
        // // Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 3.0);
        // aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);


        // aShape->Attributes()->SetSeenLineAspect(edgeAspect);
        // aShape->SetTransparency(0.5);
        m_context->Display(aShape, 0); 
        // m_context->Display(aShape, Standard_True); 
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
    ctx->Display(tri, Standard_False);
}



#pragma region projection

#include <HLRAlgo_Projector.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
 
HLRAlgo_Projector projector;

#include <TopExp.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
gp_Pnt getAccurateVertexPosition(const gp_Pnt& projectedHLRPnt) {
    // Ponto HLR como 3D plano Z=0 → recuperar aproximado 3D
    gp_Pnt approx3D = convertHLRToWorld(projectedHLRPnt);

    // Se quiseres o vértice exato de vshapes cujo projected2d mais se aproxima:
    gp_Pnt2d screen2d(projectedHLRPnt.X(), projectedHLRPnt.Y());
    Standard_Real bestDist = RealLast();
    gp_Pnt bestMatch = approx3D;

    for (auto& s : vshapes) {
        for (TopExp_Explorer exp(s, TopAbs_VERTEX); exp.More(); exp.Next()) {
            gp_Pnt P = BRep_Tool::Pnt(TopoDS::Vertex(exp.Current()));
            // projeta de novo
            gp_Pnt2d P2;
            projector.Project(P, P2);
            double d = (P2.X() - screen2d.X())*(P2.X() - screen2d.X())
                     + (P2.Y() - screen2d.Y())*(P2.Y() - screen2d.Y());
            if (d < bestDist) {
                bestDist = d;
                bestMatch = P;
            }
        }
    }
    return bestMatch;
}

gp_Trsf   invTrsf;     // inversa de viewTrsf
gp_Pnt convertHLRToWorld(const gp_Pnt& projectedHLRPnt) {
    // projectedHLRPnt deve estar no plano 2D (Z = 0)
    return projectedHLRPnt.Transformed(invTrsf);
}
gp_Pnt getAccurateVertexPositionprev(const gp_Pnt& inputWorldPoint) {
    gp_Pnt2d projected2d;
    projector.Project(inputWorldPoint, projected2d);

    // Transforma para ponto 3D no plano 2D (Z = 0)
    gp_Pnt screenPnt(projected2d.X(), projected2d.Y(), 0);

    // Procura o vértice original mais próximo com base nessa projeção
    Standard_Real bestDist = RealLast();
    gp_Pnt bestMatch;

    for (const auto& shape : vshapes) {
        for (TopExp_Explorer exp(shape, TopAbs_VERTEX); exp.More(); exp.Next()) {
            gp_Pnt candidate = BRep_Tool::Pnt(TopoDS::Vertex(exp.Current()));

            gp_Pnt2d projCandidate;
            projector.Project(candidate, projCandidate);
            gp_Pnt projCandidate3D(projCandidate.X(), projCandidate.Y(), 0);

            double dist = projCandidate3D.SquareDistance(screenPnt);
            if (dist < bestDist) {
                bestDist = dist;
                bestMatch = candidate;
            }
        }
    }

    return bestMatch;
}





gp_Pnt getAccurateVertexPositionnaofunc(const Handle(AIS_InteractiveContext)& context, int x, int y) {
    context->MoveTo(x, y, m_view, false);
    if (context->HasDetected()) {
        context->InitDetected();
        for (; context->MoreDetected(); context->NextDetected()) {
            TopoDS_Shape shape = context->DetectedShape();
            if (shape.ShapeType() == TopAbs_VERTEX) {
                return BRep_Tool::Pnt(TopoDS::Vertex(shape));
            }
        }
    }
    return gp_Pnt(0, 0, 0); // Ou algum ponto nulo padrão
}

gp_Pnt getAccurateVertexPositionnfunciona(const gp_Pnt& screenPoint) {
    // 1. First try to find the point in original shapes
    for (const auto& shape : vshapes) {
        // Simple bounding box check first
        Bnd_Box bbox;
        BRepBndLib::Add(shape, bbox);
        if (bbox.IsOut(screenPoint)) continue;
        
        // Detailed vertex search
        for (TopExp_Explorer exp(shape, TopAbs_VERTEX); exp.More(); exp.Next()) {
            gp_Pnt vertex = BRep_Tool::Pnt(TopoDS::Vertex(exp.Current()));
            if (vertex.SquareDistance(screenPoint) < Precision::SquareConfusion()) {
                return vertex; // Return exact original position
            }
        }
    }
	    return screenPoint;
}



gp_Pnt convertHLRVertexToWorld(const gp_Pnt& hlrPoint){
    // 1. First find the closest vertex in either visible or hidden edges
    TopoDS_Vertex closestVertex;
    Standard_Real minDist = RealLast();
    
    auto checkShape = [&](const TopoDS_Shape& shape) {
        for (TopExp_Explorer exp(shape, TopAbs_VERTEX); exp.More(); exp.Next()) {
            gp_Pnt vertexPos = BRep_Tool::Pnt(TopoDS::Vertex(exp.Current()));
            Standard_Real dist = vertexPos.SquareDistance(hlrPoint);
            if (dist < minDist) {
                minDist = dist;
                closestVertex = TopoDS::Vertex(exp.Current());
            }
        }
    };
    
    checkShape(vEdges); // Check visible edges
    checkShape(hEdges); // Check hidden edges

    // 2. If we found a close vertex, try to get its exact 3D position
    if (!closestVertex.IsNull()) {
        // Find the edge containing this vertex
        TopoDS_Edge containingEdge;
        auto findContainingEdge = [&](const TopoDS_Shape& shape) {
            for (TopExp_Explorer exp(shape, TopAbs_EDGE); exp.More(); exp.Next()) {
                TopoDS_Edge edge = TopoDS::Edge(exp.Current());
                TopoDS_Vertex v1, v2;
                TopExp::Vertices(edge, v1, v2);
                if (v1.IsSame(closestVertex) || v2.IsSame(closestVertex)) {
                    containingEdge = edge;
                    return;
                }
            }
        };
        
        findContainingEdge(vEdges);
        findContainingEdge(hEdges);

        // 3. If we found an edge, try to get its exact 3D curve
        if (!containingEdge.IsNull()) {
            // Get the parameter of the vertex on the edge
            Standard_Real param = BRep_Tool::Parameter(closestVertex, containingEdge);
            
            // Create curve adaptor for the edge
            BRepAdaptor_Curve curve(containingEdge);
            return curve.Value(param); // Return the exact 3D point
        }
    }

    // 4. Fallback: Use mathematical projection inversion
    const gp_Trsf& projTrsf = hlrAlgo->Projector().Transformation();
    gp_Trsf invTrsf = projTrsf.Inverted();
    gp_Pnt worldPoint = hlrPoint;
    worldPoint.Transform(invTrsf);
    
    return worldPoint;
}



 // Conversion function that uses the global projector
gp_Pnt convertHLRToWorldprev(const gp_Pnt& hlrPoint)
{
    // 1. Get the current view parameters
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    bool isOrthographic = camera->IsOrthographic();
    
    // 2. Get the transformation from the global projector
    const gp_Trsf& hlrTrsf = projector.Transformation();
    gp_Trsf inverseTrsf = hlrTrsf;
    // gp_Trsf inverseTrsf = hlrTrsf.Inverted();
    
    // 3. Handle orthographic projection
    if (isOrthographic) {
        gp_Pnt worldPoint = hlrPoint;
        worldPoint.Transform(inverseTrsf);
        return worldPoint;
    }
    // 4. Handle perspective projection
    else {
        Standard_Real focus = projector.Focus();
        Standard_Real Z = focus / (1.0 - hlrPoint.Z()/focus);
        
        gp_Pnt perspectivePoint(
            hlrPoint.X() * Z / focus,
            hlrPoint.Y() * Z / focus,
            Z
        );
        
        perspectivePoint.Transform(inverseTrsf);
        return perspectivePoint;
    }
}

void projectAndDisplayWithHLRW(const std::vector<TopoDS_Shape>& shapes){
    if(!hlr_on)return;
	perf();
    // cotm("hlr")
    // m_view->SetImmediateUpdate(Standard_False);
    if (m_context.IsNull() || m_view.IsNull()) {
        std::cerr << "Error: m_context or m_view is null." << std::endl;
        return;
    } 
    m_view->SetComputedMode(Standard_True);


    {
    // m_context->RemoveAll(false); 
        lop(i,0,vaShape.size()){
            m_context->Remove(vaShape[i],0);
        }

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
    TopoDS_Shape hEdges_ = hlrToShape_.HCompound();
    // perf("hlrAlgo->HCompound()");

    gp_Trsf invTrsf = aTrsf.Inverted();
    BRepBuilderAPI_Transform transformer(vEdges_, invTrsf);
    TopoDS_Shape transformedShape = transformer.Shape();


    visible_ = new AIS_Shape(transformedShape);
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(1.5);
    m_context->Display(visible_, false);

    
    Handle(Prs3d_LineAspect) lineAspect = new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 0.5);
    // Handle(Prs3d_Drawer) aDrawer = new Prs3d_Drawer();
    // hEdges_.SetAttributes(aDrawer);

    BRepBuilderAPI_Transform transformerh(hEdges_, invTrsf);
    TopoDS_Shape transformedShapeh = transformerh.Shape();


    hidden_ = new AIS_ColoredShape(transformedShapeh);

 
    Handle(Prs3d_LineAspect) dashedAspect = new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0);
  
    hidden_->Attributes()->SetSeenLineAspect(dashedAspect);
 

    m_context->Display(hidden_, 0); 
 
//  setbar5per();
	perf("hlr slower");
}
 
#include <BRepMesh_IncrementalMesh.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <BRepTools.hxx>

#include <BRepMesh_IncrementalMesh.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>

TopoDS_Shape vEdges;
TopoDS_Shape hEdges;
Handle(HLRBRep_PolyAlgo) hlrAlgo;
// gp_Trsf glb_invTrsf;

gp_Pnt screenPointFromMouse(int mousex, int mousey) {
    Standard_Real X, Y, Z;
    m_view->Convert(mousex, mousey, X, Y, Z);
    return gp_Pnt(X, Y, 0.0);
}
void projectAndDisplayWithHLR(const std::vector<TopoDS_Shape>& shapes) {
    if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;
	
    // m_context->RemoveAll(false);

        if(visible_) m_context->Remove(visible_,0);
        if(hidden_) m_context->Remove(hidden_,0);

    // 1. Prepara camera e transformação
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    // usa Ax3 completo (posição + orientação)
gp_Dir viewDir = -camera->Direction(); // direção da visualização
gp_Dir viewUp = camera->Up();          // vetor para cima
gp_Dir viewRight = viewUp.Crossed(viewDir); // vetor para a direita

gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight); // origem, direção Z, direção X

    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);
    invTrsf = viewTrsf.Inverted();

    // 2. Cria o projector HLR
    projector = HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(),
        camera->Scale()
    );

    // 3. Malha consistente
    Standard_Real deflection = 0.01;
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
    visible_ = new AIS_Shape(visT.Shape());
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(3);
    m_context->Display(visible_, false);

    // Ocultas
    // hEdges = hlrToShape.HCompound();
    // BRepBuilderAPI_Transform hidT(hEdges, invTrsf);
    // hidden_ = new AIS_ColoredShape(hidT.Shape());

// Disable selection completely:
// hidden_->SetPickable(Standard_False); 

// 4. For complete isolation, use owner filtering:
// m_context->AddFilter(new AIS_TypeFilter(hidden_->DynamicType()->Name()));
// m_context->Activate(hidden_, 0); // Deactivate all selection modes



    // hidden_->Attributes()->SetSeenLineAspect(
    //     new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0)
    // );
    // m_context->Display(hidden_, false);

    // 6. Guarda shapes originais para picking
    // vshapes = shapes;

    m_context->UpdateCurrentViewer();
	// draw_objs();
}

void projectAndDisplayWithHLRworking(const std::vector<TopoDS_Shape> &shapes) {
    if (!hlr_on) return;
    
    // 1. Setup and cleanup
    if (m_context.IsNull() || m_view.IsNull()) return;
    m_view->SetComputedMode(Standard_True);
    m_context->RemoveAll(false);

    // 2. Get camera orientation
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    gp_Dir viewDir = -camera->Direction();
    gp_Dir viewUp = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);

    // 3. Create transformation using ONLY orientation
    gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight);
    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);

    // 4. Create projector
    projector = HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(), // Correct negation for perspective
        camera->Scale()
    );

    // 5. Prepare geometry with consistent meshing
    Standard_Real deflection = 0.01;
    for (const auto& shape : shapes) {
        if (!shape.IsNull()) {
            BRepMesh_IncrementalMesh mesher(shape, deflection, true, 0.5, false);
        }
    }

    // 6. Compute HLR
    hlrAlgo = new HLRBRep_PolyAlgo(); //static
    for (const auto& shape : shapes) {
        if (!shape.IsNull()) {
            hlrAlgo->Load(shape);
        }
    }
    hlrAlgo->Projector(projector);
    hlrAlgo->Update();

    // 7. Extract results
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(hlrAlgo);
    
    // Critical inverse transform
    gp_Trsf invTrsf = viewTrsf.Inverted();
    // glb_invTrsf=invTrsf;
    // Process visible edges
    vEdges = hlrToShape.VCompound();
    BRepBuilderAPI_Transform visTransform(vEdges, invTrsf);
    visible_ = new AIS_Shape(visTransform.Shape());
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(1.5);

    // Process hidden edges
    hEdges = hlrToShape.HCompound();
    BRepBuilderAPI_Transform hidTransform(hEdges, invTrsf);
    hidden_ = new AIS_ColoredShape(hidTransform.Shape());
    hidden_->Attributes()->SetSeenLineAspect(
        new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0)
    );

    // 8. Display results
    m_context->Display(visible_, false);
    m_context->Display(hidden_, false);
}


void projectAndDisplayWithHLRp1(const std::vector<TopoDS_Shape>& shapes) {
    if (!hlr_on) return;
    
    // 1. Setup and cleanup
    if (m_context.IsNull() || m_view.IsNull()) return;
    m_view->SetComputedMode(Standard_True);
    m_context->RemoveAll(false);

    // 2. Get camera orientation
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    gp_Dir viewDir = -camera->Direction();
    gp_Dir viewUp = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);

    // 3. Create transformation using ONLY orientation
    gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight);
    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);

    // 4. Create projector
    projector = HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(), // Correct negation for perspective
        camera->Scale()
    );

    // 5. Prepare geometry with consistent meshing
    Standard_Real deflection = 0.01;
    for (const auto& shape : shapes) {
        if (!shape.IsNull()) {
            BRepMesh_IncrementalMesh mesher(shape, deflection, true, 0.5, false);
        }
    }

    // 6. Compute HLR
    hlrAlgo = new HLRBRep_PolyAlgo(); //static
    for (const auto& shape : shapes) {
        if (!shape.IsNull()) {
            hlrAlgo->Load(shape);
        }
    }
    hlrAlgo->Projector(projector);
    hlrAlgo->Update();

    // 7. Extract results
    HLRBRep_PolyHLRToShape hlrToShape;
    hlrToShape.Update(hlrAlgo);
    
    // Critical inverse transform
    gp_Trsf invTrsf = viewTrsf.Inverted();
    
    // Process visible edges
    vEdges = hlrToShape.VCompound();
    BRepBuilderAPI_Transform visTransform(vEdges, invTrsf);
    visible_ = new AIS_Shape(visTransform.Shape());
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(1.5);

    // Process hidden edges
    hEdges = hlrToShape.HCompound();
    BRepBuilderAPI_Transform hidTransform(hEdges, invTrsf);
    hidden_ = new AIS_ColoredShape(hidTransform.Shape());
    hidden_->Attributes()->SetSeenLineAspect(
        new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0)
    );

    // 8. Display results
    m_context->Display(visible_, false);
    m_context->Display(hidden_, false);
}


//could be faster to drag rotation
void projectAndDisplayWithHLRwell(const std::vector<TopoDS_Shape>& shapes) {
    if (!hlr_on) return;
    // perf();
    // 1. Setup and cleanup
    if (m_context.IsNull() || m_view.IsNull()) return;
    m_view->SetComputedMode(Standard_True);
    m_context->RemoveAll(false);

    // 2. Get camera orientation (but NOT position)
    const Handle(Graphic3d_Camera)& camera = m_view->Camera();
    gp_Dir viewDir = -camera->Direction();
    gp_Dir viewUp = camera->Up();
    gp_Dir viewRight = viewUp.Crossed(viewDir);

    // 3. Create transformation using ONLY orientation
    gp_Ax3 viewAxes(gp_Pnt(0,0,0), viewDir, viewRight);
    gp_Trsf viewTrsf;
    viewTrsf.SetTransformation(viewAxes);

    // 4. Create projector (perspective flag needs negation)
    projector=HLRAlgo_Projector(
        viewTrsf,
        !camera->IsOrthographic(), // Important negation here
        camera->Scale()
    );
    // HLRAlgo_Projector projector(
    //     viewTrsf,
    //     !camera->IsOrthographic(), // Important negation here
    //     camera->Scale()
    // );

    // 5. Prepare geometry with consistent meshing
    Standard_Real deflection = 0.01;
    for (const auto& shape : shapes) {
        if (!shape.IsNull()) {
            BRepMesh_IncrementalMesh(shape, deflection, true, 0.5, false);
        }
    }

    // 6. Compute HLR
    Handle(HLRBRep_PolyAlgo) hlrAlgo = new HLRBRep_PolyAlgo();
    // Handle(HLRBRep_PolyAlgo) hlrAlgo = new HLRBRep_PolyAlgo();
    for (const auto& shape : shapes) {
        hlrAlgo->Load(shape);
    }
    hlrAlgo->Projector(projector);
    hlrAlgo->Update();

    // 7. Extract results with PROPER inverse transform
    HLRBRep_PolyHLRToShape hlrToShape;
	perf();
    hlrToShape.Update(hlrAlgo);
	perf("hlrAlgo");
    
    gp_Trsf invTrsf = viewTrsf.Inverted(); // Critical inverse transform
    
    // Process visible edges
    TopoDS_Shape vEdges = hlrToShape.VCompound();
    BRepBuilderAPI_Transform visTransform(vEdges, invTrsf);
    visible_ = new AIS_Shape(visTransform.Shape());
    visible_->SetColor(Quantity_NOC_BLACK);
    visible_->SetWidth(1.5);

    // Process hidden edges
    TopoDS_Shape hEdges = hlrToShape.HCompound();
    BRepBuilderAPI_Transform hidTransform(hEdges, invTrsf);
    hidden_ = new AIS_ColoredShape(hidTransform.Shape());
    hidden_->Attributes()->SetSeenLineAspect(
        new Prs3d_LineAspect(Quantity_NOC_BLUE, Aspect_TOL_DASH, 1.0)
    );


	projector=hlrAlgo->Projector();

    // 8. Display results
    m_context->Display(visible_, false);
    m_context->Display(hidden_, false);
	// perf("hlr faster");
}


 

#pragma endregion projection
#pragma region tests
	void test2(){
		
		//test
		luadraw test;
		// test.translate(10);
		// test.translate(0,10);
		// test.rotate(90);
		test.dofromstart();
		vshapes.push_back(test.shape);
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
    Fl::add_timeout(0.001, animation_update, userdata);
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
			lop(i,0,20)occv->test(i);
            perf("test");
                {
occv->draw_objs();
            perf("draw");
// occv->m_view->FitAll();
occv->m_view->Redraw();

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
	
	{ 0 } // End marker
};



// Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
// // ... use aisShape ...
// aisShape.Nullify();
// BRepTools::Clean();

 


int main(int argc, char** argv) { 
    Fl::use_high_res_GL(1);
    // setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);

    std::cout << "OCCT version: " << OCC_VERSION_COMPLETE << std::endl;

	Fl::gl_visual( FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
    // #if defined(__linux__)
    // // #if defined(__linux__) && defined(__aarch64__)
    //     Fl::gl_visual( FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
    // // #else
    // //     Fl::gl_visual(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
    // #endif
    // #if defined(_WIN32)
	// 	Fl::gl_visual(FL_OPENGL3 | FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
    // #endif

    Fl::scheme("gleam");
    
    Fl::lock();    
    int w=1024;
	int h=576;
    Fl_Window* win = new Fl_Window(w, h, "FLTK with OpenCASCADE");
    win->color(FL_RED);
    win->callback([](Fl_Widget *widget, void* ){		
		if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
			return;
		menu->find_item("&File/Quit")->do_callback(menu);
	});
    menu = new Fl_Menu_Bar(0, 0,w,22);  
	menu->menu(items); 
  
    Fl_Group* content = new Fl_Group(0, 22, w, h-22); 

    int hc1=24;
    occv = new OCC_Viewer(0, 22, w*0.62, h-22-hc1);

    Fl_Window* woccbtn = new Fl_Window(0,h-hc1,occv->w(),hc1, "");
    content->add(woccbtn); 
    
    Fl_Group* content1 = new Fl_Group(0, 0, woccbtn->w()+0, woccbtn->h());

    
    occv->drawbuttons(woccbtn->w(),hc1);

    woccbtn->resizable(content1);
    
	win->clear_visible_focus(); 	 
	woccbtn->color(0x7AB0CfFF);
	win->resizable(content);	
	// win->position(Fl::w()/2-win->w()/2,10); 
	win->position(0,0); 
    win->show(argc, argv); 
    // win->maximize();
	// int x, y, _w, _h; 
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22);

    occv->initialize_opencascade();
    // occv->test2();
    occv->test();
    {
occv->draw_objs();
occv->m_view->FitAll();
// occv->redraw();
// occv->m_view->Update();
}
    return Fl::run();
}