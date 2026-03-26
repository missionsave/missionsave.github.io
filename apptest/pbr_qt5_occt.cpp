// pbr_qt5_occt_fixed.cpp
// Compile: g++ -std=c++11 -fPIC -I/usr/include/qt5 -I/usr/include/qt5/QtCore -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtWidgets -I/opt/occt-7_9_3/include/opencascade -L/opt/occt-7_9_3/lib -lTKOpenGl -lTKernel -lTKMath -lTKService -lTKV3d -lTKMesh -lQt5Core -lQt5Gui -lQt5Widgets -lGL -lGLU pbr_qt5_occt_fixed.cpp -o pbr_qt5_occt_fixed -lX11 -ldl
// g++ -std=c++20 -fPIC \
  -I/opt/occt-7_9_3/include/opencascade \
  pbr_qt5_occt.cpp -o pbr_qt5_occt \
  -L/opt/occt-7_9_3/lib \
  -lTKOpenGl -lTKV3d -lTKService -lTKDESTEP -lTKDEIGES -lTKDE \
  -lTKXSBase -lTKXCAF -lTKCAF -lTKCDF -lTKOffset -lTKBO -lTKBool \
  -lTKFillet -lTKFeat -lTKShHealing -lTKPrim -lTKTopAlgo -lTKBRep \
  -lTKGeomAlgo -lTKGeomBase -lTKG3d -lTKG2d -lTKHLR -lTKMesh \
  -lTKMath -lTKStd -lTKernel \
  $(pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5OpenGL Qt5Core) \
  -lGL -lGLU -lX11 -ldl
#include <QDebug>
#include <QApplication>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <Graphic3d_PBRMaterial.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <AIS_Shape.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopoDS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Xw_Window.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Quantity_Color.hxx>



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

#include <BRepTools.hxx>
#include <ShapeFix_Shape.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Solid.hxx>
#include <stdexcept>

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

#include <Geom_Circle.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <Prs3d_IsoAspect.hxx> 

#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <TopExp_Explorer.hxx>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>

#include <Geom_Surface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_SphericalSurface.hxx>

#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Graphic3d_Texture2D.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Image_PixMap.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Aspect_GradientFillMethod.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Graphic3d_AspectFillArea3d.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_AmbientLight.hxx>
#include <V3d_DirectionalLight.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Graphic3d_MaterialAspect.hxx>

#include <Image_AlienPixMap.hxx>

#include <Prs3d_Drawer.hxx>

#include <Quantity_Color.hxx>
#include <Quantity_ColorRGBA.hxx>

#include <Standard_Handle.hxx>
#include <AIS_Point.hxx>
#include <Geom_CartesianPoint.hxx>

#include <Xw_Window.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <X11/Xlib.h>

#include <QApplication>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <cmath>
#include <algorithm>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Image_PixMap.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Xw_Window.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <QApplication>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QScreen>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Image_PixMap.hxx>
#include <Xw_Window.hxx>

#include <QApplication>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QOpenGLContext>

// OCCT Includes
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Image_PixMap.hxx>
#include <Message.hxx>
#include <Message_PrinterSystemLog.hxx>
#include <QApplication>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QDebug>

// OCCT Includes
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Quantity_ColorRGBA.hxx>

class OCCTViewerWidget : public QOpenGLWidget {
public:
    OCCTViewerWidget(QWidget* parent = nullptr) : QOpenGLWidget(parent) {}

protected:
    void initializeGL() override {
        // Force the Qt context to be active before OCCT starts
        makeCurrent();

        if (!m_viewer.IsNull()) return;

        try {
            // 1. Create Display Connection
            Handle(Aspect_DisplayConnection) disp = new Aspect_DisplayConnection();

            // 2. THE NUCLEAR FIX: Standard_False for 'theIsWithDevice'
            // This parameter is CRITICAL. It tells OCCT:
            // "Don't try to initialize the graphics device/Visual yourself. 
            // Just wrap the existing OpenGL state."
            m_driver = new OpenGl_GraphicDriver(disp, Standard_False);
            
            // Disable Legacy Fixed Function Pipeline (FFP)
            m_driver->ChangeOptions().ffpEnable = Standard_False; 

            m_viewer = new V3d_Viewer(m_driver);
            m_aisContext = new AIS_InteractiveContext(m_viewer);
            m_view = m_viewer->CreateView();
            
            // 3. Aspect_NeutralWindow: This bypasses the X11 sub-window creation
            // which is where the XGetVisualInfo error is actually triggered.
            Handle(Aspect_NeutralWindow) w = new Aspect_NeutralWindow();
            w->SetSize(width(), height());
            m_view->SetWindow(w);

            m_view->SetShadingModel(Graphic3d_TOSM_PBR);
            m_view->SetBackgroundColor(Quantity_NOC_BLACK);

            setupScene();

        } catch (const Standard_Failure& e) {
            qCritical() << "OCCT Initialization Failed:" << e.GetMessageString();
        }
    }

    void setupScene() {
        // Gold PBR Material
        Graphic3d_MaterialAspect gold(Graphic3d_NOM_GOLD);
        Graphic3d_PBRMaterial gPbr;
        gPbr.SetMetallic(1.0f);
        gPbr.SetRoughness(0.2f);
        gPbr.SetColor(Quantity_ColorRGBA(1.0f, 0.71f, 0.29f, 1.0f));
        gold.SetPBRMaterial(gPbr);

        Handle(AIS_Shape) box = new AIS_Shape(BRepPrimAPI_MakeBox(10, 10, 10).Shape());
        box->SetMaterial(gold);
        
        m_aisContext->Display(box, Standard_False);
        m_view->MustBeResized();
        m_view->FitAll(0.01, Standard_False);
    }

    void paintGL() override {
        if (!m_view.IsNull()) {
            m_view->Redraw(); 
        }
    }

    void resizeGL(int w, int h) override {
        if (!m_view.IsNull()) {
            Handle(Aspect_NeutralWindow) wnd = Handle(Aspect_NeutralWindow)::DownCast(m_view->Window());
            if (!wnd.IsNull()) wnd->SetSize(w, h);
            m_view->MustBeResized();
        }
    }

private:
    Handle(OpenGl_GraphicDriver) m_driver;
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View)   m_view;
    Handle(AIS_InteractiveContext) m_aisContext;
};

int main(int argc, char** argv) {
    // Force X11 backend (non-negotiable for OCCT 7.9 on Linux)
    qputenv("QT_QPA_PLATFORM", "xcb");

    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setVersion(4, 5);
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Explicitly set 32-bit RGBA buffers. 
    // This is what OCCT's PBR engine is usually looking for.
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    fmt.setAlphaBufferSize(8); // This is usually the missing piece!
    fmt.setRedBufferSize(8);
    fmt.setGreenBufferSize(8);
    fmt.setBlueBufferSize(8);
    
    fmt.setSamples(4);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);

    OCCTViewerWidget widget;
    widget.resize(1024, 768);
    widget.show();

    return app.exec();
}