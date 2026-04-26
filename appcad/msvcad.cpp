#include "includes.hpp"
#include <BRepAlgoAPI_Splitter.hxx>
#include <AIS_Line.hxx>
#include <GC_MakeLine.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeSegment.hxx>
#include <Geom_Axis1Placement.hxx>
#include <AIS_Axis.hxx>
#include "fl_scintilla.hpp"
#include "fl_browser_msv.hpp"
#include "general.hpp"
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
		 parent=window();
        int bottom_y = parent->h() - fixed_height;
        
        // Only allow width to change, keep height fixed, and stick to bottom
        if (H != fixed_height || Y != bottom_y) {
            Fl_Window::resize(X, bottom_y, W, fixed_height);
        } else {
            Fl_Window::resize(X, Y, W, H);
        } 
        in_resize = false;
    }
};
class AIS_NonSelectableShape : public AIS_Shape {
   public:
	AIS_NonSelectableShape(const TopoDS_Shape& s) : AIS_Shape(s) { 
	}
};
class AIS_Axis_NoSelect : public AIS_Axis{
   	public:
    AIS_Axis_NoSelect(const gp_Ax1& ax, Standard_Real L)
        : AIS_Axis(ax, L)
    {
        SetAutoHilight(Standard_False);
        SetHilightMode(-1);
    }

	protected:
    // Prevent highlight presentation
    virtual void HilightOwner(
        const Handle(PrsMgr_PresentationManager)&,
        const Handle(Prs3d_Drawer)&,
        const Handle(SelectMgr_EntityOwner)&) 
    {
        // do nothing
    }

    // Prevent highlight computation
    virtual void ComputeHilightPresentation(
        const Handle(PrsMgr_PresentationManager)&,
        const Handle(Prs3d_Presentation)&,
        const Standard_Integer) 
    {
        // do nothing
    }

    // Prevent selection creation
    virtual void ComputeSelection(
        const Handle(SelectMgr_Selection)&,
        const Standard_Integer) override
    {
        // do nothing
    }
};

template <typename T>
struct ManagedPtrWrapper : public Standard_Transient {
	T* ptr;
	ManagedPtrWrapper(T* p) : ptr(p) {}
	~ManagedPtrWrapper() override {
		if(ptr)delete ptr; // delete when wrapper is destroyed
	}
};
void colorisebtn(int idx = -1);
// void inteligentmerge( TopoDS_Shape newshape);
void scaleball();
gp_Pnt vertexPnt; //highlight ball
struct luadraw;
luadraw* lua_detected(Handle(SelectMgr_EntityOwner) entOwner);
void inteligentSet(luadraw* current_part);

//region Global handles
Handle(Aspect_DisplayConnection) m_display_connection;
Handle(OpenGl_GraphicDriver)     m_graphic_driver;
Handle(V3d_Viewer)               m_viewer;
Handle(AIS_InteractiveContext)   ctx;
Handle(V3d_View)                 view;

gp_Vec end_proj_global;
gp_Vec end_up_global;
Handle(AIS_AnimationCamera) CurrentAnimation;

bool autorefined=0;

static auto last_event = std::chrono::steady_clock::now();
bool isloading=1;

Fl_Double_Window* win;
struct OCC_Viewer;
OCC_Viewer* occv;
Fl_Menu_Bar* menu;
Fl_Group* content; 
fl_browser_msv* fbm;
Fl_Help_View* helpv; 
FixedHeightWindow* woccbtn;

int lastX, lastY;
bool isRotatingz = false;
bool isRotating = false;
bool isPanning = false;
int mousex = 0;
int mousey = 0;
bool m_initialized = false;
bool hlr_on = false;
std::vector<TopoDS_Shape> vshapes;
std::vector<Handle(AIS_Shape)> vaShape;
Handle(AIS_NonSelectableShape) visible_;

lua_State* L;
static std::unique_ptr<sol::state> G;
auto& lua = *G;

extern string currfilename;
std::atomic<bool> isdebuging{0};
 
bool studyRotation=0;
gp_Pnt pickcenterforstudyrotation;
gp_Dir pickcenteraxisDir ; // circle normal
gp_Trsf initLoc;
Handle(AIS_Shape) study_trg;

struct cshapes{
	TopoDS_Shape s;
	int type=0;
};
// struct sketch_extrudes{
// 	string name;
// 	int len;
// };
// unordered_map<string,sketch_extrudes> uextrudes;
// std::unordered_map<TopoDS_Shape, std::string, ShapeHasher> shapeTags;

struct luadraw {
	TopLoc_Location Origin;
	TopLoc_Location Originl=TopLoc_Location();
	string  from_sketch="";
	float Extrude_val=0;
	int clone_qtd=0;
	string name = "";
	int currentline;
	bool visible_hardcoded = 1;
	vector<cshapes> cs;
	vector<TopoDS_Shape> vshape=vector<TopoDS_Shape>(2);
	TopoDS_Compound fshape;
	TopLoc_Location current_location=TopLoc_Location().Transformation();
	TopLoc_Location start_location=TopLoc_Location();
	Handle(AIS_Shape) acl;
	// TopoDS_Shape clocation;
	BRep_Builder builder;
	TopoDS_Compound cshape;
	TopoDS_Shape shape;
	Handle(AIS_Shape) ashape;
	vector<std::vector<gp_Vec2d>> vpoints;
	vector<float> extrudes; //for sketch parts
};
luadraw* current_part = nullptr;
vector<luadraw*> vlua;

float globalratio;
TopoDS_Shape sphereBase = BRepPrimAPI_MakeSphere(5).Shape();

//region helpers
// std::string lua_error_with_line(lua_State* L, const std::string& msg) {
//     lua_Debug ar;

//     // level 1 = caller of this function
//     if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "Sl", &ar)) {
//         std::ostringstream oss;
//         oss << msg;

//         const char* src = ar.short_src && ar.short_src[0] ? ar.short_src : ar.source;

//         if (ar.currentline > 0) {
//             oss << " (Lua: " << src << ":" << ar.currentline << ")";
//         } else {
//             oss << " (Lua: " << src << ")";
//         }

//         return oss.str();
//     }

//     return msg;
// }
TopoDS_Shape RebaseLocation(const TopoDS_Shape& s, const TopLoc_Location& newLoc)
{
    // 1. Compute global transform of the shape
    gp_Trsf global = s.Location().Transformation();

    // 2. Compute geometry-local transform relative to newLoc
    gp_Trsf local = newLoc.Transformation().Inverted() * global;

    // 3. Create a NEW shape with the same TShape but new location
    TopoDS_Shape out;
    out.TShape(s.TShape());       // share geometry
    out.Location(TopLoc_Location(local));

    return out;
}

void SetReferenceLocationWithoutMoving(TopoDS_Shape& s, const TopLoc_Location& newLoc)
{
    perf2();

    if (s.IsNull())
        return;

    const TopLoc_Location oldLoc = s.Location();

    // If locations are the same, nothing to do
    if (oldLoc.IsEqual(newLoc))
    {
        perf2("SetReferenceLocationWithoutMoving");
        return;
    }

    // 1. Compute the compensation transform so that geometry stays in the same place in global space
    // comp = oldLoc * newLoc.Inverted()   (or equivalently: newLoc.Inverted() * oldLoc, order depends on convention)
    gp_Trsf comp = oldLoc.Transformation() * newLoc.Transformation().Inverted();

    // 2. Create a new shape with the same TShape (EmptyCopied shares the underlying geometry)
    BRep_Builder B;
    TopoDS_Shape result = s.EmptyCopied();

    // 3. Re-add all direct children with the compensation applied to their locations
    for (TopoDS_Iterator it(s); it.More(); it.Next())
    {
        TopoDS_Shape sub = it.Value();           // this copies the handle + orientation + location
        sub.Move(comp);                          // applies compensation to sub's location only
        B.Add(result, sub);
    }

    // 4. Set the new reference location on the top-level shape
    result.Location(newLoc);

    // 5. Replace the original
    s = result;

    perf2("SetReferenceLocationWithoutMoving");
}
void SetReferenceLocationWithoutMoving2(TopoDS_Shape& s, const TopLoc_Location& newLoc)
{
	perf2();
    // 1. Calculate the relative compensation transform
    // comp = newLoc^-1 * oldLoc
    gp_Trsf comp = newLoc.Transformation().Inverted() * s.Location().Transformation();
    
    // 2. Prepare a builder and a new empty container of the same type
    BRep_Builder B;
    TopoDS_Shape result = s.EmptyCopied(); // Creates a new TShape of same type (Compound, Shell, etc.)
    
    // 3. Iterate through immediate children and apply the compensation
    // This is O(N) where N is the number of sub-shapes, not the whole geometry tree.
    for (TopoDS_Iterator it(s); it.More(); it.Next())
    {
        TopoDS_Shape sub = it.Value();
        sub.Move(comp); // This is a simple matrix multiplication on the sub-shape's location
        B.Add(result, sub);
    }

    // 4. Set the new top-level reference location
    result.Location(newLoc);

    // 5. Update the original reference
    s = result;
	perf2("SetReferenceLocationWithoutMoving");
}
void SetReferenceLocationWithoutMoving1(TopoDS_Shape& s, const TopLoc_Location& newLoc)
{
	perf2();
    // 1. Current global transform
    TopLoc_Location oldLoc = s.Location();
    gp_Trsf oldTrsf = oldLoc.Transformation();
    gp_Trsf newTrsf = newLoc.Transformation();

    // 2. Compensation transform
    gp_Trsf comp = newTrsf.Inverted() * oldTrsf;

    // 3. Apply compensation to the underlying geometry (copy = false)
    BRepBuilderAPI_Transform tr(s, comp, false);
    TopoDS_Shape newShape = tr.Shape();

    // 4. Apply the new reference location
    newShape.Location(newLoc);

    s = newShape;
	perf2("SetReferenceLocationWithoutMoving");
}



#include <AIS_Trihedron.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <TopLoc_Location.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

void DrawTrihedron2(
    const Handle(AIS_InteractiveContext)& ctx,
    const TopLoc_Location& loc,
    Standard_Real scale)
{
    const Standard_Real axisLen = scale;
    const Standard_Real radius  = 0.02 * scale;
    const Standard_Real coneLen = 0.15 * scale;

    BRep_Builder b;
    TopoDS_Compound comp;
    b.MakeCompound(comp);
// X axis
    TopoDS_Shape xCyl = BRepPrimAPI_MakeCylinder(radius, axisLen).Shape();
    TopoDS_Shape xCone = BRepPrimAPI_MakeCone(radius*1.5, 0, coneLen).Shape();
    {
        gp_Trsf tr;
        tr.SetTranslation(gp_Vec(axisLen, 0, 0));
        xCone.Move(tr);
    }
    b.Add(comp, xCyl);
    b.Add(comp, xCone);

    // Y axis
    TopoDS_Shape yCyl = BRepPrimAPI_MakeCylinder(radius, axisLen).Shape();
    {
        gp_Trsf tr;
        tr.SetRotation(gp::OX(), M_PI/2);
        yCyl.Move(tr);
    }
    TopoDS_Shape yCone = BRepPrimAPI_MakeCone(radius*1.5, 0, coneLen).Shape();
    {
        gp_Trsf tr;
        tr.SetRotation(gp::OX(), M_PI/2);
        tr.SetTranslation(gp_Vec(0, axisLen, 0));
        yCone.Move(tr);
    }
    b.Add(comp, yCyl);
    b.Add(comp, yCone);

    // Z axis
    TopoDS_Shape zCyl = BRepPrimAPI_MakeCylinder(radius, axisLen).Shape();
    {
        gp_Trsf tr;
        tr.SetRotation(gp::OY(), -M_PI/2);
        zCyl.Move(tr);
    }
    TopoDS_Shape zCone = BRepPrimAPI_MakeCone(radius*1.5, 0, coneLen).Shape();
    {
        gp_Trsf tr;
        tr.SetRotation(gp::OY(), -M_PI/2);
        tr.SetTranslation(gp_Vec(0, 0, axisLen));
        zCone.Move(tr);
    }
    b.Add(comp, zCyl);
    b.Add(comp, zCone);

    // Apply location
    TopoDS_Shape located = comp.Moved(loc.Transformation());

    // Wrap in non-selectable AIS_Shape
    Handle(AIS_NonSelectableShape) tri = new AIS_NonSelectableShape(located);

    	tri->SetZLayer(Graphic3d_ZLayerId_Topmost);
ctx->Display(tri, 1);
ctx->Deactivate(tri);
}

void DrawTrihedron6(const Handle(AIS_InteractiveContext)& ctx,
                   const TopLoc_Location& loc,
                   Standard_Real scale)
{
    static Handle(AIS_Axis) aisAxis;
	if(ctx->IsDisplayed(aisAxis)){
		ctx->Remove(aisAxis, Standard_True);
	}
    // if (init) return;
    // init = true;

    const Standard_Real L = scale * 50;  // desired axis length

    // Origin in local coordinates
    gp_Pnt origin(0.0, 0.0, 0.0);

    auto MakeAxis = [&](const gp_Dir& direction, Quantity_NameOfColor col)
    {
        // Create axis starting at origin, going in positive direction
        gp_Ax1 ax1(origin, direction);

        // Create finite ray with exact length L
        aisAxis = new AIS_Axis(ax1, L);
        // Handle(AIS_Axis) aisAxis = new AIS_Axis(ax1, L);

        // Visual style
        aisAxis->SetColor(col);
        aisAxis->SetWidth(3.0);                    // thickness
        aisAxis->SetDisplayAspect(new Prs3d_LineAspect(col, Aspect_TOL_SOLID, 3.0));

        // Apply your location (rotation + translation + possible scale)
        aisAxis->SetLocalTransformation(loc.Transformation());

        // Draw on top and make non-selectable to avoid interfering with shape vertices
        aisAxis->SetZLayer(Graphic3d_ZLayerId_Topmost);
        ctx->Display(aisAxis, Standard_False);
        ctx->Deactivate(aisAxis);
    };

    MakeAxis(gp::DX(), Quantity_NOC_RED);     // X axis
    MakeAxis(gp::DY(), Quantity_NOC_GREEN);   // Y axis
    MakeAxis(gp::DZ(), Quantity_NOC_BLUE1);   // Z axis

    ctx->UpdateCurrentViewer();
}
void DrawTrihedron(const Handle(AIS_InteractiveContext)& ctx,
                   TopLoc_Location _loc,
                   Standard_Real scale){
    static Handle(AIS_Axis_NoSelect) axes[3];

	static TopLoc_Location loc=_loc;
	if(_loc!=TopLoc_Location()){
		loc=_loc;
	}


    for (int i = 0; i < 3; ++i)
    {
        if (!axes[i].IsNull())
        {
            ctx->Remove(axes[i], Standard_False);
            axes[i].Nullify();
        }
    }

    const Standard_Real L = scale * 50.0;
    gp_Pnt origin(0,0,0);

	auto CreateAxis = [&](int index, const gp_Dir& direction, Quantity_NameOfColor col)
	{
		gp_Ax1 ax1(origin, direction);
		axes[index] = new AIS_Axis_NoSelect(ax1, L);

		axes[index]->SetColor(col);
		axes[index]->SetWidth(3.0);
		axes[index]->SetLocalTransformation(loc.Transformation());
		axes[index]->SetZLayer(Graphic3d_ZLayerId_Top);

		// Disable highlight
		axes[index]->SetAutoHilight(Standard_False);
		axes[index]->SetHilightMode(-1);

		// Display with NO selection mode
		ctx->Display(axes[index], AIS_Shaded, -1, Standard_False);

		// Extra safety: deactivate any selection
		ctx->Deactivate(axes[index]);

		axes[index]->SetInfiniteState(Standard_True);
	};


    CreateAxis(0, gp::DX(), Quantity_NOC_RED);
    CreateAxis(1, gp::DY(), Quantity_NOC_GREEN);
    CreateAxis(2, gp::DZ(), Quantity_NOC_BLUE1);

    ctx->UpdateCurrentViewer();
}
	vfloat GetViewportAspectRatio() {
		const Handle(Graphic3d_Camera) & camera = view->Camera();

		// Obtém a altura e largura do mundo visível na viewport
		float viewportHeight = camera->ViewDimensions().Y();	  // world
		float viewportWidth = camera->Aspect() * viewportHeight;  // Largura = ratio * altura

		Standard_Integer winWidth, winHeight;
		view->Window()->Size(winWidth, winHeight);

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
void scaleaxis(const Handle(AIS_InteractiveContext)& ctx)
{
    Standard_Real globalratio = GetViewportAspectRatio()[0];
    if (globalratio > 8) globalratio = 8;

    DrawTrihedron(ctx,TopLoc_Location(), globalratio);
}

void DrawTrihedron7(const Handle(AIS_InteractiveContext)& ctx,
                   const TopLoc_Location& loc,
                   Standard_Real scale)
{
    // We need to track all three axes to remove them properly later
    static Handle(AIS_Axis) axes[3];

    // 1. Remove existing axes if they are already displayed
    for (int i = 0; i < 3; ++i) {
        if (!axes[i].IsNull() && ctx->IsDisplayed(axes[i])) {
            ctx->Remove(axes[i], Standard_False); // False = don't update viewer yet
        }
    }

    const Standard_Real L = scale * 50.0;
    gp_Pnt origin(0.0, 0.0, 0.0);

    // Helper to create and configure each axis
    auto CreateAxis = [&](int index, const gp_Dir& direction, Quantity_NameOfColor col)
    {
        gp_Ax1 ax1(origin, direction);
        axes[index] = new AIS_Axis(ax1, L);

        // Visual Style
        axes[index]->SetColor(col);
        axes[index]->SetWidth(3.0);
        
        // Apply transformation
        axes[index]->SetLocalTransformation(loc.Transformation());

        // Push to topmost layer so it's not obscured by geometry
        axes[index]->SetZLayer(Graphic3d_ZLayerId_Topmost);

        // 1. Disable Auto-Highlighting (prevents color change on hover)
    axes[index]->SetAutoHilight(Standard_False);

    // 2. Display the object
    ctx->Display(axes[index], Standard_False);

    // 3. Completely disable all selection modes for this object
    ctx->Deactivate(axes[index]);
    
    // 4. Force the selection manager to ignore this object (no detection)
    ctx->SetSelectionModeActive(axes[index], -1, Standard_False, AIS_SelectionModesConcurrency_Single);
        
        // Optional: Ensure it doesn't participate in bounding box zoom
        axes[index]->SetInfiniteState(true);
    };

    CreateAxis(0, gp::DX(), Quantity_NOC_RED);   // X
    CreateAxis(1, gp::DY(), Quantity_NOC_GREEN); // Y
    CreateAxis(2, gp::DZ(), Quantity_NOC_BLUE1); // Z

    ctx->UpdateCurrentViewer();
}


void DrawTrihedron4( const Handle(AIS_InteractiveContext)& ctx, const TopLoc_Location& loc, Standard_Real scale){
    static bool init = false;
    if (init) return;
    init = true;

    const Standard_Real L = scale * 0.01;

    gp_Pnt O(0,0,0), X(L,0,0), Y(0,L,0), Z(0,0,L);

    O.Transform(loc.Transformation());
    X.Transform(loc.Transformation());
    Y.Transform(loc.Transformation());
    Z.Transform(loc.Transformation());

    auto MakeLine = [&](const gp_Pnt& a, const gp_Pnt& b, Quantity_NameOfColor col)
    {
        Handle(Geom_Line) gline = GC_MakeLine(a, b).Value();
        Handle(AIS_Line) ln = new AIS_Line(gline);

        ln->Attributes()->SetLineAspect(
            new Prs3d_LineAspect(col, Aspect_TOL_SOLID, 3.0));

        ln->SetZLayer(Graphic3d_ZLayerId_Topmost);

        ctx->Display(ln, Standard_False);
        ctx->Deactivate(ln); // non selectable
    };

    MakeLine(O, X, Quantity_NOC_RED);
    MakeLine(O, Y, Quantity_NOC_GREEN);
    MakeLine(O, Z, Quantity_NOC_BLUE1);

    ctx->UpdateCurrentViewer();
}
void DrawTrihedron3(
    const Handle(AIS_InteractiveContext)& ctx,
    const TopLoc_Location& loc,
    Standard_Real scale)
{
// return;
	static bool init=0;
	if(init)return;
	init=1;

    gp_Ax2 ax2(gp::Origin(), gp::DZ(), gp::DX());
    ax2.Transform(loc.Transformation());

    Handle(Geom_Axis2Placement) gax2 = new Geom_Axis2Placement(ax2);
    Handle(AIS_Trihedron) tri = new AIS_Trihedron(gax2);

	Handle(Prs3d_DatumAspect) da = new Prs3d_DatumAspect();
    tri->Attributes()->SetDatumAspect(da);
    // Handle(Prs3d_DatumAspect) da = tri->Attributes()->DatumAspect();
 
    // Set axis lengths
	const float factor=35;
    da->SetAxisLength(scale*factor, scale*factor, scale*factor);
 
	da->SetDrawLabels(0);
    // Arrow size
    // da->SetArrowLength(0.15 * scale);

    // Modify line widths (modern API)
    da->LineAspect(Prs3d_DatumParts_XAxis)->SetWidth(3.0);
    da->LineAspect(Prs3d_DatumParts_YAxis)->SetWidth(3.0);
    da->LineAspect(Prs3d_DatumParts_ZAxis)->SetWidth(3.0);

    // Optional: axis colors
    da->LineAspect(Prs3d_DatumParts_XAxis)->SetColor(Quantity_NOC_RED);
    da->LineAspect(Prs3d_DatumParts_YAxis)->SetColor(Quantity_NOC_GREEN);
    da->LineAspect(Prs3d_DatumParts_ZAxis)->SetColor(Quantity_NOC_BLUE1);
 
	tri->SetZLayer(Graphic3d_ZLayerId_Topmost);
// ctx->Display(tri, Standard_False);

// ctx->Deactivate(tri);     // remove all
// ctx->Activate(tri,1);
// ctx->Activate(tri,2);
// ctx->Activate(tri,3);
ctx->Display(tri, Standard_False);
ctx->EraseSelected(Standard_False);
ctx->Deactivate(tri);
tri->SetAutoHilight(Standard_False);
ctx->Redisplay(tri, Standard_True);
 
}

TopoDS_Solid FindSolidOfFace(const TopoDS_Shape& compound, const TopoDS_Face& face)
{
    for (TopExp_Explorer exp(compound, TopAbs_SOLID); exp.More(); exp.Next()) {
        TopoDS_Shape solid = exp.Current();

        // Map all faces of this solid
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(solid, TopAbs_FACE, faceMap);

        // Check if the detected face is inside this solid
        if (faceMap.Contains(face)) {
            return TopoDS::Solid(solid);
        }
    }

    return TopoDS_Solid(); // not found
}


// TopoDS_Shape FindSolidOf(const TopoDS_Shape& root, const TopoDS_Shape& sub)
// {
//     for (TopExp_Explorer exp(root, TopAbs_SOLID); exp.More(); exp.Next()) {
//         const TopoDS_Shape& solid = exp.Current();
//         if (sub.IsSame(solid))
//             return solid;

//         // Verifica se o sub-shape pertence ao sólido
//         if (BRepTools::IsSubShape(sub, solid))
//             return solid;
//     }
//     return TopoDS_Shape(); // vazio
// }

// Requires OpenCascade headers and linking
// Requires OpenCascade headers and linking
// Requires OpenCascade headers and linking
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>

#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepAdaptor_Surface.hxx>

#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <GeomLProp_SLProps.hxx>

#include <GProp_GProps.hxx>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>

#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>

#include <vector>
#include <optional>
#include <utility>
#include <cmath>
#include <algorithm>

// ------------------------------------------------------------
// Get displayed solids once
// ------------------------------------------------------------
static std::vector<TopoDS_Shape>
GetDisplayedSolids2(const Handle(AIS_InteractiveContext)& ctx)
{
    std::vector<TopoDS_Shape> solids;

    AIS_ListOfInteractive list;
    ctx->DisplayedObjects(list);

    for (AIS_ListIteratorOfListOfInteractive it(list); it.More(); it.Next())
    {
        Handle(AIS_Shape) aisShape =
            Handle(AIS_Shape)::DownCast(it.Value());

        if (aisShape.IsNull())
            continue;

        const TopoDS_Shape& shape = aisShape->Shape();

        for (TopExp_Explorer ex(shape, TopAbs_SOLID); ex.More(); ex.Next())
            solids.push_back(ex.Current());
    }

    return solids;
}
vector<string> solidsname;
static std::vector<TopoDS_Shape>
GetDisplayedSolids(const Handle(AIS_InteractiveContext)& ctx)
{
    std::vector<TopoDS_Shape> solids;
	solidsname.clear();

    AIS_ListOfInteractive list;
    ctx->DisplayedObjects(list);

    for (AIS_ListIteratorOfListOfInteractive it(list); it.More(); it.Next())
    {
        Handle(AIS_Shape) aisShape =
            Handle(AIS_Shape)::DownCast(it.Value());

        if (aisShape.IsNull())
            continue;
		
		string name="";
Handle(AIS_InteractiveObject) ao = Handle(AIS_InteractiveObject)::DownCast(it.Value());
if (!ao.IsNull() && ao->HasOwner()) {
    Handle(Standard_Transient) owner = ao->GetOwner();
    Handle(ManagedPtrWrapper<luadraw>) w =
        Handle(ManagedPtrWrapper<luadraw>)::DownCast(owner);

    if (!w.IsNull()) {
        luadraw* ptr = w->ptr;
		name=(ptr->name);
        // now you have your pointer
    }
}


        const TopoDS_Shape& shape = aisShape->Shape();

        for (TopExp_Explorer ex(shape, TopAbs_SOLID); ex.More(); ex.Next()){
            solids.push_back(ex.Current());
			solidsname.push_back(name);
		}
    }

    return solids;
}
// ------------------------------------------------------------
// Metrics
// ------------------------------------------------------------
// FIXED VERSION:
// Rotated solids with TopLoc_Location now work.
//
// Main issue:
// FaceArea / FacePerimeter are invariant, but FacePlaneWorld() and
// vertex extraction were mixing local/world coordinates.
//
// This version forces WORLD transforms using Located().

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

#include <TopExp_Explorer.hxx>

#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepAdaptor_Surface.hxx>

#include <GeomAbs_SurfaceType.hxx>

#include <GProp_GProps.hxx>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>

#include <TopLoc_Location.hxx>

#include <vector>
#include <optional>
#include <utility>
#include <cmath>
#include <algorithm>
static TopoDS_Edge EdgeToWorld(const TopoDS_Edge& e)
{
    if (e.Location().IsIdentity())
        return e;

    return TopoDS::Edge(e.Located(e.Location()));
}

// ------------------------------------------------------------
// metrics
// ------------------------------------------------------------
// FIXED:
//
// Previous version inspected ALL edges of the solid, including edges that
// belong to the matched face itself.
//
// You want:
// Largest edge perpendicular to the matched face,
// BUT NOT an edge that belongs to that face.
//
// So now:
// 1. Match face by area/perimeter
// 2. Collect all edges of matched face
// 3. Scan solid edges
// 4. Ignore edges belonging to matched face
// 5. Keep largest straight edge perpendicular to face normal

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <TopAbs.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>

#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepGProp.hxx>

#include <GeomAbs_SurfaceType.hxx>
#include <GeomAbs_CurveType.hxx>

#include <GProp_GProps.hxx>

#include <gp_Pln.hxx>
#include <gp_Vec.hxx>
#include <gp_Pnt.hxx>

#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>

// ------------------------------------------------------------
// metrics
// ------------------------------------------------------------
static double FaceArea(const TopoDS_Face& f)
{
    GProp_GProps p;
    BRepGProp::SurfaceProperties(f, p);
    return p.Mass();
}

static double FacePerimeter(const TopoDS_Face& f)
{
    double total = 0.0;

    for (TopExp_Explorer ex(f, TopAbs_EDGE); ex.More(); ex.Next())
    {
        GProp_GProps p;
        BRepGProp::LinearProperties(ex.Current(), p);
        total += p.Mass();
    }

    return total;
}

// ------------------------------------------------------------
// plane normal
// ------------------------------------------------------------
static bool FaceNormal(const TopoDS_Face& f, gp_Vec& n)
{
    BRepAdaptor_Surface surf(f, Standard_True);

    if (surf.GetType() != GeomAbs_Plane)
        return false;

    gp_Pln pln = surf.Plane();

    n = gp_Vec(pln.Axis().Direction());

    if (f.Orientation() == TopAbs_REVERSED)
        n.Reverse();

    if (n.Magnitude() < 1e-12)
        return false;

    n.Normalize();
    return true;
}

// ------------------------------------------------------------
// collect face edges
// ------------------------------------------------------------
static TopTools_IndexedMapOfShape GetFaceEdges(const TopoDS_Face& face)
{
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(face, TopAbs_EDGE, map);
    return map;
}

// ------------------------------------------------------------
// largest perpendicular edge NOT in face
// ------------------------------------------------------------
static double LargestPerpendicularSideEdge(
    const TopoDS_Shape& solid,
    const TopoDS_Face& face,
    const gp_Vec& normal,
    double tol = 0.15)
{
    double best = 0.0;

    TopTools_IndexedMapOfShape faceEdges = GetFaceEdges(face);

    for (TopExp_Explorer ex(solid, TopAbs_EDGE); ex.More(); ex.Next())
    {
        TopoDS_Edge e = TopoDS::Edge(ex.Current());

        // Ignore edges from matched face
        if (faceEdges.Contains(e))
            continue;

        BRepAdaptor_Curve c(e);

        if (c.GetType() != GeomAbs_Line)
            continue;

        double f = c.FirstParameter();
        double l = c.LastParameter();

        gp_Pnt p1 = c.Value(f);
        gp_Pnt p2 = c.Value(l);

        gp_Vec v(p1, p2);

        double len = v.Magnitude();

        if (len < 1e-9)
            continue;

        v.Normalize();

        // perpendicular to face => dot near ±1? NO
        // Wait:
        // extrusion edge is parallel to normal
        double d = std::abs(v.Dot(normal));

        // must be parallel to face normal
        if (d >= (1.0 - tol))
            best = std::max(best, len);
    }

    return best;
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
struct EntryAgg {
	std::string first_name;
	int total_qtty = 0;
};

std::unordered_map<float, EntryAgg> agg;
				
// static std::vector<std::pair<TopoDS_Shape,double>>
static void CountAndMeasureFromRefFace_PlaneFilter(
    const TopoDS_Face& refFace,
    const std::vector<TopoDS_Shape>& solids,
    double tolAreaRel  = 0.08,
    double tolPerimRel = 0.08)
{
    // std::vector<std::pair<TopoDS_Shape,double>> out;
	agg.clear();
    if (refFace.IsNull())
        return;

    double refA = FaceArea(refFace);
    double refP = FacePerimeter(refFace);

    for (int i=0;i<solids.size();i++)
    // for (const TopoDS_Shape& solid : solids)
    {
		const TopoDS_Shape& solid=solids[i];
        bool matched = false;
        double extrusion = 0.0;

        for (TopExp_Explorer ex(solid, TopAbs_FACE); ex.More(); ex.Next())
        {
            TopoDS_Face f = TopoDS::Face(ex.Current());

            double a = FaceArea(f);
            double p = FacePerimeter(f);

            if (a <= 0.0 || p <= 0.0)
                continue;

            if (std::abs(a - refA) / std::max(a, refA) > tolAreaRel)
                continue;

            if (std::abs(p - refP) / std::max(p, refP) > tolPerimRel)
                continue;

            gp_Vec n;
            if (!FaceNormal(f, n))
                continue;

            extrusion =
                LargestPerpendicularSideEdge(
                    solid,
                    f,
                    n);

            matched = true;
            break;
        }

        if (matched){
			auto& slot = agg[extrusion];
					// cotm2(o->from_sketch, o->name)
					if (slot.total_qtty == 0) {
						slot.first_name = solidsname[i];
					}
					slot.total_qtty += 1;
			// agg[extrusion]={solidsname[i],}
            // out.emplace_back(solid, extrusion);
		}
    }

    // return out;
}
inline void AddToCompound(TopoDS_Compound& compound, const TopoDS_Shape& shape) {
    // BRep_Builder builder;
	// static thread_local BRep_Builder builder;
    // builder.Add(compound, shape);
    current_part->builder.Add(compound, shape);
}
void inteligentmerge(TopoDS_Shape newshape, bool setlocation=1){//,int resetlocation=0) { 
	perf2();
	cotm("merge");
    if (newshape.IsNull()) return; 
    bool previous_solid = (!current_part->shape.IsNull() && current_part->shape.ShapeType() == TopAbs_SOLID);
    bool previous_compound = (!current_part->shape.IsNull() && current_part->shape.ShapeType() == TopAbs_COMPOUND); 
    // bool nsis_solid = (newshape.ShapeType() == TopAbs_SOLID); 
	bool isok=!current_part->shape.IsNull();
	auto shapetype=newshape.ShapeType();

	// if(current_part->Originl.IsIdentity())
	if(setlocation) newshape.Location(current_part->Originl);

    // if (previous_solid || previous_compound) 
    // if (previous_compound  ) 
	// if(isok && (shapetype==TopAbs_SOLID || shapetype==TopAbs_FACE ))
	if(isok && ( (shapetype==TopAbs_SOLID || shapetype==TopAbs_FACE ) ||previous_compound  ))
	{
		AddToCompound(current_part->cshape,current_part->shape);
    } 
    current_part->shape = newshape;
    perf2("imerge");
}
//region work
std::vector<TopoDS_Shape> GetDisplayedSolids1(
    const Handle(AIS_InteractiveContext)& ctx)
{
    std::vector<TopoDS_Shape> solids;

    AIS_ListOfInteractive list;
    ctx->DisplayedObjects(list);

    for (AIS_ListIteratorOfListOfInteractive it(list); it.More(); it.Next())
    {
        Handle(AIS_InteractiveObject) obj = it.Value();

        // Only AIS_Shape contains TopoDS_Shape
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        if (aisShape.IsNull())
            continue;

        const TopoDS_Shape& shape = aisShape->Shape();

        // Extract solids from the shape
        for (TopExp_Explorer ex(shape, TopAbs_SOLID); ex.More(); ex.Next())
        {
            solids.push_back(ex.Current());
        }
    }

    return solids;
}


gp_Trsf GetBakedLocation(const TopoDS_Shape& s)
{
    gp_Trsf trsf;

    // Accumulate all nested locations
    TopLoc_Location loc = s.Location();
    while (!loc.IsIdentity())
    {
        trsf = loc.Transformation() * trsf;
        loc = loc.NextLocation();
    }

    return trsf;
}
TopoDS_Compound KeepOnlySolids(const TopoDS_Compound& input)
{
    // Collect solids
    TopTools_ListOfShape solids;
    for (TopExp_Explorer ex(input, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }

    // If no solids, return empty shape
    if (solids.IsEmpty()) {
        return TopoDS_Compound();
    }

    // If exactly one solid, return it directly
    // if (solids.Extent() == 1) {
    //     return solids.First();
    // }

    // If multiple solids, pack them into a new compound
    BRep_Builder B;
    TopoDS_Compound comp;
    B.MakeCompound(comp);

    for (TopTools_ListIteratorOfListOfShape it(solids); it.More(); it.Next()) {
        B.Add(comp, it.Value());
    }

    return comp;
}

	void setbar5per() {
		Standard_Integer width, height;
		view->Window()->Size(width, height);
		Standard_Real barWidth = width * 0.05;

		static Handle(Graphic3d_Structure) barStruct;
		if (!barStruct.IsNull()) {
			barStruct->Erase();
			barStruct->Clear();
		} else {
			barStruct = new Graphic3d_Structure(view->Viewer()->StructureManager());
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
	}

#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <gp_Ax2.hxx>
#include <BOPAlgo_PaveFiller.hxx>

void locationsphereslow(luadraw* ld)
{
	// return;
    if (!ld) return;

    gp_Trsf trsf = ld->current_location.Transformation();
    gp_Pnt origin = trsf.TranslationPart();

    // Create a small red sphere at the location
    TopoDS_Shape sphereShape = BRepPrimAPI_MakeSphere(origin, 5.0).Shape();
    Handle(AIS_Shape) sphereAIS = new AIS_Shape(sphereShape);
    sphereAIS->SetColor(Quantity_Color(Quantity_NOC_RED));
    sphereAIS->SetDisplayMode(AIS_Shaded);
    ctx->Display(sphereAIS, 0);

    // Build the coordinate system (trihedron)
    gp_Dir dirZ = gp_Dir(0, 0, 1);
    gp_Dir dirX = gp_Dir(1, 0, 0);

    // Transform the axes with the same transformation
    dirZ.Transform(trsf);
    dirX.Transform(trsf);

    gp_Ax2 ax2(origin, dirZ, dirX);
    Handle(Geom_Axis2Placement) axis = new Geom_Axis2Placement(ax2);
    Handle(AIS_Trihedron) trihedron = new AIS_Trihedron(axis);

    trihedron->SetSize(20.0); // adjust to look “nice” relative to your scene
    trihedron->SetDatumDisplayMode(Prs3d_DM_WireFrame);
    trihedron->Attributes()->DatumAspect()->SetAxisLength(15, 15, 15);
    // trihedron->Attributes()->DatumAspect()->SetShadingAspect(
    //     new Prs3d_ShadingAspect());
    trihedron->SetColor(Quantity_Color(Quantity_NOC_SKYBLUE));

    ctx->Display(trihedron, 0);
    // ctx->UpdateCurrentViewer();

    // Store handles if needed
    ld->acl = sphereAIS;
    // ld->clocation = sphereShape;
}

TopoDS_Shape sphereShapeg;

void locationsphere(luadraw* ld)
{
    if (!ld) return;

    // Extract the current transformation
    gp_Trsf trsf = ld->current_location.Transformation();
    TopLoc_Location loc(trsf);

    // Create base sphere only once (centered at origin)
    if (sphereShapeg.IsNull()) {
        sphereShapeg = BRepPrimAPI_MakeSphere(5.0).Shape();
    }

    // Apply the location transform
    TopoDS_Shape sphereShape = sphereShapeg;
    sphereShape.Location(loc);

    // Display
    Handle(AIS_NonSelectableShape) sphereAIS = new AIS_NonSelectableShape(sphereShape);
    sphereAIS->SetColor(Quantity_Color(Quantity_NOC_BLUE));
    sphereAIS->SetDisplayMode(AIS_Shaded);

    ctx->Display(sphereAIS, 0);

    // Store references
    ld->acl = sphereAIS;
    // ld->clocation = sphereShape;
}


void redisplay(luadraw* ld) { 
	if (ld->visible_hardcoded) {
		if (ctx->IsDisplayed(ld->ashape)) {
			ctx->Redisplay(ld->ashape, false);
			ctx->Redisplay(ld->acl, false);
		} else {
			ctx->Display(ld->ashape, false);
			ctx->Display(ld->acl, false);
		}
	} else {
		// Only erase if it is displayed
		if (ctx->IsDisplayed(ld->ashape)) {
			ctx->Erase(ld->ashape, Standard_False);
			ctx->Erase(ld->acl, Standard_False);
		}
	}
}
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

inline void lua_error_with_where(const char* msg) {
    luaL_where(L, 1);
    std::string where = lua_tostring(L, -1);
    lua_pop(L, 1);
    throw sol::error(where + msg);
}

#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>

void ShowTrihedronAtLocation(const Handle(AIS_InteractiveContext)& ctx, const TopLoc_Location& loc,double size=50)
{
gp_Ax2 ax2;
ax2.Transform(loc.Transformation());

    gp_Pnt origin = ax2.Location();

    gp_Dir xDir = ax2.XDirection();
    gp_Dir yDir = ax2.YDirection();
    gp_Dir zDir = ax2.Direction();

    gp_Pnt px = origin.Translated(gp_Vec(xDir) * size);
    gp_Pnt py = origin.Translated(gp_Vec(yDir) * size);
    gp_Pnt pz = origin.Translated(gp_Vec(zDir) * size);

    // Create edges
    TopoDS_Edge ex = BRepBuilderAPI_MakeEdge(origin, px);
    TopoDS_Edge ey = BRepBuilderAPI_MakeEdge(origin, py);
    TopoDS_Edge ez = BRepBuilderAPI_MakeEdge(origin, pz);

    Handle(AIS_Shape) aisX = new AIS_Shape(ex);
    Handle(AIS_Shape) aisY = new AIS_Shape(ey);
    Handle(AIS_Shape) aisZ = new AIS_Shape(ez);

    aisX->SetColor(Quantity_NOC_RED);
    aisY->SetColor(Quantity_NOC_GREEN);
    aisZ->SetColor(Quantity_NOC_BLUE);

    ctx->Display(aisX, Standard_False);
    ctx->Display(aisY, Standard_False);
    ctx->Display(aisZ, Standard_True);
}
#include <TopLoc_Location.hxx>
#include <gp_Trsf.hxx>
#include <gp_Mat.hxx>
#include <gp_XYZ.hxx>
#include <cmath>
#include <iostream>

static double RadToDeg(double r) {
    return r * 180.0 / M_PI;
}

void PrintLocationDegrees(const TopLoc_Location& loc)
{
    gp_Trsf tr = loc.Transformation();

    gp_XYZ t = tr.TranslationPart();
    gp_Mat r = tr.VectorialPart();

    double sy = std::sqrt(r.Value(1,1)*r.Value(1,1) + r.Value(2,1)*r.Value(2,1));
    bool singular = sy < 1e-6;

    double x, y, z;

    if (!singular) {
        x = std::atan2(r.Value(3,2), r.Value(3,3));
        y = std::atan2(-r.Value(3,1), sy);
        z = std::atan2(r.Value(2,1), r.Value(1,1));
    } else {
        x = std::atan2(-r.Value(2,3), r.Value(2,2));
        y = std::atan2(-r.Value(3,1), sy);
        z = 0;
    }

    std::cout << "Translation (mm): "
              << t.X() << ", " << t.Y() << ", " << t.Z() << "\n";

    std::cout << "Angles (deg): "
              << RadToDeg(x) << ", "
              << RadToDeg(y) << ", "
              << RadToDeg(z) << "\n";
}
TopLoc_Location getShapePlacement(const TopoDS_Shape& s)
{
    if (s.IsNull())
        return TopLoc_Location();

    TopLoc_Location loc = s.Location();
    TopLoc_Location acc = loc;

    // Accumulate nested locations (assemblies, instances, etc.)
    while (!loc.NextLocation().IsIdentity())
    {
        loc = loc.NextLocation();
        acc = acc * loc;
    }

    return acc;
}

TopoDS_Shape resetShapePlacement(const TopoDS_Shape& s) {
	if (s.IsNull()) return s;

	TopoDS_Shape working = s;
	working.Location(TopLoc_Location());  // clear top-level

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
void ConvertVec2dToPnt2d(const std::vector<gp_Vec2d>& vpoints, std::vector<gp_Pnt2d>& ppoints) {
	ppoints.clear();
	ppoints.reserve(vpoints.size());
	for (const auto& vec : vpoints) {
		ppoints.emplace_back(vec.X(), vec.Y());
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


TopoDS_Compound CleanCompound_RemoveWiresFacesEdgesBeforeSolid(const TopoDS_Compound& src)
{
    // 1. Verificar se existe pelo menos um sólido no compound
    bool hasSolid = false;
    for (TopoDS_Iterator itCheck(src); itCheck.More(); itCheck.Next()) {
        if (itCheck.Value().ShapeType() == TopAbs_SOLID) {
            hasSolid = true;
            break;
        }
    }

    // Se não houver sólido, retornamos o original intacto
    if (!hasSolid) {
        return src;
    }

    // 2. Se houver sólido, filtramos Edges, Wires e Faces antes do primeiro sólido
    BRep_Builder B;
    TopoDS_Compound out;
    B.MakeCompound(out);

    bool solidFound = false;

    for (TopoDS_Iterator it(src); it.More(); it.Next())
    {
        const TopoDS_Shape& s = it.Value();
        const TopAbs_ShapeEnum t = s.ShapeType();

        if (!solidFound && t == TopAbs_SOLID) {
            solidFound = true;
        }

        if (!solidFound)
        {
            // Removemos EDGE, WIRE e FACE antes de encontrar o sólido
            if (t != TopAbs_EDGE && t != TopAbs_WIRE && t != TopAbs_FACE) {
                B.Add(out, s);
            }
        }
        else
        {
            // Após o primeiro sólido, aceitamos tudo
            B.Add(out, s);
        }
    }

    return out;
}

std::vector<TopoDS_Solid> ExtractSolids(const TopoDS_Shape& shape) {
    std::vector<TopoDS_Solid> solids;
    for (TopExp_Explorer exp(shape, TopAbs_SOLID); exp.More(); exp.Next()) {
        solids.push_back(TopoDS::Solid(exp.Current()));
    }
    return solids;
}

std::vector<TopoDS_Face> ExtractFaces(const TopoDS_Shape& shape) {
    std::vector<TopoDS_Face> solids;
    for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
        solids.push_back(TopoDS::Face(exp.Current()));
    }
    return solids;
}
//region temp
std::vector<TopoDS_Face> PutFacesInSameDomain(const std::vector<TopoDS_Face>& faces)
{
    std::vector<TopoDS_Face> result;
    result.reserve(faces.size());

    if (faces.empty())
        return result;

    // Reference plane in GLOBAL coordinates
    TopLoc_Location refLoc;
    Handle(Geom_Surface) refSurf = BRep_Tool::Surface(faces[0], refLoc);
    Handle(Geom_Plane) refPlane = Handle(Geom_Plane)::DownCast(refSurf);

    if (refPlane.IsNull()) {
        std::cerr << "PutFacesInSameDomain: reference face is not planar\n";
        return faces;
    }

    gp_Pln pln = refPlane->Pln();
    pln.Transform(refLoc.Transformation()); // true global placement

    Handle(Geom_Plane) sharedPlane = new Geom_Plane(pln);

    for (const TopoDS_Face& face : faces)
    {
        std::vector<TopoDS_Wire> wires;

        // IMPORTANT FIX:
        // Explorer already returns subshapes with accumulated location.
        // Applying face.Location() again was double-transforming .Moved() faces.
        for (TopExp_Explorer ex(face, TopAbs_WIRE); ex.More(); ex.Next())
            wires.push_back(TopoDS::Wire(ex.Current()));

        if (wires.empty()) {
            result.push_back(face);
            continue;
        }

        BRepBuilderAPI_MakeFace mk(sharedPlane, wires[0], Standard_True);
        if (!mk.IsDone()) {
            result.push_back(face);
            continue;
        }

        for (size_t i = 1; i < wires.size(); ++i)
            mk.Add(wires[i]);

        TopoDS_Face newFace = mk.Face();
        newFace.Location(TopLoc_Location());     // now in shared global domain
        newFace.Orientation(face.Orientation());

        result.push_back(newFace);
    }

    return result;
}

std::vector<TopoDS_Face> PutFacesInSameDomain1(std::vector<TopoDS_Face>& faces)
{
    std::vector<TopoDS_Face> result;
    result.reserve(faces.size());
    if (faces.empty())
        return result;

    // 1. Get geometric plane from first face (with location applied)
    TopoDS_Face refFace = faces[0];
    TopLoc_Location locRef = refFace.Location();

    Handle(Geom_Surface) refSurf = BRep_Tool::Surface(refFace);
    Handle(Geom_Plane) refPlane = Handle(Geom_Plane)::DownCast(refSurf);
    if (refPlane.IsNull()) {
        std::cerr << "PutFacesInSameDomain: reference face is not planar\n";
        return faces; // bail out, not planar
    }

    // Apply location to plane so we work in global coordinates
    gp_Pln pln = refPlane->Pln();
    gp_Trsf trsfRef = locRef.Transformation();
    pln.Transform(trsfRef);
    Handle(Geom_Plane) sharedPlane = new Geom_Plane(pln);

    // 2. Rebuild each face on the shared plane using 3D wires
    BRep_Builder builder;

    for (const TopoDS_Face& face : faces)
    {
        // Apply location to get real 3D geometry
        TopLoc_Location loc = face.Location();
        gp_Trsf trsf = loc.Transformation();

        // Collect transformed wires
        TopoDS_Compound wiresComp;
        builder.MakeCompound(wiresComp);

        for (TopExp_Explorer wex(face, TopAbs_WIRE); wex.More(); wex.Next()) {
            TopoDS_Wire w = TopoDS::Wire(wex.Current());
            // Bring wire to global coordinates
            TopoDS_Wire wTrsf = TopoDS::Wire(w.Moved(loc));
            builder.Add(wiresComp, wTrsf);
        }

        // Build a new face on the shared plane from 3D wires
        // (BRepBuilderAPI_MakeFace will compute p-curves on the plane)
        TopoDS_Face newFace;

        // If there is only one wire, we can pass it directly
        TopExp_Explorer wexp(wiresComp, TopAbs_WIRE);
        if (!wexp.More()) {
            // no wires? skip
            result.push_back(face);
            continue;
        }

        TopoDS_Wire outerWire = TopoDS::Wire(wexp.Current());
        BRepBuilderAPI_MakeFace mkFace(sharedPlane, outerWire, Standard_True);
        newFace = mkFace.Face();

        // Add inner wires if any
        for (wexp.Next(); wexp.More(); wexp.Next()) {
            TopoDS_Wire innerWire = TopoDS::Wire(wexp.Current());
            builder.Add(newFace, innerWire);
        }

        // New face is in global coords; clear location and set orientation
        newFace.Location(TopLoc_Location());
        newFace.Orientation(face.Orientation());

        result.push_back(newFace);
    }

    return result;
}


TopoDS_Shape UniteFaceVector(  std::vector<TopoDS_Face>& _faces) {
    if (_faces.empty()) return TopoDS_Shape();
    if (_faces.size() == 1) return _faces[0];
perf2();
	std::vector<TopoDS_Face> faces=PutFacesInSameDomain(_faces);
perf2("putas");
    // 1. Fuse all faces together
    TopoDS_Shape result = faces[0];

    for (size_t i = 1; i < faces.size(); ++i) {
        // Ensure consistent orientation (Orientation() is a getter, not a setter)
        TopoDS_Face f = faces[i];
        f.Orientation(faces[0].Orientation());
		ShapeFix_Shape fixer(f);
        fixer.Perform();
        f = TopoDS::Face(fixer.Shape());



        BRepAlgoAPI_Fuse fuser(result, f);
        // fuser.SetFuzzyValue(0.10); // Avoid fuzzy fuse unless absolutely needed
        fuser.Build();

        if (!fuser.IsDone()) {
            std::cerr << "Fuse failed at index " << i << std::endl;
            continue;
        }

        result = fuser.Shape();
    }

    // 2. Unify coplanar faces (UnifySameDomain)
    ShapeUpgrade_UnifySameDomain unifier(result, Standard_True, Standard_True, Standard_True);
    unifier.SetLinearTolerance(1e-7);
    unifier.SetAngularTolerance(1e-7);
    unifier.Build();

    // if (!unifier.IsDone()) {
    //     std::cerr << "UnifySameDomain failed" << std::endl;
    //     return result;
    // }

    return unifier.Shape();
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
void WriteBinarySTL(const TopoDS_Shape& shape, const std::string& filename) {
    // Mesh the shape
    BRepMesh_IncrementalMesh mesh(shape, 0.1);

    std::ofstream file(filename, std::ios::binary);
    char header[80] = "Binary STL generated by OpenCascade";
    file.write(header, 80);

    std::vector<std::array<gp_Pnt, 3>> triangles;

    for (TopExp_Explorer faceExp(shape, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        TopLoc_Location loc = face.Location();
        gp_Trsf trsf = loc.Transformation();

        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
        if (triangulation.IsNull()) continue;

        // Get triangle array and loop over its bounds
        const Poly_Array1OfTriangle& triArray = triangulation->Triangles();
        for (Standard_Integer i = triArray.Lower(); i <= triArray.Upper(); ++i) {
            Standard_Integer n1, n2, n3;
            triArray(i).Get(n1, n2, n3);

            gp_Pnt p1 = triangulation->Node(n1).Transformed(trsf);
            gp_Pnt p2 = triangulation->Node(n2).Transformed(trsf);
            gp_Pnt p3 = triangulation->Node(n3).Transformed(trsf);

            triangles.push_back({p1, p2, p3});
        }
    }

    uint32_t triCount = static_cast<uint32_t>(triangles.size());
    file.write(reinterpret_cast<char*>(&triCount), 4);

    for (const auto& tri : triangles) {
        gp_Vec v1(tri[0], tri[1]);
        gp_Vec v2(tri[0], tri[2]);
        gp_Vec normalVec = v1.Crossed(v2);
        if (normalVec.SquareMagnitude() > gp::Resolution())
            normalVec.Normalize();
        else
            normalVec = gp_Vec(0.0, 0.0, 0.0);

        float normal[3] = {
            static_cast<float>(normalVec.X()),
            static_cast<float>(normalVec.Y()),
            static_cast<float>(normalVec.Z())
        };
        file.write(reinterpret_cast<char*>(normal), 12);

        for (const gp_Pnt& p : tri) {
            float coords[3] = {
                static_cast<float>(p.X()),
                static_cast<float>(p.Y()),
                static_cast<float>(p.Z())
            };
            file.write(reinterpret_cast<char*>(coords), 12);
        }

        uint16_t attrByteCount = 0;
        file.write(reinterpret_cast<char*>(&attrByteCount), 2);
    }

    file.close();
}

void OnMouseClick(Standard_Integer x, Standard_Integer y, const Handle(AIS_InteractiveContext) & context, const Handle(V3d_View) & view) {
	context->Activate(TopAbs_FACE);
	// Step 1: Move the selection to the clicked point
	context->MoveTo(x, y, view, 0);

	// Step 2: Select the clicked object
	context->Select(true);	// true = update viewer

	// Step 3: Get the selected shape
	if (context->HasSelectedShape()) {
		TopoDS_Shape selectedShape = context->SelectedShape();

		// Step 4: Explore faces in the selected shape
		for (TopExp_Explorer exp(selectedShape, TopAbs_FACE); exp.More(); exp.Next()) {
			TopoDS_Face face = TopoDS::Face(exp.Current());

			// You now have the face!
			std::cout << "Picked face found!" << std::endl;

			// You can now use this face for transformation
			break;	// Just take the first face for simplicity
		}
	}
}

std::string to_string_trim(double value) {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << value;
	std::string s = oss.str();

	// if there’s a decimal point, strip trailing zeros
	if (auto pos = s.find('.'); pos != std::string::npos) {
		while (!s.empty() && s.back() == '0') s.pop_back();
		// if the last character is now the dot, remove it too
		if (!s.empty() && s.back() == '.') s.pop_back();
	}
	return s;
}
auto fmt = [](double v, int precision = 2) {
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

	return nullptr;
}
void mergeShape(TopoDS_Compound& target,  TopoDS_Shape& toAdd) {
	current_part->shape = toAdd;	// last added
	// cotm(ShapeTypeName(shape));
	if (toAdd.IsNull()) {
		std::cerr << "Warning: toAdd is null." << std::endl;
		return;
	}
	if (toAdd.IsSame(target)) {
		std::cerr << "Warning: attempted to merge compound into itself." << std::endl;
		return;
	}
	BRep_Builder builder;
	if (target.IsNull()) {
		builder.MakeCompound(target);
	}
	// toAdd.Location(current_part->current_location);
	builder.Add(target, toAdd);

	// Track transform
	// gp_Ax2 ax3(origin, normal, xdir);
	// trsf.SetTransformation(ax3);
	// trsf.Invert();
	// vtrsf.push_back(trsf);
}
void CreateWire(const std::vector<gp_Vec2d>& points, bool closed = false) {
	current_part->vpoints.push_back(points);
	BRepBuilderAPI_MakePolygon poly;
	for (auto& v : points) {
		poly.Add(gp_Pnt(v.X(), v.Y(), 0));
	}
	if (closed && points.size() > 2) poly.Close();
	TopoDS_Wire wire = poly.Wire();
	if (points.size() > 2) {
		// Make a planar face from that wire
		TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);
		BRepMesh_IncrementalMesh mesher(face, 0.5, true, 0.5, true);
		// mergeShape(current_part->cshape, face);
		// current_part->shape = face;
	face.Location(current_part->shape.Location().Transformation());
		inteligentmerge(face,0);
	} else {
		// current_part->shape = wire;
	wire.Location(current_part->shape.Location().Transformation());
		inteligentmerge(wire,0);
		// mergeShape(current_part->cshape, wire);
	}
	// current_part->shape.Location(current_part->shape.Location().Transformation());
	// current_part->shape.Location(current_part->current_location);
}

// ---------------------------------------------------------------------
//region Initialize OpenCASCADE
// ---------------------------------------------------------------------
void initialize_opencascade(Fl_Window* wingl) {
#ifdef _WIN32
    wingl->show();
    wingl->make_current();
    HWND hwnd = (HWND)fl_xid(wingl);
    Handle(WNT_Window) wind = new WNT_Window(hwnd);
    m_display_connection = new Aspect_DisplayConnection("");
#else
    Window win = (Window)fl_xid(wingl);
    Display* display = fl_display;
    m_display_connection = new Aspect_DisplayConnection(display);
    Handle(Xw_Window) wind = new Xw_Window(m_display_connection, win);
#endif

    m_graphic_driver = new OpenGl_GraphicDriver(m_display_connection);
    m_viewer = new V3d_Viewer(m_graphic_driver);

    // Pure IBL - disable all default lights
    m_viewer->SetLightOff();

    ctx = new AIS_InteractiveContext(m_viewer);
    view = m_viewer->CreateView();
    // view->SetLightOff();
    // view->SetShadingModel(Graphic3d_TOSM_PBR);

    // Graphic3d_RenderingParams& params = view->ChangeRenderingParams();
    // params.NbMsaaSamples = 0;

    view->SetWindow(wind);

	view->SetImmediateUpdate(Standard_False);
	view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_BLACK, 0.08);

	// view->ChangeRenderingParams().Method = Graphic3d_RM_RAYTRACING;
	// view->SetZFitAll(true);
	view->ZFitAll();
	setbar5per();
}

void setSceneDefault() {
	Handle(V3d_DirectionalLight) l1 = new V3d_DirectionalLight(V3d_XnegYnegZneg, Quantity_NOC_WHITE, Standard_True);
	Handle(V3d_DirectionalLight) l2 = new V3d_DirectionalLight(V3d_XposYposZpos, Quantity_NOC_GRAY80, Standard_True);
	view->Viewer()->AddLight(l1);
	view->Viewer()->SetLightOn(l1);
	view->Viewer()->AddLight(l2);
	view->Viewer()->SetLightOn(l2);
	l1->SetIntensity(0.2f);
	l2->SetIntensity(0.2f);

	l1->SetIntensity(9.5f);
	l2->SetIntensity(9.5f);

	// 1. Get the default drawer from the context
	Handle(Prs3d_Drawer) defaultDrawer = ctx->DefaultDrawer();

	// 2. Enable face boundaries globally
	defaultDrawer->SetFaceBoundaryDraw(Standard_True);

	Quantity_Color defaultColor(Quantity_NOC_STEELBLUE);
	defaultDrawer->ShadingAspect()->SetColor(defaultColor);

	// 4. (Optional) Set material properties
	// The color often depends on how the material reflects light
	// defaultDrawer->ShadingAspect()->SetMaterial(Graphic3d_NameOfMaterial_Plastic);

	// 3. Set Global Face Boundary Aspect (Color, Type, Width)
	Handle(Prs3d_LineAspect) faceBoundaryAspect =
		new Prs3d_LineAspect(Quantity_Color(Quantity_NOC_BLACK), Aspect_TOL_SOLID, 2.0);
	defaultDrawer->SetFaceBoundaryAspect(faceBoundaryAspect);

	// 4. Set Global UnFree Boundary Aspect (Edges between faces) wireframe
	Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 0.5);
	defaultDrawer->SetUnFreeBoundaryAspect(wireAsp);
}

//region scint

void lua_str(const string &str,bool isfile);
void lua_str_realtime(const string &str);
string currfilename="";

struct scint : public fl_scintilla { 
	scint(int X, int Y, int W, int H, const char* l = 0)
	: fl_scintilla(X, Y, W, H, l) {
		//  show_browser=0; set_flag(FL_ALIGN_INSIDE); 
		cotm(filename)
		currfilename=filename;
		callbackOnload=([this]() {
			isdebuging=0;
		hints={"Part"};
			lua_str(filename,1);
			win->label(filename.c_str());
			currfilename=filename;
		}); 
		}
	int handle(int e)override;
};

int scint::handle(int e){



	if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()==FL_F + 1){
		int currentPos = SendEditor( SCI_GETCURRENTPOS, 0, 0);
		int currentLine = SendEditor(SCI_LINEFROMPOSITION, currentPos, 0);
		int lineLength = SendEditor(SCI_LINELENGTH, currentLine, 0);
		if (lineLength <= 0) return 0; 
		std::string buffer(lineLength, '\0'); // allocate with room for '\0'
		SendEditor( SCI_GETLINE, currentLine, (sptr_t)buffer.data()); 
		buffer.erase(buffer.find_last_not_of("\r\n\0") + 1);
		// lua_str(buffer,0);
		return 1;
	}
	// if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()==FL_F + 2){
	// 	// cotm("f2",filename);
	// 	lua_str(filename,1);
	// 	return 1; 
	// }
	if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()=='s'){
		fl_scintilla::handle(e);
		// cotm("f2",filename)
		isdebuging=0;
		lua_str(filename,1);/////
		return 1;
	}


	if(e==FL_UNFOCUS)SendEditor(SCI_AUTOCCANCEL);


	int ret= fl_scintilla::handle(e);
	if(e == FL_KEYDOWN){
		// lua_str_realtime(getalltext());
	}
	return ret;
}


scint* editor;

void scint_init(int x,int y,int w,int h){ 
	editor = new scint(x,y,w,h);
} 
void gopart(int currentline = -1, const std::string& str = "") {
    if (str.empty() && currentline == -1)
        return;

    const int docLen = editor->SendEditor(SCI_GETTEXTLENGTH);
    if (docLen <= 0)
        return;

    int pos = -1;

    if (currentline >= 0) {
        // Clamp line if needed
        const int lastLine = editor->SendEditor(SCI_GETLINECOUNT) - 1;
        int targetLine = std::min(currentline, lastLine);

        // Get start position of that line
        pos = editor->SendEditor(SCI_POSITIONFROMLINE, targetLine);

    } else {
        // Regex search mode
        editor->SendEditor(SCI_SETSEARCHFLAGS, SCFIND_REGEXP);

        // Set search range
        editor->SendEditor(SCI_SETTARGETSTART, 0);
        editor->SendEditor(SCI_SETTARGETEND, docLen);

        std::string pattern = "^[ \\t]*Part[ \\t]*\"[ \\t]*" + str;
        pos = editor->SendEditor(SCI_SEARCHINTARGET, (sptr_t)pattern.size(),
                                 (sptr_t)pattern.c_str());
        if (pos == -1)
            return;
    }

    // Get the matched line number
    const int line = editor->SendEditor(SCI_LINEFROMPOSITION, pos);

    // Ensure the line is visible (unfolds if necessary)
    editor->SendEditor(SCI_ENSUREVISIBLEENFORCEPOLICY, line);

    // Scroll to make the line nicely positioned
    editor->SendEditor(SCI_GOTOLINE, line);

    // Move caret to position (for search) or line start (for numeric jump)
    if (currentline >= 0)
        pos = editor->SendEditor(SCI_POSITIONFROMLINE, line);

    editor->SendEditor(SCI_GOTOPOS, pos);
    editor->SendEditor(SCI_SCROLLCARET);

    editor->take_focus();
}

void gopart1(int currentline=-1, std::string str="") {
    if (str.empty() && currentline==-1) return;

    const int docLen = editor->SendEditor(SCI_GETTEXTLENGTH);
    if (docLen <= 0) return;

    // Enable regex search
    editor->SendEditor(SCI_SETSEARCHFLAGS, SCFIND_REGEXP);

    // Search the entire document
    editor->SendEditor(SCI_SETTARGETSTART, 0);
    editor->SendEditor(SCI_SETTARGETEND, docLen);

	int pos = currentline;
	if (currentline == -1) {
		// Build regex: allow spaces/tabs, then Part, then optional space/paren, then your text
		std::string pattern = "^[ \\t]*Part \"[ \\t]*" + str;

		// Perform regex search
		pos = editor->SendEditor(SCI_SEARCHINTARGET, pattern.size(), (sptr_t)pattern.c_str());

		if (pos == -1) return;	// no match
	}

	// Get the matched line
    const int line = editor->SendEditor(SCI_LINEFROMPOSITION, pos);

    // Ensure the line is visible (important if folded)
    editor->SendEditor(SCI_ENSUREVISIBLE, line);

    // Scroll so the matched line is at the top
    const int visual_line        = editor->SendEditor(SCI_VISIBLEFROMDOCLINE, line);
    const int current_visual_top = editor->SendEditor(SCI_GETFIRSTVISIBLELINE);
    const int delta              = visual_line - current_visual_top;

    if (delta != 0)
        editor->SendEditor(SCI_LINESCROLL, 0, delta-2);
 
// Move caret (this is usually the most reliable way)
    editor->SendEditor(SCI_GOTOPOS, pos); 
	editor->take_focus(); 
}
//region shelp 
struct shelpv{
	string pname="";
	string point="";
	string error="";
	string edge="";
	string mass="";
	string gentime="";
	int currentline=-1; //current line of Part

	void upd(){
	std::string html = R"(
<html>
<body marginwidth=0 marginheight=0 topmargin=0 leftmargin=0><font face=Arial > 
<b> 
$pname<br> $point $edge <br>$mass
<br><font color="Red">$error</font>
<br><font color="Bue">$gentime</font>
</font>
</body>
</html>
)";
	replace_All(html,"$point",point);
	replace_All(html, "$pname",
        pname.empty()
            ? ""
            : "<font color=\"#8B0000\">Part " + pname + "</font>"
    );
	replace_All(html,"$edge",edge);
	replace_All(html,"$mass",mass);
	replace_All(html,"$error",error);
	replace_All(html,"$gentime",gentime);

	helpv->value(html.c_str());
}

}help;

//region highlight
	Handle(AIS_Shape) myHighlightedPointAIS;  // To store the highlighting sphere
	TopoDS_Vertex myLastHighlightedVertex;	  // To store the last highlighted vertex
	void clearHighlight(Handle(AIS_InteractiveContext) &m_context) {
		if (!myHighlightedPointAIS.IsNull()) {
			m_context->Remove(myHighlightedPointAIS, Standard_True);
			myHighlightedPointAIS.Nullify();
		}
		myLastHighlightedVertex.Nullify();
	}

bool IsWorldPointGreen(const Handle(V3d_View) & view, const gp_Pnt& point, int radius = 5, unsigned char minGreen = 200, unsigned char maxRedBlue = 64) {
	// 1. Get view size
	Standard_Integer w = 0, h = 0;
	view->Window()->Size(w, h);

	// 2. Project world point → view coords → pixel coords
	Standard_Real Xv = 0., Yv = 0., Zv = 0.;
	view->Project(point.X(), point.Y(), point.Z(), Xv, Yv, Zv);

	Standard_Integer px = 0, py = 0;
	view->Convert(Xv, Yv, px, py);

	// 3. Bail out if point is outside view
	if (px < 0 || px >= w || py < 0 || py >= h) return false;
 
	// 4. Capture full-view pixmap
	Image_PixMap pix;
	if (!view->ToPixMap(pix, w, h) || pix.IsEmpty()) return false;
 
	// 5. Determine channel count and stride
	const Image_Format fmt = pix.Format();
	const size_t channels = (fmt == Image_Format_RGBA ? 4 : 3);
	const size_t rowBytes = pix.Width() * channels;
	const Standard_Byte* data = pix.Data();

	// 6. Scan a circular neighborhood for “greenish” pixels
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			if (dx * dx + dy * dy > radius * radius) continue;	// outside circle

			int sx = px + dx;
			int sy = py + dy;
			if (sx < 0 || sx >= w || sy < 0 || sy >= h) continue;  // out of bounds

			// Flip Y: mouse top-left vs pixmap bottom-left
			int yPix = h - 1 - sy;
			size_t idx = size_t(yPix) * rowBytes + size_t(sx) * channels;

			unsigned char r = data[idx + 0];
			unsigned char g = data[idx + 1];
			unsigned char b = data[idx + 2];

			if (g >= minGreen && r <= maxRedBlue && b <= maxRedBlue) return true;  // found greenish pixel
		}
	} 
	return false;  // no green near the projected point
}
void scaleball1(){
// Update scale + position anytime
globalratio = GetViewportAspectRatio()[0]; 

if (ctx->IsDisplayed(myHighlightedPointAIS)){
gp_Trsf trsf;
trsf.SetScale(vertexPnt,globalratio);


myHighlightedPointAIS->SetLocalTransformation(trsf);
ctx->Redisplay(myHighlightedPointAIS, true);
}
}
	
void scaleball() {
    globalratio = GetViewportAspectRatio()[0];
if(globalratio>8)globalratio=8;
// cotm(globalratio);

	// sphereBase= = BRepPrimAPI_MakeSphere(5.0).Shape();

// sphereBase = BRepPrimAPI_MakeSphere(5.0).Shape();

// gp_Pnt origin(0,0,0);          // scale center
// gp_Trsf trsf;
// // trsf.SetScale(origin, globalratio);    // scale factor 2
// trsf.SetScaleFactor(globalratio);    // scale factor 2

// BRepBuilderAPI_Transform scaleOp(sphereShapeg, trsf, true);
// sphereBase = scaleOp.Shape();
// perf2();

gp_Trsf trsf;
trsf.SetScale(gp_Pnt(0,0,0), globalratio);

if(1)
if (ctx->IsDisplayed(myHighlightedPointAIS)) {
// gp_Trsf trsf;
// trsf.SetScale(gp_Pnt(0,0,0), globalratio);
trsf.SetTranslationPart(gp_Vec(vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z()));
TopoDS_Shape scaledSphere = sphereBase;
myHighlightedPointAIS->Set(scaledSphere);
myHighlightedPointAIS->SetLocalTransformation(trsf);
ctx->Redisplay(myHighlightedPointAIS, true);
}

if(0)//working faster
for(int i=0;i<vlua.size();i++){
	if(!vlua[i]->visible_hardcoded)continue;
	auto &s=vlua[i]->acl;
	gp_Pnt vertexPnt=vlua[i]->current_location.Transformation().TranslationPart();
trsf.SetTranslationPart(gp_Vec(vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z()));
TopoDS_Shape scaledSphere = sphereBase;
s->Set(scaledSphere);
s->SetLocalTransformation(trsf);
ctx->Redisplay(s, 0);

}




if(0){//working
    if (ctx->IsDisplayed(myHighlightedPointAIS)) {
gp_Trsf trsf;

trsf.SetScale(gp_Pnt(0,0,0), globalratio);
trsf.SetTranslationPart(gp_Vec(vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z()));

TopoDS_Shape scaledSphere = BRepBuilderAPI_Transform(sphereBase, trsf, true).Shape();
myHighlightedPointAIS->Set(scaledSphere);
        ctx->Redisplay(myHighlightedPointAIS, true);
		cotm("hld");
    }
}
	

// perf2("scaling");
}




void highlightVertex(const TopoDS_Vertex& aVertex, luadraw* ldd = 0) {
		clearHighlight(ctx);  // Clear any existing highlight first
 
		globalratio = GetViewportAspectRatio()[0]; 

		gp_Pnt vertexPntL = BRep_Tool::Pnt(aVertex);
		vertexPnt = vertexPntL;
		if (ldd) vertexPnt = vertexPntL.Transformed(ldd->Origin);

		// check here
		//  if(!IsVertexVisible(vertexPnt,view,m_context))return;

		// Create a small red sphere at the vertex location
		// Standard_Real sphereRadius = 5 * globalratio;	 // Small radius for the highlight ball
		// TopoDS_Shape sphereShape = BRepPrimAPI_MakeSphere(vertexPnt, sphereRadius).Shape();
		// myHighlightedPointAIS = new AIS_Shape(sphereShape);

// static TopoDS_Shape sphereBase = BRepPrimAPI_MakeSphere(5).Shape();
gp_Trsf trsf;

trsf.SetScale(gp_Pnt(0,0,0), globalratio);
trsf.SetTranslationPart(gp_Vec(vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z()));

TopoDS_Shape scaledSphere = BRepBuilderAPI_Transform(sphereBase, trsf, true).Shape();


myHighlightedPointAIS = new AIS_Shape(scaledSphere);

// if(ldd)
// DrawTrihedron(ctx,ldd->shape.Location(),globalratio);
// DrawTrihedron(ctx,ldd->Originl,globalratio);


		myHighlightedPointAIS->SetColor(Quantity_NOC_GREEN);
		myHighlightedPointAIS->SetDisplayMode(AIS_Shaded);
		// myHighlightedPointAIS->SetTransparency(0.2f);			   // Slightly transparent


		myHighlightedPointAIS->SetZLayer(Graphic3d_ZLayerId_Top);  // Ensure it's drawn on top

		ctx->Display(myHighlightedPointAIS, Standard_True);
		myLastHighlightedVertex = aVertex;
		view->Redraw();
		// Fl::wait();
		if (!IsWorldPointGreen(view, vertexPnt)) return;
		// if(!IsPixelQuantityGreen(view,mousex,mousey))return;

		help.point = "";
		help.upd();
		// if(!IsVisible(myHighlightedPointAIS,view))return;
		// if(!IsShapeVisible(myHighlightedPointAIS,view,m_context))return;

		// cotm("Highlighted Vertex:", vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z());
		help.point = "Point: " + fmt(vertexPnt.X()) + "," + fmt(vertexPnt.Y()) + "," + fmt(vertexPnt.Z());
		if (ldd && !ldd->Origin.IsIdentity()) {
			help.point += " Pointl: " + fmt(vertexPntL.X()) + "," + fmt(vertexPntL.Y()) + "," + fmt(vertexPntL.Z());
		}
		help.upd();
	}
	
	void ev_highlight(Handle(AIS_InteractiveContext) &m_context) {
		if (!m_initialized) return; 

		static Handle(AIS_Shape) highlight;

		if (!highlight.IsNull()) m_context->Remove(highlight, 1);

		clearHighlight(m_context);

		// // Activate only what you need once per hover
		m_context->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
		m_context->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
		if (!hlr_on)
			m_context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
		else {
			m_context->Deactivate(visible_);
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Default);
			m_context->Deactivate(AIS_Shape::SelectionMode(TopAbs_FACE));
		}

		// Move cursor in context
		m_context->MoveTo(mousex, mousey, view, Standard_False);
		m_context->SelectDetected(AIS_SelectionScheme_Replace);

		Handle(SelectMgr_EntityOwner) anOwner = m_context->DetectedOwner();
		if (anOwner.IsNull()) {
			clearHighlight(m_context);
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			view->Redraw();
			return;
		}


		// luadraw* ldd = lua_detected(anOwner);
		// if (ldd) {
		// 	std::string pname = ldd->name;
		// 	// if (pname != help.pname) //to optimize speed
		// 	{
		// 		help.pname = pname;
		// 		help.currentline=ldd->currentline;
		// 		GProp_GProps systemProps;
		// 		// Density is not directly handled here — mass is proportional to volume.
		// 		// If you want real mass, multiply by material density later.
		// 		BRepGProp::VolumeProperties(ldd->fshape, systemProps);

		// 		// Get the mass (volume if density=1)
		// 		Standard_Real mass = systemProps.Mass();
		// 		double massa = mass / 1000;
		// 		help.mass = " Mass:" + fmt(massa, 0) + "cm³" + " Petg50%:~" + fmt(massa * 1.27 * 0.75, 0) + "g" + " Steel:~" + fmt(massa * 0.007850, 2) + "kg";
		// 		// std::cout << "Mass (assuming density=1): " << mass << std::endl;
		// 		help.upd();
		// 	}
		// } else {
		// 	if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
		// 	return;
		// }

		Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(anOwner);
		if (brepOwner.IsNull()) {
			clearHighlight(m_context);
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			view->Redraw();
			return;
		}

		TopoDS_Shape detected = brepOwner->Shape();
		if (detected.IsNull()) {
			clearHighlight(m_context);
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			view->Redraw();
			return;
		}



		luadraw* ldd = lua_detected(anOwner);
		if (ldd) {
			std::string pname = ldd->name;
			// if (pname != help.pname) //to optimize speed
			{
				help.pname = pname;
				help.currentline=ldd->currentline;
				GProp_GProps systemProps;
				// Density is not directly handled here — mass is proportional to volume.
				// If you want real mass, multiply by material density later.
				BRepGProp::VolumeProperties(ldd->fshape, systemProps);

				// Get the mass (volume if density=1)
				Standard_Real mass = systemProps.Mass();
				double massa = mass / 1000;
				help.mass = " Mass:" + fmt(massa, 0) + "cm³" + " Petg50%:~" + fmt(massa * 1.27 * 0.75, 0) + "g" + " Steel:~" + fmt(massa * 0.007850, 2) + "kg";
				// std::cout << "Mass (assuming density=1): " << mass << std::endl;
				help.upd();
				if(ldd)DrawTrihedron(ctx,ldd->current_location, GetViewportAspectRatio()[0]);
				// if(ldd)DrawTrihedron(ctx,ldd->shape.Location(), GetViewportAspectRatio()[0]);
			}
		} else {
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			return;
		}









//volume of solid
if(detected.ShapeType()==TopAbs_FACE){
auto solid=FindSolidOfFace(ldd->fshape,TopoDS::Face(detected));
if (!solid.IsNull()){
    

// Calcular volume
GProp_GProps props;
BRepGProp::VolumeProperties(solid, props);
double volume = props.Mass();
Standard_Real mass = props.Mass();
double massa = mass / 1000;
std::cout << "Volume = " << volume<< " "<<fmt(massa * 0.007850, 2) << "kg" << std::endl;
}

}
if(0){
TopoDS_Shape solid=TopoDS_Shape();
if (detected.ShapeType() == TopAbs_SOLID) {
    solid = detected;
} 
// else {
//     solid = FindSolidOf(compoundRoot, detected);
// }

if (!solid.IsNull()){
    

// Calcular volume
GProp_GProps props;
BRepGProp::VolumeProperties(solid, props);
double volume = props.Mass();

std::cout << "Volume = " << volume << std::endl;
}
	}










		if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
 

		switch (detected.ShapeType()) {
			case TopAbs_VERTEX: {
				if (0) {
					Standard_Real f, l;
					try {
						TopoDS_Edge edge = TopoDS::Edge(detected);
						// cotm("✅ This IS a circle fe") 
						Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

						if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Circle))) {
							Handle(Geom_Circle) circ = Handle(Geom_Circle)::DownCast(curve);

							if (!circ.IsNull()) {
								gp_Pnt center = circ->Location();  // ✅ CENTER HERE
								double radius = circ->Radius();
								TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(center);
								highlightVertex(v, ldd);
							}
							return;
						}
					} catch (...) {
					}
				}

				TopoDS_Vertex v = TopoDS::Vertex(detected);

				gp_Pnt vertexPnt = BRep_Tool::Pnt(v); 

				if (!myLastHighlightedVertex.IsEqual(v)) highlightVertex(v, ldd);
				break;
			} 
			case TopAbs_EDGE: {
				TopoDS_Edge edge = TopoDS::Edge(detected);

				// if(0)
				{
					Standard_Real f, l;
					Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

					if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Circle))) {
						Handle(Geom_Circle) circ = Handle(Geom_Circle)::DownCast(curve);

						if (!circ.IsNull()) {
							gp_Pnt center = circ->Location();  // ✅ CENTER HERE
							bool ctrlDown = (Fl::event_state() & FL_CTRL) != 0;
							if (ctrlDown) {
								pickcenterforstudyrotation = center;
								pickcenteraxisDir = circ->Circ().Axis().Direction();  // circle normal
								studyRotation = 1;
								// cotm(studyRotation)
							}
							double radius = circ->Radius();
							TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(center);
							highlightVertex(v, ldd);
						}
						// cotm("✅ This IS a circle")
					}
				}

				GProp_GProps props;
				BRepGProp::LinearProperties(edge, props);
				double length = props.Mass();

				string pname = "Edge length: " + to_string_trim(length);
				if (pname != help.edge) {
					help.edge = pname;
					help.upd();
				}

				if (highlight.IsNull()) highlight = new AIS_Shape(edge); 
				{
					highlight->SetLocalTransformation(ldd->Origin.Transformation());
					// cotm("ldd->Origin.Transformation()")
					// PrintLocation(ldd->Origin);
				}

				if (hlr_on) {
					if (m_context->IsDisplayed(highlight)) m_context->Remove(highlight, Standard_False);

					highlight->Set(edge);
					m_context->Deactivate(highlight);
					highlight->SetDisplayMode(AIS_WireFrame);

					Handle(Prs3d_LineAspect) la = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 3.0);

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
					view->Redraw();
					return;
				}

				if (!hlr_on) {
					if (m_context->IsDisplayed(highlight)) m_context->Remove(highlight, Standard_False);

					m_context->Deactivate(highlight);

					TopoDS_Edge edge = TopoDS::Edge(detected);
					GProp_GProps props;
					BRepGProp::LinearProperties(edge, props);
					double length = props.Mass();
					string pname = "Edge length: " + to_string_trim(length);
					if (pname != help.edge) {
						help.edge = pname;
						help.upd();
					} 
					view->Redraw();
					return;
				}
				break;
			}
			case TopAbs_FACE: {
				TopoDS_Face picked = TopoDS::Face(detected);
				TopLoc_Location origLoc = picked.Location();
				picked.Location(TopLoc_Location());	 // normalize

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
				double area = props.Mass();

				// normal
				Standard_Real u1, u2, v1, v2;
				BRepTools::UVBounds(picked, u1, u2, v1, v2);
				Handle(Geom_Surface) surf = BRep_Tool::Surface(picked);
				GeomLProp_SLProps slp(surf, (u1 + u2) * 0.5, (v1 + v2) * 0.5, 1, Precision::Confusion());
				gp_Dir normal = slp.IsNormalDefined() ? slp.Normal() : gp_Dir(0, 0, 1);
				if (picked.Orientation() == TopAbs_REVERSED) normal.Reverse();

				// std::string serialized = SerializeCompact(ldd ? ldd->name : "part", faceIndex, normal, centroid, area);
				// cotm("serialized", serialized);

				picked.Location(origLoc);  // restore
				break;
			}

			default:
				clearHighlight(m_context);
				break;
		}
		view->Redraw();
	}


//region hlr
	void projectAndDisplayWithHLR() {

		if (visible_) ctx->Remove(visible_, 0);
		if (!hlr_on || ctx.IsNull() || view.IsNull()) return;

		// 1. Camera transformation setup (kept your working version)
		const Handle(Graphic3d_Camera) & camera = view->Camera();
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
		Standard_Real deflection = 0.4;	 // Adjust this for your speed/quality tradeoff

		Standard_Real angularDeflection = 0.5;	// radians, controls curve discretization

		for (auto& s : vlua) {			
			if (s->visible_hardcoded) {
			// if (ctx->IsDisplayed(s->ashape)) {
			// if (!s->fshape.IsNull()) {
				BRepTools::Clean(s->fshape);

				// Optional: skip if already meshed sufficiently (your commented check)
				// For maximum speed, you can force remesh or skip check
				BRepMesh_IncrementalMesh(s->fshape, deflection, Standard_False, angularDeflection, Standard_True);
			}
		}

		// 4. HLR computation (sequential - no parallelism available in OCCT HLR)
		Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();

		for (const auto& s : vlua) {
			if (s->visible_hardcoded) {
			// if (ctx->IsDisplayed(s->ashape)) {
			// if (!s->fshape.IsNull()) {
				algo->Load(s->fshape);
			}
		}

		algo->Projector(projector);
		algo->Update();	 // This is the main bottleneck - coarser mesh = much faster here

		// // 5. Extract visible edges and transform back
		// HLRBRep_PolyHLRToShape hlrToShape;
		// hlrToShape.Update(algo);
		// algo.Nullify();

		// TopoDS_Shape vEdges = hlrToShape.VCompound();  // Visible sharp edges (adjust for other types if needed)
		// BRepBuilderAPI_Transform visT(vEdges, invTrsf);
		// TopoDS_Shape result = visT.Shape();

		// visible_ = new AIS_NonSelectableShape(result);

// 5. Extract visible edges (Sharp + Smooth + Outlines) fillet lines
HLRBRep_PolyHLRToShape hlrToShape;
hlrToShape.Update(algo);

BRep_Builder builder;
TopoDS_Compound visibleCompound;
builder.MakeCompound(visibleCompound);

// 1. Sharp visible edges (Hard edges)
TopoDS_Shape vEdges = hlrToShape.VCompound();
if (!vEdges.IsNull()) builder.Add(visibleCompound, vEdges);

// 2. Smooth visible edges (This captures your FILLETS)
TopoDS_Shape rg1Edges = hlrToShape.Rg1LineVCompound();
if (!rg1Edges.IsNull()) builder.Add(visibleCompound, rg1Edges);

// 3. Silhouette edges (Outlines of curved surfaces)
TopoDS_Shape outEdges = hlrToShape.OutLineVCompound();
if (!outEdges.IsNull()) builder.Add(visibleCompound, outEdges);

// Transform the combined compound back
BRepBuilderAPI_Transform visT(visibleCompound, invTrsf);
TopoDS_Shape result = visT.Shape();

visible_ = new AIS_NonSelectableShape(result);
// ... rest of your display code


		visible_->SetColor(Quantity_NOC_BLACK);
		visible_->SetWidth(2.2);
		ctx->Display(visible_, false);
		visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
		ctx->Deactivate(visible_);
	}



//region occv
struct OCC_Viewer : public Fl_Window {
	OCC_Viewer(int X, int Y, int W, int H, const char* L = 0) : Fl_Window(X, Y, W, H, L) {		
		Fl::add_timeout(10, idle_refresh_cb, 0);
	}
	static void idle_refresh_cb(void*) {
		// clear gpu usage each 10 secs
		glFlush();
		glFinish();
		Fl::repeat_timeout(10, idle_refresh_cb, 0);
	}	
	void draw() { 
		if(!m_initialized) return;
		view->Redraw(); 
	}
	void resize(int X, int Y, int W, int H) override {
		static bool in_resize = 0;
		if (in_resize) return;	// Prevent recursion
		in_resize = true;

		Fl_Window::resize(X, Y, W, window()->h() - 22 - 25);
		if (m_initialized) {
			view->MustBeResized();
			setbar5per();
			scaleaxis(ctx);
			// scaleball();
			// globalratio = GetViewportAspectRatio()[0]; 
		}
		in_resize = false;
	}
	
	int handle(int event)  {
		if (!m_initialized) return Fl_Window::handle(event);
		static auto last_event = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event);

		constexpr auto frame_interval = std::chrono::milliseconds(1000 / 30);
		// constexpr auto frame_interval = std::chrono::milliseconds(((int)(1000 / 8.0)));

		if (event == FL_DRAG && isRotating && m_initialized) { 
			if (elapsed > frame_interval) { 
				view->Rotation(Fl::event_x(), Fl::event_y());
				projectAndDisplayWithHLR(); 
				colorisebtn();////////////////////////////////////////
				redraw(); 
				last_event = std::chrono::steady_clock::now();
				return 0;
			} 
			return 0;
		}

		bool ctrlDown = (Fl::event_state() & FL_CTRL) != 0;
		if (ctrlDown && studyRotation == 1) {
			// cotm(vaShape.size()) 
			// rotateShapeByMouse(ulua[help.pname]->ashape, m_context, pickcenterforstudyrotation,													 pickcenteraxisDir, Fl::event_x(), Fl::event_y());
			// study_trg = ulua[help.pname]->ashape;
		}

		if (event == FL_PUSH && (Fl::event_state() & FL_CTRL)) {
			if (help.pname != "") {
				cotm(help.pname) 
				gopart(help.currentline, help.pname);
				editor->take_focus();
			// 	// ld->FitViewToShape(ld->view, ld->shape);
				return 1;
			}
		}

		static int start_y;
		const int edge_zone = this->w() * 0.05;	 // 5% right edge zone

		static std::chrono::steady_clock::time_point lastTime;

		if (event == FL_MOVE) {
			int x = Fl::event_x();
			int y = Fl::event_y();

			auto now = std::chrono::steady_clock::now();
			double elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();

			if ((abs(x - mousex) > 5 || abs(y - mousey) > 5) && elapsedMs > 50) {
				mousex = x;
				mousey = y; 
				ev_highlight(ctx);
				lastTime = now;
			} 
		}

 

		if (event == FL_PUSH) { 
			// OnMouseClick(mousex, mousey, m_context, view);
		}

		switch (event) {
			case FL_PUSH:
				if (Fl::event_button() == FL_LEFT_MOUSE) {
					// Check if click is in right 5% zone
					if (Fl::event_x() > (this->w() - edge_zone)) {
						isRotatingz = 1;
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
						int dy = Fl::event_y() - start_y;
						start_y = Fl::event_y();

						// Rotate view (OpenCascade)
						if (dy != 0) {
							double angle = dy * 0.005;	// Rotation sensitivity factor
							// perf();
							// Fl::wait(0.01);
							view->Rotate(0, 0,
										   angle);	// Rotate around Z-axis
							// perf("vnormal");
							projectAndDisplayWithHLR(); 
							redraw();  
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
						view->StartRotation(lastX, lastY);
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
				if (isPanning && m_initialized && Fl::event_button() == FL_RIGHT_MOUSE) {
					int dx = Fl::event_x() - lastX;
					int dy = Fl::event_y() - lastY; 
					view->Pan(dx, -dy);
					redraw();  
					lastX = Fl::event_x();
					lastY = Fl::event_y(); 
					return 1;
				}
				break;

			case FL_RELEASE:
				if (Fl::event_button() == FL_LEFT_MOUSE) {
					isRotating = false;
					isRotatingz = 0;
					isPanning = false;
					return 1;
				}
				break;
			case FL_MOUSEWHEEL:
				if (m_initialized) { 
					int mouseX = Fl::event_x();
					int mouseY = Fl::event_y();
					view->StartZoomAtPoint(mouseX, mouseY);
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
					view->ZoomAtPoint(mouseX, mouseY, endX, endY);

					scaleaxis(ctx);
					// // globalratio = GetViewportAspectRatio()[0]; 
					// scaleball();
					redraw();

					return 1;
				}
				break;
		}
		return Fl_Window::handle(event);
	}

};




//region spin
	bool IsSpinning = false;
	gp_Pnt SpinPivot;
	gp_Ax1 SpinAxis;
	gp_Dir CurrentDir;
	double SpinStep = 0.02;

	// New tracking variables
	double TargetRotation = 0;	 // 0 = infinity
	double CurrentRotation = 0;	 // Accumulated radians
static gp_Pnt computeVisibleCenter();
	// Helper to handle cleanup
	void stop_spinning() {
		IsSpinning = false;
		projectAndDisplayWithHLR();
		occv->redraw();
	}
	static void SpinCallback(void* userdata) { 

		// 1. Stop conditions (Manual toggle or Escape)
		if (!IsSpinning || Fl::event_key() == FL_Escape) {
			stop_spinning();
			return;
		}

		// 2. Limit Check: If not infinite, check if we've reached the goal
		if (TargetRotation > 0) {
			CurrentRotation += std::abs(SpinStep);
			if (CurrentRotation >= TargetRotation) {
				stop_spinning();
				return;
			}
		}

		// 3. Axis change logic (Alt + X, Y, or Z)
		int state = Fl::event_state();
		if (state & FL_ALT) {
			int key = Fl::event_key();
			gp_Dir targetDir = CurrentDir;
			if (key == 'x')
				targetDir = gp_Dir(1, 0, 0);
			else if (key == 'y')
				targetDir = gp_Dir(0, 1, 0);
			else if (key == 'z')
				targetDir = gp_Dir(0, 0, 1);

			if (!targetDir.IsEqual(CurrentDir, 1e-6)) {
				CurrentDir = targetDir;
				SpinAxis = gp_Ax1(SpinPivot, targetDir);
			}
		}

		// 4. Transformation
		Handle(Graphic3d_Camera) cam = view->Camera();
		gp_Trsf trsf;
		trsf.SetRotation(SpinAxis, SpinStep);

		cam->SetCenter(SpinPivot);
		cam->SetEye(cam->Eye().Transformed(trsf));
		cam->SetUp(cam->Up().Transformed(trsf));

		projectAndDisplayWithHLR();
		occv->redraw();
		Fl::repeat_timeout(1.0 / 60.0, SpinCallback, userdata);
	}

	void start_continuous_rotation(double spins_amount = 1) {
		if (IsSpinning) {
			IsSpinning = false;
			return;
		}

		IsSpinning = true;
		SpinPivot = computeVisibleCenter();
		// Initialize rotation tracking
		CurrentRotation = 0;
		// 1 spin = 2 * PI. If amount is 0, TargetRotation stays 0 (infinite)
		TargetRotation = spins_amount * (2.0 * M_PI);

		CurrentDir = gp_Dir(0, 1, 0);
		SpinAxis = gp_Ax1(SpinPivot, CurrentDir);

		Fl::add_timeout(1.0 / 60.0, SpinCallback, 0);
	}



//region animation
static inline gp_Dir sgnDir(const gp_Dir& axis, Standard_Real dot) {
    return (dot >= 0.0) ? axis : gp_Dir(-axis.X(), -axis.Y(), -axis.Z());
}
void GetAlignedCameraVectors(const Handle(V3d_View)& view,
                             gp_Vec& end_proj_global,
                             gp_Vec& end_up_global)
{
    // 1) Read camera forward (Proj) and Up
    Standard_Real vx, vy, vz, ux, uy, uz;
    view->Proj(vx, vy, vz);
    view->Up(ux, uy, uz);

    gp_Dir viewDir(vx, vy, vz);
    gp_Dir upDir(ux, uy, uz);

    const gp_Dir X(1,0,0), Y(0,1,0), Z(0,0,1);

    // 2) Choose nearest axis-aligned plane normal (±X, ±Y, ±Z) by max |dot|,
    //    but KEEP the sign for facing direction.
    Standard_Real dx = viewDir.Dot(X);
    Standard_Real dy = viewDir.Dot(Y);
    Standard_Real dz = viewDir.Dot(Z);

    Standard_Real ax = std::abs(dx), ay = std::abs(dy), az = std::abs(dz);

    gp_Dir normal;
    enum class Plane { YZ, ZX, XY } plane; // plane orthogonal to chosen normal

    if (ax >= ay && ax >= az) {
        normal = sgnDir(X, dx);
        plane = Plane::YZ; // normal along X -> plane is YZ
    } else if (ay >= ax && ay >= az) {
        normal = sgnDir(Y, dy);
        plane = Plane::ZX; // normal along Y -> plane is ZX
    } else {
        normal = sgnDir(Z, dz);
        plane = Plane::XY; // normal along Z -> plane is XY
    }

    // 3) Project current up onto the snapped plane (minimize roll change)
    gp_Vec upVec(upDir.X(), upDir.Y(), upDir.Z());
    gp_Vec nVec(normal.X(), normal.Y(), normal.Z());
    gp_Vec upOnPlane = upVec - (upVec.Dot(nVec)) * nVec;

    // 4) If degenerate, fall back to the axis in the plane with strongest alignment to original up
    bool degenerate = (upOnPlane.SquareMagnitude() < 1e-14);
    gp_Dir bestUp;

    auto chooseUpInPlane = [&](const gp_Dir& a, const gp_Dir& b, const gp_Vec& ref) -> gp_Dir {
        // pick among ±a, ±b the one closest to ref (use sign of dot to set direction)
        Standard_Real da = ref.Dot(gp_Vec(a.X(), a.Y(), a.Z()));
        Standard_Real db = ref.Dot(gp_Vec(b.X(), b.Y(), b.Z()));
        if (std::abs(da) >= std::abs(db)) {
            return (da >= 0.0) ? a : gp_Dir(-a.X(), -a.Y(), -a.Z());
        } else {
            return (db >= 0.0) ? b : gp_Dir(-b.X(), -b.Y(), -b.Z());
        }
    };

    if (plane == Plane::YZ) {
        bestUp = chooseUpInPlane(Y, Z, degenerate ? upVec : upOnPlane);
    } else if (plane == Plane::ZX) {
        bestUp = chooseUpInPlane(Z, X, degenerate ? upVec : upOnPlane);
    } else { // Plane::XY
        bestUp = chooseUpInPlane(X, Y, degenerate ? upVec : upOnPlane);
    }

    // 5) Build a right-handed orthonormal basis and re-derive up to ensure exact orthogonality
    gp_Vec right = nVec.Crossed(gp_Vec(bestUp.X(), bestUp.Y(), bestUp.Z()));
    if (right.SquareMagnitude() < 1e-14) {
        // Extremely rare: if bestUp accidentally parallel (numerical), pick the other axis in plane
        if (plane == Plane::YZ) {
            bestUp = (std::abs(bestUp.Dot(Y)) > 0.5) ? Z : Y;
        } else if (plane == Plane::ZX) {
            bestUp = (std::abs(bestUp.Dot(Z)) > 0.5) ? X : Z;
        } else {
            bestUp = (std::abs(bestUp.Dot(X)) > 0.5) ? Y : X;
        }
        right = nVec.Crossed(gp_Vec(bestUp.X(), bestUp.Y(), bestUp.Z()));
    }

    gp_Dir rightDir(right);
    gp_Dir correctedUp((rightDir ^ normal)); // right x normal -> up (right-handed)

    // 6) Output snapped vectors as gp_Vec
    end_proj_global = gp_Vec(normal.X(),     normal.Y(),     normal.Z());
    end_up_global   = gp_Vec(correctedUp.X(), correctedUp.Y(), correctedUp.Z());
}

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

static gp_Pnt computeVisibleCenter(){  
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
	// cotm2(winW,winH)

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

static void animation_update(void* userdata){   

    if (CurrentAnimation.IsNull())
    {
        Fl::remove_timeout(animation_update, userdata);
        return;
    }

    if (CurrentAnimation->IsStopped())
    {

        colorisebtn();
        projectAndDisplayWithHLR();
        view->Redraw();
        // redraw();

        Fl::remove_timeout(animation_update, userdata);
        return;
    }

    // Advance animation
    CurrentAnimation->UpdateTimer();
    projectAndDisplayWithHLR();
    view->Redraw();

    // 30 FPS
    Fl::repeat_timeout(1.0 / 30.0, animation_update, userdata);
}

void start_animation(){  
    // Stop any existing animation
    if (!CurrentAnimation.IsNull())
    {
        CurrentAnimation->Stop();
        CurrentAnimation.Nullify();
    }

    // Current camera
    Handle(Graphic3d_Camera) currentCamera = view->Camera();

			// cotm2("an11")
    // Pivot = center of currently visible shapes in viewport (robust)
    gp_Pnt center = computeVisibleCenter();
// cotm2(center.X());
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
    const gp_Dir targetDir(-end_proj_global.X(),
                           -end_proj_global.Y(),
                           -end_proj_global.Z());

    const gp_Pnt targetEye = center.XYZ() + targetDir.XYZ() * distance;

    cameraEnd->SetEye(targetEye);
    cameraEnd->SetCenter(center);
    cameraEnd->SetDirection(targetDir);
    cameraEnd->SetUp(gp_Dir(end_up_global.X(),
                            end_up_global.Y(),
                            end_up_global.Z()));

    // Animate
    CurrentAnimation = new AIS_AnimationCamera("ViewAnimation", view);
    CurrentAnimation->SetCameraStart(cameraStart);
    CurrentAnimation->SetCameraEnd(cameraEnd);
    CurrentAnimation->SetOwnDuration(0.6);

    CurrentAnimation->StartTimer(0.0, 1.0, Standard_True, Standard_False);

    // Kick the update loop
    Fl::add_timeout(1.0 / 12.0, animation_update, 0);
}

//region occt buttons

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
			// cotm("an1")
			if (wrapper->is_setview) {
				// cotm(1);
				// view->SetProj(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
				// cotm(2)
				// view->SetUp(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz);

				end_proj_global = gp_Vec(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
				end_up_global = gp_Vec(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz); 
				colorisebtn(wrapper->idx);
				start_animation(); 
				// animate_flip_view(occv);
			}
			if (wrapper && wrapper->func) wrapper->func();
			// cotm(wrapper->label);

			// // Later, retrieve them
			// Standard_Real dx, dy, dz, ux, uy, uz;
			// view->Proj(dx, dy, dz);
			// view->Up(ux, uy, uz);
			// cotm(dx, dy, dz, ux, uy, uz); /////////////////////////
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
				 [] {
					 Standard_Real dx, dy, dz, ux, uy, uz;
					 view->Proj(dx, dy, dz);
					 view->Up(ux, uy, uz);
					 // Reverse the projection direction
					 end_proj_global = gp_Vec(-dx, -dy, -dz);
					 end_up_global = gp_Vec(ux, uy, uz);
					 start_animation();
				 }},

			sbts{"Align",
				 [] { 
					 GetAlignedCameraVectors(view,end_proj_global,end_up_global); 
					//  cotm2(end_proj_global,end_up_global);
					 start_animation();
				 }}
		};

		sbt.insert(sbt.end(), sbtstd.begin(), sbtstd.end());
	}
	
	vector<int> check_nearest_btn_idx();

	void colorisebtn(int idx) {
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

	
	
	void drawbuttons(float w, int hc1) { 

		std::function<void()> func = [] { cotm(m_initialized, sbt[0].label) };

		sbtset(0);

		float w1 = ceil(w / sbt.size()) + 0.0;

		lop(i, 0, sbt.size()) { 
			sbt[i].idx = i;
			sbt[i].occbtn = new Fl_Button(w1 * i, 0, w1, hc1, sbt[i].label.c_str());
			sbt[i].occbtn->callback(sbts::call, &sbt[i]);  // ✅ fixed here
		}
	}
	vector<int> check_nearest_btn_idx() {
		// Get current view orientation
		Standard_Real dx, dy, dz, ux, uy, uz;
		view->Proj(dx, dy, dz);
		view->Up(ux, uy, uz);

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


//region new
void toggle_shaded_transp(bool dont_toggle=0){
	if(!dont_toggle)hlr_on=!hlr_on;
ctx->RemoveAll(0); 
	for (int i = 0; i < vlua.size(); i++) {
		if(!vlua[i]->visible_hardcoded)continue;
		// locationsphere(vlua[i]);
		if(hlr_on){
			vlua[i]->ashape->SetDisplayMode(AIS_WireFrame);
		}else{
			vlua[i]->ashape->SetDisplayMode(AIS_Shaded);
		}
		if(i==vlua.size()-1)inteligentSet(vlua[i]);
		ctx->Display(vlua[i]->ashape, 0);
	}

	projectAndDisplayWithHLR();
	scaleball();
	occv->redraw();
	// view->Redraw();
	if (isloading) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event);
		help.gentime = to_string(elapsed.count()) + "ms";
		help.upd();
		isloading=0;
	}
	DrawTrihedron(ctx,current_part->current_location,GetViewportAspectRatio()[0]);
	// DrawTrihedron(ctx,current_part->shape.Location(),GetViewportAspectRatio()[0]);
}
	
//region browser
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
int browser_position=0;
void fillbrowser(void*) {
	cotm(vlua.size());
	// toggle_shaded_transp(1); 
	// if(vshapes.empty())return;
	perf();
	static int binit = 0; 
	if (!binit) {
		binit = 1;
		fbm->setCallback([](void* data, int code, void* fbm_) {
			luadraw* ld = (luadraw*)data;
			fl_browser_msv* fbm=(fl_browser_msv*)fbm_;
			std::cout << "Callback data_ptr=" << ld->name << ", code=" << code << std::endl;

			if (code == 2 && !fbm->isrightclick) {
				gopart(ld->currentline, ld->name); 
				return;
			}
			if (code == 2 && fbm->isrightclick) {
				FitViewToShape(view, ld->shape);
				// ld->FitViewToShape(ld->m_view, ld->shape);
			}

			// hide
			if (code == 0) {
				if (mhide[ld->name] == 0)
					mhide[ld->name] = 1;
				else if (mhide[ld->name] == 1)
					mhide[ld->name] = 0;
				browser_position=fbm->position();
				fillbrowser(0); 
				fbm->position(browser_position);
			}
			if (code == 1) {
				if (msolo[ld->name] == 0)
					msolo[ld->name] = 1;
				else if (msolo[ld->name] == 1)
					msolo[ld->name] = 0; 
				browser_position=fbm->position();
				fillbrowser(0); 
				fbm->position(browser_position);
			}
		}); 
	}
 
	bool are_anysolo = anysolo(); 

	std::vector<std::vector<bool>>
	on_off = fbm->on_off;

	fbm->clear_all();
	fbm->vcols = {{18, "@B31@C64", "@B64@C31"}, {18, "@B29@C64", "@B64@C29"}, {18, "", ""}};
	// fbm->vcols={{18,"@B31@C64","@B64@C31"},{18,"@B29@C64","@B64@C29"},{18,"","@B12@C7"}};
	// fbm->color(fl_rgb_color(220, 235, 255));
	fbm->init(); 
	// if(vlua.size()>0)vlua.back()->redisplay(); //regen
	lop(ij, 0, vlua.size()) {
		luadraw* ld = vlua[ij]; 

		fbm->addnew({"H", "S", ld->name});
		fbm->data(fbm->size(), (void*)ld);
		int line = fbm->size();

		ld->visible_hardcoded = 1;

		if (mhide[ld->name] == 1) {
			ld->visible_hardcoded = 0;
			fbm->toggleon(line, 0, 1, 0);
		} 
 
		// redisplay(ld);
	}

	if (are_anysolo) {
		lop(i, 0, vlua.size()) {
			luadraw* ldi = vlua[i];
			ldi->visible_hardcoded = 0;
			if (msolo[ldi->name] == 1) {
				ldi->visible_hardcoded = 1;
				fbm->toggleon(i + 1, 1, 1, 0);
			}
			// redisplay(ldi);
		}
	}
 
	toggle_shaded_transp(1);
	// occv->redraw();
	view->Redraw();
	perf("fillbrowser");
}



void fillbrowser1(void*) { 
		toggle_shaded_transp(1);
		return;
		// vshapes is not in sync vector
		//  cotm(vaShape.size(), vshapes.size());
		for (int i = 0; i < vlua.size(); i++) {
			// if (!ctx->IsDisplayed(vaShape[i]) || !vlua[i]->visible_hardcoded) continue; 
			vlua[i]->ashape->SetDisplayMode(AIS_WireFrame);
			// vlua[i]->ashape->SetDisplayMode(AIS_Shaded);
			ctx->Display(vlua[i]->ashape, 0);
			cotm(vlua[i]->name);

			// TopoDS_Shape s = vaShape[i]->Shape();
			// if (!vlua[i]->Origin.IsIdentity()) {
			// 	s = s.Located(TopLoc_Location(vlua[i]->Origin.Transformation()));
			// }
			// vshapes.push_back(s);  // not in sync

			// vshapes.push_back(vaShape[i]->Shape());
		}
		// cotm(vaShape.size(), vshapes.size());
		hlr_on=1;
		view->Redraw();
		view->FitAll();
	}



//region sol

#include <BOPAlgo_BOP.hxx>

// ---------- Forward Declaration ----------
TopoDS_Shape FuseSolidsRobust(const TopoDS_Shape& inputShape);
TopoDS_Shape FuseSolidsFast(const TopoDS_Shape& inputShape)
{
    TopTools_ListOfShape solids;

    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next())
    {
        solids.Append(exp.Current());
    }

    if (solids.IsEmpty())
        return TopoDS_Shape();

    if (solids.Extent() == 1)
        return solids.First();

    BOPAlgo_BOP bop;
    bop.SetArguments(solids);
    bop.SetOperation(BOPAlgo_FUSE);
    bop.SetFuzzyValue(1e-6);

    bop.Perform();

    if (bop.HasErrors())
        return TopoDS_Shape();

    TopoDS_Shape result = bop.Shape();

    // Only unify when needed
    ShapeUpgrade_UnifySameDomain unify(result, true, true, true);
    unify.Build();

    return unify.Shape();
}

void inteligentSet___(luadraw* currentpart)
{
    if (!currentpart || currentpart->ashape.IsNull()) return;

    const bool hasShape  = !currentpart->shape.IsNull();
    const bool hasCShape = !currentpart->cshape.IsNull();

    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid =
        hasShape && (currentpart->shape.ShapeType() == TopAbs_SOLID);

    TopoDS_Shape fusedSolids;

    try
    {
        // ===== EXACT ORIGINAL LOGIC =====

        if (hasCShape && shapeIsSolid)
        {
            // MUST fuse both (same as original)
            BRep_Builder builder;
            TopoDS_Compound toFuse;
            builder.MakeCompound(toFuse);
            builder.Add(toFuse, currentpart->cshape);
            builder.Add(toFuse, currentpart->shape);

            fusedSolids = FuseSolidsRobust(toFuse);

            // 🔴 critical fallback (preserved)
            if (fusedSolids.IsNull())
                fusedSolids = toFuse;
        }
        else if (hasCShape)
        {
            if (currentpart->cshape.ShapeType() == TopAbs_SOLID)
            {
                // EXACT match: no fuse
                fusedSolids = currentpart->cshape;
            }
            else
            {
                BRep_Builder builder;
                TopoDS_Compound toFuse;
                builder.MakeCompound(toFuse);
                builder.Add(toFuse, currentpart->cshape);

                fusedSolids = FuseSolidsFast(toFuse);

                // 🔴 critical fallback
                if (fusedSolids.IsNull())
                    fusedSolids = toFuse;
            }
        }
        else if (shapeIsSolid)
        {
            fusedSolids = currentpart->shape;
        }

        // ===== FINAL COMPOUND (unchanged logic) =====

        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        int childCount = 0;

        if (!fusedSolids.IsNull())
        {
            builder.Add(fcomp, fusedSolids);
            childCount++;
        }

        if (hasShape && !shapeIsSolid)
        {
            builder.Add(fcomp, currentpart->shape);
            childCount++;
        }

        if (childCount == 0) return;

        currentpart->fshape = fcomp;
        currentpart->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e)
    {
        std::cerr << "OCCT Error: "
                  << e.GetMessageString() << std::endl;
    }
}


// ---------- Optimized inteligentSet ----------

void inteligentSetexcelent(luadraw* part) 
{
    if (!part || part->ashape.IsNull()) return;

    const bool hasShape  = !part->shape.IsNull();
    const bool hasCShape = !part->cshape.IsNull();
    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid = hasShape && (part->shape.ShapeType() == TopAbs_SOLID);

    // try {
        TopTools_ListOfShape solidsList;
        int solidCount = 0;
        
        // 1. Single-pass fast collection of all solids
        if (hasCShape) {
            for (TopExp_Explorer exp(part->cshape, TopAbs_SOLID); exp.More(); exp.Next()) {
                solidsList.Append(exp.Current());
                solidCount++;
            }
        }
        
        // Immediately add the main shape to the fusion list if it's a solid
        if (shapeIsSolid) {
            solidsList.Append(part->shape);
            solidCount++;
        }

        TopoDS_Shape fusedSolids;

        // 2. Embedded & Inlined Fusion Logic
        if (solidCount == 1) {
            // Fast-path: Skip BOP and fixing entirely if there's only one solid
            fusedSolids = solidsList.First();
        } 
        else if (solidCount > 1) {
            TopTools_ListIteratorOfListOfShape it(solidsList);
            
            // Fix and set initial solid
            ShapeFix_Shape fixer(it.Value());
            fixer.Perform();
            fusedSolids = fixer.Shape();
            
            // Iterative fusion (keeps your requested BRepAlgoAPI_Fuse method)
// Iterative fusion (with Bounding Box optimization)
for (it.Next(); it.More(); it.Next()) {
    const TopoDS_Shape& nextRaw = it.Value();
    if (nextRaw.IsSame(fusedSolids)) continue; // Skip identical references

    ShapeFix_Shape nextFixer(nextRaw);
    nextFixer.Perform();
    const TopoDS_Shape& nextSolid = nextFixer.Shape();

    // --- NEW: Fast Bounding Box Check ---
    Bnd_Box boxFused, boxNext;
    
    // Calculate bounding boxes for both shapes
    BRepBndLib::Add(fusedSolids, boxFused);
    BRepBndLib::Add(nextSolid, boxNext);
    
    // Enlarge the first box slightly to account for your fuzzy value tolerance (1e-6)
    boxFused.Enlarge(1e-6);

    // Check if the boxes are completely outside of each other
    if (boxFused.IsOut(boxNext)) {
        // FAST PATH: Solids definitely do not touch.
        // Skip the heavy BOP and just combine them into a compound.
        TopoDS_Compound fastComp;
        BRep_Builder builder;
        builder.MakeCompound(fastComp);
        builder.Add(fastComp, fusedSolids);
        builder.Add(fastComp, nextSolid);
        
        fusedSolids = fastComp; 
    } 
    else {
        // SLOW PATH: Bounding boxes overlap, meaning solids MIGHT intersect.
        // We must fall back to the actual Boolean operation.
        cotm("intelBRepAlgoAPI_Fuse");
        BRepAlgoAPI_Fuse fuse(fusedSolids, nextSolid);
        fuse.SetFuzzyValue(1e-6);
        fuse.Build();
        
        // If it succeeds, update fusedSolids. Otherwise, fallback to previous valid state.
        if (fuse.IsDone()) {
            fusedSolids = fuse.Shape();
        }
    }
}

            // Unify domain once at the very end to clean up geometry
            ShapeUpgrade_UnifySameDomain unify(fusedSolids, true, true, true);
            unify.Build();
            fusedSolids = unify.Shape();
        }

        // 3. Build Final Compound
        // Early exit if we end up with no fused solids and no non-solid shape to append
        if (fusedSolids.IsNull() && (!hasShape || shapeIsSolid)) {
            return;
        }

        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        if (!fusedSolids.IsNull()) {
            builder.Add(fcomp, fusedSolids);
        }

        // If the original shape exists but wasn't a solid, append it now
        if (hasShape && !shapeIsSolid) {
            builder.Add(fcomp, part->shape);
        }

        // Apply final geometry updates
        part->fshape = fcomp;
        part->ashape->SetShape(fcomp);
    // }
    // catch (const Standard_Failure& e) {
    //     std::cerr << "OCCT Error in inteligentSet: " << e.GetMessageString() << std::endl;
    // }
}
void inteligentSetexcelent1(luadraw* part) 
{
    if (!part || part->ashape.IsNull()) return;

    const bool hasShape  = !part->shape.IsNull();
    const bool hasCShape = !part->cshape.IsNull();
    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid = hasShape && (part->shape.ShapeType() == TopAbs_SOLID);

    // try {
        TopTools_ListOfShape solidsList;
        int solidCount = 0;
        
        // 1. Single-pass fast collection of all solids
        if (hasCShape) {
            for (TopExp_Explorer exp(part->cshape, TopAbs_SOLID); exp.More(); exp.Next()) {
                solidsList.Append(exp.Current());
                solidCount++;
            }
        }
        
        // Immediately add the main shape to the fusion list if it's a solid
        if (shapeIsSolid) {
            solidsList.Append(part->shape);
            solidCount++;
        }

        TopoDS_Shape fusedSolids;

        // 2. Embedded & Inlined Fusion Logic
        if (solidCount == 1) {
            // Fast-path: Skip BOP and fixing entirely if there's only one solid
            fusedSolids = solidsList.First();
        } 
        else if (solidCount > 1) {
            TopTools_ListIteratorOfListOfShape it(solidsList);
            
            // Fix and set initial solid
            ShapeFix_Shape fixer(it.Value());
            fixer.Perform();
            fusedSolids = fixer.Shape();
            
            // Iterative fusion (keeps your requested BRepAlgoAPI_Fuse method)
            for (it.Next(); it.More(); it.Next()) {
                const TopoDS_Shape& nextRaw = it.Value();
                if (nextRaw.IsSame(fusedSolids)) continue; // Skip identical references

                ShapeFix_Shape nextFixer(nextRaw);
                nextFixer.Perform();
                const TopoDS_Shape& nextSolid = nextFixer.Shape();
cotm("intelBRepAlgoAPI_Fuse");
                BRepAlgoAPI_Fuse fuse(fusedSolids, nextSolid);
                // fuse.SetFuzzyValue(0.01);
                // fuse.SetFuzzyValue(1e-12);
                fuse.SetFuzzyValue(1e-6);
                fuse.Build();
                
                // If it succeeds, update fusedSolids. Otherwise, fallback to previous valid state.
                if (fuse.IsDone()) {
                    fusedSolids = fuse.Shape();
                }
            }

            // Unify domain once at the very end to clean up geometry
            ShapeUpgrade_UnifySameDomain unify(fusedSolids, true, true, true);
            unify.Build();
            fusedSolids = unify.Shape();
        }

        // 3. Build Final Compound
        // Early exit if we end up with no fused solids and no non-solid shape to append
        if (fusedSolids.IsNull() && (!hasShape || shapeIsSolid)) {
            return;
        }

        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        if (!fusedSolids.IsNull()) {
            builder.Add(fcomp, fusedSolids);
        }

        // If the original shape exists but wasn't a solid, append it now
        if (hasShape && !shapeIsSolid) {
            builder.Add(fcomp, part->shape);
        }

        // Apply final geometry updates
        part->fshape = fcomp;
        part->ashape->SetShape(fcomp);
    // }
    // catch (const Standard_Failure& e) {
    //     std::cerr << "OCCT Error in inteligentSet: " << e.GetMessageString() << std::endl;
    // }
}



void inteligentSetbest(luadraw* part)
{
    if (!part || part->ashape.IsNull()) return;

    const bool hasShape  = !part->shape.IsNull();
    const bool hasCShape = !part->cshape.IsNull();
    if (!hasShape && !hasCShape) return;

    TopoDS_Shape fusedSolids;
    const bool shapeIsSolid = hasShape && (part->shape.ShapeType() == TopAbs_SOLID);

    try {
        // === Fast Solid Fusion Logic ===
        if (hasCShape) {
            const bool cIsSolid = (part->cshape.ShapeType() == TopAbs_SOLID);

            // Case 1: cshape solid + shape solid → add + fuse immediately (fast path)
            if (shapeIsSolid) {
                AddToCompound(part->cshape, part->shape);
                fusedSolids = FuseSolidsRobust(part->cshape);
                if (fusedSolids.IsNull()) fusedSolids = part->cshape;
            } 
            // Case 2: cshape is solid, shape is not → skip fuse
            else if (cIsSolid) {
                fusedSolids = part->cshape;
            } 
            // Case 3: cshape is compound (non-solid)
            else {
                // Quick check for multiple solids (no full exploration)
                const int solidCount = part->cshape.TShape()->NbChildren();
                if (solidCount > 1) {
                    fusedSolids = FuseSolidsRobust(part->cshape);
                    if (fusedSolids.IsNull()) fusedSolids = part->cshape;
                } else fusedSolids = part->cshape;
            }
        } 
        // Case 4: only shape solid, no cshape
        else if (shapeIsSolid) {
            fusedSolids = part->shape;
        }

        // === Final Build Compound (fast branch) ===
        if (fusedSolids.IsNull() && (!hasShape || part->shape.IsNull())) return;

        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        if (!fusedSolids.IsNull()) builder.Add(fcomp, fusedSolids);
        if (hasShape && !shapeIsSolid) builder.Add(fcomp, part->shape);

        part->fshape = fcomp;
        part->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
}

void inteligentSetverygood2(luadraw* part) 
{
    if (!part || part->ashape.IsNull()) return;

    const bool hasShape   = !part->shape.IsNull();
    const bool hasCShape  = !part->cshape.IsNull();
    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid = hasShape && (part->shape.ShapeType() == TopAbs_SOLID);

    try {
        TopoDS_Shape fusedSolids;

        // === Fusion Logic ===
        if (hasCShape && shapeIsSolid) {
            // Only fuse when both are non-null and at least one isn't a pure solid 
			AddToCompound(part->cshape,part->shape);
            fusedSolids = FuseSolidsRobust(part->cshape);
            if (fusedSolids.IsNull()) fusedSolids = part->cshape;
        } 
        else if (hasCShape) {
            const bool cIsSolid = (part->cshape.ShapeType() == TopAbs_SOLID);
            if (cIsSolid) {
                fusedSolids = part->cshape;
            } else {
                // Only fuse if it contains multiple solids
                int solidCount = current_part->cshape.TShape()->NbChildren();
                // int solidCount = 0;
                // for (TopExp_Explorer exp(part->cshape, TopAbs_SOLID); exp.More(); exp.Next())
                //     if (++solidCount > 1) break;

                if (solidCount > 1) {
                    fusedSolids = FuseSolidsRobust(part->cshape);
                    if (fusedSolids.IsNull()) fusedSolids = part->cshape;
                } else {
                    fusedSolids = part->cshape;
                }
            }
        } 
        else if (shapeIsSolid) {
            fusedSolids = part->shape;
        }

        // === Build Final Compound ===
        if (fusedSolids.IsNull() && (!hasShape || part->shape.IsNull()))
            return;

        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        if (!fusedSolids.IsNull())
            builder.Add(fcomp, fusedSolids);

        if (hasShape && !shapeIsSolid)
            builder.Add(fcomp, part->shape);

        part->fshape = fcomp;
        part->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
}


void inteligentSetverygood(luadraw* part) 
{
    if (!part || part->ashape.IsNull()) return;

    const bool hasShape   = !part->shape.IsNull();
    const bool hasCShape  = !part->cshape.IsNull();
    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid = hasShape && (part->shape.ShapeType() == TopAbs_SOLID);

    try {
        TopoDS_Shape fusedSolids;

        // === Fusion Logic ===
        if (hasCShape && shapeIsSolid) {
            // Only fuse when both are non-null and at least one isn't a pure solid
            BRep_Builder builder;
            TopoDS_Compound compound;
            builder.MakeCompound(compound);
            builder.Add(compound, part->cshape);
            builder.Add(compound, part->shape);
            fusedSolids = FuseSolidsRobust(compound);
            if (fusedSolids.IsNull()) fusedSolids = compound;
        } 
        else if (hasCShape) {
            const bool cIsSolid = (part->cshape.ShapeType() == TopAbs_SOLID);
            if (cIsSolid) {
                fusedSolids = part->cshape;
            } else {
                // Only fuse if it contains multiple solids
                int solidCount = 0;
                for (TopExp_Explorer exp(part->cshape, TopAbs_SOLID); exp.More(); exp.Next())
                    if (++solidCount > 1) break;

                if (solidCount > 1) {
                    fusedSolids = FuseSolidsRobust(part->cshape);
                    if (fusedSolids.IsNull()) fusedSolids = part->cshape;
                } else {
                    fusedSolids = part->cshape;
                }
            }
        } 
        else if (shapeIsSolid) {
            fusedSolids = part->shape;
        }

        // === Build Final Compound ===
        if (fusedSolids.IsNull() && (!hasShape || part->shape.IsNull()))
            return;

        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        if (!fusedSolids.IsNull())
            builder.Add(fcomp, fusedSolids);

        if (hasShape && !shapeIsSolid)
            builder.Add(fcomp, part->shape);

        part->fshape = fcomp;
        part->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
}

// ---------- Optimized FuseSolidsRobust ----------
TopoDS_Shape FuseSolidsRobust(const TopoDS_Shape& inputShape)
{
	perf1("");
    TopTools_ListOfShape solids;
    solids.Clear();

    // Collect valid solids, skipping empty/faulty shapes
    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next()) {
        const auto& s = exp.Current();
        if (!s.IsNull()) {
            ShapeFix_Shape fixer(s);
            fixer.Perform();
            solids.Append(fixer.Shape());
        }
    }

    if (solids.IsEmpty()) 
        return TopoDS_Shape();

    // Short fast-path for single solid (skip BOP)
    if (solids.Extent() == 1)
        return solids.First();

    // Iterative, stable fusion
    TopoDS_Shape result = solids.First();
    for (TopTools_ListIteratorOfListOfShape it(solids); it.More(); it.Next()) {
        const TopoDS_Shape& nextSolid = it.Value();
        if (nextSolid.IsSame(result)) continue; // skip identical

        BRepAlgoAPI_Fuse fuse(result, nextSolid);
        fuse.SetFuzzyValue(1e-6);
        fuse.Build();
        if (!fuse.IsDone())
            return result; // Keep partial if fuse failed (faster fallback)

        result = fuse.Shape();
    }

    // Simplify topology domains
    ShapeUpgrade_UnifySameDomain unify(result, true, true, true);
    unify.Build();
	perf1("FuseSolidsRobust");
    return unify.Shape();
}


#include <TopExp_Explorer.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopTools_ListOfShape.hxx>

TopoDS_Shape FuseSolidsRobustp(const TopoDS_Shape& inputShape)
{
    TopTools_ListOfShape args;
    TopTools_ListOfShape tools;
    bool isFirst = true;

    // 1. Collect and conditionally fix solids
    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next())
    {
        TopoDS_Shape currentSolid = exp.Current();
        
        // OPTIMIZATION 1: Conditional Fixing
        // ShapeFix_Shape is extremely heavy. We only run it if the shape is actually invalid.
        // BRepCheck_Analyzer is significantly faster than a blind fix.
        BRepCheck_Analyzer checker(currentSolid);
        if (!checker.IsValid()) 
        {
            ShapeFix_Shape fixer(currentSolid);
            fixer.Perform();
            currentSolid = fixer.Shape();
        }

        // Separate the first solid into 'arguments' and the rest into 'tools'
        if (isFirst) {
            args.Append(currentSolid);
            isFirst = false;
        } else {
            tools.Append(currentSolid);
        }
    }

    if (args.IsEmpty()) return TopoDS_Shape();
    
    // If there was only one solid, skip fusion entirely
    if (tools.IsEmpty()) {
        ShapeUpgrade_UnifySameDomain unifier(args.First(), true, true, true);
        unifier.Build();
        return unifier.Shape();
    }

    // 2. Multi-Argument Fusion (MASSIVE OPTIMIZATION)
    BRepAlgoAPI_Fuse fuse;
    fuse.SetArguments(args);
    fuse.SetTools(tools);
    fuse.SetFuzzyValue(1e-6);
    
    // OPTIMIZATION 2: Multi-threading
    // BOP algorithms in OCCT support parallel processing
    fuse.SetRunParallel(Standard_True); 
    
    fuse.Build();

    if (!fuse.IsDone())
        return TopoDS_Shape();

    TopoDS_Shape result = fuse.Shape();

    // 3. Clean result
    ShapeUpgrade_UnifySameDomain unifier(result, true, true, true);
    unifier.Build();
    
    return unifier.Shape();
}





#include <vector>
#include <unordered_map>
#include <iostream>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <Standard_Failure.hxx>

void inteligentSetnotgood(luadraw* currentpart)
{
    if (!currentpart || currentpart->ashape.IsNull()) return;

    const TopoDS_Shape& shape  = currentpart->shape;
    TopoDS_Compound& cshape    = currentpart->cshape;

    const bool hasShape  = !shape.IsNull();
    const bool hasCShape = !cshape.IsNull();

    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid = hasShape && (shape.ShapeType() == TopAbs_SOLID);

    // Assuming AddToCompound is your internal helper
    if(shapeIsSolid) AddToCompound(cshape, shape); 

    TopoDS_Shape fusedSolids;

    try
    {
        // ===== 1. COLLECT SOLIDS =====
        std::vector<TopoDS_Shape> solids;
        solids.reserve(16);

        if (hasCShape)
        {
            for (TopExp_Explorer exp(cshape, TopAbs_SOLID); exp.More(); exp.Next())
                solids.push_back(exp.Current());
        }

        // ===== 2. PROCESS SOLIDS =====
        if (!solids.empty())
        {
            if (solids.size() == 1)
            {
                fusedSolids = solids[0];
            }
            else
            {
                // ===== 3. BUILD BOUNDING BOXES (OPTIMIZED) =====
                std::vector<Bnd_Box> boxes(solids.size());
                for (size_t i = 0; i < solids.size(); ++i)
                {
                    // AddOptimal is much faster than Add if a triangulation exists
                    BRepBndLib::AddOptimal(solids[i], boxes[i], Standard_True, Standard_False);
                }

                // ===== 4. UNION-FIND (BROAD PHASE) =====
                std::vector<int> parent(solids.size());
                for (size_t i = 0; i < parent.size(); ++i) parent[i] = (int)i;

                auto find = [&](int x) {
                    int root = x;
                    while (parent[root] != root) root = parent[root];
                    while (x != root) { int next = parent[x]; parent[x] = root; x = next; } // Path compression
                    return root;
                };

                auto unite = [&](int a, int b) {
                    int rootA = find(a);
                    int rootB = find(b);
                    if (rootA != rootB) parent[rootB] = rootA;
                };

                for (size_t i = 0; i < solids.size(); ++i) {
                    for (size_t j = i + 1; j < solids.size(); ++j) {
                        if (!boxes[i].IsOut(boxes[j])) unite((int)i, (int)j);
                    }
                }

                // ===== 5. BUILD CLUSTERS =====
                std::unordered_map<int, std::vector<int>> clusters;
                for (size_t i = 0; i < solids.size(); ++i)
                    clusters[find((int)i)].push_back((int)i);

                // ===== 6. DIVIDE & CONQUER FUSION (NARROW PHASE) =====
                BRep_Builder builder;
                TopoDS_Compound comp;
                builder.MakeCompound(comp);

                for (auto& kv : clusters)
                {
                    const std::vector<int>& idxs = kv.second;

                    if (idxs.size() == 1)
                    {
                        builder.Add(comp, solids[idxs[0]]);
                        continue;
                    }

                    // Tree Fusion Setup
                    std::vector<TopoDS_Shape> currentLayer;
                    currentLayer.reserve(idxs.size());
                    for (int idx : idxs) currentLayer.push_back(solids[idx]);

                    bool failed = false;

                    // Reduce pairs of shapes recursively (O(N log N) instead of O(N^2))
                    while (currentLayer.size() > 1 && !failed)
                    {
                        std::vector<TopoDS_Shape> nextLayer;
                        nextLayer.reserve((currentLayer.size() / 2) + 1);

                        for (size_t i = 0; i < currentLayer.size(); i += 2)
                        {
                            if (i + 1 < currentLayer.size())
                            {
                                BRepAlgoAPI_Fuse fuse(currentLayer[i], currentLayer[i+1]);
                                fuse.SetFuzzyValue(1e-6);
                                fuse.SetRunParallel(Standard_True); // Enable multi-threading in BOP
                                fuse.Build();

                                if (!fuse.IsDone()) {
                                    failed = true;
                                    break;
                                }
                                nextLayer.push_back(fuse.Shape());
                            }
                            else
                            {
                                nextLayer.push_back(currentLayer[i]); // Carry over the odd one out
                            }
                        }
                        currentLayer = std::move(nextLayer);
                    }

                    // ===== 7. REFINEMENT & FALLBACK =====
                    if (!failed && !currentLayer.empty())
                    {
                        // Unify Same Domain on the fully fused cluster
                        ShapeUpgrade_UnifySameDomain unify(currentLayer[0], true, true, true);
                        unify.Build();
                        builder.Add(comp, unify.Shape());
                    }
                    else
                    {
                        // Fallback: keep originals if ANY fuse operation failed
                        for (int idx : idxs) builder.Add(comp, solids[idx]);
                    }
                }

                fusedSolids = comp;
            }
        }

        // ===== 8. FINAL COMPOUND =====
        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        int childCount = 0;

        if (!fusedSolids.IsNull())
        {
            builder.Add(fcomp, fusedSolids);
            childCount++;
        }

        if (hasShape && !shapeIsSolid)
        {
            builder.Add(fcomp, shape);
            childCount++;
        }

        if (childCount == 0) return;

        currentpart->fshape = fcomp;
        currentpart->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e)
    {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
}
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
#include <Quantity_Color.hxx>
#include <AIS_InteractiveContext.hxx>

 
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Iterator.hxx>
#include <BRep_Builder.hxx>
#include <BOPAlgo_BOP.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Iterator.hxx>
#include <BOPAlgo_Builder.hxx>
#include <TopExp_Explorer.hxx>

#include <BRepAlgoAPI_Fuse.hxx>

#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_ListOfShape.hxx>

TopoDS_Shape FuseAndRefine(const TopoDS_Compound& original)
{
    // 1. Perform General Fusion
    // This merges all shapes in the compound into a single manifold result.
    BOPAlgo_Builder builder;
    
    // Add the compound as a single argument (or iterate through sub-shapes)
    builder.AddArgument(original);
    builder.SetFuzzyValue(1e-6); // Slight tolerance for robustness
    builder.Perform();

    if (builder.HasErrors()) {
        return TopoDS_Shape();
    }

    TopoDS_Shape fusedResult = builder.Shape();

    // 2. Refine the Shape (Unify Same Domain)
    // This removes "seams" between coplanar faces and redundant edges.
    ShapeUpgrade_UnifySameDomain unifier;
    unifier.Initialize(fusedResult, Standard_True, Standard_True, Standard_True);
    unifier.Build();

    return unifier.Shape();
}
TopoDS_Compound CopyCompoundAndAdd(const TopoDS_Compound& original,
                                   const TopoDS_Shape& toAdd)
{
    BRep_Builder builder;

    TopoDS_Compound result;
    builder.MakeCompound(result);

    // Copy existing children (shallow copy of handles)
    for (TopoDS_Iterator it(original); it.More(); it.Next())
    {
        builder.Add(result, it.Value());
    }

    // Add new shape
    builder.Add(result, toAdd);

    return result;
}
#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <TopTools_ListOfShape.hxx>

#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepCheck_Analyzer.hxx>

TopoDS_Shape FuseSolidsRefined(const TopoDS_Shape& inputShape)
{
    TopTools_ListOfShape solids;
    
    // 1. Extract and FIX solids
    // Sometimes inputs have tiny gaps or self-intersections that break the BOP
    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next())
    {
        ShapeFix_Shape fixer(exp.Current());
        fixer.Perform();
        solids.Append(fixer.Shape());
    }

    if (solids.IsEmpty()) return TopoDS_Shape();

    // 2. Perform General Fusion with a more "forgiving" fuzzy value
    BOPAlgo_Builder builder;
    builder.SetArguments(solids);
    
    // If your shapes are slightly misaligned, 1e-6 is too tight. 
    // Try 1e-4 or even 1e-3 if they are "dirty" models.
    builder.SetFuzzyValue(1e-4); 
    builder.Perform();

    if (builder.HasErrors()) return TopoDS_Shape();

    TopoDS_Shape fusedResult = builder.Shape();

    // 3. Aggressive Refinement
    // We initialize with 'Standard_True' for all three: 
    // Unify Faces, Unify Edges, and Remove Linear Vertices.
    ShapeUpgrade_UnifySameDomain unifier;
    unifier.Initialize(fusedResult, Standard_True, Standard_True, Standard_True);
    
    // This allows the unifier to merge faces even if the 
    // underlying surfaces are slightly different (within tolerance)
    unifier.SetSafeInputMode(Standard_True);
    unifier.Build();

    TopoDS_Shape finalShape = unifier.Shape();
TopExp_Explorer exp(finalShape, TopAbs_SOLID);
int solidCount = 0;
for (; exp.More(); exp.Next()) solidCount++;

std::cout << "Resulting solids: " << solidCount << std::endl;
    // 4. Final Pass: Fix any orientation issues caused by the fusion
    ShapeFix_Shape finalFixer(finalShape);
    finalFixer.Perform();

    return finalFixer.Shape();
}
#include <BOPAlgo_GlueEnum.hxx>

TopoDS_Shape FuseSolidsAggressive(const TopoDS_Shape& inputShape)
{
    TopTools_ListOfShape solids;
    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next())
    {
        // 1. Repair inputs first
        ShapeFix_Shape fixer(exp.Current());
        fixer.Perform();
        solids.Append(fixer.Shape());
    }

    if (solids.IsEmpty()) return TopoDS_Shape();

    BOPAlgo_Builder builder;
    builder.SetArguments(solids);

    // 2. Increase tolerance significantly to bridge gaps
    builder.SetFuzzyValue(1e-3); 

    // 3. Enable Gluing for face-to-face contacts
    // BOPAlgo_GlueFull is the most aggressive gluing mode
    builder.SetGlue(BOPAlgo_GlueFull);
    
    builder.Perform();

    if (builder.HasErrors()) return TopoDS_Shape();

    TopoDS_Shape result = builder.Shape();

    // 4. Clean up the result
    ShapeUpgrade_UnifySameDomain unifier;
    unifier.Initialize(result, Standard_True, Standard_True, Standard_True);
    unifier.SetSafeInputMode(Standard_True);
    unifier.Build();

    return unifier.Shape();
}
TopoDS_Shape FuseSolidsRobustgood(const TopoDS_Shape& inputShape)
{
    TopTools_ListOfShape solids;

    // 1. Collect + fix solids
    for (TopExp_Explorer exp(inputShape, TopAbs_SOLID); exp.More(); exp.Next())
    {
        ShapeFix_Shape fixer(exp.Current());
        fixer.Perform();
        solids.Append(fixer.Shape());
    }

    if (solids.IsEmpty()) return TopoDS_Shape();

    // 2. Iterative fuse (much more stable)
    TopoDS_Shape result = solids.First();
    TopTools_ListIteratorOfListOfShape it(solids);
    it.Next();

    for (; it.More(); it.Next())
    {
        BRepAlgoAPI_Fuse fuse(result, it.Value());

        // moderate tolerance
        fuse.SetFuzzyValue(1e-6);

        fuse.Build();
        if (!fuse.IsDone())
            return TopoDS_Shape();

        result = fuse.Shape();
    }

    // 3. Clean result
    ShapeUpgrade_UnifySameDomain unifier(result, true, true, true);
    unifier.Build();

    return unifier.Shape();
}
void inteligentset(bool upd_loc=1){}
// void inteligentSet(luadraw* current_part){}
// void inteligentset(bool upd_loc=1)

void inteligentSet_(luadraw* current_part)
{
    if (!current_part || current_part->ashape.IsNull()) return;

    const bool hasShape = !current_part->shape.IsNull();
    const bool hasCShape = !current_part->cshape.IsNull();
    
    // 1. Determine if the main shape is a solid
    const bool shapeIsSolid = hasShape && (current_part->shape.ShapeType() == TopAbs_SOLID);

    try 
    {
        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);

        // 2. Handle the "Fusion" group
        // We always want to fuse everything inside cshape. 
        // If 'shape' is solid, we include it in this fusion.
        TopoDS_Compound toFuse;
        builder.MakeCompound(toFuse);
        bool needsFusion = false;

        if (hasCShape) {
            builder.Add(toFuse, current_part->cshape);
            needsFusion = true;
        }

        if (shapeIsSolid) {
            builder.Add(toFuse, current_part->shape);
            needsFusion = true;
        }

        // Perform fusion if we have solids
        TopoDS_Shape fusedSolids;
        if (needsFusion) {
            fusedSolids = FuseSolidsAggressive(toFuse);
            if (fusedSolids.IsNull()) fusedSolids = toFuse; // Fallback to raw compound if fuse fails
            builder.Add(fcomp, fusedSolids);
        }

        // 3. Handle the "Non-Solid" group
        // If 'shape' exists but isn't a solid, it bypasses fusion and goes straight to the result
        if (hasShape && !shapeIsSolid) {
            builder.Add(fcomp, current_part->shape);
        }

        // 4. Finalize
        if (fcomp.IsNull() || fcomp.TShape()->NbChildren() == 0) return;

        current_part->fshape = fcomp;
        current_part->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
}
void inteligentSetgood(luadraw* current_part)
{
    // 1. Early fast-path exits
    if (!current_part || current_part->ashape.IsNull()) return;

    const bool hasShape = !current_part->shape.IsNull();
    const bool hasCShape = !current_part->cshape.IsNull();

    // If both are missing, skip the try block entirely
    if (!hasShape && !hasCShape) return;

    const bool shapeIsSolid = hasShape && (current_part->shape.ShapeType() == TopAbs_SOLID);

    try 
    {
        TopoDS_Shape fusedSolids;
        
        // 2. OPTIMIZATION: Avoid Boolean Operations (FuseSolidsRobust) when unnecessary.
        // BOPs are incredibly expensive. We only fuse if there are actually multiple bodies.
        if (hasCShape && shapeIsSolid) 
        {
            // Both exist: Fusion is strictly required
            BRep_Builder builder;
            TopoDS_Compound toFuse;
            builder.MakeCompound(toFuse);
            builder.Add(toFuse, current_part->cshape);
            builder.Add(toFuse, current_part->shape);
            
            fusedSolids = FuseSolidsRobust(toFuse);
            if (fusedSolids.IsNull()) fusedSolids = toFuse; 
        } 
        else if (hasCShape) 
        {
            // Only cshape is present. 
            // If it's already a single solid, it physically cannot fuse with anything!
            if (current_part->cshape.ShapeType() == TopAbs_SOLID) {
                fusedSolids = current_part->cshape;
            } else {
                // It's likely a compound, fuse it just in case it contains internal overlapping bodies.
                BRep_Builder builder;
                TopoDS_Compound toFuse;
                builder.MakeCompound(toFuse);
                builder.Add(toFuse, current_part->cshape);
                
                fusedSolids = FuseSolidsRobust(toFuse);
                if (fusedSolids.IsNull()) fusedSolids = toFuse;
            }
        } 
        else if (shapeIsSolid) 
        {
            // Only shape is present and it's a solid. No fusion needed.
            fusedSolids = current_part->shape;
        }

        // 3. Build the Final Compound
        BRep_Builder builder;
        TopoDS_Compound fcomp;
        builder.MakeCompound(fcomp);
        
        // We track children manually to avoid querying TShape() later
        int childCount = 0; 

        if (!fusedSolids.IsNull()) {
            builder.Add(fcomp, fusedSolids);
            childCount++;
        }

        // 4. Handle Non-Solid group
        if (hasShape && !shapeIsSolid) {
            builder.Add(fcomp, current_part->shape);
            childCount++;
        }

        // 5. Finalize using our manual counter (much faster than traversing OCCT hierarchy)
        if (childCount == 0) return;

        current_part->fshape = fcomp;
        current_part->ashape->SetShape(fcomp);
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
}
// fast
void inteligentSetFast(luadraw* current_part) {
    if (!current_part || current_part->cshape.IsNull() || current_part->shape.IsNull()) return;

    bool newShapeIsSolid = (current_part->shape.ShapeType() == TopAbs_SOLID);
    
    // 1. Passada única para coletar shapes e localizar o último sólido
    std::vector<TopoDS_Shape> shapes;
    int lastSolidIndex = -1;
    int currentIndex = 0;

    for (TopoDS_Iterator it(current_part->cshape); it.More(); it.Next()) {
        const TopoDS_Shape& s = it.Value();
        if (s.ShapeType() == TopAbs_SOLID) {
            lastSolidIndex = currentIndex;
        }
        shapes.push_back(s);
        currentIndex++;
    }

    BRep_Builder B;
    TopoDS_Compound finalComp;
    B.MakeCompound(finalComp);

    // 2. Lógica de Filtragem Corrigida
    for (int i = 0; i < (int)shapes.size(); ++i) {
        const TopoDS_Shape& s = shapes[i];
        bool currentIsSolid = (s.ShapeType() == TopAbs_SOLID);

        if (newShapeIsSolid) {
            // REGRA: Se a nova shape é sólida, limpa TUDO que não for sólido do passado
            if (currentIsSolid) {
                B.Add(finalComp, s);
            }
        } 
        else {
            // REGRA: Se a nova shape NÃO é sólida (é rastro)
            if (lastSolidIndex == -1) {
                // Se não existem sólidos no histórico, mantém tudo (modo rastro livre)
                B.Add(finalComp, s);
            } else {
                // Se existem sólidos, mantém os sólidos + o rastro após o último
                if (currentIsSolid || i > lastSolidIndex) {
                    B.Add(finalComp, s);
                }
            }
        }
    }

    // 3. Adiciona a shape atual e finaliza
    B.Add(finalComp, current_part->shape);
    current_part->fshape = finalComp;
	current_part->ashape->SetShape(current_part->fshape);
}
void inteligentSetFast2(luadraw* current_part) {
    // 1. Validações básicas
    if (!current_part || current_part->cshape.IsNull() || current_part->shape.IsNull()) return;

    // 2. Coleta e localização do último sólido em uma única passada
    std::vector<TopoDS_Shape> shapes;
    int lastSolidIndex = -1;
    int currentIndex = 0;

    for (TopoDS_Iterator it(current_part->cshape); it.More(); it.Next()) {
        const TopoDS_Shape& s = it.Value();
        if (s.ShapeType() == TopAbs_SOLID) {
            lastSolidIndex = currentIndex;
        }
        shapes.push_back(s);
        currentIndex++;
    }

    // 3. Preparação do resultado
    BRep_Builder B;
    TopoDS_Compound finalComp;
    B.MakeCompound(finalComp);

    // 4. Lógica de Filtragem
    if (lastSolidIndex == -1) {
        // CASO SEM SÓLIDOS: Adiciona absolutamente tudo do cshape
        for (const auto& s : shapes) {
            B.Add(finalComp, s);
        }
    } else {
        // CASO COM SÓLIDOS: Aplica a regra de Sólidos + Rastro Final
        for (int i = 0; i < (int)shapes.size(); ++i) {
            if (shapes[i].ShapeType() == TopAbs_SOLID || i > lastSolidIndex) {
                B.Add(finalComp, shapes[i]);
            }
        }
    }

    // 5. Adiciona a shape de trabalho atual e finaliza
    B.Add(finalComp, current_part->shape);
    current_part->fshape = finalComp;
	current_part->ashape->SetShape(current_part->fshape);
}
// fast
void inteligentSetFast1(luadraw* current_part) {
	if (current_part->ashape.IsNull()) return;
	if (current_part->shape.IsNull()) return;

    // Collect solids
    TopTools_ListOfShape solids=TopTools_ListOfShape();
    for (TopExp_Explorer ex(current_part->cshape, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }

	if(solids.IsEmpty()){
		for (TopExp_Explorer ex(current_part->cshape, TopAbs_FACE); ex.More(); ex.Next()) {
			solids.Append(ex.Current());
		}
	}

    // If multiple solids, pack them into a new compound
    BRep_Builder B;
    TopoDS_Compound comp;
    B.MakeCompound(comp);

    for (TopTools_ListIteratorOfListOfShape it(solids); it.More(); it.Next()) {
        B.Add(comp, it.Value());
    }
	B.Add(comp,current_part->shape);

	current_part->fshape=comp;




// current_part->fshape=KeepOnlySolids(current_part->cshape);
// AddToCompound(current_part->fshape,current_part->shape);

// 	bool is3d=0;
// 	for (TopExp_Explorer exp(toclone->cshape, TopAbs_SOLID); exp.More(); exp.Next()) {
//         is3d=1;break;
//     }
// 	const bool shapeIsSolid = (current_part->shape.ShapeType() == TopAbs_SOLID);

// 	if()
// current_part->fshape= CopyCompoundAndAdd(current_part->cshape, current_part->shape);

// // auto solids=ExtractSolids(current_part->fshape);
// if (is3d){
// 	current_part->fshape=KeepOnlySolids(current_part->cshape);
// }

// current_part->ashape->Set(current_part->fshape);
current_part->ashape->SetShape(current_part->fshape);
return;
	int compound_count = current_part->cshape.TShape()->NbChildren();
	bool currshapesolid=current_part->shape.ShapeType() == TopAbs_SOLID;
	cotm(currshapesolid);
	// cotm("inteligentSet", compound_count);
	TopoDS_Shape first;
	if (compound_count==1){
		first = TopExp_Explorer(current_part->cshape, TopAbs_SHAPE).Current();
	}
	// if(!scomp.IsNull())
	//  builder.Add(comp, scomp);
	if (1 && (compound_count>=1  && (!first.IsNull()) && currshapesolid)) {
		TopoDS_Compound comp = CopyCompoundAndAdd(current_part->cshape, current_part->shape);
		// if(!current_part->cshape.IsNull()){
		// cotm(888);
		TopoDS_Shape scomp = FuseSolidsRobust(comp);

		// CRITICAL: Check if the shape is null before adding it
		if (scomp.IsNull()) {
			// Handle error: maybe the fusion failed or there were no solids
			// std::cerr << "Error: Fusion returned a null shape!" << std::endl;
			current_part->fshape = comp;
		} else {
			BRep_Builder builder;
			TopoDS_Compound fcomp;
			builder.MakeCompound(fcomp);
			builder.Add(fcomp, scomp);
			current_part->fshape = fcomp;
			// cotm("success");
		}
		// current_part->ashape->SetShape(current_part->fshape);
	}else if((1 && (compound_count>=1    && !currshapesolid))){
		cotm(999);
				TopoDS_Shape scomp = FuseSolidsRobust(current_part->cshape);

		 {
			BRep_Builder builder;
			TopoDS_Compound fcomp;
			builder.MakeCompound(fcomp);
			builder.Add(fcomp, scomp);
			builder.Add(fcomp, current_part->shape);
			current_part->fshape = fcomp;
			// cotm("success");
		}
	}
	
	
	else {
					BRep_Builder builder;
			TopoDS_Compound fcomp;
			builder.MakeCompound(fcomp);
			if(current_part->cshape.TShape()->NbChildren()==1){
			TopoDS_Shape first = TopExp_Explorer(current_part->cshape, TopAbs_SHAPE).Current();
			builder.Add(fcomp,first);
			}
			builder.Add(fcomp, current_part->shape);
			current_part->fshape = fcomp;
		current_part->fshape = fcomp;
		// current_part->ashape->SetShape(comp);
		// current_part->fshape = comp;
		// current_part->ashape->SetShape(comp);
	}
	current_part->ashape->SetShape(current_part->fshape);

	// locationsphere(current_part);
	// current_part->current_location = comp.Location();

	bool is_solid = (!current_part->shape.IsNull() && current_part->shape.ShapeType() == TopAbs_SOLID);
	// if(upd_loc)current_part->current_location = current_part->shape.Location();

	// if(current_part->shape.ShapeType() != TopAbs_COMPOUND)current_part->current_location =
	// current_part->shape.Location();


	// cotm("rotateint");
	// PrintLocationDegrees(current_part->shape.Location());

return;




    TopoDS_Shape finalDisplay;

    try 
    {
        if (!current_part->cshape.IsNull() && !current_part->shape.IsNull())
        { 
            TopoDS_Compound comp=CopyCompoundAndAdd(current_part->cshape,current_part->shape);
            finalDisplay = comp;
        }
        else if (!current_part->cshape.IsNull())
            finalDisplay = current_part->cshape;
        else if (!current_part->shape.IsNull())
            finalDisplay = current_part->shape;
        else
            return;

        if (finalDisplay.IsNull()) return;

        // === Critical: clear old presentation before changing shape ===
        // if (context)
        //     context->Remove(current_part->ashape, Standard_False);   // remove old one

        current_part->ashape->SetShape(finalDisplay);
        // current_part->ashape->SetLocalTransformation(gp_Trsf());
// current_part->current_location = finalDisplay.Location();
        // if (context)
        // {
        //     context->Display(current_part->ashape, Standard_False);
        //     context->RecomputePrsOnly(current_part->ashape, Standard_True, Standard_True);
        //     context->UpdateCurrentViewer();
        // }
        // else
        // {
        //     current_part->ashape->Redisplay();
        // }
    }
    catch (const Standard_Failure& e)
    {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "std exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception in inteligentSet() - most likely during shaded presentation" << std::endl;
    }
}

// void inteligentmerge(TopoDS_Shape newshape){
// 	bool nsis_solid=newshape.ShapeType() == TopAbs_SOLID;

// 	if(!current_part->fshape.IsNull()){
// 		if (nsis_solid && current_part->vshape[1].IsNull()) {
// 			current_part->vshape[0]=current_part->fshape;	
// 			current_part->vshape[1]=newshape;	
// 			current_part->fshape=BRepAlgoAPI_Fuse(current_part->vshape[0], current_part->vshape[1]);
// 		}

// 	}


// 	// if(!current_part->fshape.IsNull() && current_part->vshape[0].IsNull()){
// 	// 	current_part->shape=newshape;
// 	// 	return;
// 	// }

// 	if(current_part->fshape.IsNull() && nsis_solid){
// 		current_part->fshape=newshape;
// 		return;
// 	}

// 	if(current_part->fshape.IsNull()){
// 		current_part->shape=newshape;
// 		return;
// 	}
// }
// void inteligentset(){
// 	if(!current_part->shape.IsNull() && !current_part->fshape.IsNull()){
// 		BRep_Builder builder;
// 		TopoDS_Compound compound;
// 		builder.MakeCompound(compound);
// 		builder.Add(compound, current_part->fshape);
// 		builder.Add(compound, current_part->shape);
// 		current_part->ashape->Set(compound);
// 		return;
// 	}
// 	if(current_part->shape.IsNull() && !current_part->fshape.IsNull()){
// 		current_part->ashape->Set(current_part->fshape);
// 		return;
// 	}
// 	if(!current_part->shape.IsNull() && current_part->fshape.IsNull()){
// 		current_part->ashape->Set(current_part->shape);
// 		return;
// 	}	
// }

void inteligentSet(luadraw* current_part){
	if(autorefined)inteligentSetexcelent(current_part);
	else inteligentSetFast(current_part);
}
bool lua_var_exists(const std::string& name) {
    sol::state& L = *G;
    sol::object v = L.globals()[name];
    return v.valid();
}
#include <utility>

std::pair<TopoDS_Shape, TopLoc_Location> GetLastFromCompound(const TopoDS_Compound& aCompound) {
    TopoDS_Iterator it(aCompound);
    TopoDS_Shape last;

    for (; it.More(); it.Next()) {
        last = it.Value();
    }

    if (last.IsNull()) {
        return { TopoDS_Shape(), TopLoc_Location() };
    }

    return { last, last.Location() };
}
TopLoc_Location LastShapeLocation(const TopoDS_Compound& compound)
{
    TopLoc_Location lastLoc;

    for (TopExp_Explorer exp(compound, TopAbs_SHAPE); exp.More(); exp.Next())
    {
        lastLoc = exp.Current().Location();
		// return lastLoc;
    }

    return lastLoc;
}
TopoDS_Shape LastShape(const TopoDS_Compound& compound)
{
    TopoDS_Shape lastLoc;

    for (TopExp_Explorer exp(compound, TopAbs_SHAPE); exp.More(); exp.Next())
    {
        lastLoc = exp.Current();
		// return lastLoc;
    }

    return lastLoc;
}
void Originl(float x=0, float y=0, float z=0,float rx=0, float ry=0, float rz=0){
    if (!current_part) return;

    const double dx = rx * M_PI / 180.0;
    const double dy = ry * M_PI / 180.0;
    const double dz = rz * M_PI / 180.0;

    gp_Trsf Rx, Ry, Rz;

    Rx.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp::DX()), dx);
    Ry.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp::DY()), dy);
    Rz.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp::DZ()), dz);

    gp_Trsf trsf = Rz * Ry * Rx;  // ZYX order (industry standard)
    trsf.SetTranslationPart(gp_Vec(x, y, z));

    current_part->Originl = TopLoc_Location(trsf);
}
void OriginlApply(luadraw* current_part){

}



luadraw* Part(const std::string& name){
	// perf2();    
	

	if(vlua.size()>0)inteligentSet(vlua.back());
	// perf2("test");
	auto* obj = new luadraw;
	obj->name=name;

	lua_Debug ar;
	if (lua_getstack(L, 1, &ar)) {// level 1 = caller of this function
        lua_getinfo(L, "l", &ar);// S = source, l = current line
        obj->currentline=ar.currentline;
    }

	// BRep_Builder builder;
	// builder.MakeCompound(TopoDS::Compound(obj->cshape));
	obj->ashape = new AIS_Shape(obj->shape);

	int counter = 1;
	std::string new_name = obj->name;
	while (lua_var_exists(new_name)) {
		new_name = obj->name + std::to_string(counter); 
		counter++;
	}
	obj->name = new_name;
	(*G)[obj->name] = obj;  // same as: name = obj in Lua, now  part names are luadraw
	current_part = obj;
	Originl();
  
	Handle(ManagedPtrWrapper<luadraw>) wrapper = new ManagedPtrWrapper<luadraw>(obj);
	obj->ashape->SetOwner(wrapper);

	vlua.push_back(obj);

	current_part->current_location=TopLoc_Location();
	// current_part->Originl=Originl();

	
	obj->builder.MakeCompound(obj->cshape);

	return obj;
}

		TopoDS_Shape FuseAndRefineWithAPI(const TopoDS_Shape& inputShape) {
			auto solids = ExtractSolids(inputShape);
			if (solids.empty()) {
				// std::cerr << "⚠️ No solids found in input shape\n";
				// cotm(name, "⚠️ No solids found in input shape");
				// lua_error_with_line(L, "No solids found in input shape\n");
				return current_part->shape; 
			}
			if (solids.size() == 1) {
				return solids[0];
				// Only one solid → just refine it
				ShapeUpgrade_UnifySameDomain unify(solids[0], true, true, true);
				unify.Build();
				return unify.Shape();
			}
			return current_part->cshape;
		}
		void update_placement() { 
			current_part->shape = FuseAndRefineWithAPI(current_part->cshape);
			current_part->ashape->Set(current_part->cshape); 
		}

void Rec(float width, float height = 0){
    if (height == 0)
        height = width;

    // 1. Extract current placement
    gp_Trsf partTrsf;// = current_part->shape.Location().Transformation();

    // 2. Local axes transformed to global
    gp_Dir localZ = gp::DZ().Transformed(partTrsf);
    gp_Dir localX = gp::DX().Transformed(partTrsf);
    gp_Dir localY = gp::DY().Transformed(partTrsf);

    // 3. Local origin transformed to global
    gp_Pnt origin = gp_Pnt(0,0,0).Transformed(partTrsf);

    // 4. Build plane
    gp_Ax2 axis(origin, localZ, localX);

    // 5. Convert gp_Ax2 → gp_Trsf
    gp_Trsf ax2Trsf;
    ax2Trsf.SetTransformation(axis);

    // 6. Rectangle points in LOCAL coordinates (corner at 0,0)
    gp_Pnt p1_local(0,      0,       0);
    gp_Pnt p2_local(width,  0,       0);
    gp_Pnt p3_local(width,  height,  0);
    gp_Pnt p4_local(0,      height,  0);

    // 7. Transform to GLOBAL coordinates
    gp_Pnt p1 = p1_local.Transformed(ax2Trsf);
    gp_Pnt p2 = p2_local.Transformed(ax2Trsf);
    gp_Pnt p3 = p3_local.Transformed(ax2Trsf);
    gp_Pnt p4 = p4_local.Transformed(ax2Trsf);

    // 8. Build edges
    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p1);

    // 9. Build wire
    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(e1, e2, e3, e4);

    // 10. Build face
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

    // 11. Apply part location
    face.Location(current_part->current_location.Transformation());
    // face.Location(current_part->shape.Location().Transformation());

    // 12. Merge into model
    inteligentmerge(face, 0);
}

void Circle(float radius){
    // cotm("circle");
    // PrintLocationDegrees(current_part->current_location);

    // 1. Extract the current placement
    gp_Trsf partTrsf;// = current_part->shape.Location().Transformation();
    // gp_Trsf partTrsf = current_part->shape.Location().Transformation();
    // gp_Trsf partTrsf = current_part->current_location.Transformation();

    // 2. Compute global center of the part
    gp_Pnt center = gp_Pnt(0,0,0).Transformed(partTrsf);

    // 3. Compute global Z direction
    gp_Dir localZ = gp::DZ().Transformed(partTrsf);

    // 4. Build circle axis
    gp_Ax2 axis(center, localZ);

    // 5. Build circle
    gp_Circ circ(axis, radius);
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circ);
    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

	face.Location(current_part->shape.Location().Transformation());

    inteligentmerge(face,0);
    // inteligentset(0);

    // 6. UPDATE current_part->current_location so its origin = circle center
    // gp_Trsf newTrsf;
    // newTrsf.SetTranslation(gp::Origin(), center);
    // current_part->current_location = TopLoc_Location(newTrsf);
}
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <ShapeFix_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp.hxx>
void Invert(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){
    TopoDS_Shape src =
        (original == nullptr) ? current_part->shape : original->fshape;

    if (src.IsNull()) return;

    cotm(9999);

    Bnd_Box bbox;
    BRepBndLib::Add(src, bbox);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt c(
        (xmin + xmax) * 0.5,
        (ymin + ymax) * 0.5,
        (zmin + zmax) * 0.5
    );

    gp_Dir axis;

    if (x)
    {
        c.SetX(c.X() + offset);
        axis = gp::DY(); // mimic mirror X
    }
    else if (y)
    {
        c.SetY(c.Y() + offset);
        axis = gp::DX(); // mimic mirror Y
    }
    else
    {
        c.SetZ(c.Z() + offset);
        axis = gp::DX(); // mimic mirror Z
    }

    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(c, axis), M_PI);

    TopoDS_Shape out = src.Moved(TopLoc_Location(tr));

    inteligentmerge(out,0);
    inteligentset();
}
void InvertfailonFaces(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {
    TopoDS_Shape toinvert = (original == 0) ? current_part->shape : original->fshape;
    if(toinvert.IsNull()) return;

    cotm(9999);
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    // Set up the normal for the mirror plane
    if (x != 0) { 
        normal = gp::DX(); 
        planeOrigin.SetX(planeOrigin.X() + offset); 
    } 
    else if (y != 0) { 
        normal = gp::DY(); 
        planeOrigin.SetY(planeOrigin.Y() + offset); 
    } 
    else { 
        normal = gp::DZ(); 
        planeOrigin.SetZ(planeOrigin.Z() + offset); 
    }

    // Create a true reflection matrix
    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // --- THE FAST WAY ---
    // Apply the mirror matrix directly to the location.
    // This updates the TopLoc_Location (and flips the orientation flag automatically)
    // without rebuilding the underlying BRep geometry!
    TopoDS_Shape mirrored = toinvert.Moved(TopLoc_Location(mirrorTrsf));

    inteligentmerge(mirrored);
    inteligentset();
}
void Invertgood2(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {
    // Resolve the shape to operate on safely
    TopoDS_Shape toinvert;
    if (original == 0) {
        toinvert = current_part->shape; 
    } else {
        toinvert = original->fshape;
    }
    
    cotm(9999);
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    
    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir rotAxis;
    
    // Set up axes for the "mirror mimic" 180-degree rotation
    if (x != 0) { 
        planeOrigin.SetX(planeOrigin.X() + offset); 
        rotAxis = gp::DY(); // Rotate around Y to mirror along X
    } 
    else if (y != 0) { 
        planeOrigin.SetY(planeOrigin.Y() + offset); 
        rotAxis = gp::DX(); // Rotate around X to mirror along Y
    } 
    else { 
        planeOrigin.SetZ(planeOrigin.Z() + offset); 
        rotAxis = gp::DZ(); 
    }

    gp_Trsf rotTrsf;
    rotTrsf.SetRotation(gp_Ax1(planeOrigin, rotAxis), M_PI);

    // --- THE FAST WAY ---
    // Use .Moved() to multiply the transformation matrices (T * L_old)
    // This applies the new rotation ON TOP of the existing location (preserving Y).
    TopoDS_Shape rotated = toinvert.Moved(TopLoc_Location(rotTrsf));

    inteligentmerge(rotated);
    inteligentset();
}
void Invertslow(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {
    TopoDS_Shape toinvert = (original == nullptr) ? current_part->shape : original->fshape;
    if (toinvert.IsNull()) return;

    // --- robust bbox ---
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);

    if (bbox.IsVoid() || toinvert.ShapeType() == TopAbs_COMPOUND) {
        bbox.SetVoid();
        bool added = false;
        for (TopExp_Explorer ex(toinvert, TopAbs_SOLID); ex.More(); ex.Next()) {
            BRepBndLib::Add(ex.Current(), bbox);
            added = true;
        }
        if (!added) {
            for (TopExp_Explorer ex(toinvert, TopAbs_FACE); ex.More(); ex.Next()) {
                BRepBndLib::Add(ex.Current(), bbox);
                added = true;
            }
        }
        if (!added || bbox.IsVoid()) {
            // fallback to origin-centered plane if nothing contributes to bbox
            bbox.Update(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
        }
    }

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir rotAxis;

    if (x != 0) { planeOrigin.SetX(planeOrigin.X() + offset); rotAxis = gp::DY(); }
    else if (y != 0) { planeOrigin.SetY(planeOrigin.Y() + offset); rotAxis = gp::DX(); }
    else { planeOrigin.SetZ(planeOrigin.Z() + offset); rotAxis = gp::DX(); }

    // 180-degree rotation about axis through planeOrigin
    gp_Trsf rotTrsf;
    rotTrsf.SetRotation(gp_Ax1(planeOrigin, rotAxis), M_PI);

    // Compose with existing location: new = rot * old
    TopLoc_Location oldLoc = toinvert.Location();
    gp_Trsf oldTrsf = oldLoc.Transformation();
    gp_Trsf composed = rotTrsf * oldTrsf; // important order

    TopLoc_Location newLoc(composed);

    // Apply location-only change
    TopoDS_Shape locatedShape = toinvert;
    locatedShape.Location(newLoc);

    // Optional: fix orientation/validity
    ShapeFix_Shape fixer(locatedShape);
    fixer.Perform();
    TopoDS_Shape fixed = fixer.Shape();

    inteligentmerge(fixed);
    inteligentset();
}

void Invertgood(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) { 
	// perf2("");  
	TopoDS_Shape toinvert;
	if(original==0)toinvert=current_part->shape; else toinvert=original->fshape;
	cotm(9999);
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir rotAxis;

    if (x != 0) { 
        planeOrigin.SetX(planeOrigin.X() + offset); 
        rotAxis = gp::DY(); 
    } 
    else if (y != 0) { 
        planeOrigin.SetY(planeOrigin.Y() + offset); 
        rotAxis = gp::DX(); 
    } 
    else { 
        planeOrigin.SetZ(planeOrigin.Z() + offset); 
        rotAxis = gp::DX(); 
    }

    gp_Trsf rotTrsf;
    rotTrsf.SetRotation(gp_Ax1(planeOrigin, rotAxis), M_PI);

    // --- THE FAST WAY ---
    // .Moved() applies the transformation matrix on top of the shape's existing location.
    // It returns a new TopoDS_Shape sharing the same geometry, but positioned differently.

    TopoDS_Shape rotated= toinvert;//.Location(TopLoc_Location(rotTrsf));
	rotated=rotated.Located(TopLoc_Location(rotTrsf));
    // TopoDS_Shape rotated = original->fshape.Moved(TopLoc_Location(rotTrsf));

// perf2("inverse");
    inteligentmerge(rotated);
    inteligentset();
    
    // Store the updated location
	// current_part->current_location = rotTrsf * current_part->current_location;
    // current_part->current_location = rotated.Location(); 
}

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <TopoDS_Shape.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp.hxx>

void Inversenw(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0)
{
    TopoDS_Shape toinvert = (original == nullptr) ? current_part->shape : original->fshape;

    // 1) Extract existing location transform (local -> world)
    TopLoc_Location loc = toinvert.Location();
    gp_Trsf baseTrsf = loc.Transformation();

    // 2) Compute a reliable pivot in world coordinates: use center of mass if available
    GProp_GProps props;
    BRepGProp::VolumeProperties(toinvert, props);
    gp_Pnt worldCenter = props.CentreOfMass();

    // Fallback: if volume properties fail, use axis-aligned bbox center in world coords
    if (worldCenter.IsEqual(gp_Pnt(0,0,0), 0.0) && props.Mass() == 0.0) {
        Bnd_Box bbox;
        BRepBndLib::Add(toinvert, bbox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        worldCenter.SetX((xmin + xmax) * 0.5 + (x ? offset : 0.0));
        worldCenter.SetY((ymin + ymax) * 0.5 + (y ? offset : 0.0));
        worldCenter.SetZ((zmin + zmax) * 0.5 + (z ? offset : 0.0));
    } else {
        // apply offset along chosen global axis if requested
        if (x) worldCenter.SetX(worldCenter.X() + offset);
        if (y) worldCenter.SetY(worldCenter.Y() + offset);
        if (z) worldCenter.SetZ(worldCenter.Z() + offset);
    }

    // 3) Choose the global axis direction (always in world coords)
    gp_Dir globalAxis;
    if (x) globalAxis = gp_Dir(1.0, 0.0, 0.0);
    else if (y) globalAxis = gp_Dir(0.0, 1.0, 0.0);
    else globalAxis = gp_Dir(0.0, 0.0, 1.0);

    // 4) Build a rotation around the world axis through the world pivot
    gp_Trsf rot;
    rot.SetRotation(gp_Ax1(worldCenter, globalAxis), M_PI);

    // 5) Compose transforms so rotation is applied in world space to the already-located shape
    gp_Trsf finalTrsf = rot * baseTrsf;

    // 6) Apply composed transform as new location
    TopoDS_Shape rotated = toinvert;
    rotated.Location(TopLoc_Location(finalTrsf));

    // 7) Merge and set as before
    inteligentmerge(rotated);
    inteligentset();
}
#include <Standard_Real.hxx>
#include <TopoDS_Shape.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>
#include <TopLoc_Location.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax2.hxx>
#include <gp.hxx>

// forward declarations for your helpers
// void inteligentmerge(const TopoDS_Shape& s);
// extern /* your context */ TopoDS_Shape current_part_shape; // adapt to your actual variable
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <ShapeFix_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax2.hxx>
#include <gp.hxx>

void Invertcomplex(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0)
{
    TopoDS_Shape toinvert = (original == 0) ? current_part->shape : original->fshape;
	cotm(9)
    if (toinvert.IsNull()) return;

cotm(1)
    // --- robust bounding box computation ---
    Bnd_Box bbox;

    // First try the whole shape
    BRepBndLib::Add(toinvert, bbox);

    // If it's a compound or bbox is void, explicitly accumulate from children
    if (bbox.IsVoid() || toinvert.ShapeType() == TopAbs_COMPOUND) {
        bbox.SetVoid();
cotm(2);
        // Prefer solids; if none, fall back to faces
        bool hasSomething = false;

        // Solids
        for (TopExp_Explorer ex(toinvert, TopAbs_SOLID); ex.More(); ex.Next()) {
            const TopoDS_Shape& s = ex.Current();
            BRepBndLib::Add(s, bbox);
            hasSomething = true;
        }

        // If no solids, try faces
        if (!hasSomething) {
            for (TopExp_Explorer ex(toinvert, TopAbs_FACE); ex.More(); ex.Next()) {
                const TopoDS_Shape& f = ex.Current();
                BRepBndLib::Add(f, bbox);
                hasSomething = true;
            }
        }
        // Still nothing? bail out safely
        if (!hasSomething || bbox.IsVoid()) {
            // Fallback: mirror around origin using only offset
            gp_Pnt planeOrigin(0.0, 0.0, 0.0);
            gp_Dir normal;

            if (x != 0) { normal = gp::DX(); planeOrigin.SetX(planeOrigin.X() + offset); }
            else if (y != 0) { normal = gp::DY(); planeOrigin.SetY(planeOrigin.Y() + offset); }
            else { normal = gp::DZ(); planeOrigin.SetZ(planeOrigin.Z() + offset); }

            gp_Trsf mirrorTrsf;
            mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

            TopLoc_Location oldLoc = toinvert.Location();
            gp_Trsf oldTrsf = oldLoc.Transformation();
            gp_Trsf composed = mirrorTrsf * oldTrsf;
            TopLoc_Location newLoc(composed);

            TopoDS_Shape locatedShape = toinvert;
            locatedShape.Location(newLoc);

            ShapeFix_Shape fixer(locatedShape);
            fixer.Perform();
            inteligentmerge(fixer.Shape());
            return;
        }
    }
cotm(3)
    // Now bbox is guaranteed non-void
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5,
                       (ymin + ymax) * 0.5,
                       (zmin + zmax) * 0.5);
    gp_Dir normal;

    if (x != 0) { normal = gp::DX(); planeOrigin.SetX(planeOrigin.X() + offset); }
    else if (y != 0) { normal = gp::DY(); planeOrigin.SetY(planeOrigin.Y() + offset); }
    else { normal = gp::DZ(); planeOrigin.SetZ(planeOrigin.Z() + offset); }

    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // LOCATION-ONLY: compose with existing location
    TopLoc_Location oldLoc = toinvert.Location();
    gp_Trsf oldTrsf = oldLoc.Transformation();
    gp_Trsf composed = mirrorTrsf * oldTrsf;
    TopLoc_Location newLoc(composed);

    TopoDS_Shape locatedShape = toinvert;
    locatedShape.Location(newLoc);

    ShapeFix_Shape fixer(locatedShape);
    fixer.Perform();
    TopoDS_Shape fixed = fixer.Shape();

    inteligentmerge(fixed);
}

void Invert5(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {
    TopoDS_Shape toinvert = (original == 0) ? current_part->shape : original->fshape;
    
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
cotm("invert");

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    // Use the exact same mirroring planes as your original function
    if (x != 0) { normal = gp::DX(); planeOrigin.SetX(planeOrigin.X() + offset); }
    else if (y != 0) { normal = gp::DY(); planeOrigin.SetY(planeOrigin.Y() + offset); }
    else { normal = gp::DZ(); planeOrigin.SetZ(planeOrigin.Z() + offset); }

    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // 1. Apply the exact mirror transformation purely via Location
    TopoDS_Shape inverted = toinvert.Moved(TopLoc_Location(mirrorTrsf));
    
    // 2. Fix the "inside out" faces caused by the negative determinant
    // .Reversed() just flips the topological orientation (Forward <-> Reversed).
    // This is virtually instantaneous and requires zero geometry rebuilding!
    inverted.Reverse(); 
    inteligentmerge(inverted);
}


#include <Standard_Real.hxx>
#include <TopoDS_Shape.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>
#include <TopLoc_Location.hxx>
gp_Trsf BuildMirroredLocation(
    const TopLoc_Location& loc,
    const gp_Trsf& mirrorTrsf)
{
    // Get the current transformation of the location
    gp_Trsf oldTrsf = loc.Transformation();

    // Transform the local coordinate system basis (origin + two axes) into world space
    gp_Pnt origin(0.0, 0.0, 0.0);
    gp_Pnt px(1.0, 0.0, 0.0);
    gp_Pnt pz(0.0, 0.0, 1.0);

    origin.Transform(oldTrsf);
    px.Transform(oldTrsf);
    pz.Transform(oldTrsf);

    // Now apply the mirror transformation to these world-space points
    gp_Pnt mirroredOrigin = origin;
    gp_Pnt mirroredPx     = px;
    gp_Pnt mirroredPz     = pz;

    mirroredOrigin.Transform(mirrorTrsf);
    mirroredPx.Transform(mirrorTrsf);
    mirroredPz.Transform(mirrorTrsf);

    // Build new right-handed axes from the mirrored points
    gp_Vec newX(mirroredOrigin, mirroredPx);
    gp_Vec newZ(mirroredOrigin, mirroredPz);

    // Ensure the vectors are normalized (good practice)
    newX.Normalize();
    newZ.Normalize();

    // Create the new axis placement (right-handed coordinate system)
    gp_Ax3 newAx3(mirroredOrigin, gp_Dir(newZ), gp_Dir(newX));

    // Build the resulting transformation from the new coordinate system
    gp_Trsf result;
    result.SetTransformation(newAx3);

    return result;
}
void Mirrortry(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){
    TopoDS_Shape toinvert = (original == nullptr) ? current_part->shape : original->fshape;
    if (toinvert.IsNull()) return;

    // --- Compute mirror plane from bounding box + offset ---
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    if (x != 0) {
        normal = gp::DX();
        planeOrigin.SetX(offset >= 0 ? xmin + offset : xmax + offset);
    }
    else if (y != 0) {
        normal = gp::DY();
        planeOrigin.SetY(offset >= 0 ? ymin + offset : ymax + offset);
    }
    else {
        normal = gp::DZ();
        planeOrigin.SetZ(offset >= 0 ? zmin + offset : zmax + offset);
    }

    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // 1. Mirror the geometry (this puts the shape in the correct global position)
    TopoDS_Shape inverted = toinvert.Moved(TopLoc_Location(mirrorTrsf));
    inverted.Reverse();   // Mirror flips orientation

    // 2. Compute the target reference location so that the shape stays in the same global place
    // Correct composition: targetLoc = mirrorTrsf * oldLoc
    // But because we already applied mirror via Moved(), we now want to "absorb" it into the reference.
    TopLoc_Location oldLoc = toinvert.Location();
    TopLoc_Location mirrorLoc(mirrorTrsf);

    // Full mirrored location
    TopLoc_Location targetLoc = mirrorLoc * oldLoc;
// gp_Trsf newLoc = inverted.Location().Transformation();
    // 3. Rebase the reference location without moving the geometry in global space
	{
		gp_Trsf old = toinvert.Location().Transformation();

// original frame
gp_Ax3 ax(
    old.TranslationPart(),
    gp_Dir(old.Value(1,3), old.Value(2,3), old.Value(3,3)), // Z
    gp_Dir(old.Value(1,1), old.Value(2,1), old.Value(3,1))  // X
);

// mirror origin + directions
gp_Pnt p = ax.Location();
gp_Dir xdir = ax.XDirection();
gp_Dir zdir = ax.Direction();

p.Transform(mirrorTrsf);
xdir.Transform(mirrorTrsf);
zdir.Transform(mirrorTrsf);

// rebuild clean mirrored frame
gp_Ax3 mirroredAx(p, zdir, xdir);

// gp_Trsf newLoc;
// newLoc.SetTransformation(mirroredAx);
gp_Trsf newLoc =
    BuildMirroredLocation(toinvert.Location(), mirrorTrsf);
    SetReferenceLocationWithoutMoving(inverted, newLoc);
	}

    inteligentmerge(inverted, 0);
}
void Mirror(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {
    TopoDS_Shape toinvert = (original == 0) ? current_part->shape : original->fshape;
    if (toinvert.IsNull()) return;

    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

// --- OFFSET LOGIC: positive = from left(min), negative = from right(max) ---
if (x != 0) {
    normal = gp::DX();
    if (offset >= 0)
        planeOrigin.SetX(xmin + offset);
    else
        planeOrigin.SetX(xmax + offset); // offset is negative
}
else if (y != 0) {
    normal = gp::DY();
    if (offset >= 0)
        planeOrigin.SetY(ymin + offset);
    else
        planeOrigin.SetY(ymax + offset);
}
else {
    normal = gp::DZ();
    if (offset >= 0)
        planeOrigin.SetZ(zmin + offset);
    else
        planeOrigin.SetZ(zmax + offset);
}


    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

	if(0)
    if (toinvert.ShapeType() != TopAbs_SOLID) {
        BRepBuilderAPI_Transform transformer(toinvert, mirrorTrsf, Standard_True);
        if (!transformer.IsDone()) return;

        TopoDS_Shape mirrored = transformer.Shape();
        // ShapeFix_Shape fixer(mirrored);
        // fixer.Perform();
        // mirrored = fixer.Shape();
        inteligentmerge(mirrored,0);
        return;
    }

    TopoDS_Shape inverted = toinvert.Moved(TopLoc_Location(mirrorTrsf));
    inverted.Reverse();

{
	    // ShapeFix_Shape fixer(inverted);
        // fixer.Perform();
        // inverted = fixer.Shape();
		// 1. Get the transformation of the original shape
// 2. Get original transformation
    gp_Trsf originalTrsf = toinvert.Location().Transformation();
    
    // 3. Find where the original origin landed after mirroring
    // We transform the original translation vector by the mirror matrix
    gp_Pnt oldPos(originalTrsf.TranslationPart());
    gp_Pnt newPos = oldPos.Transformed(mirrorTrsf);

    // 4. Construct the Target Location
    gp_Trsf targetTrsf;
    // targetTrsf.SetRotation(originalTrsf.GetRotation()); // Keep original orientation
    targetTrsf.SetTranslationPart(newPos.XYZ());        // Set to the mirrored origin point
    
    // TopLoc_Location targetLoc(targetTrsf);
	//     gp_Dir axisDir(0, 1, 0);
    // double angleRad = 90 * (M_PI / 180.0);

    // gp_Trsf localRot;
    // localRot.SetRotation(gp_Ax1(gp_Pnt(0,0,0), axisDir), angleRad);
	// gp_Trsf delta;
    // delta.SetTranslation(gp_Vec(100, 0, 0));

    // // 5. Apply the "Zeroed" reference to the shape
    // SetReferenceLocationWithoutMoving(inverted, targetTrsf);
}

	// inverted=RebaseLocation(inverted,TopLoc_Location().Transformation());
	// SetReferenceLocationWithoutMoving(inverted,TopLoc_Location().Transformation());
    inteligentmerge(inverted,0);
}

//Fast real Mirror but only works on 3d
void Mirror11(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {
	// cotm("isinv")
    TopoDS_Shape toinvert = (original == 0) ? current_part->shape : original->fshape;
    if(toinvert.IsNull())return;
	// toinvert.Reverse();
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    // Use the exact same mirroring planes as your original function
    if (x != 0) { normal = gp::DX(); planeOrigin.SetX(planeOrigin.X() + offset); }
    else if (y != 0) { normal = gp::DY(); planeOrigin.SetY(planeOrigin.Y() + offset); }
    else { normal = gp::DZ(); planeOrigin.SetZ(planeOrigin.Z() + offset); }

    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

	if (toinvert.ShapeType() != TopAbs_SOLID){
	// Use BRepBuilderAPI_Transform with copy = true
		BRepBuilderAPI_Transform transformer(toinvert, mirrorTrsf, Standard_True);
		
		if (!transformer.IsDone()) {
			// Handle error
			return;
		}
		
		TopoDS_Shape mirrored = transformer.Shape();
		
		// Fix the face orientations after mirroring
		ShapeFix_Shape fixer(mirrored);
		fixer.Perform();
		mirrored = fixer.Shape();

		inteligentmerge(mirrored);
		return;

	}
    // 1. Apply the exact mirror transformation purely via Location
    TopoDS_Shape inverted = toinvert.Moved(TopLoc_Location(mirrorTrsf));
    
    // 2. Fix the "inside out" faces caused by the negative determinant
    // .Reversed() just flips the topological orientation (Forward <-> Reversed).
    // This is virtually instantaneous and requires zero geometry rebuilding!
    inverted.Reverse(); 

// 	TopoDS_Face sketchFace=TopoDS_Face();
// 	if (inverted.ShapeType() == TopAbs_FACE){
// 	 for (TopExp_Explorer ex(inverted, TopAbs_FACE); ex.More(); ex.Next())
//     {
//         TopoDS_Face f = TopoDS::Face(ex.Current());
//         if (BRep_Tool::Surface(f)->DynamicType() == STANDARD_TYPE(Geom_Plane))
//             sketchFace = f;//.Reverse();
//     }
// 	if(!sketchFace.IsNull()){
// 		cotm("sketchface")
// 		// sketchFace.Reverse(); 
// 		inteligentmerge(sketchFace);
// 		return;
// 	}
// }
{
	// ShapeFix_Shape fixer(inverted);
    // fixer.Perform();
    // inverted = fixer.Shape();
	// inteligentmerge(inverted);
	// return;
}


    inteligentmerge(inverted);
}


void Mirrorrobustactual(luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0) {  
	TopoDS_Shape toinvert = (original == 0) ? current_part->shape : original->fshape;
	if(toinvert.IsNull())return;
    Bnd_Box bbox;
    BRepBndLib::Add(toinvert, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    if (x != 0) { normal = gp::DX(); planeOrigin.SetX(planeOrigin.X() + offset); }
    else if (y != 0) { normal = gp::DY(); planeOrigin.SetY(planeOrigin.Y() + offset); }
    else { normal = gp::DZ(); planeOrigin.SetZ(planeOrigin.Z() + offset); }

    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // Use BRepBuilderAPI_Transform with copy = true
    BRepBuilderAPI_Transform transformer(toinvert, mirrorTrsf, Standard_True);
    
    if (!transformer.IsDone()) {
        // Handle error
        return;
    }
    
    TopoDS_Shape mirrored = transformer.Shape();
    
    // Fix the face orientations after mirroring
    ShapeFix_Shape fixer(mirrored);
    fixer.Perform();
    mirrored = fixer.Shape();

    inteligentmerge(mirrored);
    inteligentset();
	// Store the actual transform used
// current_part->current_location = TopLoc_Location(mirrorTrsf);


	    //     gp_Trsf pure_transformation = mirrored.Location().Transformation();
        
        // // Create a completely independent location for the clone using that matrix.
        // current_part->current_location = TopLoc_Location(pure_transformation);



    // 3. Wrap in a Compound
    // TopoDS_Compound newCompound;
    // BRep_Builder builder;
    // builder.MakeCompound(newCompound);
    // builder.Add(newCompound, mirrored);

    // current_part->cshape = newCompound;
	// current_part->shape = mirrored; ///for location to work
    
    // // Sync metadata
    // current_part->Extrude_val = original->Extrude_val;
    // current_part->from_sketch = original->from_sketch;


	// current_part->cshape=CleanCompound_RemoveWiresFacesBeforeSolid(current_part->cshape);
	
}
void Pl(const std::string& coords) {
    if (!current_part)
        luaL_error((*G).lua_state(), "No current part. Call Part(name) first.");

    sol::state& L = *G;  // usa o estado Lua global
	
	int currentline=-1;
	lua_Debug ar;
	if (lua_getstack(L, 1, &ar)) {// level 1 = caller of this function
        lua_getinfo(L, "l", &ar);// S = source, l = current line
        currentline=ar.currentline;
    }
auto eval = [&](const std::string& expr, int line) {
    std::string chunk = "return " + expr;
    std::string name  = "Pl:" + std::to_string(line);

    sol::load_result lr = L.load(chunk, name);
    if (!lr.valid()) {
        sol::error e = lr;
        luaL_error(L.lua_state(), "%s", e.what());
    }

    sol::protected_function pf = lr;
    sol::protected_function_result r = pf();
    if (!r.valid()) {
        sol::error e = r;
        luaL_error(L.lua_state(), "%s", e.what());
    }

    return r.get<double>();
};

    std::vector<gp_Vec2d> out;
    double lastX = 0.0;
    double lastY = 0.0;
    bool have_last = false;

    std::istringstream ss(coords);
    std::string token;

    while (ss >> token) {
        bool relative = false;

        // Detecta forma relativa: "@dx,dy"
        if (!token.empty() && token.front() == '@') {
            relative = true;
            token.erase(0, 1);
        }

        // Divide "x,y"
        const auto commaPos = token.find(',');
        if (commaPos == std::string::npos)
            continue;

        std::string xs = token.substr(0, commaPos);
        std::string ys = token.substr(commaPos + 1);

        // Avalia cada expressão no Lua
        // double x = L.script("return " + xs);
        // double y = L.script("return " + ys);
		double x = eval(xs, currentline);
		double y = eval(ys, currentline);


        if (relative) {
            if (have_last) {
                lastX += x;
                lastY += y;
            } else {
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

    // Cria o wire com os pontos calculados
    CreateWire(out, false);

	// inteligentmerge()
	inteligentset();

	// ShowTrihedronAtLocation(ctx,current_part->current_location);


	// current_part->ashape->Set(current_part->shape); //important when moves and changes....upd
	// current_part->current_location=LastShapeLocation(current_part->cshape);
}

void Offset(double distance) {
	vector<gp_Pnt2d> ppoints;
	ConvertVec2dToPnt2d(current_part->vpoints.back(), ppoints);

	bool closed = ((current_part->vpoints.back()[0].X() == current_part->vpoints.back().back().X()) &&
					(current_part->vpoints.back()[0].Y() == current_part->vpoints.back().back().Y()));
	TopoDS_Face f;
	if (closed) {
		f = TopoDS::Face(MakeOffsetRingFace(ppoints, -distance));  // well righ
	} else {
		TopoDS_Wire wOff = TopoDS::Wire(MakeOneSidedOffsetWire(ppoints, distance));	 // well profile
		if(distance<0)wOff.Reverse();  // <--- FIX for the normal not be reversed becaus it was contructed clockwise
		f = BRepBuilderAPI_MakeFace(wOff);
	}
	BRepMesh_IncrementalMesh mesher(f, 0.5, true, 0.5, true);  // adjust deflection/angle
	current_part->shape=f;
	// mergeShape(current_part->cshape, f);
	// inteligentmerge(f);
	// inteligentset();
}
// void Clone(luadraw* toclone, bool copy_placement = false){

//     // 1. Copiar dados básicos
//     current_part->vpoints     = toclone->vpoints;
//     current_part->Extrude_val = toclone->Extrude_val;
	
// 	//this is ok
// 	inteligentmerge(toclone->fshape);
// 	inteligentset();
	
// 	//this is not ok because when I use it in move (translation) it dont work well. what I want is to get the last corresponding current_location
// 	current_part->current_location=toclone->current_location ;



// }

TopoDS_Shape ResetPlacement(const TopoDS_Shape& s)
{
    if (s.IsNull()) return s;

    // 1. Extract location BEFORE clearing it
    TopLoc_Location loc = s.Location();
    gp_Trsf trsf = loc.Transformation();

    // 2. Remove the wrapper location
    TopoDS_Shape base = s;
    base.Location(TopLoc_Location());

    // 3. Bake the transform into the geometry
    if (trsf.Form() != gp_Identity)
        base = BRepBuilderAPI_Transform(base, trsf, true).Shape();

    // 4. Recurse into compounds
    if (base.ShapeType() == TopAbs_COMPOUND)
    {
        BRep_Builder b;
        TopoDS_Compound out;
        b.MakeCompound(out);

        for (TopExp_Explorer ex(base, TopAbs_SHAPE); ex.More(); ex.Next())
            b.Add(out, ResetPlacement(ex.Current()));

        return out;
    }

    return base;
}
// Return the full accumulated transform of a shape (does not modify the shape)
gp_Trsf GetAccumulatedTransform(const TopoDS_Shape& s)
{
    gp_Trsf total; // identity by default
    TopLoc_Location loc = s.Location();

    // Accumulate nested locations: total = L_n * ... * L_2 * L_1
    while (!loc.IsIdentity())
    {
        total = loc.Transformation() * total;
        loc = loc.NextLocation();
    }

    return total;
}
#include <TopExp_Explorer.hxx>
#include <BRep_Builder.hxx>

TopoDS_Shape ClearLocationsRecursive(const TopoDS_Shape& s)
{
    if (s.IsNull()) return s;

    // Make a local copy and clear this wrapper
    TopoDS_Shape out = s;
    out.Location(TopLoc_Location()); // clear wrapper for this node

    // If compound, rebuild with cleared children
    if (out.ShapeType() == TopAbs_COMPOUND) {
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);

        for (TopExp_Explorer ex(s, TopAbs_SHAPE); ex.More(); ex.Next()) {
            TopoDS_Shape child = ex.Current();
            builder.Add(compound, ClearLocationsRecursive(child));
        }
        return compound;
    }

    // For non-compound shapes we already cleared the wrapper; return it
    return out;
}
#include <BRepBuilderAPI_Copy.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopLoc_Location.hxx>

// Returns a new shape that is a copy of `s` with all TopLoc_Location wrappers cleared.
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>

void ClearAllLocationsInPlace(TopoDS_Shape& s)
{
    if (s.IsNull()) return;

    // Clear top-level wrapper
    s.Location(TopLoc_Location());

    // Clear wrapper on every subshape
    for (TopExp_Explorer ex(s, TopAbs_SHAPE); ex.More(); ex.Next()) {
		cotm("iter")
        TopoDS_Shape sub = ex.Current();
        sub.Location(TopLoc_Location());
    }
}
// TopoDS_Shape ClearLocationsRebuild(const TopoDS_Shape& s)
// {
//     if (s.IsNull()) return s;

//     // Clear wrapper of this node
//     TopoDS_Shape cleaned = s;
//     cleaned.Location(TopLoc_Location());

//     // If this node has children, rebuild them
//     if (s.ShapeType() == TopAbs_COMPOUND ||
//         s.ShapeType() == TopAbs_COMPSOLID ||
//         s.ShapeType() == TopAbs_SOLID ||
//         s.ShapeType() == TopAbs_SHELL ||
//         s.ShapeType() == TopAbs_WIRE)
//     {
//         BRep_Builder builder;
//         TopoDS_Compound out;
//         builder.MakeCompound(out);
// cotm("isiter")
// 		s.Location(TopLoc_Location());
//         // for (TopoDS_Iterator it(s); it.More(); it.Next())
//         // {
//         //     TopoDS_Shape child = it.Value();
//         //     TopoDS_Shape cleanedChild = ClearLocationsRebuild(child);
//         //     builder.Add(out, cleanedChild);
//         // }

//         return out;
//     }

//     // Leaf (face/edge/vertex)
//     return cleaned;
// }

void Clone(luadraw* toclone, sol::optional<int> _copy_placement){
    if (!toclone)
        lua_error_with_where("Clone name dont exist");

    current_part->vpoints     = toclone->vpoints;
    current_part->Extrude_val = toclone->Extrude_val;

    int copy_placement = _copy_placement.value_or(0);

    // Detect if 3D
    bool is3d = false;
    for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
        is3d = true;
        break;
    }

    // ------------------------------------------------------------
    // 1. Find FIRST subshape to use as baseLoc
    // ------------------------------------------------------------
    TopLoc_Location baseLoc;
    bool baseFound = false;

    if (is3d) {
        for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
            baseLoc = exp.Current().Location();
            baseFound = true;
            break;
        }
    } else {
        for (TopExp_Explorer exp(toclone->fshape, TopAbs_FACE); exp.More(); exp.Next()) {
            baseLoc = exp.Current().Location();
            baseFound = true;
            break;
        }
    }

    if (!baseFound)
        lua_error_with_where("Clone: no base shape found");

    gp_Trsf baseTrsf = baseLoc.Transformation();

    // ------------------------------------------------------------
    // 2. Build compound
    // ------------------------------------------------------------
    BRep_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);

    int c = 0;

    if (is3d) {
        // ------------------ 3D SOLIDS ------------------
        for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
            TopoDS_Shape s = exp.Current();

            gp_Trsf sTrsf = s.Location().Transformation();

            // diff = sTrsf * baseTrsf^-1
            gp_Trsf diffTrsf = sTrsf * baseTrsf.Inverted();
            s.Location(TopLoc_Location(diffTrsf));

            builder.Add(comp, s);
            cotm(c++);
        }

        // Reapply placement if requested
        if (copy_placement)
            comp.Location(baseLoc);
        else
            comp.Location(TopLoc_Location().Transformation());

        inteligentmerge(comp, 0);
        return;

    } else {
        // ------------------ 2D FACES ------------------
        for (TopExp_Explorer exp(toclone->fshape, TopAbs_FACE); exp.More(); exp.Next()) {
            TopoDS_Shape s = exp.Current();

            gp_Trsf sTrsf = s.Location().Transformation();
            gp_Trsf diffTrsf = sTrsf * baseTrsf.Inverted();
            s.Location(TopLoc_Location(diffTrsf));

            if (copy_placement)
                s.Location(baseLoc * s.Location());

            inteligentmerge(s);
        }
        return;
    }
}

void Clonealmost(luadraw* toclone, sol::optional<int> _copy_placement)
{
    if (!toclone)
        lua_error_with_where("Clone name dont exist");

    current_part->vpoints     = toclone->vpoints;
    current_part->Extrude_val = toclone->Extrude_val;

    int copy_placement = _copy_placement.value_or(0);

    // Detect if there is any solid
    bool is3d = false;
    for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
        is3d = true;
        break;
    }

    BRep_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);

    // Base placement of the source object
    TopLoc_Location baseLoc = toclone->shape.Location();
    gp_Trsf baseTrsf = baseLoc.Transformation();

    int c = 0;

    if (is3d) {
        // -------- 3D: clone solids --------
        for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
            TopoDS_Shape s = exp.Current(); // copy

            // Current shape location
            TopLoc_Location sLoc = s.Location();
            gp_Trsf sTrsf = sLoc.Transformation();

            // Remove toclone's placement: diff = sTrsf * baseTrsf^-1
            gp_Trsf diffTrsf = sTrsf * baseTrsf.Inverted();
            TopLoc_Location diffLoc(diffTrsf);
            s.Location(diffLoc);

            builder.Add(comp, s);
            cotm(c++);
        }

        // Reapply placement to the whole compound if requested
        if (copy_placement) {
            comp.Location(baseLoc);          // keep original placement
        } else {
            comp.Location(TopLoc_Location()); // no placement, pure local geometry
        }

        inteligentmerge(comp, 0);
        return;

    } else {
        // -------- 2D: clone faces --------
        for (TopExp_Explorer exp(toclone->fshape, TopAbs_FACE); exp.More(); exp.Next()) {
            TopoDS_Shape s = exp.Current(); // copy

            TopLoc_Location sLoc = s.Location();
            gp_Trsf sTrsf = sLoc.Transformation();

            // Same logic: strip toclone's placement
            gp_Trsf diffTrsf = sTrsf * baseTrsf.Inverted();
            TopLoc_Location diffLoc(diffTrsf);
            s.Location(diffLoc);

            if (copy_placement) {
                // Put it back under the same placement as toclone
                s.Location(baseLoc * s.Location());
            }

            inteligentmerge(s);
        }
        return;
    }
}

void Clonepr(luadraw* toclone, sol::optional<int> _copy_placement) {
    if (!toclone)
		lua_error_with_where("Clone name dont exist");
    current_part->vpoints     = toclone->vpoints;
    current_part->Extrude_val = toclone->Extrude_val;

	int copy_placement=_copy_placement.value_or(0);

	bool is3d=0;
	for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
        is3d=1;break;
    }
int c=0;
	BRep_Builder builder;
	TopoDS_Compound comp;
	builder.MakeCompound(comp);
	if(is3d){ 
		for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
			TopoDS_Shape s = exp.Current();        // copy

			//subtract the toclone->shape location
			TopLoc_Location diff = s.Location()  / toclone->fshape.Location() ;
			s.Location(diff);


			// if(!copy_placement)s.Location(TopLoc_Location());         // clear location 
			// if(!copy_placement)s.Location(TopLoc_Location());
			// if(copy_placement){
			// 	s.Location(toclone->shape.Location());
			// }else s.Location(TopLoc_Location());
			builder.Add(comp,s);
			// inteligentmerge(s); 
			cotm(c++);
		}
		if(copy_placement)
			comp.Location(toclone->shape.Location().Transformation());
		// else
		// 	comp.Location(TopLoc_Location());
		inteligentmerge(comp,1);
		return;
	
	}else{
		for (TopExp_Explorer exp(toclone->fshape, TopAbs_FACE); exp.More(); exp.Next()) {
			TopoDS_Shape s = exp.Current();        // copy
			if(!copy_placement)s.Location(TopLoc_Location());         // clear location 
			inteligentmerge(s); 
		}
		return;
	}

}
void Clone1(luadraw* toclone, int copy_placement=0) {
    if (!toclone)// || current_part->shape.IsNull())
		lua_error_with_where("Clone name dont exist");

    // 1. Copy basic data
    current_part->vpoints     = toclone->vpoints;
    current_part->Extrude_val = toclone->Extrude_val;
    
    // 2. Setup the shape (as you mentioned, this works well)

	// current_part->cshape=toclone->cshape;
	// current_part->shape=toclone->shape;
	// current_part->fshape=toclone->fshape;
    // inteligentmerge(toclone->fshape);

	int solidCount=0;
	for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
        solidCount=1;break;
    }

	// auto solids = ExtractFaces(toclone->fshape);
	// auto solids = ExtractSolids(toclone->fshape);
	// // cotm(solids.size());
	// const int solidCount = toclone->fshape.TShape()->NbChildren();
	// cotm(solidCount);
    if (solidCount==1){
    // if (!solids.empty()){
	// if(toclone->shape.ShapeType() == TopAbs_SOLID)	{
	// current_part->shape=toclone->fshape;
	if(copy_placement){
	inteligentmerge(toclone->fshape);
	}else{
		// TopoDS_Shape shape=(toclone->fshape);
		// TopLoc_Location loc = getShapePlacement(toclone->fshape);
		// shape.Location(loc); 
		// shape.Location(TopLoc_Location()); 
		// TopoDS_Shape shape=(toclone->shape);
		// TopoDS_Shape shape=ResetPlacement(toclone->fshape);
		// inteligentmerge(shape);
// 		gp_Trsf baked = GetAccumulatedTransform(toclone->fshape);
// gp_Trsf inv = baked.Inverted();

// current_part->shape = BRepBuilderAPI_Transform(current_part->shape, inv, true).Shape();
//working
{
// current_part->shape=toclone->shape;
// current_part->shape.Location(TopLoc_Location()); 
}
{
// TopoDS_Shape shape=toclone->shape;
// shape.Location(TopLoc_Location()); 
// inteligentmerge(shape);
}
{
// TopoDS_Shape shape;//=toclone->fshape;
// TopoDS_Compound shape;//=toclone->fshape;
for (TopExp_Explorer exp(toclone->fshape, TopAbs_SOLID); exp.More(); exp.Next()) {
    TopoDS_Shape s = exp.Current();        // copy
    s.Location(TopLoc_Location());         // clear location
	// shape=s;
	inteligentmerge(s);
    cotm("loc");
}

// ClearLocationsRebuild(shape); 
// inteligentmerge(shape);
}

		cotm("iei");
		return;
	}
    //     // current_part->shape = toclone->shape;
	// cotm("have solids");
	}else{
		inteligentmerge(toclone->shape);
    // //     // current_part->shape = toclone->shape;
	// // 	// inteligentmerge(toclone->shape);
	}
//  return;
    // return;
	// cotm(copy_placement);
    // 3. Handle the location safely
    if (copy_placement) {
        // Extract the pure transformation matrix (gp_Trsf) from the original.
        // This strips away the shared TopLoc_Location history and gives you 
        // the absolute mathematical placement.
        gp_Trsf pure_transformation = toclone->current_location.Transformation();
        
        // Create a completely independent location for the clone using that matrix.
        current_part->current_location = TopLoc_Location(pure_transformation);
    } 
    else {
        // If copy_placement is false, reset to the origin (Identity matrix)
		// TopoDS_Shape shape=resetShapePlacement(toclone->shape);
		// current_part->cshape=resetShapePlacement(toclone->shape);
		// current_part->shape=resetShapePlacement(current_part->shape);
		// inteligentmerge(shape);
		// inteligentmerge(toclone->shape);
        current_part->current_location = TopLoc_Location(); 

		if(solidCount>0){
TopLoc_Location loc = getShapePlacement(toclone->fshape);
// TopLoc_Location loc = toclone->current_location;
// TopLoc_Location loc = toclone->start_location;
// TopLoc_Location loc = toclone->fshape.Location();
gp_Trsf trsf = loc.Transformation();
// trsf=GetBakedLocation(toclone->fshape);
gp_Trsf inv = trsf.Inverted();

		current_part->shape.Location(inv); 
		cotm("bl");
		return;
		// current_part->cshape.Location(TopLoc_Location()); 
		}


// if (solidCount > 0) {

//     TopLoc_Location loc = toclone->cshape.Location();
//     gp_Trsf trsf = loc.Transformation();

//     // Invert the transform
//     gp_Trsf inv = trsf.Inverted();

//     // Apply inverse to geometry
//     current_part->shape = BRepBuilderAPI_Transform(current_part->shape, inv, true).Shape();

//     // Clear location wrapper
//     // current_part->shape.Location(TopLoc_Location());
// }




    }
    inteligentset();
}
void Clone1(luadraw* toclone, bool copy_placement = false)
{
    if (!toclone || toclone->cshape.IsNull())
        return;

    // 1. Copiar dados básicos
    current_part->vpoints     = toclone->vpoints;
    current_part->Extrude_val = toclone->Extrude_val;

    // 2. Começamos do shape original
    TopoDS_Shape src = toclone->cshape;

	// current_part->cshape=toclone->cshape;
	// current_part->shape=toclone->shape;
	// current_part->fshape=toclone->fshape;
	// inteligentmerge(toclone->cshape);
	// inteligentmerge(toclone->cshape);
	inteligentmerge(toclone->fshape);
	inteligentset();
	// current_part->current_location= current_part->current_location * toclone->current_location  ;
	current_part->current_location=toclone->current_location ;

	return;




    // 3. Normalizar placement se NÃO queremos copiar a posição
    if (!copy_placement)
    {
        TopLoc_Location loc = src.Location();
        if (!loc.IsIdentity())
        {
            gp_Trsf invTrsf = loc.Transformation();
            invTrsf.Invert();

            // "Cozinhar" o placement para a origem
            BRepBuilderAPI_Transform tr(src, invTrsf, true); // true = copy geometry
            src = tr.Shape();
        }

        // Garantir que não sobra Location pendurado
        src.Location(TopLoc_Location());
    }
    // se copy_placement == true, não mexemos no placement

    // 4. Copiar o shape inteiro (sem desmontar o compound)
    BRepBuilderAPI_Copy copier(src);
    TopoDS_Shape cloned = copier.Shape();

    // 5. Criar SEMPRE um Compound novo e meter o clone lá dentro
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    builder.Add(compound, cloned);

    // Agora temos a certeza absoluta que cshape é um Compound
    current_part->cshape = compound;

    // 6. Escolher shape ativo (primeiro sólido, se existir)
    auto solids = ExtractSolids(current_part->cshape);
    if (!solids.empty())
        current_part->shape = solids[0];
    else
        current_part->shape = current_part->cshape;

    // 7. Atualizar visualização
    if (current_part->ashape)
        current_part->ashape->Set(current_part->cshape);
}

void Copy_placement(luadraw* val){
	if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

	{
    TopLoc_Location newLoc = val->shape.Location();
		current_part->cshape.Location(newLoc);
		current_part->shape.Location(newLoc);
		return;
	}
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
}

#include <BRepBuilderAPI_Transform.hxx>

#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <GeomLProp_SLProps.hxx>
#include <BRepFeat_SplitShape.hxx>

void Extrude(float val = 0){
// cotm(99999);
    if (val == 0)
        lua_error_with_where("Extrude must have a value.");

    if (!current_part){// || current_part->shape.IsNull()){ 
		lua_error_with_where("No shape to extrude."); 
	}

    TopoDS_Shape baseShape = current_part->shape;

    // 1) Save transform
    TopLoc_Location savedLoc = baseShape.Location();

    // 2) Remove transform
    TopoDS_Shape localShape = baseShape;
    localShape.Location(TopLoc_Location());

    // 3) Ensure we have a face
    TopoDS_Face face;
    if (localShape.ShapeType() == TopAbs_FACE) {
        face = TopoDS::Face(localShape);
    } else {
        // Try to build a face from wire
        if (localShape.ShapeType() == TopAbs_WIRE) {
            BRepBuilderAPI_MakeFace mf(TopoDS::Wire(localShape));
            if (!mf.IsDone())
                lua_error_with_where("Failed to create face from wire.");
            face = mf.Face();
        } else {
            lua_error_with_where("Extrude requires a face or planar wire.");
        }
    }

    // 4) Compute geometric normal
    BRepAdaptor_Surface surf(face);
    if (surf.GetType() != GeomAbs_Plane)
        lua_error_with_where("Extrude only supports planar faces.");

    gp_Pln plane = surf.Plane();
    gp_Dir normal = plane.Axis().Direction();

    // IMPORTANT: respect face orientation
    if (face.Orientation() == TopAbs_REVERSED)
        normal.Reverse();

    gp_Vec extrusionVec(normal);
    extrusionVec *= val;

    // 5) Extrude
    BRepPrimAPI_MakePrism prism(face, extrusionVec, Standard_False);
    prism.Build();

    if (!prism.IsDone())
        lua_error_with_where("Extrusion failed.");

    TopoDS_Shape extrudedLocal = prism.Shape();

    // 6) Restore transform
    TopoDS_Shape extruded = extrudedLocal;
    extruded.Location(savedLoc);

    // 7) Update
    current_part->Extrude_val = abs(val);
    inteligentmerge(extruded,0);
    inteligentset();
}
void Movel(float x = 0, float y = 0, float z = 0,int w=0) {
    if (!current_part) {
        lua_error_with_where("No current part.");
    }

    if (current_part->shape.IsNull()) {
        return;
    }

    // 1. Create the translation matrix for your delta move (x, y, z)
    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(x, y, z));
    TopLoc_Location transLoc(translation);

    // 2. CRITICAL FIX: Extract the location directly from the OCCT Shape.
    // This ignores your cached tracker and prevents desync bugs after cloning.
    TopLoc_Location actualLoc = current_part->shape.Location();

    // 3. Calculate the new location by multiplying the translation.
    // (transLoc * actualLoc) moves the object along the GLOBAL World Axes.
    TopLoc_Location newLoc = transLoc * actualLoc;

    // 4. Apply the updated location to the shape
    // current_part->shape.Location(newLoc);

	// if(current_part->shape.ShapeType() == TopAbs_COMPOUND){
		current_part->shape.Location(newLoc);
		// current_part->current_location = newLoc * current_part->current_location;
	// 	cotm("m1");
	// }else{
	// 	current_part->shape.Location(newLoc);
	// 	current_part->current_location = newLoc;
	// 	cotm("m2");
	// }

    // 5. Sync your custom variable so it matches OpenCASCADE perfectly
    // current_part->current_location = newLoc;

	// if(current_part->shape.ShapeType() == TopAbs_COMPOUND){
	// 	current_part->current_location = newLoc * current_part->current_location;
	// }else
	// 	current_part->current_location = newLoc;

    // 6. Post-process (update your UI or internal state)
    inteligentset();
}
void Movel_local(float x = 0, float y = 0, float z = 0,int inworld=0)
{
    if (!current_part) {
        lua_error_with_where("No current part.");
    }
    if (current_part->shape.IsNull()) {
        return;
    }

    // 1. Build the delta translation in LOCAL coordinates
    gp_Trsf delta;
    delta.SetTranslation(gp_Vec(x, y, z));

    // 2. Get the fixed Originl transform
    gp_Trsf origin;
	if(inworld==0)
		origin= current_part->current_location;
		// origin= current_part->shape.Location();
    else origin = current_part->Originl.Transformation();

    // 3. Compute inverse Originl
    gp_Trsf originInv = origin.Inverted();

    // 4. Extract the shape's current world transform
    gp_Trsf current = current_part->shape.Location().Transformation();

    // 5. Compute new transform:
    //    Remove Originl → apply delta → reapply Originl
    gp_Trsf newTrsf = origin * delta * originInv * current;

    // 6. Apply to shape
    current_part->shape.Location(TopLoc_Location(newTrsf));
	current_part->current_location=TopLoc_Location(newTrsf);
}

void Movelp(float x = 0, float y = 0, float z = 0) {
    if (!current_part) {
        lua_error_with_where("No current part.");
    }

    if (current_part->shape.IsNull()) {
        return;
    }

    // 1. Create the translation matrix for your delta move (x, y, z)
    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(x, y, z));
    TopLoc_Location transLoc(translation);

    // 2. CRITICAL FIX: Extract the location directly from the OCCT Shape.
    // This ignores your cached tracker and prevents desync bugs after cloning.
    // TopLoc_Location actualLoc = current_part->Originl.Transformation();
    TopLoc_Location actualLoc = current_part->shape.Location();

    // 3. Calculate the new location by multiplying the translation.
    // (transLoc * actualLoc) moves the object along the GLOBAL World Axes.
    TopLoc_Location newLoc = transLoc * actualLoc;

    // 4. Apply the updated location to the shape
    // current_part->shape.Location(newLoc);

	// if(current_part->shape.ShapeType() == TopAbs_COMPOUND){
		current_part->shape.Location(newLoc);
		// current_part->current_location = newLoc * current_part->current_location;
	// 	cotm("m1");
	// }else{
	// 	current_part->shape.Location(newLoc);
	// 	current_part->current_location = newLoc;
	// 	cotm("m2");
	// }

    // 5. Sync your custom variable so it matches OpenCASCADE perfectly
    // current_part->current_location = newLoc;

	// if(current_part->shape.ShapeType() == TopAbs_COMPOUND){
	// 	current_part->current_location = newLoc * current_part->current_location;
	// }else
	// 	current_part->current_location = newLoc;

    // 6. Post-process (update your UI or internal state)
    // inteligentset();
}
void Movel1(float x = 0, float y = 0, float z = 0) {
    if (!current_part)
        lua_error_with_where("No current part.");

    // TopoDS_Compound& compound = current_part->cshape;
    TopoDS_Shape& shapeToMove = current_part->shape;

    // BRep_Builder builder;
    // TopoDS_Compound newCompound;
    // builder.MakeCompound(newCompound);

    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(x, y, z));
    TopLoc_Location transLoc(translation);

	TopLoc_Location newLoc = transLoc * current_part->current_location;
	
	if(!current_part->shape.IsNull()){


    	current_part->shape.Location(newLoc);
		
	// if(current_part->shape.ShapeType() != TopAbs_COMPOUND)
    // 	current_part->shape.Location(newLoc);
	// 	else
	// 	current_part->shape.Location(transLoc);
		// current_part->ashape->Set(current_part->shape);
	}
// 		else{
//     	current_part->cshape.Location(newLoc);
// 		// current_part->ashape->Set(current_part->fshape);
// 		}
// cotm(900000);
	// inteligentmerge(current_part->shape);


	inteligentset();
	// current_part->current_location=newLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*newLoc;

	// if(current_part->shape.ShapeType() != TopAbs_COMPOUND)
		current_part->current_location=transLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*transLoc;
    // current_part->current_location = current_part->shape.Location();

}
 
std::vector<std::pair<TopoDS_Shape, TopLoc_Location>> GetShapesWithLocations(const TopoDS_Shape& compound)
{
    std::vector<std::pair<TopoDS_Shape, TopLoc_Location>> result;

    for (TopExp_Explorer exp(compound, TopAbs_SHAPE); exp.More(); exp.Next())
    {
        const TopoDS_Shape& s = exp.Current();
        result.emplace_back(s, s.Location());
    }

    return result;
}
void Rotatelp(float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 1) { // z=1 como padrão
    if (!current_part ) return;
    // if (!current_part || current_part->shape.IsNull()) return;

    // 1. Validar direção para evitar crash
    if (x == 0 && y == 0 && z == 0) return; 

	// cotm("rotate");
	// PrintLocationDegrees(current_part->current_location);
    // 2. Definir o eixo de rotação
    // Pegamos a translação atual para girar em torno do "centro" da peça
    // gp_Trsf currentTrsf = current_part->current_location.Transformation();
    gp_Trsf currentTrsf;
	// if(current_part->Originl.IsIdentity())
		currentTrsf= current_part->Originl.Transformation();
	// else
	// 	currentTrsf = current_part->shape.Location().Transformation();
    // gp_Trsf currentTrsf = current_part->shape.Location().Transformation();
    gp_Pnt pivot = currentTrsf.TranslationPart(); 
    gp_Dir direction(x, y, z);
    gp_Ax1 axis(pivot, direction);

    // 3. Criar a transformação de rotação
    gp_Trsf rotation;
    rotation.SetRotation(axis, angleDegrees * (M_PI / 180.0));
    TopLoc_Location rotLoc(rotation);
	TopLoc_Location newLoc = rotLoc * current_part->shape.Location();
	
	
	// if(current_part->shape.ShapeType() != TopAbs_COMPOUND)
    // 	current_part->shape.Location(newLoc);
	// 	else
	// 	current_part->shape.Location(rotLoc);
	
	
	// current_part->shape.Location(newLoc);

	if(current_part->shape.ShapeType() == TopAbs_COMPOUND){
		current_part->shape.Location(newLoc);
		// current_part->current_location = rotLoc * current_part->current_location;
	}else{
		current_part->shape.Location(newLoc);
		// current_part->current_location = rotLoc*current_part->current_location ;
	}


	// cotm("rotate");
	// PrintLocationDegrees(current_part->shape.Location());

    // 4. Aplicar a rotação na shape alvo (acumulativo)
    // LastShape(current_part->cshape).Move(rotLoc);

	// if(!current_part->shape.IsNull())
    	// current_part->shape.Move(rotLoc);
		// else
    	// current_part->cshape.Move(rotLoc);

	// inteligentmerge(current_part->shape);
	inteligentset();
	// current_part->current_location=newLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*newLoc;
	// current_part->current_location=rotLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*rotLoc;

    // current_part->current_location = newLoc;
    // current_part->current_location = current_part->shape.Location();
 
}
void Rotatel(float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 1) { // z=1 como padrão
    if (!current_part ) return;
    // if (!current_part || current_part->shape.IsNull()) return;

    // 1. Validar direção para evitar crash
    if (x == 0 && y == 0 && z == 0) return; 

	// cotm("rotate");
	// PrintLocationDegrees(current_part->current_location);
    // 2. Definir o eixo de rotação
    // Pegamos a translação atual para girar em torno do "centro" da peça
    // gp_Trsf currentTrsf = current_part->current_location.Transformation();
    gp_Trsf currentTrsf = current_part->shape.Location().Transformation();
    gp_Pnt pivot = currentTrsf.TranslationPart(); 
    gp_Dir direction(x, y, z);
    gp_Ax1 axis(pivot, direction);

    // 3. Criar a transformação de rotação
    gp_Trsf rotation;
    rotation.SetRotation(axis, angleDegrees * (M_PI / 180.0));
    TopLoc_Location rotLoc(rotation);
	TopLoc_Location newLoc = rotLoc * current_part->shape.Location();
	
	
	// if(current_part->shape.ShapeType() != TopAbs_COMPOUND)
    // 	current_part->shape.Location(newLoc);
	// 	else
	// 	current_part->shape.Location(rotLoc);
	
	
	// current_part->shape.Location(newLoc);

	if(current_part->shape.ShapeType() == TopAbs_COMPOUND){
		current_part->shape.Location(newLoc);
		// current_part->current_location = rotLoc * current_part->current_location;
	}else{
		current_part->shape.Location(newLoc);
		// current_part->current_location = rotLoc*current_part->current_location ;
	}


	// cotm("rotate");
	// PrintLocationDegrees(current_part->shape.Location());

    // 4. Aplicar a rotação na shape alvo (acumulativo)
    // LastShape(current_part->cshape).Move(rotLoc);

	// if(!current_part->shape.IsNull())
    	// current_part->shape.Move(rotLoc);
		// else
    	// current_part->cshape.Move(rotLoc);

	// inteligentmerge(current_part->shape);
	inteligentset();
	// current_part->current_location=newLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*newLoc;
	// current_part->current_location=rotLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*rotLoc;

    // current_part->current_location = newLoc;
    // current_part->current_location = current_part->shape.Location();
 
}
void Rotatel_local(float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 1){
    if (!current_part || current_part->shape.IsNull())
        return;

    // 1. Validate axis
    if (x == 0 && y == 0 && z == 0)
        return;

    // 2. Build rotation in LOCAL coordinates
    gp_Dir axisDir(x, y, z);
    double angleRad = angleDegrees * (M_PI / 180.0);

    gp_Trsf localRot;
    localRot.SetRotation(gp_Ax1(gp_Pnt(0,0,0), axisDir), angleRad);

    // 3. Get Originl (fixed reference frame)
    gp_Trsf origin = current_part->current_location;
    // gp_Trsf origin = current_part->shape.Location();
    // gp_Trsf origin = current_part->Originl.Transformation();
    gp_Trsf originInv = origin.Inverted();

    // 4. Get current world transform of the shape
    gp_Trsf current = current_part->shape.Location().Transformation();

    // 5. Compute new world transform:
    //    world' = Originl * R_local * Originl^-1 * world
    gp_Trsf newTrsf = origin * localRot * originInv * current;

    // 6. Apply to shape
    current_part->shape.Location(TopLoc_Location(newTrsf));
	current_part->current_location=TopLoc_Location(newTrsf);

    // 7. Your post‑merge logic
    inteligentset();
}


void Rotatel1(float angleDegrees = 0.0f, int x = 0, int y = 0, int z = 1) { // z=1 como padrão
    if (!current_part ) return;
    // if (!current_part || current_part->shape.IsNull()) return;

    // 1. Validar direção para evitar crash
    if (x == 0 && y == 0 && z == 0) return; 

	cotm("rotate");
	PrintLocationDegrees(current_part->current_location);
    // 2. Definir o eixo de rotação
    // Pegamos a translação atual para girar em torno do "centro" da peça
    gp_Trsf currentTrsf = current_part->current_location.Transformation();
    // gp_Trsf currentTrsf = current_part->shape.Location().Transformation();
    gp_Pnt pivot = currentTrsf.TranslationPart(); 
    gp_Dir direction(x, y, z);
    gp_Ax1 axis(pivot, direction);

    // 3. Criar a transformação de rotação
    gp_Trsf rotation;
    rotation.SetRotation(axis, angleDegrees * (M_PI / 180.0));
    TopLoc_Location rotLoc(rotation);
	TopLoc_Location newLoc = rotLoc * current_part->shape.Location();
	
	
	// if(current_part->shape.ShapeType() != TopAbs_COMPOUND)
    // 	current_part->shape.Location(newLoc);
	// 	else
	// 	current_part->shape.Location(rotLoc);
	
	
	current_part->shape.Location(newLoc);


	cotm("rotate");
	PrintLocationDegrees(current_part->shape.Location());

    // 4. Aplicar a rotação na shape alvo (acumulativo)
    // LastShape(current_part->cshape).Move(rotLoc);

	// if(!current_part->shape.IsNull())
    	// current_part->shape.Move(rotLoc);
		// else
    	// current_part->cshape.Move(rotLoc);

	// inteligentmerge(current_part->shape);
	inteligentset();
	// current_part->current_location=newLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*newLoc;
	current_part->current_location=rotLoc*current_part->current_location;
	// current_part->current_location=current_part->current_location*rotLoc;

    // current_part->current_location = current_part->shape.Location();

    // 5. Reconstruir o Compound
    // BRep_Builder builder;
    // TopoDS_Compound newCompound;
    // builder.MakeCompound(newCompound);

    // TopoDS_Iterator it(current_part->cshape);
    // for (; it.More(); it.Next()) {
    //     const TopoDS_Shape& s = it.Value();
        
    //     // Se for a shape que estamos movendo, adicionamos a versão atualizada
    //     // Usamos Partner para comparar geometria, já que a Location mudou
    //     if (s.IsPartner(current_part->shape)) {
    //         builder.Add(newCompound, current_part->shape);
    //     } else {
    //         builder.Add(newCompound, s);
    //     }
    // }

    // // 6. Atualizar sistema e visualização
    // current_part->cshape = newCompound;
    // if (current_part->ashape) {
    //     current_part->ashape->Set(current_part->cshape);
    // }
    
    // // IMPORTANTE: Atualize sua variável global de localização aqui!
    // current_part->current_location = current_part->shape.Location();
}


void Subtract() {
    if (!current_part)
        lua_error_with_where( "No current part. Call Part(name) first.");

	
	AddToCompound(current_part->cshape,current_part->shape);
    TopoDS_Compound& compound = current_part->cshape;
    if (compound.IsNull()) {
        lua_error_with_where("Current part's compound is null.");
    }

    // Collect top-level solids and faces
    std::vector<TopoDS_Solid> solids;
    std::vector<TopoDS_Face>  faces;

    for (TopExp_Explorer ex(compound, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }
    for (TopExp_Explorer ex(compound, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.push_back(TopoDS::Face(ex.Current()));
    }
	
	if(current_part->shape.ShapeType() == TopAbs_SOLID)
		solids.push_back(TopoDS::Solid(current_part->shape));
  

    bool is3D = !solids.empty();
    bool is2D = !is3D && !faces.empty();

    size_t count = is3D ? solids.size() : faces.size();
    if (count < 2) {
        lua_error_with_where("Subtract requires at least two objects (solids or faces).");
    }

    TopoDS_Shape object; // base (from which we subtract)
    TopoDS_Shape tool;   // cutter (subtracted)

    if (is3D) {
        object = solids[solids.size() - 2];
        tool   = solids.back();
    } else {
        object = faces[faces.size() - 2];
        tool   = faces.back();
    }

    // Boolean cut
    BRepAlgoAPI_Cut cutOp(object, tool);

    // Increase fuzzy tolerance for better robustness (especially useful for 2D-like cases)
    cutOp.SetFuzzyValue(1e-6);

    cutOp.Build();

    if (!cutOp.IsDone()) {
        lua_error_with_where("Boolean subtract (cut) failed.");
    }

    TopoDS_Shape result = cutOp.Shape();
    if (result.IsNull()) {
        lua_error_with_where("Boolean subtract produced null shape.");
    }

    // Optional cleanup – unify coincident edges/vertices
    ShapeUpgrade_UnifySameDomain unifier(result, true, true, true);
    unifier.Build();
    if (!unifier.Shape().IsNull()) {
        result = unifier.Shape();
    }

    // Rebuild compound: keep all except the last two, add the new result
    // TopoDS_Compound newCompound;
    // BRep_Builder builder;
    // builder.MakeCompound(newCompound);

    // Collect all top-level objects in order
    TopTools_ListOfShape allShapes;
    for (TopExp_Explorer ex(compound, is3D ? TopAbs_SOLID : TopAbs_FACE); ex.More(); ex.Next()) {
        allShapes.Append(ex.Current());
    }

    // Convert to vector for indexing
    std::vector<TopoDS_Shape> shapeVec;
    for (TopTools_ListIteratorOfListOfShape it(allShapes); it.More(); it.Next()) {
        shapeVec.push_back(it.Value());
    }



    current_part->cshape=TopoDS_Compound();
    // current_part->cshape.Nullify();
	current_part->builder=BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();


	// current_part->cshape=TopoDS_Compound();
	// cotm(shapeVec.size())
	if (shapeVec.size() > 1) {
		// Add all except the last two
		for (size_t i = 0; i < shapeVec.size() - 2; ++i) {
			current_part->builder.Add(current_part->cshape, shapeVec[i]);
		}

		// Add the result
		// builder.Add(newCompound, result);

		// Update current part
		// current_part->cshape = newCompound;
	}
	// cotm(1)
	// TopoDS_Face f= BRepBuilderAPI_MakeFace(result);

// TopoDS_Solid solid=TopoDS_Solid();
// if(solid.IsNull()){
// 	printf("isnull\n");
// 	getchar();
// }



	if (is2D) {
		TopoDS_Face face;
		TopExp_Explorer ex(result, TopAbs_FACE);
		if (ex.More()) {
			face = TopoDS::Face(ex.Current());
		}
		inteligentmerge(face);
	} else {
		TopoDS_Solid solid=TopoDS_Solid();
		TopExp_Explorer ex(result, TopAbs_SOLID);
		if (ex.More()) {
			solid = TopoDS::Solid(ex.Current());
		}
		if(!solid.IsNull())
			inteligentmerge(solid);
			else{
			// inteligentmerge(current_part->shape);
			// inteligentSetFast(current_part);
			lua_error_with_where("Solid dont intersect to subtract");
			}
	}
	// current_part->shape  = result;  // last result is the "active" shape
}

//Common = Subtract, it only change BRepAlgoAPI_Cut to BRepAlgoAPI_Common
void CommonAndSubtract(bool iscommon=0) {
    if (!current_part)
        lua_error_with_where( "No current part. Call Part(name) first.");
	
	TopLoc_Location preserve=current_part->shape.Location();

	AddToCompound(current_part->cshape,current_part->shape);
    TopoDS_Compound& compound = current_part->cshape;
    if (compound.IsNull()) {
        lua_error_with_where("Current part's compound is null.");
    }

    // Collect top-level solids and faces
    std::vector<TopoDS_Solid> solids;
    std::vector<TopoDS_Face>  faces;

    for (TopExp_Explorer ex(compound, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }
    for (TopExp_Explorer ex(compound, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.push_back(TopoDS::Face(ex.Current()));
    }
	
	// if(current_part->shape.ShapeType() == TopAbs_SOLID)
	// 	solids.push_back(TopoDS::Solid(current_part->shape));
  

    bool is3D = !solids.empty();
    bool is2D = !is3D && !faces.empty();

    size_t count = is3D ? solids.size() : faces.size();
    if (count < 2) {
        lua_error_with_where("Subtract requires at least two objects (solids or faces).");
    }

    TopoDS_Shape object; // base (from which we subtract)
    TopoDS_Shape tool;   // cutter (subtracted)

    if (is3D) {
        object = solids[solids.size() - 2];
        tool   = solids.back();
    } else {
        object = faces[faces.size() - 2];
        tool   = faces.back();
    }



std::unique_ptr<BRepAlgoAPI_BooleanOperation> cutOp;

if (iscommon) {
    cutOp = std::make_unique<BRepAlgoAPI_Common>(object, tool);
} else {
    cutOp = std::make_unique<BRepAlgoAPI_Cut>(object, tool);
}


    // Boolean cut
	// if(iscommon)
    // 	BRepAlgoAPI_Common cutOp(object, tool);
	// 	else
	// 	BRepAlgoAPI_Cut cutOp(object, tool);

    // Increase fuzzy tolerance for better robustness (especially useful for 2D-like cases)
    cutOp->SetFuzzyValue(1e-6);
	// cutOp->SetRunParallel(Standard_True);
    cutOp->Build();

    if (!cutOp->IsDone()) {
        lua_error_with_where("Boolean subtract (cut) failed.");
    }

    TopoDS_Shape result = cutOp->Shape();
    if (result.IsNull()) {
        lua_error_with_where("Boolean subtract produced null shape.");
    }

    // Optional cleanup – unify coincident edges/vertices
    ShapeUpgrade_UnifySameDomain unifier(result, true, true, true);
    unifier.Build();
    if (!unifier.Shape().IsNull()) {
        result = unifier.Shape();
    }

    // Rebuild compound: keep all except the last two, add the new result
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);

    // Collect all top-level objects in order
    TopTools_ListOfShape allShapes;
    for (TopExp_Explorer ex(compound, is3D ? TopAbs_SOLID : TopAbs_FACE); ex.More(); ex.Next()) {
        allShapes.Append(ex.Current());
    }

    // Convert to vector for indexing
    std::vector<TopoDS_Shape> shapeVec;
    for (TopTools_ListIteratorOfListOfShape it(allShapes); it.More(); it.Next()) {
        shapeVec.push_back(it.Value());
    }

	current_part->cshape=TopoDS_Compound();
	// cotm(shapeVec.size())
	if (shapeVec.size() > 1) {
		// Add all except the last two
		for (size_t i = 0; i < shapeVec.size() - 2; ++i) {
			builder.Add(newCompound, shapeVec[i]);
		}

		// Add the result
		// builder.Add(newCompound, result);

		// Update current part
		current_part->cshape = newCompound;
	}
	// cotm(1)
	if(is2D){
		TopoDS_Face face;
		TopExp_Explorer ex(result, TopAbs_FACE);
		if (ex.More()) {
			face = TopoDS::Face(ex.Current());
		}
		current_part->shape  = face;
		// inteligentmerge(face,0);
	}
	else{
		// SetReferenceLocationWithoutMoving(result,preserve);
		current_part->shape  = result;  // last result is the "active" shape
		// inteligentmerge(result,0);
		// SetReferenceLocationWithoutMoving(current_part->shape,preserve.Transformation());
	}
}
void Fusenw() {
    if (!current_part)
        lua_error_with_where( "No current part. Call Part(name) first.");

	// AddToCompound(current_part->cshape,current_part->shape);
    TopoDS_Compound& compound = current_part->cshape;
    if (compound.IsNull()) {
        lua_error_with_where("Current part's compound is null.");
    }

    // Collect top-level solids and faces
    std::vector<TopoDS_Solid> solids;
    std::vector<TopoDS_Face>  faces;

    for (TopExp_Explorer ex(compound, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }

	
    for (TopExp_Explorer ex(compound, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.push_back(TopoDS::Face(ex.Current()));
    }
	
	if(current_part->shape.ShapeType() == TopAbs_SOLID)
		solids.push_back(TopoDS::Solid(current_part->shape));
  

    bool is3D = !solids.empty();
    bool is2D = !is3D && !faces.empty();

    size_t count = is3D ? solids.size() : faces.size();
    if (count < 2) {
        lua_error_with_where("Subtract requires at least two objects (solids or faces).");
    }

    TopoDS_Shape object; // base (from which we subtract)
    TopoDS_Shape tool;   // cutter (subtracted)

    if (is3D) {
        object = solids[solids.size() - 2];
        tool   = solids.back();
    } else {
        object = faces[faces.size() - 2];
        tool   = faces.back();
    }

    // Boolean cut
    BRepAlgoAPI_Fuse cutOp(object, tool);

    // Increase fuzzy tolerance for better robustness (especially useful for 2D-like cases)
    cutOp.SetFuzzyValue(1e-6);

    cutOp.Build();

    if (!cutOp.IsDone()) {
        lua_error_with_where("Boolean subtract (cut) failed.");
    }

    TopoDS_Shape result = cutOp.Shape();
    if (result.IsNull()) {
        lua_error_with_where("Boolean subtract produced null shape.");
    }

    // Optional cleanup – unify coincident edges/vertices
    ShapeUpgrade_UnifySameDomain unifier(result, true, true, true);
    unifier.Build();
    if (!unifier.Shape().IsNull()) {
        result = unifier.Shape();
    }

    // Rebuild compound: keep all except the last two, add the new result
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);

    // Collect all top-level objects in order
    TopTools_ListOfShape allShapes;
    for (TopExp_Explorer ex(compound, is3D ? TopAbs_SOLID : TopAbs_FACE); ex.More(); ex.Next()) {
        allShapes.Append(ex.Current());
    }

    // Convert to vector for indexing
    std::vector<TopoDS_Shape> shapeVec;
    for (TopTools_ListIteratorOfListOfShape it(allShapes); it.More(); it.Next()) {
        shapeVec.push_back(it.Value());
    }

	current_part->cshape=TopoDS_Compound();
	// cotm(shapeVec.size())
	if (shapeVec.size() > 1) {
		// Add all except the last two
		for (size_t i = 0; i < shapeVec.size() - 2; ++i) {
			builder.Add(newCompound, shapeVec[i]);
		}

		// Add the result
		builder.Add(newCompound, result);

		// Update current part
		current_part->cshape = newCompound;
	}
	// cotm(1)
	current_part->shape  = result;  // last result is the "active" shape
}
void Fusenw1() {
    if (!current_part)
        lua_error_with_where( "No current part. Call Part(name) first.");

	AddToCompound(current_part->cshape,current_part->shape);
    TopoDS_Compound& compound = current_part->cshape;
    if (compound.IsNull()) {
        lua_error_with_where("Current part's compound is null.");
    }

    // Collect top-level solids and faces
    std::vector<TopoDS_Solid> solids;
    std::vector<TopoDS_Face>  faces;

    for (TopExp_Explorer ex(compound, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }
    for (TopExp_Explorer ex(compound, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.push_back(TopoDS::Face(ex.Current()));
    }
	
	// if(current_part->shape.ShapeType() == TopAbs_SOLID)
	// 	solids.push_back(TopoDS::Solid(current_part->shape));
  

    bool is3D = !solids.empty();
    bool is2D = !is3D && !faces.empty();

    size_t count = is3D ? solids.size() : faces.size();
    if (count < 2) {
        lua_error_with_where("Subtract requires at least two objects (solids or faces).");
    }

    TopoDS_Shape object; // base (from which we subtract)
    TopoDS_Shape tool;   // cutter (subtracted)

    if (is3D) {
        object = solids[solids.size() - 2];
        tool   = solids.back();
    } else {
        object = faces[faces.size() - 2];
        tool   = faces.back();
    }

    // Boolean cut
    BRepAlgoAPI_Fuse cutOp(object, tool);

    // Increase fuzzy tolerance for better robustness (especially useful for 2D-like cases)
    cutOp.SetFuzzyValue(1e-6);

    cutOp.Build();

    if (!cutOp.IsDone()) {
        lua_error_with_where("Boolean subtract (cut) failed.");
    }

    TopoDS_Shape result = cutOp.Shape();
    if (result.IsNull()) {
        lua_error_with_where("Boolean subtract produced null shape.");
    }

    // Optional cleanup – unify coincident edges/vertices
    ShapeUpgrade_UnifySameDomain unifier(result, true, true, true);
    unifier.Build();
    if (!unifier.Shape().IsNull()) {
        result = unifier.Shape();
    }

    // Rebuild compound: keep all except the last two, add the new result
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);

    // Collect all top-level objects in order
    TopTools_ListOfShape allShapes;
    for (TopExp_Explorer ex(compound, is3D ? TopAbs_SOLID : TopAbs_FACE); ex.More(); ex.Next()) {
        allShapes.Append(ex.Current());
    }

    // Convert to vector for indexing
    std::vector<TopoDS_Shape> shapeVec;
    for (TopTools_ListIteratorOfListOfShape it(allShapes); it.More(); it.Next()) {
        shapeVec.push_back(it.Value());
    }

	current_part->cshape=TopoDS_Compound();
	// cotm(shapeVec.size())
	if (shapeVec.size() > 1) {
		// Add all except the last two
		for (size_t i = 0; i < shapeVec.size() - 2; ++i) {
			builder.Add(newCompound, shapeVec[i]);
		}

		// Add the result
		builder.Add(newCompound, result);

		// Update current part
		current_part->cshape = newCompound;
	}
	// cotm(1)
	current_part->shape  = result;  // last result is the "active" shape
}

void Fuse1() {
    if (!current_part)
        lua_error_with_where( "No current part. Call Part(name) first.");

	AddToCompound(current_part->cshape,current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect solids (3D) ---
    std::vector<TopoDS_Solid> solids;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.push_back(TopoDS::Solid(ex.Current()));
    }

    // --- Collect wires (2D) ---
    std::vector<TopoDS_Wire> wires;
    for (TopExp_Explorer ex(c, TopAbs_WIRE); ex.More(); ex.Next()) {
        wires.push_back(TopoDS::Wire(ex.Current()));
    }

    // --- Collect edges (2D, e.g. circles as single edges) ---
    std::vector<TopoDS_Edge> edges;
    for (TopExp_Explorer ex(c, TopAbs_EDGE); ex.More(); ex.Next()) {
        const TopoDS_Shape& s = ex.Current();
        // Skip edges that are already part of collected wires/solids if needed,
        // but for now we just collect all edges.
        edges.push_back(TopoDS::Edge(s));
    }

    const bool has3D = solids.size() >= 2;
    bool has2D_wires = (!has3D && wires.size() >= 2);
    bool has2D_edges = (!has3D && !has2D_wires && edges.size() >= 2);

    if (!has3D && !has2D_wires && !has2D_edges) {
        lua_error_with_where("Need at least two solids, two wires, or two edges to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE
    // ============================================================
    if (has3D) {
        const TopoDS_Solid& a = solids[solids.size() - 2];
        const TopoDS_Solid& b = solids.back();

        BRepAlgoAPI_Fuse fuseOp(a, b);
        fuseOp.Build();
        if (!fuseOp.IsDone()) {
            lua_error_with_where("3D fuse operation failed.");
        }

        fused = fuseOp.Shape();
    }

    // ============================================================
    //                       2D FUSE (WIRES)
    // ============================================================
    else if (has2D_wires) {
        TopoDS_Wire w1 = wires[wires.size() - 2];
        TopoDS_Wire w2 = wires.back();

        if (!BRep_Tool::IsClosed(w1))
            lua_error_with_where( "Wire 1 is not closed.");
        if (!BRep_Tool::IsClosed(w2))
            lua_error_with_where( "Wire 2 is not closed.");

        TopoDS_Face f1 = BRepBuilderAPI_MakeFace(w1, true);
        TopoDS_Face f2 = BRepBuilderAPI_MakeFace(w2, true);

        BRepAlgoAPI_Fuse fuseOp(f1, f2);
        fuseOp.Build();
        if (!fuseOp.IsDone()) {
            lua_error_with_where( "2D fuse operation (wires) failed.");
        }

        // Keep full fused face shape (can be one or more faces)
        fused = fuseOp.Shape();
    }

    // ============================================================
    //                       2D FUSE (EDGES, e.g. circles)
    // ============================================================
    else if (has2D_edges) {
        TopoDS_Edge e1 = edges[edges.size() - 2];
        TopoDS_Edge e2 = edges.back();

        // Build wires from edges (for circles, each edge is already closed)
        BRepBuilderAPI_MakeWire mw1;
        mw1.Add(e1);
        if (!mw1.IsDone())
            lua_error_with_where("Failed to build wire from edge 1.");
        TopoDS_Wire w1 = mw1.Wire();

        BRepBuilderAPI_MakeWire mw2;
        mw2.Add(e2);
        if (!mw2.IsDone())
            lua_error_with_where( "Failed to build wire from edge 2.");
        TopoDS_Wire w2 = mw2.Wire();

        if (!BRep_Tool::IsClosed(w1))
            lua_error_with_where( "Edge 1 wire is not closed.");
        if (!BRep_Tool::IsClosed(w2))
            lua_error_with_where("Edge 2 wire is not closed.");

        TopoDS_Face f1 = BRepBuilderAPI_MakeFace(w1, true);
        TopoDS_Face f2 = BRepBuilderAPI_MakeFace(w2, true);

        BRepAlgoAPI_Fuse fuseOp(f1, f2);
        fuseOp.Build();
        if (!fuseOp.IsDone()) {
            lua_error_with_where( "2D fuse operation (edges) failed.");
        }

        // Keep full fused face shape
        fused = fuseOp.Shape();
    }

    // ============================================================
    //                GEOMETRY REFINEMENT (optional)
    // ============================================================
    {
        ShapeUpgrade_UnifySameDomain refine(fused, true, true, true);
        refine.Build();
        TopoDS_Shape refined = refine.Shape();
        if (!refined.IsNull()) {
            fused = refined;
        }
    }

    // ============================================================
    //                REBUILD COMPOUND
    // ============================================================
    // TopoDS_Compound result;
    // TopoDS_Builder builder;
    // builder.MakeCompound(result);

    // for (TopExp_Explorer ex(c, TopAbs_SHAPE); ex.More(); ex.Next()) {
    //     const TopoDS_Shape& s = ex.Current();

    //     if (has3D && s.ShapeType() == TopAbs_SOLID) {
    //         const TopoDS_Solid sol = TopoDS::Solid(s);
    //         if (sol.IsSame(solids.back()) || sol.IsSame(solids[solids.size() - 2]))
    //             continue;
    //     }

    //     if (has2D_wires && s.ShapeType() == TopAbs_WIRE) {
    //         const TopoDS_Wire w = TopoDS::Wire(s);
    //         if (w.IsSame(wires.back()) || w.IsSame(wires[wires.size() - 2]))
    //             continue;
    //     }

    //     if (has2D_edges && s.ShapeType() == TopAbs_EDGE) {
    //         const TopoDS_Edge e = TopoDS::Edge(s);
    //         if (e.IsSame(edges.back()) || e.IsSame(edges[edges.size() - 2]))
    //             continue;
    //     }

    //     builder.Add(result, s);
    // }

    // builder.Add(result, fused);

		cotm("3dfuse");
    current_part->cshape.Nullify();
	current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
	inteligentmerge(fused);
}

#include <BOPAlgo_BuilderFace.hxx>
#include <BRepExtrema_ExtFF.hxx>
void Fuse2() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect shapes into lists for OpenCASCADE API ---
    TopTools_ListOfShape solids, faces;
    
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.Append(ex.Current());
    }

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE (ALL SOLIDS)
    // ============================================================
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it); // First solid as base
        for (++it; it != solids.end(); ++it) {
            tools.Append(*it); // All other solids as tools
        }

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        // fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();

        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();
    }

 // ... (solids logic remains the same)

    // ============================================================
    //                       2D FUSE (ALL FACES)
    // ============================================================
// ============================================================
    //                       2D FUSE (REPROCESSED)
    // ============================================================
// TopTools_ListOfShape is a linked list, not std::vector.
// It has no operator[] and no size() indexing.
//
// Use iterator API.

else if (has2D)
{
    // Special case: only one face
    if (faces.Extent() <= 1) {
        fused = (faces.Extent() == 1) ? faces.First() : TopoDS_Shape();
        // continue to unify / finalize
    } 
    else {
        // Build list of individual faces (this is the key fix)
        TopTools_ListOfShape args;

        Standard_Integer validCount = 0;
        for (TopTools_ListIteratorOfListOfShape it(faces); it.More(); it.Next()) {
            const TopoDS_Shape& s = it.Value();
            if (!s.IsNull() && s.ShapeType() == TopAbs_FACE) {
                args.Append(s);
                validCount++;
            }
        }

        if (validCount < 2) {
            fused = (validCount == 1) ? faces.First() : TopoDS_Shape();
            lua_error_with_where("2D fuse: too few valid faces.");
            // continue anyway or return
        } else {
            // Use BOPAlgo_Builder with multiple arguments (each face separately)
            BOPAlgo_Builder builder;
            builder.SetArguments(args);
            builder.SetFuzzyValue(1e-5);
            builder.SetNonDestructive(Standard_True);
            builder.Perform();

            if (builder.HasErrors()) {
                std::ostringstream oss;
                oss << "2D fuse failed in BOPAlgo_Builder";
                builder.DumpErrors(oss);
                lua_error_with_where(oss.str().c_str());
            }

            fused = builder.Shape();
        }
    }

    // 3. Unify coplanar faces + remove duplicate edges (this is what makes a "real" 2D fused face)
if (!fused.IsNull()) {
    ShapeUpgrade_UnifySameDomain unify(fused, Standard_True, Standard_True, Standard_False);
    unify.SetLinearTolerance(1e-5);
    unify.SetAngularTolerance(1e-3);
    unify.Build();

    if (!unify.Shape().IsNull()) {
        fused = unify.Shape();
    }

    // Flatten nested topology — check if result is a compound or shell of one face
    TopoDS_Face onlyFace;
    int faceCount = 0;
    for (TopExp_Explorer exp(fused, TopAbs_FACE); exp.More(); exp.Next()) {
        onlyFace = TopoDS::Face(exp.Current());
        faceCount++;
    }

    if (faceCount == 1) {
        fused = onlyFace; // collapse into a single face
    } else {
        // Optional: try to sew into one shell or face
        BRepBuilderAPI_Sewing sew;
        for (TopExp_Explorer ex(fused, TopAbs_FACE); ex.More(); ex.Next()) {
            sew.Add(ex.Current());
        }
		cotm(faceCount);
        sew.Perform();
        TopoDS_Shape sewed = sew.SewedShape();
        if (!sewed.IsNull()) fused = sewed;
    }
}


        
        // 5. Cleanup: If the result is a Compound/Shell containing one face, extract it
        // TopExp_Explorer faceExp(fused, TopAbs_FACE);
        // if (faceExp.More()) {
        //     TopoDS_Face firstFace = TopoDS::Face(faceExp.Current());
        //     faceExp.Next();
        //     if (!faceExp.More()) {
        //         fused = firstFace; // Successfully merged into exactly one face
        //     }
        // }
    }
    // ============================================================
    //                GEOMETRY REFINEMENT (CRITICAL FOR 2D)
    // ============================================================
    // This part actually "dissolves" the inner edges to make one face
    // ShapeUpgrade_UnifySameDomain refine(fused, true, true, true);
    
    // // Ensure the refinement has enough tolerance to see the faces as one
    // refine.SetLinearTolerance(1e-5);
    // refine.SetAngularTolerance(1e-3); // Roughly 0.05 degrees
    
    // refine.Build();
    
    // if (!refine.Shape().IsNull()) {
    //     fused = refine.Shape();
    // }

// ... (Finalize logic remains the same)

// TopExp_Explorer faceExp(fused, TopAbs_FACE);
// if (faceExp.More()) {
//     fused = faceExp.Current(); // Grab the first (and hopefully only) face
// }

    // Finalize: Clear compound and set the fused result
    cotm("3dfuse"); // Kept original logging/macro
    current_part->cshape.Nullify();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
    
    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
void Fusegoodbutfailoverlap() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect shapes into lists for OpenCASCADE API ---
    TopTools_ListOfShape solids, faces;
    
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.Append(ex.Current());
    }

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE (ALL SOLIDS)
    // ============================================================
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) {
            tools.Append(*it);
        }

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();

        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();
        
        // Unify same domain for 3D result
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // ============================================================
    //                       2D FUSE (ALL FACES)
    // ============================================================
    else if (has2D) {
        // Special case: only one face
        if (faces.Extent() <= 1) {
            fused = (faces.Extent() == 1) ? faces.First() : TopoDS_Shape();
        } 
        else {
            // First, sew the faces together to create a shell
            BRepBuilderAPI_Sewing sewer(1e-5);
            
            for (TopTools_ListIteratorOfListOfShape it(faces); it.More(); it.Next()) {
                const TopoDS_Shape& s = it.Value();
                if (!s.IsNull() && s.ShapeType() == TopAbs_FACE) {
                    sewer.Add(s);
                }
            }
            
            sewer.Perform();
            TopoDS_Shape sewedShape = sewer.SewedShape();
            
            if (sewedShape.IsNull()) {
                lua_error_with_where("2D fuse: sewing failed");
            }
            
            // Now use BRepAlgoAPI_Fuse on the sewed shell/faces
            TopTools_ListOfShape args;
            TopTools_ListOfShape tools;
            
            // Collect all faces from the sewed shape
            TopTools_ListOfShape allFaces;
            for (TopExp_Explorer ex(sewedShape, TopAbs_FACE); ex.More(); ex.Next()) {
                allFaces.Append(ex.Current());
            }
            
            if (allFaces.Extent() >= 2) {
                // Use BRepAlgoAPI_Fuse for 2D faces
                auto it = allFaces.begin();
                args.Append(*it);
                for (++it; it != allFaces.end(); ++it) {
                    tools.Append(*it);
                }
                
                BRepAlgoAPI_Fuse fuseOp;
                fuseOp.SetArguments(args);
                fuseOp.SetTools(tools);
                fuseOp.SetFuzzyValue(1e-5);
                fuseOp.Build();
                
                if (fuseOp.IsDone()) {
                    fused = fuseOp.Shape();
                } else {
                    // Fallback to BOPAlgo_Builder if Fuse fails
                    BOPAlgo_Builder builder;
                    builder.SetArguments(allFaces);
                    builder.SetFuzzyValue(1e-5);
                    builder.Perform();
                    // BOPAlgo_Builder uses HasErrors() instead of IsDone()
                    if (!builder.HasErrors()) {
                        fused = builder.Shape();
                    }
                }
            } else if (allFaces.Extent() == 1) {
                fused = allFaces.First();
            }
        }
        
        // Critical: Unify coplanar faces to merge them into a single face
        if (!fused.IsNull()) {
            // First attempt: UnifySameDomain with strong settings
            ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
            unify.SetLinearTolerance(1e-4);  // Increased tolerance
            unify.SetAngularTolerance(1e-2); // ~0.57 degrees
            unify.Build();
            
            TopoDS_Shape refined = unify.Shape();
            if (!refined.IsNull()) {
                fused = refined;
            }
            
            // Extract the resulting faces
            TopTools_ListOfShape resultFaces;
            for (TopExp_Explorer ex(fused, TopAbs_FACE); ex.More(); ex.Next()) {
                resultFaces.Append(ex.Current());
            }
            
            // If we still have multiple faces, try to merge them using BRepBuilderAPI_MakeFace with planar wires
            if (resultFaces.Extent() > 1) {
                // Collect all wires from all faces
                TopTools_ListOfShape allWires;
                for (TopTools_ListIteratorOfListOfShape it(resultFaces); it.More(); it.Next()) {
                    const TopoDS_Face& face = TopoDS::Face(it.Value());
                    for (TopExp_Explorer wireExp(face, TopAbs_WIRE); wireExp.More(); wireExp.Next()) {
                        allWires.Append(wireExp.Current());
                    }
                }
                
                // Try to create a single face from all wires using BRepBuilderAPI_MakeFace
                if (allWires.Extent() > 0) {
                    // Get the plane from the first face
                    const TopoDS_Face& firstFace = TopoDS::Face(resultFaces.First());
                    Handle(Geom_Surface) surface = BRep_Tool::Surface(firstFace);
                    
                    if (!surface.IsNull()) {
                        BRepBuilderAPI_MakeFace faceMaker(surface, 1e-5);
                        
                        // Add all wires - need to convert TopoDS_Shape to TopoDS_Wire
                        for (TopTools_ListIteratorOfListOfShape wireIt(allWires); wireIt.More(); wireIt.Next()) {
                            const TopoDS_Shape& wireShape = wireIt.Value();
                            if (wireShape.ShapeType() == TopAbs_WIRE) {
                                TopoDS_Wire wire = TopoDS::Wire(wireShape);
                                faceMaker.Add(wire);
                            }
                        }
                        
                        if (faceMaker.IsDone()) {
                            TopoDS_Face newFace = faceMaker.Face();
                            if (!newFace.IsNull()) {
                                fused = newFace;
                            }
                        }
                    }
                }
            }
            
            // Final cleanup: If the result is a compound with one face, extract it
            TopExp_Explorer faceExp(fused, TopAbs_FACE);
            if (faceExp.More()) {
                TopoDS_Face firstFace = TopoDS::Face(faceExp.Current());
                faceExp.Next();
                if (!faceExp.More()) {
                    fused = firstFace; // Exactly one face
                }
            }
        }
    }

    // Finalize: Clear compound and set the fused result
    current_part->cshape.Nullify();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
    
    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
// Required OCCT headers (add to your file top if not already present)
#include <BRepAlgoAPI_Fuse.hxx>
#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Tool.hxx> 
#include <BRepLib.hxx>
#include <BRepTools.hxx>
#include <Standard_TypeDef.hxx>

// Your existing helper declarations (assumed)
// extern void lua_error_with_where(const char* msg);
// extern void AddToCompound(TopoDS_Compound& c, const TopoDS_Shape& s);
// extern void inteligentmerge(const TopoDS_Shape& s);

// The improved Fuse function
// Headers (add to top of file if not already present)
#include <BRepAlgoAPI_Fuse.hxx>
#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepLib.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <Standard_TypeDef.hxx>
 
void Fusenotworking() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;
    current_part->start_location = getShapePlacement(c);

    // Collect solids and faces
    TopTools_ListOfShape solids, faces;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) solids.Append(ex.Current());
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) faces.Append(ex.Current());

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D)
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");

    TopoDS_Shape fused;

    // -------------------------
    // 3D fuse (unchanged)
    // -------------------------
    if (has3D) {
        TopTools_ListOfShape args, tools;
        auto it = solids.begin();
        args.Append(*it);
        for (++it; it != solids.end(); ++it) tools.Append(*it);

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(args);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();
        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();

        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // -------------------------
    // 2D fuse (robust: split -> trim -> sew -> face)
    // -------------------------
    else if (has2D) {
        const double tol = 1e-5;
        const double mapTol = tol * 10.0;

        // 1) Collect raw edges from the compound
        std::vector<TopoDS_Edge> rawEdges;
        {
            TopExp_Explorer exE(c, TopAbs_EDGE);
            while (exE.More()) {
                rawEdges.push_back(TopoDS::Edge(exE.Current()));
                exE.Next();
            }
        }
        if (rawEdges.empty()) lua_error_with_where("No edges found for 2D fuse.");

        // 2) Build a compound of all edges (for splitting)
        // Build list of edges for splitter
TopTools_ListOfShape argList;
TopTools_ListOfShape toolList;

for (const TopoDS_Edge& e : rawEdges) {
    argList.Append(e);
    toolList.Append(e);
}

// Split edges by all edges
BRepAlgoAPI_Splitter splitter;
splitter.SetArguments(argList);
splitter.SetTools(toolList);
splitter.SetFuzzyValue(mapTol);
splitter.Build();

if (!splitter.IsDone())
    lua_error_with_where("Edge splitting failed.");

TopoDS_Shape splitShape = splitter.Shape();


        // 4) Extract split edges
        std::vector<TopoDS_Edge> splitEdges;
        for (TopExp_Explorer ex(splitShape, TopAbs_EDGE); ex.More(); ex.Next())
            splitEdges.push_back(TopoDS::Edge(ex.Current()));
        if (splitEdges.empty()) lua_error_with_where("No split edges produced.");

        // Helper: get vertex point
        auto vertexPoint = [&](const TopoDS_Vertex& v)->gp_Pnt { return BRep_Tool::Pnt(v); };

        // Helper: quantize double to long long for stable keys
        auto q = [&](double x, double qtol)->long long { return (long long)std::llround(x / qtol); };

        // 5) Group colinear linear segments by line key
        struct LineGroup {
            gp_Pnt origin;
            gp_Dir dir;
            std::vector<std::pair<double,double>> intervals;
        };
        typedef std::tuple<long long,long long,long long,long long,long long,long long> LineKey;
        std::map<LineKey, LineGroup> lineGroups;

        for (const TopoDS_Edge& e : splitEdges) {
            Standard_Real f, l;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(e, f, l);
            if (curve.IsNull()) continue;

            // Only treat straight segments for trimming/merging
            Handle(Geom_Line) gline = Handle(Geom_Line)::DownCast(curve);
            if (gline.IsNull()) continue;

            gp_Lin pl = gline->Lin();
            gp_Pnt origin = pl.Location();
            gp_Dir dir = pl.Direction();

            // endpoints
            TopoDS_Vertex va, vb;
            TopExp::Vertices(e, va, vb);
            if (va.IsNull() || vb.IsNull()) continue;
            gp_Pnt pa = vertexPoint(va);
            gp_Pnt pb = vertexPoint(vb);

            // project endpoints onto line to get scalar parameters
            gp_Vec v0(origin, pa);
            gp_Vec v1(origin, pb);
            double t0 = v0.Dot(gp_Vec(dir));
            double t1 = v1.Dot(gp_Vec(dir));
            if (t1 < t0) std::swap(t0, t1);

            // normalize direction sign so opposite directions map to same key
            gp_Dir ndir = dir;
            if (ndir.X() < -1e-12 || (std::abs(ndir.X()) < 1e-12 && ndir.Y() < -1e-12) ||
                (std::abs(ndir.X()) < 1e-12 && std::abs(ndir.Y()) < 1e-12 && ndir.Z() < -1e-12)) {
                ndir.Reverse();
                double nt0 = -t1;
                double nt1 = -t0;
                t0 = nt0; t1 = nt1;
            }

            // quantize direction and origin to form key
            long long dx = q(ndir.X(), mapTol);
            long long dy = q(ndir.Y(), mapTol);
            long long dz = q(ndir.Z(), mapTol);
            long long ox = q(origin.X(), mapTol);
            long long oy = q(origin.Y(), mapTol);
            long long oz = q(origin.Z(), mapTol);

            LineKey key = std::make_tuple(dx,dy,dz,ox,oy,oz);

            auto it = lineGroups.find(key);
            if (it == lineGroups.end()) {
                LineGroup lg;
                lg.origin = origin;
                lg.dir = ndir;
                lg.intervals.push_back(std::make_pair(t0, t1));
                lineGroups.emplace(key, std::move(lg));
            } else {
                it->second.intervals.push_back(std::make_pair(t0, t1));
            }
        }

        // 6) Merge overlapping intervals and recreate trimmed edges
        std::vector<TopoDS_Edge> processedEdges;
        for (auto& kv : lineGroups) {
            LineGroup& lg = kv.second;
            auto& iv = lg.intervals;
            if (iv.empty()) continue;

            std::sort(iv.begin(), iv.end(), [](const std::pair<double,double>& a, const std::pair<double,double>& b){
                if (a.first == b.first) return a.second < b.second;
                return a.first < b.first;
            });

            std::vector<std::pair<double,double>> merged;
            double cur0 = iv[0].first;
            double cur1 = iv[0].second;
            for (size_t i = 1; i < iv.size(); ++i) {
                double a0 = iv[i].first;
                double a1 = iv[i].second;
                if (a0 <= cur1 + mapTol) {
                    cur1 = std::max(cur1, a1);
                } else {
                    merged.push_back({cur0, cur1});
                    cur0 = a0; cur1 = a1;
                }
            }
            merged.push_back({cur0, cur1});

            for (const auto& seg : merged) {
                double t0 = seg.first;
                double t1 = seg.second;
                gp_Pnt p0(lg.origin.X() + lg.dir.X() * t0,
                          lg.origin.Y() + lg.dir.Y() * t0,
                          lg.origin.Z() + lg.dir.Z() * t0);
                gp_Pnt p1(lg.origin.X() + lg.dir.X() * t1,
                          lg.origin.Y() + lg.dir.Y() * t1,
                          lg.origin.Z() + lg.dir.Z() * t1);

                BRepBuilderAPI_MakeEdge me(p0, p1);
                if (me.IsDone()) processedEdges.push_back(TopoDS::Edge(me.Edge()));
            }
        }

        // 7) Add back non-linear split edges (curves) unchanged
        for (const TopoDS_Edge& e : splitEdges) {
            Standard_Real f, l;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(e, f, l);
            if (curve.IsNull()) continue;
            Handle(Geom_Line) gline = Handle(Geom_Line)::DownCast(curve);
            if (gline.IsNull()) {
                processedEdges.push_back(e);
            }
        }

        if (processedEdges.empty()) lua_error_with_where("No processed edges after trimming.");

        // 8) Sew processed edges into wires
        BRepBuilderAPI_Sewing wireSewer(mapTol, true, true, true);
        for (const TopoDS_Edge& e : processedEdges) wireSewer.Add(e);
        wireSewer.Perform();
        TopoDS_Shape sewed = wireSewer.SewedShape();

        // 9) Extract wires
        std::vector<TopoDS_Wire> wires;
        for (TopExp_Explorer exW(sewed, TopAbs_WIRE); exW.More(); exW.Next())
            wires.push_back(TopoDS::Wire(exW.Current()));

        // Last-resort: global sewing of original split edges if no wires found
        if (wires.empty()) {
            BRepBuilderAPI_Sewing globalSewer(mapTol, true, true, true);
            for (const TopoDS_Edge& e : splitEdges) globalSewer.Add(e);
            globalSewer.Perform();
            TopoDS_Shape sewedAll = globalSewer.SewedShape();
            for (TopExp_Explorer exW(sewedAll, TopAbs_WIRE); exW.More(); exW.Next())
                wires.push_back(TopoDS::Wire(exW.Current()));
        }

        if (wires.empty()) lua_error_with_where("Failed to build wires for 2D fuse after trimming.");

        // 10) Build faces from wires: compute area and sort (largest outer)
        struct WA { TopoDS_Wire w; double area; };
        std::vector<WA> wa;
        for (const TopoDS_Wire& w : wires) {
            double area = 0.0;
            BRepBuilderAPI_MakeFace mfTry(w);
            if (mfTry.IsDone()) {
                GProp_GProps props;
                BRepGProp::SurfaceProperties(mfTry.Face(), props);
                area = props.Mass();
            }
            wa.push_back({ w, area });
        }
        std::sort(wa.begin(), wa.end(), [](const WA& a, const WA& b){ return a.area > b.area; });

        // 11) Create final face (outer + holes)
        BRepBuilderAPI_MakeFace finalFace;
        bool faceOk = false;
        if (!wa.empty()) {
            finalFace = BRepBuilderAPI_MakeFace(wa[0].w);
            if (finalFace.IsDone()) {
                for (size_t i = 1; i < wa.size(); ++i) finalFace.Add(wa[i].w);
                if (finalFace.IsDone()) faceOk = true;
            }
        }

        if (!faceOk) {
            // fallback: sew wires into faces and pick the largest face
            TopoDS_Compound compW;
            current_part->builder.MakeCompound(compW);
            for (const WA& w : wa) current_part->builder.Add(compW, w.w);
            BRepBuilderAPI_Sewing faceSewer(mapTol);
            faceSewer.Add(compW);
            faceSewer.Perform();
            TopoDS_Shape sewedFaces = faceSewer.SewedShape();
            for (TopExp_Explorer exF(sewedFaces, TopAbs_FACE); exF.More(); exF.Next()) {
                fused = exF.Current();
                break;
            }
            if (fused.IsNull()) lua_error_with_where("Failed to construct fused face from wires after trimming.");
        } else {
            fused = finalFace.Face();
        }
    }

    // -------------------------
    // Finalize
    // -------------------------
    current_part->cshape = TopoDS_Compound();
    current_part->builder = BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    inteligentmerge(fused);
}
//region fuse
#include <vector>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
TopoDS_Shape PutAllFacesInSameDomain(const TopoDS_Shape& compound)
{
    // 1. Find the reference face (first face in the compound)
    TopoDS_Face refFace;
    for (TopExp_Explorer ex(compound, TopAbs_FACE); ex.More(); ex.Next()) {
        refFace = TopoDS::Face(ex.Current());
        break;
    }

    if (refFace.IsNull()) {
        std::cerr << "Compound has no faces" << std::endl;
        return compound;
    }

    // 2. Extract the reference surface
    Handle(Geom_Surface) refSurf = BRep_Tool::Surface(refFace);
    if (refSurf.IsNull()) {
        std::cerr << "Reference face has no surface" << std::endl;
        return compound;
    }

    // 3. Prepare result compound
    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);

    // 4. Rebuild every face in the compound on the reference surface
    for (TopExp_Explorer ex(compound, TopAbs_FACE); ex.More(); ex.Next()) {
        TopoDS_Face face = TopoDS::Face(ex.Current());

        // Extract UV bounds of the original face
        Standard_Real umin, umax, vmin, vmax;
        BRepTools::UVBounds(face, umin, umax, vmin, vmax);

        // Create new face on the reference surface
        TopoDS_Face newFace = BRepBuilderAPI_MakeFace(
            refSurf, umin, umax, vmin, vmax, Precision::Confusion());

        // Transfer wires
        for (TopExp_Explorer wex(face, TopAbs_WIRE); wex.More(); wex.Next()) {
            builder.Add(newFace, wex.Current());
        }

        // Preserve orientation
        newFace.Orientation(face.Orientation());

        // Add to result
        builder.Add(result, newFace);
    }

    return result;
}

TopoDS_Shape PutShapeBInShapeADomain(
    const TopoDS_Shape& shapeA,
    const TopoDS_Shape& shapeB)
{
    // 1. Extract the reference surface from ShapeA (first face)
    TopoDS_Face faceA;
    for (TopExp_Explorer ex(shapeA, TopAbs_FACE); ex.More(); ex.Next()) {
        faceA = TopoDS::Face(ex.Current());
        break;
    }
    if (faceA.IsNull()) {
        std::cerr << "ShapeA has no faces" << std::endl;
        return shapeB;
    }

    Handle(Geom_Surface) surfA = BRep_Tool::Surface(faceA);
    if (surfA.IsNull()) {
        std::cerr << "ShapeA has no underlying surface" << std::endl;
        return shapeB;
    }

    // 2. Prepare builder for the new shape
    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);

    // 3. Rebuild each face of ShapeB using ShapeA's surface
    for (TopExp_Explorer ex(shapeB, TopAbs_FACE); ex.More(); ex.Next()) {
        TopoDS_Face faceB = TopoDS::Face(ex.Current());

        // Extract UV bounds of faceB
        Standard_Real umin, umax, vmin, vmax;
        BRepTools::UVBounds(faceB, umin, umax, vmin, vmax);

        // Create new face using ShapeA's surface
        TopoDS_Face newFace = BRepBuilderAPI_MakeFace(
            surfA, umin, umax, vmin, vmax, Precision::Confusion());

        // Transfer wires from faceB
        for (TopExp_Explorer wex(faceB, TopAbs_WIRE); wex.More(); wex.Next()) {
            TopoDS_Wire wire = TopoDS::Wire(wex.Current());
            builder.Add(newFace, wire);
        }

        // Preserve orientation
        newFace.Orientation(faceB.Orientation());

        // Add to result
        builder.Add(result, newFace);
    }

    return result;
}



void Fusetesting() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");
    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
	
    const TopoDS_Compound& c = current_part->cshape;
	std::vector<TopoDS_Face> ex=ExtractFaces(c);
	TopoDS_Shape ts=UniteFaceVector(ex);
	cotm(ex.size())
	{
		ex=ExtractFaces(ts);
		cotm(ex.size());
		// ts=UniteFaceVector(ex);
		// cotm(ex.size())
	}






// // 1. Fuse the faces
// BRepAlgoAPI_Fuse aFuse(ex[0], ex[1]);
// aFuse.Build();
// TopoDS_Shape fusedShape = aFuse.Shape();

// // 2. Unify the same domain to remove the shared edge
// ShapeUpgrade_UnifySameDomain unifier(fusedShape, 0, Standard_True);
// unifier.SetSafeInputMode(0);
// unifier.Build();
// TopoDS_Shape finalFace = unifier.Shape();



// // std::vector<TopoDS_Face> ex2=ExtractFaces(fusedShape);
// std::vector<TopoDS_Face> ex2=ExtractFaces(finalFace);
// cotm(ex2.size())

// ShapeUpgrade_UnifySameDomain unifier(fusedShape, Standard_True, Standard_True);
// unifier.SetSafeInputMode(0);
// unifier.Build();
// TopoDS_Shape finalFace = unifier.Shape();




// TopoDS_Face ff=TopoDS::Face(finalFace);

    // Finalize: Clear compound and set the fused result
    current_part->cshape=TopoDS_Compound();
    // current_part->cshape.Nullify();
	current_part->builder=BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(ex[0]);
}
// Replacement Fuse function
void Fusenothing() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");
    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
	
    const TopoDS_Compound& c = current_part->cshape;
	current_part->start_location=getShapePlacement(c);

    // Collect solids and faces
    TopTools_ListOfShape solids, faces;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) solids.Append(ex.Current());
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) faces.Append(ex.Current());

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // 3D fuse (solids)
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) tools.Append(*it);

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();
        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();

        // Unify same domain
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // 2D fuse (faces)
    else if (has2D) {
        if (faces.Extent() == 1) {
            fused = faces.First();
        } else {
            // Tolerance used for sewing and vertex matching
            const double tol = 1e-5;

            // 1) Sew faces
            BRepBuilderAPI_Sewing sewer(tol, true, true, true);
            for (TopTools_ListIteratorOfListOfShape it(faces); it.More(); it.Next()) sewer.Add(it.Value());
            sewer.Perform();
            TopoDS_Shape sewed = sewer.SewedShape();

            // 2) Try boolean fuse on sewed faces (BRepAlgoAPI_Fuse or BOPAlgo_Builder fallback)
            bool fuseDone = false;
            {
                TopTools_ListOfShape sewedFaces;
                for (TopExp_Explorer ex(sewed, TopAbs_FACE); ex.More(); ex.Next()) sewedFaces.Append(ex.Current());

                if (sewedFaces.Extent() <= 1) {
                    fused = (sewedFaces.Extent() == 1) ? sewedFaces.First() : sewed;
                    fuseDone = true;
                } else {
                    TopTools_ListOfShape args;
                    TopTools_ListOfShape tools;
                    auto it2 = sewedFaces.begin();
                    args.Append(*it2);
                    for (++it2; it2 != sewedFaces.end(); ++it2) tools.Append(*it2);

                    BRepAlgoAPI_Fuse fuseOp;
                    fuseOp.SetArguments(args);
                    fuseOp.SetTools(tools);
                    fuseOp.SetFuzzyValue(tol);
                    fuseOp.Build();
                    if (fuseOp.IsDone()) {
                        fused = fuseOp.Shape();
                        fuseDone = true;
                    } else {
                        BOPAlgo_Builder builder;
                        builder.SetArguments(sewedFaces);
                        builder.SetFuzzyValue(tol);
                        builder.Perform();
                        if (!builder.HasErrors()) {
                            fused = builder.Shape();
                            fuseDone = true;
                        }
                    }
                }
            }

            if (!fuseDone) lua_error_with_where("2D fuse operation failed.");
// Extra attempt to merge coplanar faces if multiple remain after unify
{
    TopTools_ListOfShape tmpFaces;
    for (TopExp_Explorer ex(fused, TopAbs_FACE); ex.More(); ex.Next())
        tmpFaces.Append(ex.Current());

    if (tmpFaces.Extent() > 1) {
        BRepAlgoAPI_Fuse fuseAgain;
        TopTools_ListOfShape args, tools;
        auto it3 = tmpFaces.begin();
        args.Append(*it3);
        for (++it3; it3 != tmpFaces.end(); ++it3) tools.Append(*it3);
        fuseAgain.SetArguments(args);
        fuseAgain.SetTools(tools);
        fuseAgain.SetFuzzyValue(1e-5);
        fuseAgain.Build();
        if (fuseAgain.IsDone()) {
            fused = fuseAgain.Shape();
            // Clean up any tiny residuals
            ShapeUpgrade_UnifySameDomain unify2(fused, true, true, false);
            unify2.AllowInternalEdges(true);
            unify2.SetLinearTolerance(1e-5);
            unify2.SetAngularTolerance(1e-3);
            unify2.Build();
            if (!unify2.Shape().IsNull()) fused = unify2.Shape();
        }
    }
}

            // 3) Unify coplanar faces and allow internal edges
            if (!fused.IsNull()) {
                ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
                // unify.AllowInternalEdges(true);
                unify.SetLinearTolerance(tol);
                unify.SetAngularTolerance(1e-3);
                unify.Build();
                if (!unify.Shape().IsNull()) fused = unify.Shape();
            }

            // 4) If still multiple faces, rebuild faces from boundary wires
            if (!fused.IsNull()) {
                // Count faces and capture first face
                TopExp_Explorer faceExp(fused, TopAbs_FACE);
                TopoDS_Face firstFace;
                int faceCount = 0;
                while (faceExp.More()) {
                    if (faceCount == 0) firstFace = TopoDS::Face(faceExp.Current());
                    faceCount++;
                    faceExp.Next();
                }

                if (faceCount == 1) {
                    fused = firstFace;
                } else if (faceCount > 1) {
                    // Build edge->face adjacency
                    TopTools_IndexedDataMapOfShapeListOfShape edgeToFaces;
                    TopExp::MapShapesAndAncestors(fused, TopAbs_EDGE, TopAbs_FACE, edgeToFaces);

                    // Collect boundary edges (adjacent to exactly one face)
                    TopTools_IndexedMapOfShape boundaryEdgesMap;
                    for (int i = 1; i <= edgeToFaces.Extent(); ++i) {
                        const TopoDS_Edge& e = TopoDS::Edge(edgeToFaces.FindKey(i));
                        const TopTools_ListOfShape& adjFaces = edgeToFaces.FindFromIndex(i);
                        if (adjFaces.Extent() == 1) boundaryEdgesMap.Add(e);
                    }

                    // If no boundary edges found, fallback to all edges
                    if (boundaryEdgesMap.Extent() == 0) {
                        TopExp_Explorer exE(fused, TopAbs_EDGE);
                        while (exE.More()) {
                            boundaryEdgesMap.Add(TopoDS::Edge(exE.Current()));
                            exE.Next();
                        }
                    }

                    // Helper: get vertex point
                    auto vertexPoint = [&](const TopoDS_Vertex& v)->gp_Pnt {
                        return BRep_Tool::Pnt(v);
                    };

                    // Helper: get the two vertices of an edge
                    auto edgeVertices = [&](const TopoDS_Edge& e, TopoDS_Vertex& v1, TopoDS_Vertex& v2) {
                        TopExp::Vertices(e, v1, v2);
                    };

                    // Convert boundary edges to a list for ordering
                    std::vector<TopoDS_Edge> edges;
                    edges.reserve(boundaryEdgesMap.Extent());
                    for (int i = 1; i <= boundaryEdgesMap.Extent(); ++i) edges.push_back(TopoDS::Edge(boundaryEdgesMap(i)));

                    // Function to order edges into closed loops using vertex matching
                    auto build_wires_from_edges = [&](const std::vector<TopoDS_Edge>& inputEdges, double matchTol)
                        -> std::vector<TopoDS_Wire>
                    {
                        std::vector<bool> used(inputEdges.size(), false);
                        std::vector<TopoDS_Wire> wires;

                        for (size_t startIdx = 0; startIdx < inputEdges.size(); ++startIdx) {
                            if (used[startIdx]) continue;

                            // Start a new wire
                            std::vector<TopoDS_Edge> loopEdges;
                            loopEdges.push_back(inputEdges[startIdx]);
                            used[startIdx] = true;

                            // Get current end point
                            TopoDS_Vertex va, vb;
                            edgeVertices(loopEdges.back(), va, vb);
                            gp_Pnt curEnd = vertexPoint(vb);

                            bool progressed = true;
                            while (progressed) {
                                progressed = false;
                                for (size_t j = 0; j < inputEdges.size(); ++j) {
                                    if (used[j]) continue;
                                    TopoDS_Vertex ea, eb;
                                    edgeVertices(inputEdges[j], ea, eb);
                                    gp_Pnt pa = vertexPoint(ea);
                                    gp_Pnt pb = vertexPoint(eb);

                                    if (curEnd.Distance(pa) <= matchTol) {
                                        loopEdges.push_back(inputEdges[j]);
                                        used[j] = true;
                                        curEnd = pb;
                                        progressed = true;
                                        break;
                                    } else if (curEnd.Distance(pb) <= matchTol) {
                                        // need to reverse edge orientation when adding
                                        TopoDS_Edge revE = TopoDS::Edge(inputEdges[j]);
                                        revE.Reverse();
                                        loopEdges.push_back(revE);
                                        used[j] = true;
                                        // new end is the other vertex (pa)
                                        curEnd = pa;
                                        progressed = true;
                                        break;
                                    }
                                }
                            }

                            // Try to close the loop by checking first vertex
                            TopoDS_Vertex firstA, firstB;
                            edgeVertices(loopEdges.front(), firstA, firstB);
                            gp_Pnt firstStart = vertexPoint(firstA);
                            if (curEnd.Distance(firstStart) <= matchTol) {
                                // Build wire from ordered edges
                                BRepBuilderAPI_MakeWire wireMaker;
                                for (const TopoDS_Edge& e : loopEdges) {
                                    wireMaker.Add(e);
                                }
                                if (wireMaker.IsDone()) {
                                    wires.push_back(wireMaker.Wire());
                                } else {
                                    // Try sewing the edges into a wire as fallback
                                    BRepBuilderAPI_MakeWire fallbackWire;
                                    for (const TopoDS_Edge& e : loopEdges) fallbackWire.Add(e);
                                    if (fallbackWire.IsDone()) wires.push_back(fallbackWire.Wire());
                                }
                            } else {
                                // Could not close; attempt to create a wire anyway (may fail later)
                                BRepBuilderAPI_MakeWire wireMaker;
                                for (const TopoDS_Edge& e : loopEdges) wireMaker.Add(e);
                                if (wireMaker.IsDone()) wires.push_back(wireMaker.Wire());
                            }
                        }

                        return wires;
                    };

                    // Build wires
                    std::vector<TopoDS_Wire> wires = build_wires_from_edges(edges, tol * 10.0); // slightly larger matching tolerance

                    // Rebuild faces from wires using reference surfaces from faces in fused
                    TopTools_ListOfShape rebuiltFaces;
                    TopExp_Explorer fe(fused, TopAbs_FACE);
                    Handle(Geom_Surface) refSurf;
                    if (fe.More()) {
                        TopoDS_Face rf = TopoDS::Face(fe.Current());
                        refSurf = BRep_Tool::Surface(rf);
                    }

                    for (const TopoDS_Wire& w : wires) {
                        if (w.IsNull()) continue;
                        // If we have a reference surface, use it; otherwise MakeFace will infer surface
                        BRepBuilderAPI_MakeFace faceMaker(refSurf, w, Standard_True);
                        if (!faceMaker.IsDone()) {
                            // fallback: let MakeFace infer surface
                            BRepBuilderAPI_MakeFace faceMaker2(w);
                            if (faceMaker2.IsDone()) rebuiltFaces.Append(faceMaker2.Face());
                        } else {
                            rebuiltFaces.Append(faceMaker.Face());
                        }
                    }

                    // If we rebuilt at least one face, combine them into a single shape (or single face if only one)
                    if (!rebuiltFaces.IsEmpty()) {
                        if (rebuiltFaces.Extent() == 1) {
                            fused = rebuiltFaces.First();
                        } else {
                            // Make a compound of rebuilt faces
                            TopoDS_Compound comp;
                            current_part->builder.MakeCompound(comp);
                            for (TopTools_ListIteratorOfListOfShape it(rebuiltFaces); it.More(); it.Next()) {
                                current_part->builder.Add(comp, it.Value());
                            }
                            fused = comp;
                        }
                    }
                } // end faceCount > 1
            } // end if fused not null
        } // end else faces.Extent() > 1
    } // end 2D branch

    // Finalize: Clear compound and set the fused result
    current_part->cshape=TopoDS_Compound();
    // current_part->cshape.Nullify();
	current_part->builder=BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
void Fuseactual() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");
    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
	
    const TopoDS_Compound& c = current_part->cshape;
	current_part->start_location=getShapePlacement(c);

    // Collect solids and faces
    TopTools_ListOfShape solids, faces;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) solids.Append(ex.Current());
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) faces.Append(ex.Current());

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // 3D fuse (solids)
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) tools.Append(*it);

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();
        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();

        // Unify same domain
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // 2D fuse (faces)
    else if (has2D) {
        if (faces.Extent() == 1) {
            fused = faces.First();
        } else {
            // Tolerance used for sewing and vertex matching
            const double tol = 1e-5;

            // 1) Sew faces
            BRepBuilderAPI_Sewing sewer(tol, true, true, true);
            for (TopTools_ListIteratorOfListOfShape it(faces); it.More(); it.Next()) sewer.Add(it.Value());
            sewer.Perform();
            TopoDS_Shape sewed = sewer.SewedShape();

            // 2) Try boolean fuse on sewed faces (BRepAlgoAPI_Fuse or BOPAlgo_Builder fallback)
            bool fuseDone = false;
            {
                TopTools_ListOfShape sewedFaces;
                for (TopExp_Explorer ex(sewed, TopAbs_FACE); ex.More(); ex.Next()) sewedFaces.Append(ex.Current());

                if (sewedFaces.Extent() <= 1) {
                    fused = (sewedFaces.Extent() == 1) ? sewedFaces.First() : sewed;
                    fuseDone = true;
                } else {
                    TopTools_ListOfShape args;
                    TopTools_ListOfShape tools;
                    auto it2 = sewedFaces.begin();
                    args.Append(*it2);
                    for (++it2; it2 != sewedFaces.end(); ++it2) tools.Append(*it2);

                    BRepAlgoAPI_Fuse fuseOp;
                    fuseOp.SetArguments(args);
                    fuseOp.SetTools(tools);
                    fuseOp.SetFuzzyValue(tol);
                    fuseOp.Build();
                    if (fuseOp.IsDone()) {
                        fused = fuseOp.Shape();
                        fuseDone = true;
                    } else {
                        BOPAlgo_Builder builder;
                        builder.SetArguments(sewedFaces);
                        builder.SetFuzzyValue(tol);
                        builder.Perform();
                        if (!builder.HasErrors()) {
                            fused = builder.Shape();
                            fuseDone = true;
                        }
                    }
                }
            }

            if (!fuseDone) lua_error_with_where("2D fuse operation failed.");

            // 3) Unify coplanar faces and allow internal edges
            if (!fused.IsNull()) {
                ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
                unify.AllowInternalEdges(true);
                unify.SetLinearTolerance(tol);
                unify.SetAngularTolerance(1e-3);
                unify.Build();
                if (!unify.Shape().IsNull()) fused = unify.Shape();
            }

            // 4) If still multiple faces, rebuild faces from boundary wires
            if (!fused.IsNull()) {
                // Count faces and capture first face
                TopExp_Explorer faceExp(fused, TopAbs_FACE);
                TopoDS_Face firstFace;
                int faceCount = 0;
                while (faceExp.More()) {
                    if (faceCount == 0) firstFace = TopoDS::Face(faceExp.Current());
                    faceCount++;
                    faceExp.Next();
                }

                if (faceCount == 1) {
                    fused = firstFace;
                } else if (faceCount > 1) {
                    // Build edge->face adjacency
                    TopTools_IndexedDataMapOfShapeListOfShape edgeToFaces;
                    TopExp::MapShapesAndAncestors(fused, TopAbs_EDGE, TopAbs_FACE, edgeToFaces);

                    // Collect boundary edges (adjacent to exactly one face)
                    TopTools_IndexedMapOfShape boundaryEdgesMap;
                    for (int i = 1; i <= edgeToFaces.Extent(); ++i) {
                        const TopoDS_Edge& e = TopoDS::Edge(edgeToFaces.FindKey(i));
                        const TopTools_ListOfShape& adjFaces = edgeToFaces.FindFromIndex(i);
                        if (adjFaces.Extent() == 1) boundaryEdgesMap.Add(e);
                    }

                    // If no boundary edges found, fallback to all edges
                    if (boundaryEdgesMap.Extent() == 0) {
                        TopExp_Explorer exE(fused, TopAbs_EDGE);
                        while (exE.More()) {
                            boundaryEdgesMap.Add(TopoDS::Edge(exE.Current()));
                            exE.Next();
                        }
                    }

                    // Helper: get vertex point
                    auto vertexPoint = [&](const TopoDS_Vertex& v)->gp_Pnt {
                        return BRep_Tool::Pnt(v);
                    };

                    // Helper: get the two vertices of an edge
                    auto edgeVertices = [&](const TopoDS_Edge& e, TopoDS_Vertex& v1, TopoDS_Vertex& v2) {
                        TopExp::Vertices(e, v1, v2);
                    };

                    // Convert boundary edges to a list for ordering
                    std::vector<TopoDS_Edge> edges;
                    edges.reserve(boundaryEdgesMap.Extent());
                    for (int i = 1; i <= boundaryEdgesMap.Extent(); ++i) edges.push_back(TopoDS::Edge(boundaryEdgesMap(i)));

                    // Function to order edges into closed loops using vertex matching
                    auto build_wires_from_edges = [&](const std::vector<TopoDS_Edge>& inputEdges, double matchTol)
                        -> std::vector<TopoDS_Wire>
                    {
                        std::vector<bool> used(inputEdges.size(), false);
                        std::vector<TopoDS_Wire> wires;

                        for (size_t startIdx = 0; startIdx < inputEdges.size(); ++startIdx) {
                            if (used[startIdx]) continue;

                            // Start a new wire
                            std::vector<TopoDS_Edge> loopEdges;
                            loopEdges.push_back(inputEdges[startIdx]);
                            used[startIdx] = true;

                            // Get current end point
                            TopoDS_Vertex va, vb;
                            edgeVertices(loopEdges.back(), va, vb);
                            gp_Pnt curEnd = vertexPoint(vb);

                            bool progressed = true;
                            while (progressed) {
                                progressed = false;
                                for (size_t j = 0; j < inputEdges.size(); ++j) {
                                    if (used[j]) continue;
                                    TopoDS_Vertex ea, eb;
                                    edgeVertices(inputEdges[j], ea, eb);
                                    gp_Pnt pa = vertexPoint(ea);
                                    gp_Pnt pb = vertexPoint(eb);

                                    if (curEnd.Distance(pa) <= matchTol) {
                                        loopEdges.push_back(inputEdges[j]);
                                        used[j] = true;
                                        curEnd = pb;
                                        progressed = true;
                                        break;
                                    } else if (curEnd.Distance(pb) <= matchTol) {
                                        // need to reverse edge orientation when adding
                                        TopoDS_Edge revE = TopoDS::Edge(inputEdges[j]);
                                        revE.Reverse();
                                        loopEdges.push_back(revE);
                                        used[j] = true;
                                        // new end is the other vertex (pa)
                                        curEnd = pa;
                                        progressed = true;
                                        break;
                                    }
                                }
                            }

                            // Try to close the loop by checking first vertex
                            TopoDS_Vertex firstA, firstB;
                            edgeVertices(loopEdges.front(), firstA, firstB);
                            gp_Pnt firstStart = vertexPoint(firstA);
                            if (curEnd.Distance(firstStart) <= matchTol) {
                                // Build wire from ordered edges
                                BRepBuilderAPI_MakeWire wireMaker;
                                for (const TopoDS_Edge& e : loopEdges) {
                                    wireMaker.Add(e);
                                }
                                if (wireMaker.IsDone()) {
                                    wires.push_back(wireMaker.Wire());
                                } else {
                                    // Try sewing the edges into a wire as fallback
                                    BRepBuilderAPI_MakeWire fallbackWire;
                                    for (const TopoDS_Edge& e : loopEdges) fallbackWire.Add(e);
                                    if (fallbackWire.IsDone()) wires.push_back(fallbackWire.Wire());
                                }
                            } else {
                                // Could not close; attempt to create a wire anyway (may fail later)
                                BRepBuilderAPI_MakeWire wireMaker;
                                for (const TopoDS_Edge& e : loopEdges) wireMaker.Add(e);
                                if (wireMaker.IsDone()) wires.push_back(wireMaker.Wire());
                            }
                        }

                        return wires;
                    };

                    // Build wires
                    std::vector<TopoDS_Wire> wires = build_wires_from_edges(edges, tol * 10.0); // slightly larger matching tolerance

                    // Rebuild faces from wires using reference surfaces from faces in fused
                    TopTools_ListOfShape rebuiltFaces;
                    TopExp_Explorer fe(fused, TopAbs_FACE);
                    Handle(Geom_Surface) refSurf;
                    if (fe.More()) {
                        TopoDS_Face rf = TopoDS::Face(fe.Current());
                        refSurf = BRep_Tool::Surface(rf);
                    }

                    for (const TopoDS_Wire& w : wires) {
                        if (w.IsNull()) continue;
                        // If we have a reference surface, use it; otherwise MakeFace will infer surface
                        BRepBuilderAPI_MakeFace faceMaker(refSurf, w, Standard_True);
                        if (!faceMaker.IsDone()) {
                            // fallback: let MakeFace infer surface
                            BRepBuilderAPI_MakeFace faceMaker2(w);
                            if (faceMaker2.IsDone()) rebuiltFaces.Append(faceMaker2.Face());
                        } else {
                            rebuiltFaces.Append(faceMaker.Face());
                        }
                    }

                    // If we rebuilt at least one face, combine them into a single shape (or single face if only one)
                    if (!rebuiltFaces.IsEmpty()) {
                        if (rebuiltFaces.Extent() == 1) {
                            fused = rebuiltFaces.First();
                        } else {
                            // Make a compound of rebuilt faces
                            TopoDS_Compound comp;
                            current_part->builder.MakeCompound(comp);
                            for (TopTools_ListIteratorOfListOfShape it(rebuiltFaces); it.More(); it.Next()) {
                                current_part->builder.Add(comp, it.Value());
                            }
                            fused = comp;
                        }
                    }
                } // end faceCount > 1
            } // end if fused not null
        } // end else faces.Extent() > 1
    } // end 2D branch

    // Finalize: Clear compound and set the fused result
    current_part->cshape=TopoDS_Compound();
    // current_part->cshape.Nullify();
	current_part->builder=BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
void Fuse() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");
    if (current_part->shape.IsNull())
        lua_error_with_where("No current shape. Call after doing shapes.");
    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
	
    const TopoDS_Compound& c = current_part->cshape;
	// current_part->start_location=getShapePlacement(c);

    // Collect solids and faces
    TopTools_ListOfShape solids, faces;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) solids.Append(ex.Current());
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) faces.Append(ex.Current());

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // 3D fuse (solids)
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) tools.Append(*it);

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();
        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();

        // Unify same domain
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // 2D fuse (faces)
    else if (has2D) {
        std::vector<TopoDS_Face> ex=ExtractFaces(c);
		fused=UniteFaceVector(ex);
		fused=ExtractFaces(fused)[0];
    } // end 2D branch

    // Finalize: Clear compound and set the fused result
    current_part->cshape=TopoDS_Compound();
    // current_part->cshape.Nullify();
	current_part->builder=BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>

#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS.hxx>

#include <BOPAlgo_Builder.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>
#include <Standard_Failure.hxx>
#include <sstream>

#include <BRep_Builder.hxx>
#include <BRepAlgoAPI_BuilderAlgo.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_ListOfShape.hxx>

void Fuseno() {
    if (!current_part) {
        lua_error_with_where("No current part. Call Part(name) first.");
        return;
    }

    // 1. Gather faces from current compound
    AddToCompound(current_part->cshape, current_part->shape);
    TopoDS_Shape inputShape = current_part->cshape;

    TopTools_ListOfShape faces;
    for (TopExp_Explorer exp(inputShape, TopAbs_FACE); exp.More(); exp.Next()) {
        faces.Append(TopoDS::Face(exp.Current()));
    }

    if (faces.Extent() == 0) {
        lua_error_with_where("No faces found to fuse.");
        return;
    }

    // 2. Fuse all faces together (handles overlaps and trims)
    BRepAlgoAPI_BuilderAlgo fuseAlgo;
	fuseAlgo.SetGlue(BOPAlgo_GlueShift);
    fuseAlgo.SetArguments(faces);
    fuseAlgo.SetRunParallel(Standard_True);
    fuseAlgo.Build();
    if (fuseAlgo.HasErrors()) {
        lua_error_with_where("Fuse operation failed.");
        return;
    }

    TopoDS_Shape fusedShape = fuseAlgo.Shape();

    // 3. Simplify (unify tangential/overlapping faces into a single surface)
    ShapeUpgrade_UnifySameDomain unifier(fusedShape, /* unifyEdges */ Standard_True, /* unifyFaces */ Standard_True);
    unifier.Build();
    // if (!unifier.IsDone()) {
    //     lua_error_with_where("Unification failed.");
    //     return;
    // }
    fusedShape = unifier.Shape();

    // 4. Clean up and store result
    current_part->cshape = TopoDS_Compound();
    current_part->builder = BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    // intelligentmerge handles storing or displaying the result
    inteligentmerge(fusedShape);
}

   
//         // Finalize: Clear compound and set the fused result
//     current_part->cshape=TopoDS_Compound();
//     // current_part->cshape.Nullify();
// 	current_part->builder=BRep_Builder();
//     current_part->builder.MakeCompound(current_part->cshape);
//     current_part->shape.Nullify();

//     // intelligentmerge handles adding the 'fused' result back to the system
//     inteligentmerge(fusedShape);
// }

void Fusetest() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");
    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
	
    const TopoDS_Compound& c = current_part->cshape;
	current_part->start_location=getShapePlacement(c);

    // Collect solids and faces
    TopTools_ListOfShape solids, faces;
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) solids.Append(ex.Current());
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) faces.Append(ex.Current());

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;
solids=faces;
    // 3D fuse (solids)
    if (1) 
    // if (has3D) 
	{
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) tools.Append(*it);

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();
        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();

        // Unify same domain
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // 2D fuse (faces)
    else if (has2D) {
        if (faces.Extent() == 1) {
            fused = faces.First();
        } else {
            // Tolerance used for sewing and vertex matching
            const double tol = 1e-5;

            // 1) Sew faces
            BRepBuilderAPI_Sewing sewer(tol, true, true, true);
            for (TopTools_ListIteratorOfListOfShape it(faces); it.More(); it.Next()) sewer.Add(it.Value());
            sewer.Perform();
            TopoDS_Shape sewed = sewer.SewedShape();

            // 2) Try boolean fuse on sewed faces (BRepAlgoAPI_Fuse or BOPAlgo_Builder fallback)
            bool fuseDone = false;
            {
                TopTools_ListOfShape sewedFaces;
                for (TopExp_Explorer ex(sewed, TopAbs_FACE); ex.More(); ex.Next()) sewedFaces.Append(ex.Current());

                if (sewedFaces.Extent() <= 1) {
                    fused = (sewedFaces.Extent() == 1) ? sewedFaces.First() : sewed;
                    fuseDone = true;
                } else {
                    TopTools_ListOfShape args;
                    TopTools_ListOfShape tools;
                    auto it2 = sewedFaces.begin();
                    args.Append(*it2);
                    for (++it2; it2 != sewedFaces.end(); ++it2) tools.Append(*it2);

                    BRepAlgoAPI_Fuse fuseOp;
                    fuseOp.SetArguments(args);
                    fuseOp.SetTools(tools);
                    fuseOp.SetFuzzyValue(tol);
                    fuseOp.Build();
                    if (fuseOp.IsDone()) {
                        fused = fuseOp.Shape();
                        fuseDone = true;
                    } else {
                        BOPAlgo_Builder builder;
                        builder.SetArguments(sewedFaces);
                        builder.SetFuzzyValue(tol);
                        builder.Perform();
                        if (!builder.HasErrors()) {
                            fused = builder.Shape();
                            fuseDone = true;
                        }
                    }
                }
            }

            if (!fuseDone) lua_error_with_where("2D fuse operation failed.");

            // 3) Unify coplanar faces and allow internal edges
            if (!fused.IsNull()) {
                ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
                unify.AllowInternalEdges(true);
                unify.SetLinearTolerance(tol);
                unify.SetAngularTolerance(1e-3);
                unify.Build();
                if (!unify.Shape().IsNull()) fused = unify.Shape();
            }

            // 4) If still multiple faces, rebuild faces from boundary wires
            if (!fused.IsNull()) {
                // Count faces and capture first face
                TopExp_Explorer faceExp(fused, TopAbs_FACE);
                TopoDS_Face firstFace;
                int faceCount = 0;
                while (faceExp.More()) {
                    if (faceCount == 0) firstFace = TopoDS::Face(faceExp.Current());
                    faceCount++;
                    faceExp.Next();
                }

                if (faceCount == 1) {
                    fused = firstFace;
                } else if (faceCount > 1) {
                    // Build edge->face adjacency
                    TopTools_IndexedDataMapOfShapeListOfShape edgeToFaces;
                    TopExp::MapShapesAndAncestors(fused, TopAbs_EDGE, TopAbs_FACE, edgeToFaces);

                    // Collect boundary edges (adjacent to exactly one face)
                    TopTools_IndexedMapOfShape boundaryEdgesMap;
                    for (int i = 1; i <= edgeToFaces.Extent(); ++i) {
                        const TopoDS_Edge& e = TopoDS::Edge(edgeToFaces.FindKey(i));
                        const TopTools_ListOfShape& adjFaces = edgeToFaces.FindFromIndex(i);
                        if (adjFaces.Extent() == 1) boundaryEdgesMap.Add(e);
                    }

                    // If no boundary edges found, fallback to all edges
                    if (boundaryEdgesMap.Extent() == 0) {
                        TopExp_Explorer exE(fused, TopAbs_EDGE);
                        while (exE.More()) {
                            boundaryEdgesMap.Add(TopoDS::Edge(exE.Current()));
                            exE.Next();
                        }
                    }

                    // Helper: get vertex point
                    auto vertexPoint = [&](const TopoDS_Vertex& v)->gp_Pnt {
                        return BRep_Tool::Pnt(v);
                    };

                    // Helper: get the two vertices of an edge
                    auto edgeVertices = [&](const TopoDS_Edge& e, TopoDS_Vertex& v1, TopoDS_Vertex& v2) {
                        TopExp::Vertices(e, v1, v2);
                    };

                    // Convert boundary edges to a list for ordering
                    std::vector<TopoDS_Edge> edges;
                    edges.reserve(boundaryEdgesMap.Extent());
                    for (int i = 1; i <= boundaryEdgesMap.Extent(); ++i) edges.push_back(TopoDS::Edge(boundaryEdgesMap(i)));

                    // Function to order edges into closed loops using vertex matching
                    auto build_wires_from_edges = [&](const std::vector<TopoDS_Edge>& inputEdges, double matchTol)
                        -> std::vector<TopoDS_Wire>
                    {
                        std::vector<bool> used(inputEdges.size(), false);
                        std::vector<TopoDS_Wire> wires;

                        for (size_t startIdx = 0; startIdx < inputEdges.size(); ++startIdx) {
                            if (used[startIdx]) continue;

                            // Start a new wire
                            std::vector<TopoDS_Edge> loopEdges;
                            loopEdges.push_back(inputEdges[startIdx]);
                            used[startIdx] = true;

                            // Get current end point
                            TopoDS_Vertex va, vb;
                            edgeVertices(loopEdges.back(), va, vb);
                            gp_Pnt curEnd = vertexPoint(vb);

                            bool progressed = true;
                            while (progressed) {
                                progressed = false;
                                for (size_t j = 0; j < inputEdges.size(); ++j) {
                                    if (used[j]) continue;
                                    TopoDS_Vertex ea, eb;
                                    edgeVertices(inputEdges[j], ea, eb);
                                    gp_Pnt pa = vertexPoint(ea);
                                    gp_Pnt pb = vertexPoint(eb);

                                    if (curEnd.Distance(pa) <= matchTol) {
                                        loopEdges.push_back(inputEdges[j]);
                                        used[j] = true;
                                        curEnd = pb;
                                        progressed = true;
                                        break;
                                    } else if (curEnd.Distance(pb) <= matchTol) {
                                        // need to reverse edge orientation when adding
                                        TopoDS_Edge revE = TopoDS::Edge(inputEdges[j]);
                                        revE.Reverse();
                                        loopEdges.push_back(revE);
                                        used[j] = true;
                                        // new end is the other vertex (pa)
                                        curEnd = pa;
                                        progressed = true;
                                        break;
                                    }
                                }
                            }

                            // Try to close the loop by checking first vertex
                            TopoDS_Vertex firstA, firstB;
                            edgeVertices(loopEdges.front(), firstA, firstB);
                            gp_Pnt firstStart = vertexPoint(firstA);
                            if (curEnd.Distance(firstStart) <= matchTol) {
                                // Build wire from ordered edges
                                BRepBuilderAPI_MakeWire wireMaker;
                                for (const TopoDS_Edge& e : loopEdges) {
                                    wireMaker.Add(e);
                                }
                                if (wireMaker.IsDone()) {
                                    wires.push_back(wireMaker.Wire());
                                } else {
                                    // Try sewing the edges into a wire as fallback
                                    BRepBuilderAPI_MakeWire fallbackWire;
                                    for (const TopoDS_Edge& e : loopEdges) fallbackWire.Add(e);
                                    if (fallbackWire.IsDone()) wires.push_back(fallbackWire.Wire());
                                }
                            } else {
                                // Could not close; attempt to create a wire anyway (may fail later)
                                BRepBuilderAPI_MakeWire wireMaker;
                                for (const TopoDS_Edge& e : loopEdges) wireMaker.Add(e);
                                if (wireMaker.IsDone()) wires.push_back(wireMaker.Wire());
                            }
                        }

                        return wires;
                    };

                    // Build wires
                    std::vector<TopoDS_Wire> wires = build_wires_from_edges(edges, tol * 10.0); // slightly larger matching tolerance

                    // Rebuild faces from wires using reference surfaces from faces in fused
                    TopTools_ListOfShape rebuiltFaces;
                    TopExp_Explorer fe(fused, TopAbs_FACE);
                    Handle(Geom_Surface) refSurf;
                    if (fe.More()) {
                        TopoDS_Face rf = TopoDS::Face(fe.Current());
                        refSurf = BRep_Tool::Surface(rf);
                    }

                    for (const TopoDS_Wire& w : wires) {
                        if (w.IsNull()) continue;
                        // If we have a reference surface, use it; otherwise MakeFace will infer surface
                        BRepBuilderAPI_MakeFace faceMaker(refSurf, w, Standard_True);
                        if (!faceMaker.IsDone()) {
                            // fallback: let MakeFace infer surface
                            BRepBuilderAPI_MakeFace faceMaker2(w);
                            if (faceMaker2.IsDone()) rebuiltFaces.Append(faceMaker2.Face());
                        } else {
                            rebuiltFaces.Append(faceMaker.Face());
                        }
                    }

                    // If we rebuilt at least one face, combine them into a single shape (or single face if only one)
                    if (!rebuiltFaces.IsEmpty()) {
                        if (rebuiltFaces.Extent() == 1) {
                            fused = rebuiltFaces.First();
                        } else {
                            // Make a compound of rebuilt faces
                            TopoDS_Compound comp;
                            current_part->builder.MakeCompound(comp);
                            for (TopTools_ListIteratorOfListOfShape it(rebuiltFaces); it.More(); it.Next()) {
                                current_part->builder.Add(comp, it.Value());
                            }
                            fused = comp;
                        }
                    }
                } // end faceCount > 1
            } // end if fused not null
        } // end else faces.Extent() > 1
    } // end 2D branch

    // Finalize: Clear compound and set the fused result
    current_part->cshape=TopoDS_Compound();
    // current_part->cshape.Nullify();
	current_part->builder=BRep_Builder();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();

    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}

void Fusealmost4() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect shapes into lists for OpenCASCADE API ---
    TopTools_ListOfShape solids, faces;
    
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.Append(ex.Current());
    }

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE (ALL SOLIDS)
    // ============================================================
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) {
            tools.Append(*it);
        }

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();

        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();
        
        // Unify same domain for 3D result
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // ============================================================
    //                       2D FUSE (ALL FACES)
    // ============================================================
    else if (has2D) {
        if (faces.Extent() <= 1) {
            fused = (faces.Extent() == 1) ? faces.First() : TopoDS_Shape();
        } 
        else {
            // Directly use BRepAlgoAPI_Fuse on the faces (DO NOT sew overlapping faces)
            TopTools_ListOfShape args;
            TopTools_ListOfShape tools;
            
            auto it = faces.begin();
            args.Append(*it);
            for (++it; it != faces.end(); ++it) {
                tools.Append(*it);
            }
            
            BRepAlgoAPI_Fuse fuseOp;
            fuseOp.SetArguments(args);
            fuseOp.SetTools(tools);
            fuseOp.SetFuzzyValue(1e-5);
            fuseOp.Build();
            
            if (fuseOp.IsDone()) {
                fused = fuseOp.Shape();
            } else {
                // Fallback to BOPAlgo_Builder (General Fuse) if binary fuse fails
                BOPAlgo_Builder builder;
                builder.SetArguments(faces);
                builder.SetFuzzyValue(1e-5);
                builder.Perform();
                if (!builder.HasErrors()) {
                    fused = builder.Shape();
                } else {
                    lua_error_with_where("2D fuse operation failed.");
                }
            }
        }
        
        // Critical: Unify coplanar faces to merge them into a single face 
        // and remove internal intersection edges created by the overlap.
        if (!fused.IsNull()) {
            ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
            unify.SetLinearTolerance(1e-4);
            unify.SetAngularTolerance(1e-2);
            unify.Build();
            
            if (!unify.Shape().IsNull()) {
                fused = unify.Shape();
            }
            
            // Final cleanup: If the result is a compound with exactly one face, extract it natively
            TopExp_Explorer faceExp(fused, TopAbs_FACE);
            TopoDS_Face firstFace;
            int faceCount = 0;
            
            while (faceExp.More()) {
                if (faceCount == 0) firstFace = TopoDS::Face(faceExp.Current());
                faceCount++;
                faceExp.Next();
            }
            
            if (faceCount == 1) {
                fused = firstFace; // Exactly one face, unwrap it from the compound
            }
        }
		
    }

    // Finalize: Clear compound and set the fused result
    current_part->cshape.Nullify();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
    
    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
void Fusealmost3() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect shapes into lists for OpenCASCADE API ---
    TopTools_ListOfShape solids, faces;
    
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.Append(ex.Current());
    }

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE (ALL SOLIDS)
    // ============================================================
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) {
            tools.Append(*it);
        }

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();

        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();
        
        // Unify same domain for 3D result
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // ============================================================
    //                       2D FUSE (ALL FACES)
    // ============================================================
    else if (has2D) {
        if (faces.Extent() <= 1) {
            fused = (faces.Extent() == 1) ? faces.First() : TopoDS_Shape();
        } 
        else {
            // Directly use BRepAlgoAPI_Fuse on the faces (DO NOT sew overlapping faces)
            TopTools_ListOfShape args;
            TopTools_ListOfShape tools;
            
            auto it = faces.begin();
            args.Append(*it);
            for (++it; it != faces.end(); ++it) {
                tools.Append(*it);
            }
            
            BRepAlgoAPI_Fuse fuseOp;
            fuseOp.SetArguments(args);
            fuseOp.SetTools(tools);
            fuseOp.SetFuzzyValue(1e-5);
            fuseOp.Build();
            
            if (fuseOp.IsDone()) {
                fused = fuseOp.Shape();
            } else {
                // Fallback to BOPAlgo_Builder (General Fuse) if binary fuse fails
                BOPAlgo_Builder builder;
                builder.SetArguments(faces);
                builder.SetFuzzyValue(1e-5);
                builder.Perform();
                if (!builder.HasErrors()) {
                    fused = builder.Shape();
                } else {
                    lua_error_with_where("2D fuse operation failed.");
                }
            }
        }
        
        // Critical: Unify coplanar faces to merge them into a single face 
        // and remove internal intersection edges created by the overlap.
        if (!fused.IsNull()) {
            ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
            unify.SetLinearTolerance(1e-4);
            unify.SetAngularTolerance(1e-2);
            unify.Build();
            
            if (!unify.Shape().IsNull()) {
                fused = unify.Shape();
            }
            
            // Final cleanup: If the result is a compound with exactly one face, extract it natively
            TopExp_Explorer faceExp(fused, TopAbs_FACE);
            TopoDS_Face firstFace;
            int faceCount = 0;
            
            while (faceExp.More()) {
                if (faceCount == 0) firstFace = TopoDS::Face(faceExp.Current());
                faceCount++;
                faceExp.Next();
            }
            
            if (faceCount == 1) {
                fused = firstFace; // Exactly one face, unwrap it from the compound
            }
        }
		
    }

    // Finalize: Clear compound and set the fused result
    current_part->cshape.Nullify();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
    
    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
void Fusealmost() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect shapes into lists for OpenCASCADE API ---
    TopTools_ListOfShape solids, faces;
    
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.Append(ex.Current());
    }

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE (ALL SOLIDS)
    // ============================================================
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) {
            tools.Append(*it);
        }

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();

        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();
        
        // Unify same domain for 3D result
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // ============================================================
    //                       2D FUSE (ALL FACES)
    // ============================================================
    else if (has2D) {
        if (faces.Extent() <= 1) {
            fused = (faces.Extent() == 1) ? faces.First() : TopoDS_Shape();
        } 
        else {
            // Directly use BRepAlgoAPI_Fuse on the faces (DO NOT sew overlapping faces)
            TopTools_ListOfShape args;
            TopTools_ListOfShape tools;
            
            auto it = faces.begin();
            args.Append(*it);
            for (++it; it != faces.end(); ++it) {
                tools.Append(*it);
            }
            
            BRepAlgoAPI_Fuse fuseOp;
            fuseOp.SetArguments(args);
            fuseOp.SetTools(tools);
            fuseOp.SetFuzzyValue(1e-5);
            fuseOp.Build();
            
            if (fuseOp.IsDone()) {
                fused = fuseOp.Shape();
            } else {
                // Fallback to BOPAlgo_Builder (General Fuse) if binary fuse fails
                BOPAlgo_Builder builder;
                builder.SetArguments(faces);
                builder.SetFuzzyValue(1e-5);
                builder.Perform();
                if (!builder.HasErrors()) {
                    fused = builder.Shape();
                } else {
                    lua_error_with_where("2D fuse operation failed.");
                }
            }
        }
        
        // Critical: Unify coplanar faces to merge them into a single face 
        // and remove internal intersection edges created by the overlap.
        if (!fused.IsNull()) {
            ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
            unify.SetLinearTolerance(1e-4);
            unify.SetAngularTolerance(1e-2);
            unify.Build();
            
            if (!unify.Shape().IsNull()) {
                fused = unify.Shape();
            }
            
            // Final cleanup: If the result is a compound with exactly one face, extract it natively
            TopExp_Explorer faceExp(fused, TopAbs_FACE);
            TopoDS_Face firstFace;
            int faceCount = 0;
            
            while (faceExp.More()) {
                if (faceCount == 0) firstFace = TopoDS::Face(faceExp.Current());
                faceCount++;
                faceExp.Next();
            }
            
            if (faceCount == 1) {
                fused = firstFace; // Exactly one face, unwrap it from the compound
            }
        }
    }

    // Finalize: Clear compound and set the fused result
    current_part->cshape.Nullify();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
    
    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}
void Fusealmost2() {
    if (!current_part)
        lua_error_with_where("No current part. Call Part(name) first.");

    // Add the most recent shape to the compound before processing
    AddToCompound(current_part->cshape, current_part->shape);
    const TopoDS_Compound& c = current_part->cshape;

    // --- Collect shapes into lists for OpenCASCADE API ---
    TopTools_ListOfShape solids, faces;
    
    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) {
        solids.Append(ex.Current());
    }
    for (TopExp_Explorer ex(c, TopAbs_FACE); ex.More(); ex.Next()) {
        faces.Append(ex.Current());
    }

    const bool has3D = solids.Extent() >= 2;
    const bool has2D = (!has3D && faces.Extent() >= 2);

    if (!has3D && !has2D) {
        lua_error_with_where("Need at least two shapes of the same type (Solids or Faces) to fuse.");
    }

    TopoDS_Shape fused;

    // ============================================================
    //                       3D FUSE (ALL SOLIDS)
    // ============================================================
    if (has3D) {
        TopTools_ListOfShape arguments, tools;
        auto it = solids.begin();
        arguments.Append(*it);
        for (++it; it != solids.end(); ++it) {
            tools.Append(*it);
        }

        BRepAlgoAPI_Fuse fuseOp;
        fuseOp.SetArguments(arguments);
        fuseOp.SetTools(tools);
        fuseOp.SetFuzzyValue(1e-5);
        fuseOp.Build();

        if (!fuseOp.IsDone()) lua_error_with_where("3D solid fuse operation failed.");
        fused = fuseOp.Shape();
        
        // Unify same domain for 3D result
        ShapeUpgrade_UnifySameDomain unify(fused, true, true, false);
        unify.SetLinearTolerance(1e-5);

			unify.AllowInternalEdges(0);
        unify.Build();
        if (!unify.Shape().IsNull()) fused = unify.Shape();
    }

    // ============================================================
    //                       2D FUSE (ALL FACES)
    // ============================================================
    else if (has2D) {
        if (faces.Extent() <= 1) {
            fused = (faces.Extent() == 1) ? faces.First() : TopoDS_Shape();
        } 
        else {
            // Step 1: Directly use BRepAlgoAPI_Fuse to handle overlaps properly
            TopTools_ListOfShape args;
            TopTools_ListOfShape tools;
            
            auto it = faces.begin();
            args.Append(*it);
            for (++it; it != faces.end(); ++it) {
                tools.Append(*it);
            }
            
            BRepAlgoAPI_Fuse fuseOp;
            fuseOp.SetArguments(args);
            fuseOp.SetTools(tools);
            fuseOp.SetFuzzyValue(1e-5);
            fuseOp.Build();
            
            if (fuseOp.IsDone()) {
                fused = fuseOp.Shape();
            } else {
                // Fallback to BOPAlgo_Builder if binary fuse fails
                BOPAlgo_Builder builder;
                builder.SetArguments(faces);
                builder.SetFuzzyValue(1e-5);
                builder.Perform();
                if (!builder.HasErrors()) {
                    fused = builder.Shape();
                } else {
                    lua_error_with_where("2D fuse operation failed.");
                }
            }
        }
        
        // Step 2: Unify and Glue into a single Face
        if (!fused.IsNull()) {
            // Try standard unification first to clean up the topology
            ShapeUpgrade_UnifySameDomain unify(fused, true, true, 0);
			unify.AllowInternalEdges(0);
            unify.SetLinearTolerance(1e-4);
            unify.SetAngularTolerance(1e-2);
            unify.Build();
            
            TopoDS_Shape refined = unify.Shape();
            if (!refined.IsNull()) {
                fused = refined;
            }
            
            
        }
    }

    // Finalize: Clear compound and set the fused result
    current_part->cshape.Nullify();
    current_part->builder.MakeCompound(current_part->cshape);
    current_part->shape.Nullify();
    
    // intelligentmerge handles adding the 'fused' result back to the system
    inteligentmerge(fused);
}


void luainit() {
    if (G){
		G.reset();
	}  
    G = std::make_unique<sol::state>();
    auto& lua = *G; 
	// lua.open_libraries(
	// 	sol::lib::base,
	// 	sol::lib::package,
	// 	sol::lib::math,
	// 	sol::lib::table,
	// 	sol::lib::string
	// );
	lua.open_libraries(sol::lib::base,
                   sol::lib::package,
                   sol::lib::coroutine,
                   sol::lib::string,
                   sol::lib::table,
                   sol::lib::math,
                   sol::lib::io,
                   sol::lib::os,
                   sol::lib::debug);

    lua["package"]["path"] = std::string("./lua/?.lua;") + std::string(lua["package"]["path"]);
    L = lua.lua_state();

//region binds
    lua.set_function("Part", &Part);
    lua.set_function("Originl", &Originl);
    lua.set_function("Pl", &Pl);
    lua.set_function("Circle", &Circle);
    lua.set_function("Rec", &Rec);
    lua.set_function("Extrude", &Extrude);
    lua.set_function("Offset", &Offset);
    lua.set_function("Clone",sol::protect( &Clone));
    lua.set_function("Copy_placement", &Copy_placement);
    // lua.set_function("Mirror", &Mirror);
	lua.set_function("Mirrorx",sol::protect( [&](luadraw* original,float offset) {Mirror(original, offset, 1,0,0);}));
	lua.set_function("Mirrory",sol::protect( [&](luadraw* original,float offset) {Mirror(original, offset, 0,1,0);}));
	lua.set_function("Mirrorz",sol::protect( [&](luadraw* original,float offset) {Mirror(original, offset, 0,0,1);}));
	lua.set_function("Mirrorlx", [&](float offset) {Mirror(0, offset, 1,0,0);});
	lua.set_function("Mirrorly", [&](float offset) {Mirror(0, offset, 0,1,0);});
	lua.set_function("Mirrorlz", [&](float offset) {Mirror(0, offset, 0,0,1);});
    // lua.set_function("Invert", &Invert);
	lua.set_function("Invertx",sol::protect( [&](luadraw* original, float offset) {Invert(original, offset, 1,0,0);}));
	lua.set_function("Inverty",sol::protect( [&](luadraw* original, float offset) {Invert(original, offset, 0,1,0);}));
	lua.set_function("Invertz",sol::protect( [&](luadraw* original, float offset){Invert(original, offset, 0,0,1);}));
	lua.set_function("Invertlx", [&](float offset) {Invert(0, offset, 1,0,0);});
	lua.set_function("Invertly", [&](float offset) {Invert(0, offset, 0,1,0);});
	lua.set_function("Invertlz", [&](float offset) {Invert(0, offset, 0,0,1);});
    lua.set_function("Subtract", [&]() {CommonAndSubtract(0);});
    lua.set_function("Common", [&]() {CommonAndSubtract(1);});
    lua.set_function("Movel", [&](float x, float y, float z) {Movel(x,y,z,0);});
    lua.set_function("Movew", [&](float x, float y, float z) {Movel(x,y,z,1);});
    lua.set_function("Fuse", &Fuse);
    // lua.set_function("Movel", &Movel);  
	lua.set_function("Rotatelx", [&](float angleDegrees = 0.0f) {Rotatel(angleDegrees, 1,0,0);});
	lua.set_function("Rotately", [&](float angleDegrees = 0.0f) {Rotatel(angleDegrees, 0,1,0);});
	lua.set_function("Rotatelz", [&](float angleDegrees = 0.0f) {Rotatel(angleDegrees, 0,0,1);});

// 	lua.script(R"(
//     debug.sethook(function(event, line)
// 		print(line)
//         if line >= 227 then
//             print("BREAKPOINT at line 227")
// 			error()
//         end
//     end, "l")
// )");


// Set a hook that triggers every 1 LUA instruction
// lua_sethook(L,
// [](lua_State* L, lua_Debug* ar)
// {
//     lua_getinfo(L, "l", ar);

//     if (ar->currentline ==231)
//         luaL_error(L, "STOP");
// },
// LUA_MASKLINE, 0);
// editor->breakpoint=231;
// lua_sethook(L,
// [](lua_State* L, lua_Debug* ar)
// {
//     lua_getinfo(L, "lS", ar);

// 	// cotm(ar->currentline,ar->source);
// 	// cotm(editor->breakpoint)
//     if (ar->currentline == (editor->breakpoint+1) &&
//         ar->source &&
//         strstr(ar->source, editor->filename.c_str()))
//     {
// 		//make run current line here

// 		Fl::awake(fillbrowser);
// 		while(ar->currentline == (editor->breakpoint+1) && isdebuging){
// 			sleepms(100);
// 		} 
// 		if(!isdebuging)
//         luaL_error(L, "Breakpoint hit");
//     }

// }, LUA_MASKLINE, 0);

lua_sethook(L,
[](lua_State* L, lua_Debug* ar)
{
    lua_getinfo(L, "lS", ar);

	// cotm(ar->currentline,ar->source);
	// cotm(editor->breakpoint)
	static bool flagruncurrentline=1;
    if (ar->currentline+1 == (editor->breakpoint+2) &&
        ar->source &&
        strstr(ar->source, editor->filename.c_str()))
    { 
		Fl::awake(fillbrowser);
		while(ar->currentline == (editor->breakpoint+1) && isdebuging){
			sleepms(100);
		} 
		if(!isdebuging)
        luaL_error(L, "Breakpoint hit");
    }

}, LUA_MASKLINE, 0);

}
nmutex lua_mtx("lua_mtx", 1);
void lua_str(const string &str, bool isfile) { 
	thread([str,isfile](){
        lua_mtx.lock(); 
		perf();
        luainit();

		vlua.clear();
		// ctx->RemoveAll(1); 
		help.error="";
		help.upd();
		
		last_event = std::chrono::steady_clock::now(); 
		isloading=1;

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
			isdebuging=1;
            status = luaL_loadbuffer(L, src.data(), src.size(), str.c_str()); 
        } else { 
            status = luaL_loadbuffer(L, str.data(), str.size(), "chunk"); 
        }
		string lerror="";
        if (status == LUA_OK) {
            if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
				stringstream strm;
				strm<<lua_tostring(L, -1);
				help.error=strm.str();
				help.upd();
				lerror=lua_tostring(L, -1);
                std::cerr << "runtimer error: " << lerror << std::endl;
                lua_pop(L, 1);  
            }
			perf("lua");
			if (lerror.find("Breakpoint hit") == std::string::npos)
            	Fl::awake(fillbrowser);
 
        } else {
			Fl::awake(fillbrowser);
			stringstream strm;
			strm<<lua_tostring(L, -1);
			help.error=strm.str();
			help.upd();
            std::cerr << "Load error: " << lua_tostring(L, -1) << std::endl;
			
            lua_pop(L, 1);
        }
		isdebuging=0;
        lua_mtx.unlock(); 
	}).detach();
}
//region test
void working1(const Handle(AIS_InteractiveContext)& ctx, const Handle(V3d_View)& view) {

Handle(V3d_DirectionalLight) l1 = new V3d_DirectionalLight(V3d_XnegYnegZneg, Quantity_NOC_WHITE, Standard_True);
Handle(V3d_DirectionalLight) l2 = new V3d_DirectionalLight(V3d_XposYposZpos, Quantity_NOC_GRAY80, Standard_True);
view->Viewer()->AddLight(l1);
view->Viewer()->SetLightOn(l1);
view->Viewer()->AddLight(l2);
view->Viewer()->SetLightOn(l2);
l1->SetIntensity(0.2f);
l2->SetIntensity(0.2f);

l1->SetIntensity(9.5f);
l2->SetIntensity(9.5f);
 

// 1. Get the default drawer from the context
Handle(Prs3d_Drawer) defaultDrawer = ctx->DefaultDrawer();

// 2. Enable face boundaries globally
defaultDrawer->SetFaceBoundaryDraw(Standard_True);

Quantity_Color defaultColor(Quantity_NOC_STEELBLUE); 
defaultDrawer->ShadingAspect()->SetColor(defaultColor);

// 4. (Optional) Set material properties
// The color often depends on how the material reflects light
// defaultDrawer->ShadingAspect()->SetMaterial(Graphic3d_NameOfMaterial_Plastic);


// 3. Set Global Face Boundary Aspect (Color, Type, Width)
Handle(Prs3d_LineAspect) faceBoundaryAspect = new Prs3d_LineAspect(
    Quantity_Color(Quantity_NOC_BLACK), 
    Aspect_TOL_SOLID, 
    2.0
);
defaultDrawer->SetFaceBoundaryAspect(faceBoundaryAspect);

// 4. Set Global UnFree Boundary Aspect (Edges between faces) wireframe
Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect(
    Quantity_NOC_GRAY, 
    Aspect_TOL_DASH, 
    0.5
);
defaultDrawer->SetUnFreeBoundaryAspect(wireAsp);









 return;



     TopoDS_Shape box      = BRepPrimAPI_MakeBox(10, 10, 10).Shape(); 
 
    Handle(AIS_Shape) aisBox      = new AIS_Shape(box); 
    aisBox->UnsetColor();
// 1. Criar o material PBR
Graphic3d_PBRMaterial aPbrMat;
// aPbrMat.SetColor(Quantity_Color(Quantity_NOC_BLUE));  <----- aqui nao faz nada
aPbrMat.SetMetallic(0.5f);  // Quase metal para testar brilho
aPbrMat.SetRoughness(0.5f); // Bem polido
// aPbrMat.SetIOR(1.5f);
// aPbrMat.SetColor(Quantity_ColorRGBA(0.0, 1.0, 0.0, 1.0));


// 2. Configurar o MaterialAspect (CRUCIAL: definir a cor aqui também)
Graphic3d_MaterialAspect aMat(Graphic3d_NameOfMaterial_UserDefined);
aMat.SetPBRMaterial(aPbrMat);
aMat.SetColor(Quantity_Color(Quantity_NOC_BLUE)); // Sincroniza Albedo com Diffuse
// aMat.SetInteriorColor(Quantity_Color(Quantity_NOC_GREEN)); // Sincroniza Albedo com Diffuse
aMat.SetShininess(1.0f);
// aMat.

// 3. Aplicar diretamente ao AIS_Shape (Método mais limpo)
aisBox->SetMaterial(aMat); 
aisBox->SetDisplayMode(1);

// 4. Forçar o Shading Model no Aspecto do objeto
Handle(Prs3d_ShadingAspect) aShading = aisBox->Attributes()->ShadingAspect();
aShading->Aspect()->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
aShading->Aspect()->SetInteriorStyle(Aspect_IS_SOLID);

// 5. Exibir
ctx->Display(aisBox, Standard_True);
view->FitAll();
}
//region menu
void fill_menu() {
	menu->add(
		"File/Quit", 0,
		[](Fl_Widget*, void* ud) {
			exit(0);
		}
	);


	menu->add(
		"View/Fit all", FL_ALT + 'f',
		[](Fl_Widget* mnu, void* ud) {
			view->FitAll();
			occv->redraw();
		},occv, 0);


	menu->add(
		"View/Show fillets", FL_ALT + 'i',
		[](Fl_Widget* mnu, void* ud) {
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();	// This gets the actually clicked item
			
			// if (!item->value()) { 
			// 	occv->show_fillets=0;
			// } else {
			// 	occv->show_fillets=1;
			// }
			// lua_str(currfilename,1); 
			// occv->redraw(); 
		},
		0, FL_MENU_TOGGLE);

	menu->add(
		"View/Animation", FL_ALT + 'a',
		[](Fl_Widget* mnu, void* ud) {
			start_continuous_rotation();
		},occv, 0);

	menu->add(
		"View/Transparent", FL_ALT + 't',
		[](Fl_Widget* mnu, void* ud) {
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();	// This gets the actually clicked item
			//  fillvectopo();
			if (!item->value()) {
				toggle_shaded_transp();
			} else {
				toggle_shaded_transp();
			}
			occv->redraw(); 
		},
		0, FL_MENU_TOGGLE);

	// menu->add(
	// 	"View/Nice", FL_ALT + 'n',
	// 	[](Fl_Widget* mnu, void* ud) {
	// 		Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
	// 		const Fl_Menu_Item* item = menu->mvalue();	// This gets the actually clicked item
	// 		// NiceSteelStandard(occv->m_view, vaShape );
	// 		// NiceSteel(m_view, vaShape, item->value());
	// 		occv->redraw(); 
	// 	},
	// 	0, FL_MENU_TOGGLE);





	menu->add(
		"Tools/List of profiles with length and quantity", 0,
		[](Fl_Widget*, void* ud) {
			OCC_Viewer* v = (OCC_Viewer*)(ud);

			// once:
// auto solids = GetDisplayedSolids(ctx);


// 			TopoDS_Face sketchFace=TopoDS_Face();
			// TopExp_Explorer ex(vlua[0]->shape, TopAbs_FACE);
			// if (ex.More()) {
			// 	sketchFace = TopoDS::Face(ex.Current());
			// }

    // for (TopExp_Explorer ex(vlua[1]->shape, TopAbs_FACE); ex.More(); ex.Next())
    // {
    //     TopoDS_Face f = TopoDS::Face(ex.Current());
    //     if (BRep_Tool::Surface(f)->DynamicType() == STANDARD_TYPE(Geom_Plane))
    //         sketchFace = f;
    // }

// 	Handle(Geom_Surface) s = BRep_Tool::Surface(sketchFace);
// std::cout << (s.IsNull() ? "NULL" : s->DynamicType()->Name()) << std::endl;

// sketchFace=vlua[0]->shape;


// later, when you have the correct reference face (the planar cap from the extruded solid):
// auto matches = CountAndMeasureFromRefFace_PlaneFilter(sketchFace, solids, 0.05, 0.05);

// std::cout << "Matched solids: " << matches.size() << "\n";
// for (auto &m : matches) {
//     std::cout << "Extrusion length (approx): " << m.second << "\n";
// }

// cotm(vlua[1]->name,solids.size());

// CountAndMeasureFromRefFace_PlaneFilter(sketchFace, solids, 0.05, 0.05);


// 			std::vector<TopoDS_Shape> solids=GetDisplayedSolids(ctx);

// 			TopoDS_Face sketchFace=TopoDS_Face();
// 			// TopExp_Explorer ex(vlua[0]->shape, TopAbs_FACE);
// 			// if (ex.More()) {
// 			// 	sketchFace = TopoDS::Face(ex.Current());
// 			// }

//     for (TopExp_Explorer ex(vlua[0]->shape, TopAbs_FACE); ex.More(); ex.Next())
//     {
//         TopoDS_Face f = TopoDS::Face(ex.Current());
//         if (BRep_Tool::Surface(f)->DynamicType() == STANDARD_TYPE(Geom_Plane))
//             sketchFace = f;
//     }

// 	Handle(Geom_Surface) s = BRep_Tool::Surface(sketchFace);
// std::cout << (s.IsNull() ? "NULL" : s->DynamicType()->Name()) << std::endl;

// // sketchFace=vlua[0]->shape;

// 			// if(sketchFace.IsNull())return;
// 			// Handle(AIS_Shape) acl=new AIS_Shape(sketchFace);
// 			// ctx->RemoveAll(0);
// 			// ctx->Display(acl,1);

// 			int cnt=CountSolidsMatchingSketchFace(sketchFace,ctx);
// 			// int cnt=CountSolidsMatchingSketchFace(sketchFace,solids);

// 			cotm(vlua[0]->name,solids.size(),cnt);

			// return;

			stringstream strm;
			// string nm = "sketch_profile";

			strm << "<html><body>";

auto solids = GetDisplayedSolids(ctx);


			

			for (int ji = 0; ji < vlua.size(); ++ji) {
				auto jo = vlua[ji];
				if (!Contains(jo->name, "sketch")) continue;

				TopoDS_Face sketchFace=TopoDS_Face();

 for (TopExp_Explorer ex(jo->shape, TopAbs_FACE); ex.More(); ex.Next())
    {
        TopoDS_Face f = TopoDS::Face(ex.Current());
        if (BRep_Tool::Surface(f)->DynamicType() == STANDARD_TYPE(Geom_Plane))
            sketchFace = f;
    }
if(sketchFace.IsNull())continue;
CountAndMeasureFromRefFace_PlaneFilter(sketchFace, solids, 0.05, 0.05);


				string nm = jo->name;

				strm << "<h2>Profile Summary: <b>" << nm << "</b></h2>";

				strm << "<table border='1' cellpadding='4' cellspacing='0' "
						"style='border-collapse:collapse;'>";

				strm << "<tr style='background:#eee;'>"
						"<th style='padding:4px 8px; line-height:1.2; height:20px;'>Name</th>"
						"<th style='padding:4px 8px; line-height:1.2; height:20px;'>Length</th>"
						"<th style='padding:4px 8px; line-height:1.2; height:20px;'>Quantity</th>"
						"<th style='padding:4px 8px; line-height:1.2; height:20px;'>Total Length</th>"
						"</tr>";

				// struct EntryAgg {
				// 	std::string first_name;
				// 	int total_qtty = 0;
				// };

				// std::unordered_map<float, EntryAgg> agg;

				// for (int i = 0; i < vlua.size(); ++i) {
				// 	auto o = vlua[i];
				// 	// cotm(o->from_sketch,nm);
				// 	if (o->from_sketch != nm) continue;
				// 	if (!o->visible_hardcoded) continue;

				// 	float val = std::abs(o->Extrude_val);
				// 	int qtty = o->clone_qtd;

				// 	auto& slot = agg[val];
				// 	// cotm2(o->from_sketch, o->name)
				// 	if (slot.total_qtty == 0) {
				// 		slot.first_name = o->name;
				// 	}
				// 	slot.total_qtty += qtty;
				// }

				float total_len = 0.0f;

				// strm << "<pre>Name\t\tLength\tQuantity\tTotal Length\n";

				// for (auto &p : agg) {
				//     float val = p.first;
				//     const auto &e = p.second;
				//     float subtotal = val * e.total_qtty;

				//     strm << e.first_name << "\t\t"
				//          << val << "\t"
				//          << e.total_qtty << "\t"
				//          << subtotal << "\n";
				// }

				// strm << "TOTAL\t\t\t" << total_len << "\n";

				for (auto& p : agg) {
					float val = p.first;
					const auto& e = p.second;

					float subtotal = val * e.total_qtty;
					total_len += subtotal;

					strm << "<tr style='padding:4px 8px; line-height:1.2; height:20px;'>"
							"<td style='padding:4px 8px; line-height:1.2; height:20px;'><b>"
						 << e.first_name
						 << "</b></td>"
							"<td style='padding:4px 8px; line-height:1.2; height:20px;'>"
						 << val
						 << "</td>"
							"<td style='padding:4px 8px; line-height:1.2; height:20px;'>"
						 << e.total_qtty
						 << "</td>"
							"<td style='padding:4px 8px; line-height:1.2; height:20px;'>"
						 << subtotal
						 << "</td>"
							"</tr>";

					// cotm2(e.first_name, val, e.total_qtty);
				}

				strm << "<tr style='background:#ddd;'>"
						"<td colspan='3'><b>Total Length</b></td>"
						"<td><b>"
					 << total_len
					 << "</b></td>"
						"</tr>";

				strm << "</table>";
			}

			strm << "</body></html>";

			// cotm2(nm, total_len);

			Fl_Group::current(nullptr);	 // clear current group

			static Fl_Window* fhelp = new Fl_Window(win->x() + win->w() / 4,  // position relative to parent
													win->y() + win->h() / 4, win->w() / 2, win->h() / 2, "Help");
			// fhelp->set_menu_window();

			// make it a child of 'parent' so no new taskbar icon
			fhelp->set_modal();	 // owned by parent, not independent

			static Fl_Help_View* fh = new Fl_Help_View(0, 0, fhelp->w(), fhelp->h());
			fh->textfont(0);
			fh->value(strm.str().c_str());
			fhelp->end();
			fhelp->show();

			// cotm2(strm.str());
			std::string html_data = strm.str();
			copy_html(html_data);
			return;
			// Fl::copy(html_data.c_str(), (int)html_data.length(), 1, "text/html");
			string plain = "teste";
			/* 1️⃣ HTML clipboard */
			Fl::copy(html_data.c_str(), (int)html_data.size(), 1, "text/html");

			/* 2️⃣ Plain text clipboard (CRITICAL) */
			Fl::copy(plain.c_str(), (int)plain.size(), 1, "text/plain;charset=utf-8");
		},
		occv, 0);

	menu->add(
		"Tools/Export visible solids/STL Single solid", 0,
		[](Fl_Widget*, void* ud) {
			OCC_Viewer* v = (OCC_Viewer*)(ud);

			// Create a single compound
			BRep_Builder B;
			TopoDS_Compound comp;
			B.MakeCompound(comp);

			bool added = false;

			for (int i = 0; i < vlua.size(); i++) {
				if (!ctx->IsDisplayed(vlua[i]->ashape) || !vlua[i]->visible_hardcoded) continue;

				TopoDS_Shape s = vlua[i]->cshape;
				// TopoDS_Shape s = v->vaShape[i]->Shape();
				if (s.IsNull()) continue;

				if (!vlua[i]->Origin.IsIdentity()) {
					s = s.Located(TopLoc_Location(vlua[i]->Origin.Transformation()));
				}

				// B.Add(comp, s);
				ExtractSolids(s, comp, B);
				added = true;
			}

			if (!added) {
				fl_alert("No visible shapes to export.");
				return;
			}

			// Force triangulation of the entire compound
			// BRepMesh_IncrementalMesh(comp, 0.05, false);

			TopoDS_Shape oriented = comp.Reversed();

			WriteBinarySTL(oriented, "../approbot/stl/test2.stl");
		},
		occv, 0);

	menu->add(
		"Tools/Export visible solids/STL Multiple solids", 0,
		[](Fl_Widget*, void* ud) {
			OCC_Viewer* v = (OCC_Viewer*)(ud);

			bool exported_any = false;

			for (int i = 0; i < vlua.size(); i++) {
				if (!ctx->IsDisplayed(vlua[i]->ashape) || !vlua[i]->visible_hardcoded) continue;

				TopoDS_Shape s = vlua[i]->cshape;
				if (s.IsNull()) continue;

				// Apply origin transform if needed
				if (!vlua[i]->Origin.IsIdentity()) {
					s = s.Located(TopLoc_Location(vlua[i]->Origin.Transformation()));
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
				fname += vlua[i]->name;
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


	
	// menu->add(
	// 	"Options/Tune view ", 0,
	// 	[](Fl_Widget* mnu, void* ud) {
	// 		// floccv=occv;
	// 		Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
	// 		const Fl_Menu_Item* item = menu->mvalue();
	// 		// flshapes=occv->vaShape;
	// 		// m_view=occv->m_view;
	// 		// slidercfg(); 
	// 	},
	// 	0, 0);
	
	menu->add(
		"Options/Autorefine ", 0,
		[](Fl_Widget* mnu, void* ud) {
			// floccv=occv;
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();
			autorefined=item->value();
			lua_str(currfilename,1);
			// flshapes=occv->vaShape;
			// m_view=occv->m_view;
			// slidercfg(); 
		},
		0, FL_MENU_TOGGLE);

	menu->add(
		//region help
		"Help", 0,
		[](Fl_Widget*, void* ud) {
			Fl_Group::current(nullptr);	 // clear current group

			static Fl_Window* fhelp = new Fl_Window(win->x() + win->w() / 4,  // position relative to parent
													win->y() + win->h() / 4, win->w() / 2, win->h() / 2, "Help");
			// fhelp->set_menu_window();

			// make it a child of 'parent' so no new taskbar icon
			fhelp->set_modal();	 // owned by parent, not independent

			static Fl_Help_View* fh = new Fl_Help_View(0, 0, fhelp->w(), fhelp->h());
			fh->textfont(0);

			stringstream strm;
			strm << "<html><body>";
			strm << "<h2>Keys:</h2>";
			strm << "<p><b>Ctrl + Mouse Click </b> Go to the code of the Part under the mouse cursor";
			strm << "<p><p>";

			strm << "<h2>Commands:</h2>";
			strm << "<p><b>Origin(x,y,z) </b> Move all subsequent Parts to the specified (x,y,z) relative to world "
					"coordinates until another Origin is defined";
			strm << "<p><b>Part [label] </b> Create a new Part with a label of your choice";
			strm << "<p><b>Clone ([label],[copy placement=0]) </b> Clone the Part with the given label; [copy "
					"placement]: "
					"0 = no, 1 = yes";
			strm << "<p><b>Pl [coords] </b> Create polyline with coords Autocad style, dont accept variables yet, e.g. "
					"Pl "
					"0,0 10,0 @0,10 @-10,0 0,0  =  a square";
			strm << "<p><b>Offset ([thickness]) </b> Creates offset of last polyline, e.g. (closed polyline) Offset -3 "
					" = "
					"offset 3 inside, Offset 3  = offset 3 outside. e.g. (opened polyline) Offset -3  = offset 3 to "
					"right, "
					"Offset 3  = offset 3 to left";
			strm << "<p><b>Circle (radius,x,y) </b>  ";
			strm << "<p><b>Extrude ([height]) </b> Extrusion of last polyline or circle from Part ";
			strm << "<p><b>Dup () </b> Duplicates last shape from Part, so the new be moved or rotated ";
			strm << "<p><b>Fuse () </b> Fuse the 2 last solids or 2d in the same Part ";
			strm << "<p><b>Subtract () </b> Subtract the last solid or 2d from previous shape in the same Part ";
			strm << "<p><b>Common () </b> Intersect with the last solid or 2d from previous shape in the same Part ";
			strm << "<p><b>Movel (x,y,z) </b> Move last solid from Part ";
			strm << "<p><b>Rotatelx (angle) </b> Rotate x on point relative to 0,0,0 of last solid from Part ";
			strm << "<p><b>Rotately (angle) </b> Rotate y on point relative to 0,0,0 of last solid from Part ";
			strm << "<p><b>Rotatelz (angle) </b> Rotate z on point relative to 0,0,0 of last solid from Part ";

			strm << "</body></html>";

			if (fh->value() == 0) fh->value(strm.str().c_str());

			fhelp->end();
			fhelp->show();
		},
		occv, 0);
}

//region main
int main(int argc, char** argv) { 
	// Fl::set_font(FL_HELVETICA, "DejaVu Sans");
	Fl::use_high_res_GL(1);
	// setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
	std::cout << "FLTK version: " << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION << "." << FL_PATCH_VERSION
			  << std::endl;
	std::cout << "FLTK ABI version: " << FL_ABI_VERSION << std::endl;
	std::cout << "OCCT version: " << OCC_VERSION_COMPLETE << std::endl;
	// OSD_Parallel::SetUseOcctThreads(1);
	std::cout << "Parallel mode: " << OSD_Parallel::ToUseOcctThreads() << std::endl;

	Fl::visual(FL_DOUBLE | FL_INDEX);
	// Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL ); 
	Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE); 
	Fl::scheme("oxy");	
	
	Fl_Group::current(content);
  
	Fl::lock();
	int w = 1024;
	int h = 576;
	int firstblock = w * 0.62;
	int secondblock = w * 0.12;
	int lastblock = w - firstblock - secondblock;
 
	win = new Fl_Double_Window(0, 0, w, h, "msvcad"); 
	// win->color(FL_RED);
	win->callback([](Fl_Widget* widget, void*) {
		if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) return;
		menu->find_item("File/Quit")->do_callback(menu);
	});
	menu = new Fl_Menu_Bar(0, 0, firstblock, 22); 


	content = new Fl_Group(0, 22, w, h - 22); 
	int hc1 = 24;
	occv = new OCC_Viewer(0, 22, firstblock, h - 22 - hc1-1);

	Fl_Group::current(content);
	woccbtn = new FixedHeightWindow(0, h - hc1, firstblock, hc1, ""); 
	 
	drawbuttons(woccbtn->w(), hc1);
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
 
	win->position(0, 0);  
	// int x, y, _w, _h;
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22); 
	// win->maximize();
	win->show();  
	Fl::flush();  // make sure everything gets drawn
	win->flush();	
	
    initialize_opencascade(occv); 
	m_initialized = 1;
	setSceneDefault();

	// working1(ctx,view);  

	Fl::add_timeout(0.7,[](void* d) {lua_str(currfilename,1);win->label(currfilename.c_str());},0);				
	Fl::add_timeout(0.9,[](void* d) {view->FitAll();},0);				
	Fl::add_timeout(1.5,[](void* d) {sbt[6].occbtn->do_callback(); },0);				
	Fl::add_timeout(2.4,[](void* d) {view->FitAll(); },0);		 
	return Fl::run();
}