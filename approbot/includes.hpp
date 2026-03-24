
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
// #include "GraphicsCostEstimator_patch.h"
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

#include <osgFX/Scribe>

#include <thread>
#include <mutex>

#ifdef _WIN32 
#include <lua.hpp>
	#else
#include <lua5.4/lua.hpp> 
#endif
#include <sol/sol.hpp> 

// #include "fl_scintilla.hpp"

// #include "frobot.hpp"