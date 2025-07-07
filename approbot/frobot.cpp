
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

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/Version>

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



    ViewerFLTK(int x, int y, int w, int h, const char* label = 0): Fl_Gl_Window(x, y, w, h, label), osgViewer::Viewer(), rightMousePressed(false) {  // Ensure osgViewer::Viewer constructor is called


        // this->setUpViewerAsEmbeddedInWindow(x, y, w, h);



        //  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

        //  traits->alpha = false;

        //  traits->depth = false;

        //  traits->stencil = false;

        //  traits->doubleBuffer = true;

            

        // mode(FL_OPENGL3 |FL_RGB );
        // mode(FL_OPENGL3 |FL_RGB | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
        // mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

        _gw = new osgViewer::GraphicsWindowEmbedded(x, y, w, h);

        

        getCamera()->setViewport(new osg::Viewport(0, 0, w, h));

        getCamera()->setProjectionMatrixAsPerspective(

            30.0f, static_cast<double>(w) / static_cast<double>(h), 1.0f, 10000.0f);

        getCamera()->setGraphicsContext(_gw);

        getCamera()->setDrawBuffer(GL_BACK);

        getCamera()->setReadBuffer(GL_BACK);

		getCamera()->setClearColor(osg::Vec4(1.0f, 0.5f, 0.5f, 0.5f)); // RGBA

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
    int handle(int event) override {

        if (event == FL_SHOW && is_minimized){
            last_event = std::chrono::steady_clock::now(); 
            is_minimized=0;
        }

        if(is_minimized)
            return Fl_Gl_Window::handle(event);

// 		if(event==FL_RELEASE)
// tmr->setCenter(computeSceneBoundingSphere().center()); 

        if (event == FL_KEYUP || event == FL_KEYDOWN) {
            if (Fl::event_key() == FL_Escape) {
                cotm("esco")
                return 0;
            }
        }

        if (event == FL_PUSH) {
            Fl::focus(this);
        }

	//zoom
	if (event == FL_MOUSEWHEEL) {
		int dy = Fl::event_dy();
		if (dy == 0) return 0;

		int mouseX = Fl::event_x();
		int mouseY = Fl::event_y();
		int windowWidth = w();
		int windowHeight = h();

		// OSG coordinates (invert Y)
		int osgMouseY = windowHeight - mouseY - 1;

		// Get current camera transform
		osg::Vec3d eye, center, up;
		tmr->getTransformation(eye, center, up);

		osg::Matrixd viewMatrix = getCamera()->getViewMatrix();
		osg::Matrixd projMatrix = getCamera()->getProjectionMatrix();
		osg::Matrixd inverseVP = osg::Matrixd::inverse(viewMatrix * projMatrix);

		// Mouse in NDC
		double ndcX = (2.0 * mouseX) / windowWidth - 1.0;
		double ndcY = (2.0 * osgMouseY) / windowHeight - 1.0;

		// Ray in world space
		osg::Vec3d nearPoint = osg::Vec3d(ndcX, ndcY, -1.0) * inverseVP;
		osg::Vec3d farPoint = osg::Vec3d(ndcX, ndcY, 1.0) * inverseVP;
		osg::Vec3d rayDir = farPoint - nearPoint;
		rayDir.normalize();

		// View direction and distance
		osg::Vec3d viewDir = center - eye;
		double distance = viewDir.length();
		viewDir.normalize();

		// Zoom factor
		double zoomFactor = (dy > 0) ? 1.1 : 0.9;
		double scale = (1.0 - zoomFactor);

		// Get camera right and up vectors from view matrix
		osg::Matrixd invView = osg::Matrixd::inverse(viewMatrix);
		osg::Vec3d camRight(invView(0, 0), invView(0, 1), invView(0, 2));
		osg::Vec3d camUp(invView(1, 0), invView(1, 1), invView(1, 2));

		// Pan in screen pixels (mouse position from center)
		double px = mouseX - windowWidth * 0.5;
		double py = windowHeight * 0.5 - mouseY;  // invert Y to match screen coords

		// Compute pan in world units
		double fovy, aspectRatio, zNear, zFar;
		projMatrix.getPerspective(fovy, aspectRatio, zNear, zFar);
		double tanHalfFovy = tan(osg::DegreesToRadians(fovy * 0.5));
		double pixelSize = 2.0 * distance * tanHalfFovy / windowHeight;

		osg::Vec3d panOffset = camRight * (px * pixelSize * scale) +
							camUp * (py * pixelSize * scale);

		osg::Vec3d zoomOffset = viewDir * distance * scale;

		osg::Vec3d newEye = eye + zoomOffset + panOffset;
		osg::Vec3d newCenter = center + zoomOffset + panOffset;

		tmr->setTransformation(newEye, newCenter, up);
		return 1;
	}


		//rotation
// 		if (event==FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE){

// const osg::BoundingSphere& bs = group->getBound();

// // 2) Configure o manipulador para “olhar para” esse centro:
// osg::Vec3d center = bs.center();
// double   dist   = bs.radius() * 3.0; // ou um fator que ache melhor

// // Define posição do olho (eye), centro (center) e up vector:
// osg::Vec3d eye( center.x(), center.y() - dist, center.z() );
// osg::Vec3d up ( 0.0, 0.0, 1.0 );

// // 3) Ajusta o home e o pivot do trackball:
// tmr->setHomePosition(eye, center, up);
// tmr->home(0.0);            // reposiciona imediatamente na “home”
// tmr->setCenter(center);    // define o ponto fixo de rotação
// tmr->setDistance(dist);     // distância inicial da câmera


static int _lastMouseX = 0, _lastMouseY = 0;

if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
	if (_gw.valid()) _gw->getEventQueue()->mouseButtonPress(Fl::event_x(), Fl::event_y(), Fl::event_button());
    _lastMouseX = Fl::event_x();
    _lastMouseY = Fl::event_y();
    return 1;
}

if (event == FL_DRAG && Fl::event_button() == FL_LEFT_MOUSE) {
	
    osg::Vec3d _rotationCenter = _pivot;

    int currentX = Fl::event_x();
    int currentY = Fl::event_y();

	float percentX = float(_lastMouseX) / float(w());
if (percentX > 0.95f) {
    float dy = currentY - _lastMouseY;
	_lastMouseY=currentY;

    // Invert and reduce sensitivity
    float angle = osg::DegreesToRadians(-dy * 0.1f);

    osg::Vec3d eye, center, up;
    tmr->getTransformation(eye, center, up);
    osg::Vec3d viewDir = center - eye;
    viewDir.normalize();

    osg::Quat roll(angle, viewDir);
    osg::Vec3d newUp = roll * up;

    tmr->setTransformation(eye, center, newUp);
    return 1;
}


    // Mouse delta
    float dx = currentX - _lastMouseX;
    float dy = currentY - _lastMouseY;
    _lastMouseX = currentX;
    _lastMouseY = currentY;

    // Get camera basis
    osg::Vec3d eye, center, up;
    tmr->getTransformation(eye, center, up);

    osg::Vec3d viewDir = center - eye;
    viewDir.normalize();
    osg::Vec3d right = viewDir ^ up;
    right.normalize();

    // Adjust angles and fix direction
    float rotX = osg::DegreesToRadians(-dx * 0.2f);
    float rotY = osg::DegreesToRadians(-dy * 0.2f);

    osg::Quat rotH(rotX, up);
    osg::Quat rotV(rotY, right);
    osg::Quat totalRot = rotH * rotV;

    // Rotate around pivot
    osg::Vec3d eyeOffset = eye - _rotationCenter;
    osg::Vec3d centerOffset = center - _rotationCenter;

    eyeOffset = totalRot * eyeOffset;
    centerOffset = totalRot * centerOffset;

    osg::Vec3d newEye = _rotationCenter + eyeOffset;
    osg::Vec3d newCenter = _rotationCenter + centerOffset;

    tmr->setTransformation(newEye, newCenter, totalRot*up);
    return 1;
}


// 		if (event==FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE){
// 			// if (_gw.valid()) _gw->getEventQueue()->mouseButtonPress(Fl::event_x(), Fl::event_y(), Fl::event_button());
// 			return 1;
// 		}


// // Modified mouse drag handler:
// if (event == FL_DRAG && Fl::event_button() == FL_LEFT_MOUSE) {
// 	static int _lastMouseX, _lastMouseY;
// 	osg::Vec3d _rotationCenter=_pivot;
//     int currentX = Fl::event_x();
//     int currentY = Fl::event_y();
    
//     // Calculate mouse movement delta
//     float dx = currentX - _lastMouseX;
//     float dy = currentY - _lastMouseY;
//     _lastMouseX = currentX;
//     _lastMouseY = currentY;
    
//     // Get current camera position
//     osg::Vec3d eye, center, up;
//     tmr->getTransformation(eye, center, up);
    
//     // Calculate rotation angles (adjust sensitivity as needed)
//     float rotX = osg::DegreesToRadians(dx * 0.5f);
//     float rotY = osg::DegreesToRadians(dy * 0.5f);
    
//     // Create rotation quaternions
//     osg::Quat rotXQuat(rotX, up);
//     osg::Vec3d right = (center-eye)^up;
//     right.normalize();
//     osg::Quat rotYQuat(rotY, right);
    
//     // Combine rotations
//     osg::Quat totalRot = rotXQuat * rotYQuat;
    
//     // Rotate around scene center
//     osg::Vec3d eyeOffset = eye - _rotationCenter;
//     osg::Vec3d centerOffset = center - _rotationCenter;
    
//     eyeOffset = totalRot * eyeOffset;
//     centerOffset = totalRot * centerOffset;
    
//     // Apply new positions
//     tmr->setTransformation(_rotationCenter + eyeOffset, 
//                           _rotationCenter + centerOffset, 
//                           totalRot * up);
    
//     return 1;
// }				
		// if (event==FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE) {
		// 	// Pass other mouse releases to OSG
		// 	if (_gw.valid()) _gw->getEventQueue()->mouseButtonRelease(Fl::event_x(), Fl::event_y(), Fl::event_button());
		// 	return 1;
		// }


		// if (event==FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE){
		// 	if (_gw.valid()) _gw->getEventQueue()->mouseButtonPress(Fl::event_x(), Fl::event_y(), Fl::event_button());
		// 	return 1;
		// }
		// if (event==FL_DRAG && Fl::event_button() == FL_LEFT_MOUSE){
		// 	// Pass other mouse movements to OSG
		// 	if (_gw.valid()) _gw->getEventQueue()->mouseMotion(Fl::event_x(), Fl::event_y());
		// 	return 1;
		// }				
		if (event==FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE) {
			// Pass other mouse releases to OSG
			if (_gw.valid()) _gw->getEventQueue()->mouseButtonRelease(Fl::event_x(), Fl::event_y(), Fl::event_button());
			return 1;
		}

		//pan
		if (event==FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE) {
			// Handle right mouse for panning - don't pass to OSG
			rightMousePressed = true;
			lastMouseX = Fl::event_x();
			lastMouseY = Fl::event_y();
			return 1;
		}
		if (event==FL_DRAG && rightMousePressed && Fl::event_button() == FL_RIGHT_MOUSE) {
				// Handle right mouse drag for panning
				int deltaX = Fl::event_x() - lastMouseX;
				int deltaY = Fl::event_y() - lastMouseY;                    

				// Get current transformation
				osg::Vec3d eye, center, up;
				tmr->getTransformation(eye, center, up);                 

				// Calculate pan vectors in screen space
				osg::Vec3d viewDir = center - eye;
				viewDir.normalize();
				osg::Vec3d right = viewDir ^ up;
				right.normalize();
				osg::Vec3d trueUp = right ^ viewDir;
				trueUp.normalize();                    

				// Pan sensitivity factor
				double panFactor = (center - eye).length() * 0.001;                    

				// Apply panning
				osg::Vec3d panOffset = right * (-deltaX * panFactor) + trueUp * (deltaY * panFactor);
				osg::Vec3d newEye = eye + panOffset;
				osg::Vec3d newCenter = center + panOffset;
				tmr->setTransformation(newEye, newCenter, up);
				lastMouseX = Fl::event_x();
				lastMouseY = Fl::event_y();
				return 1;
			}                 
			if (event==FL_RELEASE && Fl::event_button() == FL_RIGHT_MOUSE) {
				// Handle right mouse release for panning
				rightMousePressed = false;
				return 1;
			}





// if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
//     _leftDown = true;
//     _lastDragX = Fl::event_x();
//     _lastDragY = Fl::event_y();
//     return 1;
// }

// if (event == FL_DRAG && _leftDown) {
//     int x = Fl::event_x(), y = Fl::event_y();
//     float dx = float(x - _lastDragX) / float(w());
//     float dy = float(y - _lastDragY) / float(h());
//     _lastDragX = x;
//     _lastDragY = y;

//     osg::Vec3d eye, center, up;
//     if (tmr) tmr->getTransformation(eye, center, up);

//     osg::Vec3d viewDir = center - eye; viewDir.normalize();
//     osg::Vec3d right = viewDir ^ up; right.normalize();

//     const float angleScale = osg::PI * 2.0f;
//     osg::Quat rotH(dx * angleScale, up);
//     osg::Quat rotV(dy * angleScale, right);
//     osg::Quat rot = rotH * rotV;

//     // aplica rotação em torno do pivot local (sem drift acumulado)
//     osg::Matrix M = _rootXform->getMatrix();
//     osg::Matrix R = 
//         osg::Matrix::translate(_pivot_local) *
//         osg::Matrix::rotate(rot) *
//         osg::Matrix::translate(-_pivot_local);

//     _rootXform->setMatrix(R * M);  // acumula suavemente
//     return 1;
// }



// if (event == FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE) {
//     _leftDown = false;
//     return 1;
// }














// 			if (event == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
//     _pivot = calculateWorldIntersectionPivot(Fl::event_x(), Fl::event_y(), this);
//     _leftDown = true;
//     _lastDragX = Fl::event_x();
//     _lastDragY = Fl::event_y();
//     return 1;
// }

// if (event == FL_DRAG && _leftDown) {
//     int x = Fl::event_x(), y = Fl::event_y();
//     float dx = float(x - _lastDragX) / float(w());
//     float dy = float(y - _lastDragY) / float(h());
//     _lastDragX = x;
//     _lastDragY = y;

//     osg::Vec3d eye, center, up;
//     tmr->getTransformation(eye, center, up);
//     osg::Vec3d viewDir = center - eye; viewDir.normalize();
//     osg::Vec3d right = viewDir ^ up; right.normalize();

//     const float angleScale = osg::PI * 2.0f;
//     osg::Quat rotH(dx * angleScale, up);
//     osg::Quat rotV(dy * angleScale, right);
//     osg::Quat rot = rotH * rotV;

//     // Atenção: como o _pivot está em coordenadas de mundo, temos que
//     // converter a rotação em uma matriz de transformação no mundo também.

//     // osg::Matrix local = _rootXform->getMatrix();
// // 1. Inverte a matriz atual (de mundo) para converter o pivot para o espaço local:
// osg::Matrixd invLocal = osg::Matrixd::inverse(_rootXform->getMatrix());
// osg::Vec3d localPivot = _pivot * invLocal;

// // 2. Aplica a rotação ao redor do pivot agora no espaço local:
// osg::Matrix rotation =
//     osg::Matrix::translate(localPivot) *
//     osg::Matrix::rotate(rot) *
//     osg::Matrix::translate(-localPivot);

// // 3. Aplica à transformação existente:
// osg::Matrix local = _rootXform->getMatrix();
// _rootXform->setMatrix(local * rotation);


//     return 1;
// }

// if (event == FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE) {
//     _leftDown = false;
//     return 1;
// }







		return Fl_Gl_Window::handle(event);
    }



    void draw() override {  

        if(!is_minimized && !done()) {
    		// Fl::wait(0.01);			
            frame(); 
			// damage()
		}

    }

     

};
ViewerFLTK* ViewerFLTK::instance = nullptr;



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
	osggl->setCameraManipulator(osggl->tmr);
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
    Fl::scheme("gleam");	
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

    osggl=new ViewerFLTK(w*0.3,  22, w*0.45, h*1); 
	
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

osg::ref_ptr<osg::ShapeDrawable> marker = new osg::ShapeDrawable(new osg::Sphere(osggl->_pivot, 50.0));
marker->setColor(osg::Vec4(1, 0, 0, 1));
osg::ref_ptr<osg::Geode> g = new osg::Geode;
g->addDrawable(marker.get());
osggl->_rootXform->addChild(g); // adiciona ao mesmo espaço







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

  
	win->clear_visible_focus(); 	 
	// win->color(0x7AB0CfFF);
	win->color(FL_RED);
	win->resizable(content);
	 
	
	win->position(0,0);
	// win->position(Fl::w()/2-win->w()/2,10);
	osggl->setbar5per();
	win->show(); 

	// int x, y, _w, _h;
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22);
 
	// malloc_trim(0);
    return Fl::run(); 
}