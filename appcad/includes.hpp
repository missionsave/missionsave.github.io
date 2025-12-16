#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

#include <GL/glew.h> 
#ifdef __linux__
extern "C" {
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>
}
#endif
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
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Type.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_Shape.hxx>
#include <iostream>
#include <unordered_set>

#include <unordered_set>

#include <chrono>
#include <thread>
#include <functional>
#include <cmath> 
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <vector>


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
#include <gp_Vec2d.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <Geom_Plane.hxx> 
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>


#include <AIS_Shape.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_OffsetCurve.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAbs_JoinType.hxx>
#include <Geom_Curve.hxx>
#include <OSD_Parallel.hxx>
#include <Precision.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec2d.hxx>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <GProp_GProps.hxx> 
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <GeomLProp_SLProps.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <gp_Ax3.hxx>
#include <gp_Trsf.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Surface.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <gp_Ax3.hxx>
#include <gp_Trsf.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Surface.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <BRepGProp_Face.hxx>
#include <GProp_GProps.hxx>
#include <BOPAlgo_BOP.hxx>
#include <TopoDS_Shape.hxx>
#include <Geom2dAdaptor_Curve.hxx>

#include <AIS_ListOfInteractive.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>

#include <V3d_View.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <Graphic3d_BufferType.hxx>
#include <Image_PixMap.hxx>
#include <Image_AlienPixMap.hxx>

#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_Camera.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <BRepIntCurveSurface_Inter.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS.hxx>
#include <Precision.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_BufferType.hxx>
#include <Image_PixMap.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <V3d_View.hxx>

#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <V3d_View.hxx>
#include <Graphic3d_BufferType.hxx>
#include <Image_PixMap.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Precision.hxx>
#include <Standard_TypeDef.hxx>
#include <FL/Fl.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Window.H>

#include <chrono>
#include <execution> // Para C++17 paralelismo

#ifdef _WIN32 
#include <lua.hpp>
	#else
#include <lua5.4/lua.hpp> 
#endif
#include <sol/sol.hpp> 

#include "general.hpp"

void scint_init(int x,int y,int w,int h);



using namespace std;