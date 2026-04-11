#include "includes.hpp"
#include <Graphic3d_CubeMapPacked.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Graphic3d_TypeOfShadingModel.hxx>
#include <V3d_SpotLight.hxx>
#include <Graphic3d_Texture2D.hxx>
#include <TColStd_ListOfTransient.hxx>


#include <FL/Fl_Scroll.H>



#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_AmbientLight.hxx>

#include <Graphic3d_RenderingParams.hxx>
#include <Graphic3d_TextureEnv.hxx>
#include <Graphic3d_PBRMaterial.hxx>
#include <Graphic3d_MaterialAspect.hxx>

#include <Image_AlienPixMap.hxx>

#include <Prs3d_Drawer.hxx>

#include <Quantity_Color.hxx>
#include <Quantity_ColorRGBA.hxx>

#include <Standard_Handle.hxx>
#include "fl_scintilla.hpp"
#include "fl_browser_msv.hpp"
#include "general.hpp"

//region helpers
//this is not giving errors but isnt that nice, please improve or tune, give complete code
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

#include <iostream>
#include <algorithm>
#include <cmath>


Handle(AIS_InteractiveContext) ctx;
Handle(V3d_SpotLight) sun;  
Handle(V3d_View) m_view; 
double strcfg(const std::string& label,double min=0,double max=10);
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Trsf.hxx>

gp_Dir RotateDir(const gp_Dir& dir,
                 const gp_Dir& axis,
                 double degrees)
{
    // Convert degrees to radians
    double radians = degrees * M_PI / 180.0;

    // Build rotation transform
    gp_Trsf trsf;
    trsf.SetRotation(gp_Ax1(gp_Pnt(0,0,0), axis), radians);

    // Apply to a vector (gp_Dir cannot be transformed directly)
    gp_Vec v(dir.X(), dir.Y(), dir.Z());
    v.Transform(trsf);

    // Return normalized direction
    return gp_Dir(v);
}

void addViewportLights(const Handle(V3d_View)& view, const Handle(V3d_Viewer)& viewer)
{
    // Clear existing lights (your code is fine)
    TColStd_ListOfTransient lights;
    for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
        lights.Append(viewer->ActiveLight());

    for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
        viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));


// const int qtty = strcfg("qtty", 1, 31);
// gp_Dir dir(strcfg("gx", 0.8), strcfg("gy", -0.6), strcfg("gz", 0.9));  // tune base if you want

// for(int i = 0; i < qtty; ++i)
// {
//     double angle = i * (360.0 / qtty);   // even spread around

//     // gp_Dir dir = base;
//     dir = RotateDir(dir, gp_Dir(0,0,1), angle);           // around Z
//     dir = RotateDir(dir, gp_Dir(1,0,0), 25.0 + i*3.0);    // small variation in elevation

//     Handle(V3d_DirectionalLight) light = new V3d_DirectionalLight(dir, Quantity_NOC_WHITE, Standard_True);

//     // vary intensity a bit so not all lights are equal
//     light->SetIntensity(strcfg("Intensity1", 0.5, 2.0) * (0.7 + 0.6 * sin(i * 1.3)));

//     viewer->AddLight(light);
//     viewer->SetLightOn(light);
// }



gp_Dir d1(strcfg("gx",-0.99,1), strcfg("gy",-0.99,1), strcfg("gz",-0.99,1));

// Get camera rotation
const Graphic3d_Mat4d& mat = view->Camera()->OrientationMatrix();


gp_Mat R(
    mat.GetValue(0,0), mat.GetValue(0,1), mat.GetValue(0,2),
    mat.GetValue(1,0), mat.GetValue(1,1), mat.GetValue(1,2),
    mat.GetValue(2,0), mat.GetValue(2,1), mat.GetValue(2,2)
);

for(int i = 1; i < strcfg("qtty",1,31); i++)
{
    // gp_Dir d = d1;
    d1 = RotateDir(d1, gp_Dir(0,0,1), i * strcfg("z",1,90));
    d1 = RotateDir(d1, gp_Dir(0,1,0), i * strcfg("y",1,90));
    d1 = RotateDir(d1, gp_Dir(1,0,0), i * strcfg("x",1,90));

    // Anchor to viewport
    gp_Dir d_view = gp_Dir(R * d1.XYZ());

    Handle(V3d_DirectionalLight) keyLight =
        new V3d_DirectionalLight(d_view, Quantity_NOC_WHITE, 0);

    keyLight->SetIntensity(strcfg("Intensity1", 0.1, 24.0));
    viewer->AddLight(keyLight);
    viewer->SetLightOn(keyLight);





	// gp_Dir d1(strcfg("gx",-0.99,1),strcfg("gy",-0.99,1), strcfg("gz",-0.99,1));
	// for(int i=1;i<strcfg("qtty",1,31);i++){
    // // 1. KEY LIGHT: Primary bright light (Front-Top-Right in world)

	// // gp_Dir d1(1.0, -1.0, 1.0);
	// d1=RotateDir(d1,gp_Dir(0, 0, 1),i*strcfg("z",1,90));
	// d1=RotateDir(d1,gp_Dir(0, 1, 0),i*strcfg("y",1,90));
	// d1=RotateDir(d1,gp_Dir(1, 0, 0),i*strcfg("x",1,90));
    // Handle(V3d_DirectionalLight) keyLight = new V3d_DirectionalLight(
    //     // gp_Dir(1.0, -1.0, 1.0),      // direction towards the light (or from light - both work, OCCT normalizes)
	// 	d1,
    //     Quantity_NOC_WHITE,
    //     0);             // <-- Important: not a headlight

    // keyLight->SetIntensity(strcfg("Intensity1", 0.1, 24.0));
    // viewer->AddLight(keyLight);
    // viewer->SetLightOn(keyLight);
// continue;
//     // 2. FILL LIGHT: Softer fill from opposite side (Front-Left)
//     Handle(V3d_DirectionalLight) fillLight = new V3d_DirectionalLight(
//         // gp_Dir(-1.0, -0.5, 1.0),
// 		V3d_TypeOfOrientation_Yup_AxoRight,
//         Quantity_NOC_WHITE,
//         i);

//     fillLight->SetIntensity(strcfg("Intensity2", 0.1, 24.0));
//     viewer->AddLight(fillLight);
//     viewer->SetLightOn(fillLight);

//     // 3. RIM / BACK LIGHT: Creates nice edge highlight from behind/top
//     Handle(V3d_DirectionalLight) rimLight = new V3d_DirectionalLight(
//         // gp_Dir(0.2, 1.0, 0.8),       // mostly from back + a bit up
// 		V3d_TypeOfOrientation_Yup_Top,
//         Quantity_NOC_WHITE,
//         i);

//     rimLight->SetIntensity(strcfg("Intensity3", 0.1, 24.0));
//     viewer->AddLight(rimLight);
//     viewer->SetLightOn(rimLight);
	}

    // AMBIENT: very subtle
    Handle(V3d_AmbientLight) ambLight = new V3d_AmbientLight(Quantity_NOC_WHITE);
    ambLight->SetIntensity(strcfg("amb",0.01,1));   // tweak to taste (0.1–0.25 usually good)
    viewer->AddLight(ambLight);
    viewer->SetLightOn(ambLight);

    viewer->UpdateLights();
}

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------
struct ViewState
{
    Graphic3d_TypeOfShadingModel shading;
    Quantity_Color               bg1;
    bool                         ibl = false;
};

static ViewState g_state;
static bool      g_saved = false;

struct StudioEnv
{
    Handle(Graphic3d_TextureEnv) env;
    Handle(Image_PixMap)         image;
};


static StudioEnv CreateStudioEnv()
{
    StudioEnv out;

    const int W = 2048;
    const int H = 1024;

    out.image = new Image_PixMap();
    if (!out.image->InitTrash(Image_Format_RGB, W, H))
        return out;

    auto gaussian = [](float x, float mu, float sigma)
    {
        float d = (x - mu) / sigma;
        return std::exp(-0.5f * d * d);
    };

    for (int y = 0; y < H; ++y)
    {
        float v   = float(y) / float(H - 1);
        float lat = (v - 0.5f) * float(M_PI);
        float latWeight = std::max(0.2f, std::cos(lat));

        for (int x = 0; x < W; ++x)
        {
            float u = float(x) / float(W - 1);

            float light =
                gaussian(u, 0.25f, 0.03f) +
                gaussian(u, 0.75f, 0.03f) +
                0.6f * gaussian(u, 0.5f, 0.08f);

            light *= latWeight;

            float base = 0.12f + 0.25f * v; // darker → less shiny

            float r = std::min(base + light * 0.9f, 1.0f);
            float g = std::min(base + light * 0.9f, 1.0f);
            float b = std::min(base + light * 1.1f, 1.0f);

            out.image->SetPixelColor(x, y, Quantity_Color(r, g, b, Quantity_TOC_RGB));
        }
    }

    out.env = new Graphic3d_TextureEnv(out.image);
    return out;
}


static Handle(Graphic3d_Texture2D) PixmapToTexture(const Handle(Image_PixMap)& img)
{
    if (img.IsNull())
        return Handle(Graphic3d_Texture2D)();

    return new Graphic3d_Texture2D(img);
}


// -----------------------------------------------------------------------------
// Restore view
// -----------------------------------------------------------------------------
static void RestoreView(const Handle(V3d_View)& view)
{
    if (!g_saved || view.IsNull())
        return;

    view->SetShadingModel(g_state.shading);
    view->SetImageBasedLighting(g_state.ibl);

    view->SetBackgroundImage(Handle(Graphic3d_Texture2D)(), Aspect_FM_STRETCH, Standard_False);
    view->SetBgGradientColors(g_state.bg1, g_state.bg1, Aspect_GFM_NONE, Standard_False);
    view->SetBackgroundColor(g_state.bg1);

    view->SetTextureEnv(Handle(Graphic3d_TextureEnv)());
}

// -----------------------------------------------------------------------------
// Ensure PBR environment (debug helper, optional)
// -----------------------------------------------------------------------------
static void EnsurePBREnvironment(const Handle(V3d_View)& view)
{
    if (view.IsNull())
        return;

    view->SetShadingModel(Graphic3d_TOSM_PBR);

    StudioEnv studio = CreateStudioEnv();
    view->SetTextureEnv(studio.env);

    Graphic3d_RenderingParams& params = view->ChangeRenderingParams();
    params.Method                = Graphic3d_RM_RASTERIZATION;
    params.PbrEnvPow2Size        = 9;
    params.PbrEnvSpecMapNbLevels = 6;

    // Shadows (supported in 7.9.3)
    params.IsShadowEnabled     = Standard_True;
    params.ShadowMapResolution = 1024;
    params.ShadowMapBias       = 0.002f;

    view->SetImageBasedLighting(Standard_True);
    view->GeneratePBREnvironment();
}
#include <V3d_Light.hxx>
// -----------------------------------------------------------------------------
// Apply PBR industrial look
// -----------------------------------------------------------------------------
static void ApplyPBR(const Handle(V3d_View)& view)
{
    if (view.IsNull())
        return;

    if (!g_saved)
    {
        g_state.shading = view->ShadingModel();
        g_state.bg1     = view->BackgroundColor();
        g_state.ibl     = view->IsImageBasedLighting();
        g_saved         = true;
    }

    view->SetShadingModel(Graphic3d_TOSM_PBR);
	


{
// 	Graphic3d_RenderingParams& aParams = view->ChangeRenderingParams();
// // specifies rendering mode
// aParams.Method = Graphic3d_RM_RAYTRACING;
// // maximum ray-tracing depth
// aParams.RaytracingDepth = 3;
// // enable shadows rendering
// aParams.IsShadowEnabled = true;
// // enable specular reflections
// aParams.IsReflectionEnabled = true;
// // enable adaptive anti-aliasing
// aParams.IsAntialiasingEnabled = true;
// // enable light propagation through transparent media
// aParams.IsTransparentShadowEnabled = true;
// // update the view
// view->Update();
}


Handle(V3d_Viewer) viewer = view->Viewer();


// Enable default lights (VERY IMPORTANT)
viewer->SetDefaultLights();
viewer->SetLightOn();

// Optional: add a directional light for stronger gradient
Handle(V3d_DirectionalLight) dirLight =
    new V3d_DirectionalLight(
        gp_Dir(-1.0, -1.0, -1.0),  // light direction
        Quantity_Color(Quantity_NOC_WHITE),
        Standard_True              // headlight = false here
    );
dirLight->SetIntensity(1000.0);
viewer->AddLight(dirLight);
viewer->SetLightOn(dirLight);

// Create view
// Handle(V3d_View) view = viewer->CreateView();

// Set background (helps contrast)
view->SetBackgroundColor(Quantity_NOC_BLACK);

// MUST: enable shading mode
view->SetShadingModel(V3d_GOURAUD);


    StudioEnv studio = CreateStudioEnv();
    view->SetTextureEnv(studio.env);
// 	Handle(V3d_DirectionalLight) l1 = new V3d_DirectionalLight(V3d_XnegYnegZneg, Quantity_NOC_WHITE, Standard_True);
// Handle(V3d_DirectionalLight) l2 = new V3d_DirectionalLight(V3d_XposYposZpos, Quantity_NOC_GRAY80, Standard_True);
// view->Viewer()->AddLight(l1);
// view->Viewer()->SetLightOn(l1);
// view->Viewer()->AddLight(l2);
// view->Viewer()->SetLightOn(l2);
// l1->SetIntensity(0.2f);
// l2->SetIntensity(0.2f);

// l1->SetIntensity(100.5f);
// l2->SetIntensity(100.5f);


 
// Criar uma luz posicional (Point Light)
// Handle(V3d_Light) light = new V3d_Light (Graphic3d_TypeOfLightSource_Positional);
// light->SetPosition (gp_Pnt (500.0, 500.0, 500.0));
// light->SetColor (Quantity_NOC_WHITE);
// light->SetIntensity (1000.0f); // Valor normalizado

// // Adicionar ao visualizador
// view->Viewer()->AddLight (light);
// view->Viewer()->SetLightOn (light);

// Importante para realismo: Ativar o modelo de sombreamento PHONG na View
// view->SetShadingModel (Graphic3d_TOSM_PHONG);



    Graphic3d_RenderingParams& p = view->ChangeRenderingParams();
    p.Method                = Graphic3d_RM_RASTERIZATION;
    p.PbrEnvPow2Size        = 9;
    p.PbrEnvSpecMapNbLevels = 6;

    // Shadows only (SSAO not available)
    p.IsShadowEnabled     = Standard_True;
    p.ShadowMapResolution = 1024;
    p.ShadowMapBias       = 0.002f;

    view->SetImageBasedLighting(Standard_True);
    view->GeneratePBREnvironment();

    // Industrial background (no gradient, no image)
    view->SetBackgroundImage(Handle(Graphic3d_Texture2D)(), Aspect_FM_STRETCH, Standard_False);
    view->SetBgGradientColors(g_state.bg1, g_state.bg1, Aspect_GFM_NONE, Standard_False);
    view->SetBackgroundColor(Quantity_Color(0.92, 0.92, 0.92, Quantity_TOC_RGB));
}

static void ApplyFakeBevelNormals(const TopoDS_Shape& theShape,
                                  const gp_Pnt& theCenter)
{
    for (TopExp_Explorer ex(theShape, TopAbs_FACE); ex.More(); ex.Next())
    {
        TopoDS_Face face = TopoDS::Face(ex.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull())
            continue;

        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
        Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surf);
        if (plane.IsNull())
            continue;

        gp_Dir nFace = plane->Pln().Axis().Direction();
        if (face.Orientation() == TopAbs_REVERSED)
            nFace.Reverse();

        const Standard_Integer nbNodes = tri->NbNodes();
        Handle(TShort_HArray1OfShortReal) norms =
            new TShort_HArray1OfShortReal(1, 3 * nbNodes);

        for (Standard_Integer i = 1; i <= nbNodes; ++i)
        {
            gp_Pnt p = tri->Node(i).Transformed(loc.Transformation());

            // normal radial (rounded cube)
            gp_Dir nRad(p.X() - theCenter.X(),
                        p.Y() - theCenter.Y(),
                        p.Z() - theCenter.Z());

            // ângulo entre normais
            Standard_Real angle = nRad.Angle(nFace);

            gp_Dir n;
            if (angle < (M_PI / 3.0)) // < 60º → suaviza (fake bevel)
            {
                n = gp_Dir(
                    nRad.X() * 0.6 + nFace.X() * 0.4,
                    nRad.Y() * 0.6 + nFace.Y() * 0.4,
                    nRad.Z() * 0.6 + nFace.Z() * 0.4
                );
            }
            else
            {
                n = nFace;
            }

            Standard_Integer base = 3 * (i - 1) + 1;
            norms->SetValue(base + 0, (Standard_ShortReal)n.X());
            norms->SetValue(base + 1, (Standard_ShortReal)n.Y());
            norms->SetValue(base + 2, (Standard_ShortReal)n.Z());
        }

        tri->SetNormals(norms);
    }
}

static Handle(Graphic3d_TextureEnv) CreateCadEnv() {
    const int W = 1024;
    const int H = 512;

    Handle(Image_PixMap) img = new Image_PixMap();
    if (!img->InitTrash(Image_Format_RGB32, W, H)) {
        std::cerr << "Failed to create env map\n";
        return nullptr;
    }

    auto softbox = [](float u, float v, float cu, float cv, float su, float sv) -> float {
        float du = fabs(u - cu); if (du > 0.5f) du = 1.0f - du;
        float dv = fabs(v - cv);
        float fu = std::max(0.0f, 1.0f - du / su);
        float fv = std::max(0.0f, 1.0f - dv / sv);
        fu = fu * fu * (3.0f - 2.0f * fu);
        fv = fv * fv * (3.0f - 2.0f * fv);
        return fu * fv;
    };

    // 🔥 controla a intensidade global do environment map
    float envIntensity = 0.25f;   // 25% da intensidade original

    for (int y = 0; y < H; ++y) {
        float v = static_cast<float>(y) / static_cast<float>(H - 1);
        for (int x = 0; x < W; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(W - 1);
            u = std::max(0.0f, std::min(1.0f, u));
            v = std::max(0.0f, std::min(1.0f, v));

            float val = 0.05f;

            val += 3.2f * softbox(u, v, 0.5f, 0.18f, 0.20f, 0.24f);
            val += 2.5f * softbox(u, v, 0.10f, 0.48f, 0.08f, 0.42f);
            val += 2.2f * softbox(u, v, 0.90f, 0.52f, 0.07f, 0.38f);
            val += 1.4f * softbox(u, v, 0.5f, 0.75f, 0.32f, 0.22f);

            float r = val;
            float g = val;
            float b = val;

            // aplicar intensidade global
            r *= envIntensity;
            g *= envIntensity;
            b *= envIntensity;

            r = std::max(0.0f, std::min(1.0f, r));
            g = std::max(0.0f, std::min(1.0f, g));
            b = std::max(0.0f, std::min(1.0f, b));

            img->SetPixelColor(x, y, Quantity_Color(r, g, b, Quantity_TOC_RGB));
        }
    }

    return new Graphic3d_TextureEnv(img);
}
void ray(const Handle(V3d_View)& myView,  const std::vector<Handle(AIS_Shape)>& shapes){
		// 1. Configurar a View para Raytracing
myView->ChangeRenderingParams().Method = Graphic3d_RM_RAYTRACING;
myView->ChangeRenderingParams().IsAntialiasingEnabled = Standard_True; // Melhora o aspeto do espelho

// 2. Criar o material de espelho
Graphic3d_MaterialAspect aMirrorMat(Graphic3d_NameOfMaterial_Chrome);
aMirrorMat.SetSpecularColor(Quantity_NOC_WHITE);
aMirrorMat.SetAmbientColor(Quantity_NOC_BLACK);
aMirrorMat.SetDiffuseColor(Quantity_NOC_BLACK);
aMirrorMat.SetShininess(1.0);

// // 3. Aplicar ao Shape
// Handle(AIS_Shape) aAISMirror = new AIS_Shape(minhaShape);
// aAISMirror->SetMaterial(aMirrorMat);

// // 4. Mostrar
// myContext->Display(aAISMirror, Standard_True);	 

// 	// 1. Ativar o Raytracing na View
// view->SetRaytracingMode();

// // 2. Configurar o material para ser um espelho perfeito
// Graphic3d_MaterialAspect aMirrorMat(Graphic3d_NameOfMaterial_Mirror);

// // No modo Raytracing, a "Refratividade" e "Refletividade" são controladas assim:
// aMirrorMat.SetReflectionModeColor(Quantity_NOC_WHITE); // Reflete todas as cores
// aMirrorMat.SetAmbientColor(Quantity_NOC_BLACK);       // Sem cor base (opcional)
// aMirrorMat.SetDiffuseColor(Quantity_NOC_BLACK);       // Sem cor difusa (opcional)

// // 3. Aplicar ao Shape
        for (const auto& shape : shapes)
        {
            if (shape.IsNull())
                continue;
shape->SetMaterial(aMirrorMat);

ctx->Redisplay(shape, Standard_True);
		}
}
// -----------------------------------------------------------------------------
//region NiceSteel 
// -----------------------------------------------------------------------------
void NiceSteel(const Handle(V3d_View)& view,
               const std::vector<Handle(AIS_Shape)>& shapes,
               bool enable)
{
    if (view.IsNull())
        return;

    if (enable)
    {
		// ray(view,shapes);return;


// view->SetShadingModel(V3d_GOURAUD);


    // StudioEnv studio = CreateStudioEnv();
    // view->SetTextureEnv(studio.env);//return;
        // ApplyPBR(view); return;

    Handle(V3d_Viewer) viewer = view->Viewer();
 

// // Desliga todas as luzes existentes
// TColStd_ListOfTransient lights;

// viewer->InitActiveLights();
// while (viewer->MoreActiveLights()) {
//     lights.Append(viewer->ActiveLight());
//     viewer->NextActiveLights();
// }

// for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
//     Handle(V3d_Light) L = Handle(V3d_Light)::DownCast(it.Value());
//     viewer->DelLight(L);
// }

    viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

//  return;
    // ============================================================
    // APLICAR ENVIRONMENT MAP (NÃO REMOVE NADA DO TEU CÓDIGO)
    // ============================================================
    Handle(Graphic3d_TextureEnv) env = CreateCadEnv();
    // Handle(Graphic3d_TextureEnv) env = CreateCadStudioEnv();
	// Handle(Graphic3d_TextureEnv) anEnv = new Graphic3d_TextureEnv(Graphic3d_NOT_ENV_CLOUDS);
    view->SetTextureEnv(env);
	return;
    // viewer->SetUseEnvironmentMap(Standard_True);
// 	viewer->SetEnvironmentTexture(env);
// viewer->SetEnvironmentTextureEnabled(Standard_True);
// view->ChangeRenderingParams().IsIBLEnabled = Standard_True;
Graphic3d_RenderingParams& aParams = view->ChangeRenderingParams();
aParams.Exposure = 0.50f; // Tenta aumentar a exposição
// aParams.ToneMappingMethod = Graphic3d_TonneMappingMethod_Filmic;
// aParams.IsReflectionEnabled = 1; // Tenta aumentar a exposição
// aParams.IsGlobalIlluminationEnabled = 1; // Tenta aumentar a exposição
// aParams.IsTransparentShadowEnabled = 1; // Tenta aumentar a exposição
// aParams.Method = Graphic3d_RM_RASTERIZATION; // Garante que estás em rasterização moderna

// Se estiveres a usar o Ray Tracing (opcional, mas o PBR brilha aqui):
// aParams.Method = Graphic3d_RM_RAYTRACING;
    // ============================================================

    // viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    // view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    // view->SetShadingModel(Graphic3d_TypeOfShadingModel_Phong);

	// viewer->SetLightOn();
    // // viewer->SetLightOff();

    // Luz ambiente
    // Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
    // amb->SetIntensity(1.4f);
    // viewer->AddLight(amb);
    // viewer->SetLightOn(amb);

// Criar spotlight
// // 1. Criar a luz
// sun = new V3d_SpotLight(gp_Pnt(0,0,0), gp_Dir(0,0,-1), Quantity_NOC_WHITE);

// // 2. Ângulo pequeno e correto (aprox 30 graus de abertura total)
// sun->SetAngle(0.5); // radianos

// // 3. Intensidade massiva para compensar o PBR em escalas grandes
// sun->SetIntensity(10000000.0f); 

// // 4. Suavidade da borda (0.0 a 1.0 é o ideal para começar)
// sun->SetConcentration(1.0); 

// // 5. Alcance (Range) - Certifique-se que o objeto está DENTRO deste raio
// sun->SetRange(20000.0); 

// viewer->AddLight(sun);
// viewer->SetLightOn(sun);
 

// {
// 	gp_Pnt eye    = view->Camera()->Eye();
// gp_Pnt center = view->Camera()->Center();

// gp_Vec vecFromEyeToCenter(eye, center);   // vetor do olho para o centro
// gp_Dir lookDir(vecFromEyeToCenter);

// gp_Dir lightDir = lookDir.Reversed();

// Handle(V3d_DirectionalLight) sun = new V3d_DirectionalLight(lightDir, Quantity_NOC_WHITE);
// viewer->AddLight(sun);
// viewer->SetLightOn(sun);
// viewer->UpdateLights();
// }

// {
// 	// Remove all default lights
// viewer->SetLightOff();

// gp_Pnt eye    = view->Camera()->Eye();
// gp_Pnt center = view->Camera()->Center();
// gp_Dir dir(center.XYZ() - eye.XYZ());

// Handle(V3d_SpotLight) spot =
//     new V3d_SpotLight(eye, dir, Quantity_NOC_WHITE);

// spot->SetIntensity(3000000000.0);
// spot->SetAngle(2.12);          // 70 degrees
// // spot->SetConcentration(100.0);
// // spot->SetAttenuation(1.0, 0.0);
// spot->SetCastShadows(Standard_False);
// spot->SetHeadlight(Standard_True);

// viewer->AddLight(spot);
// viewer->SetLightOn(spot);
// }


    // Luz direcional

    // Handle(V3d_DirectionalLight) sun =
    //     new V3d_DirectionalLight(gp_Dir(0.4, -0.7, -1.0), Quantity_NOC_WHITE);
    // sun->SetIntensity(9.5f);
    // viewer->AddLight(sun);
    // viewer->SetLightOn(sun);
	// sun->SetHeadlight(Standard_True);
// gp_Dir worldDir = view->Camera()->Direction();
// worldDir.Reverse();
// Handle(V3d_DirectionalLight) sun = new V3d_DirectionalLight(worldDir, Quantity_NOC_WHITE);
// sun->SetIntensity(20.0f);
// sun->SetHeadlight(Standard_True);
// viewer->AddLight(sun);
// viewer->SetLightOn(sun);
// viewer->UpdateLights();

// gp_Dir worldDir = view->Camera()->Direction();
// worldDir.Reverse(); 
// sun = new V3d_DirectionalLight(worldDir, Quantity_NOC_WHITE);
// sun->SetIntensity(1000.0); // Valor normalizado padrão
// // sun->SetHeadlight(Standard_True); // Faz a luz seguir a câmera sozinho

// viewer->AddLight(sun);
// viewer->SetLightOn(sun);
// viewer->UpdateLights();



// 	Handle(V3d_SpotLight) spot =
//     new V3d_SpotLight(gp_Pnt(0,5000,10), gp_Dir(0,1,0), Quantity_NOC_WHITE);

// spot->SetAngle(0.8); // “largura” do cone
// spot->SetIntensity(5.0f);

// viewer->AddLight(spot);
// viewer->SetLightOn(spot);



        for (const auto& shape : shapes)
        {
            if (shape.IsNull())
                continue;


shape->UnsetAttributes();

TopoDS_Shape bshape = shape->Shape();

    // BRepMesh_IncrementalMesh mesher1(bshape, 0.005, Standard_False, 0.01, Standard_True);
    // BRepMesh_IncrementalMesh mesher1(bshape, 0.1, Standard_False, 0.1, Standard_True);

    // ApplyFakeBevelNormals(bshape, view->Camera()->Eye());
    // ApplyFakeBevelNormals(bshape, gp_Pnt(5.0, 5.0, 5.0));

    // Handle(AIS_Shape) ais1 = new AIS_Shape(box1);
// cotm(99999999999);
    Graphic3d_PBRMaterial pbr1;
    pbr1.SetMetallic(0.04f);
    pbr1.SetRoughness(0.04f);

	auto srgbToLinear = [](double c)->double {
    if (c <= 0.04045) return c / 12.92;
    return pow((c + 0.055) / 1.055, 2.4);
};

Quantity_Color srgb(0.54118,0.54118,0.61569, Quantity_TOC_RGB);
Quantity_Color linear(
    srgbToLinear(srgb.Red()),
    srgbToLinear(srgb.Green()),
    srgbToLinear(srgb.Blue()),
    Quantity_TOC_RGB
);
// pbr1.Albedo = linear;

    Graphic3d_MaterialAspect mat1(Graphic3d_NameOfMaterial_UserDefined);
    mat1.SetPBRMaterial(pbr1);
	mat1.SetColor(linear);
	mat1.SetEmissiveColor(Quantity_NOC_RED);
    // mat1.SetColor( Quantity_NOC_DARKGOLDENROD2); // azul
    // mat1.SetColor(Quantity_Color(0.54118,0.54118,0.61569, Quantity_TOC_RGB));; // azul
    // mat1.SetColor(Quantity_NOC_GRAY30); // azul
    // mat1.SetColor(Quantity_Color(0.2, 0.4, 1.0, Quantity_TOC_RGB)); // azul

    shape->SetMaterial(mat1);
    shape->Attributes()->ShadingAspect()->Aspect()
        ->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

		

    // REALÇAR ARESTAS (outline)
    shape->Attributes()->SetFaceBoundaryDraw(Standard_True);
    shape->Attributes()->SetFaceBoundaryAspect(
        new Prs3d_LineAspect(Quantity_Color(0,0,0,Quantity_TOC_RGB),
                             Aspect_TOL_SOLID, 2.0)
    );


            if (auto ctx = shape->InteractiveContext())
                ctx->Redisplay(shape, Standard_False);


// Handle(Prs3d_Drawer) drawer = shape->Attributes();
// drawer->SetShadingAspect(new Prs3d_ShadingAspect());
// drawer->ShadingAspect()->SetInterpolation(Graphic3d_TOSM_VERTEX);

continue;

shape->UnsetColor();
// shape->UnsetAttributes();
// 1. Criar o material PBR
Graphic3d_PBRMaterial aPbrMat;
// aPbrMat.SetColor(Quantity_Color(Quantity_NOC_BLUE));  <----- aqui nao faz nada
aPbrMat.SetMetallic(0.5f);  // Quase metal para testar brilho
aPbrMat.SetRoughness(0.5f); // Bem polido

// 2. Configurar o MaterialAspect (CRUCIAL: definir a cor aqui também)
Graphic3d_MaterialAspect aMat(Graphic3d_NameOfMaterial_UserDefined);
aMat.SetPBRMaterial(aPbrMat);
aMat.SetColor(Quantity_Color(Quantity_NOC_GRAY25)); // Sincroniza Albedo com Diffuse
aMat.SetShininess(1.0f);

// 3. Aplicar diretamente ao AIS_Shape (Método mais limpo)
shape->SetMaterial(aMat); 
shape->SetDisplayMode(1);

// 4. Forçar o Shading Model no Aspecto do objeto
Handle(Prs3d_ShadingAspect) aShading = shape->Attributes()->ShadingAspect();
aShading->Aspect()->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
aShading->Aspect()->SetInteriorStyle(Aspect_IS_SOLID);

            if (auto ctx = shape->InteractiveContext())
                ctx->Redisplay(shape, Standard_False);
        }
    }
    else
    {
        RestoreView(view);

        for (const auto& shape : shapes)
        {
            if (shape.IsNull())
                continue;

            shape->Attributes()->SetFaceBoundaryDraw(Standard_True);
            shape->SynchronizeAspects();

            if (auto ctx = shape->InteractiveContext())
                ctx->Redisplay(shape, Standard_False);
        }
    }

    view->Redraw();
}




 // 5. Fallback: Standard metallic material (works without PBR)
void NiceSteelStandard(
    const Handle(V3d_View)& view,
    const std::vector<Handle(AIS_Shape)>& shapes)
{
    if (view.IsNull()) return;
    
    // Use standard lighting model - use FACET which is commonly available
    view->SetShadingModel(Graphic3d_TOSM_FACET);
    
    // Set dark background
    view->SetBackgroundColor(Quantity_Color(0.1, 0.1, 0.12, Quantity_TOC_RGB));
    
    for (const auto& shape : shapes) {
        if (shape.IsNull()) continue;
        
        // Create metallic material using standard material
        Graphic3d_MaterialAspect mat;
        
        // Steel-like colors for standard material
        mat.SetAmbientColor(Quantity_Color(0.3f, 0.3f, 0.35f, Quantity_TOC_RGB));
        mat.SetDiffuseColor(Quantity_Color(0.75f, 0.75f, 0.8f, Quantity_TOC_RGB));
        mat.SetSpecularColor(Quantity_Color(0.9f, 0.9f, 1.0f, Quantity_TOC_RGB));
        mat.SetShininess(80.0f);  // High shininess for metallic look
        
        // Apply material
        shape->Attributes()->SetupOwnShadingAspect();
        shape->Attributes()->ShadingAspect()->SetMaterial(mat);
        shape->Attributes()->SetFaceBoundaryDraw(Standard_False);
        shape->SynchronizeAspects();
        
        Handle(AIS_InteractiveContext) ctx = shape->InteractiveContext();
        if (!ctx.IsNull()) {
            ctx->Redisplay(shape, Standard_False);
        }
    }
    
    view->Redraw();
}




TopoDS_Shape ExtractFilletEdges(const TopoDS_Shape& shape)
{
    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);

    for (TopExp_Explorer fexp(shape, TopAbs_FACE); fexp.More(); fexp.Next())
    {
        TopoDS_Face face = TopoDS::Face(fexp.Current());
        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

        bool isFillet =
            surf->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface) ||
            surf->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface) ||
            surf->DynamicType() == STANDARD_TYPE(Geom_ToroidalSurface) ||
            surf->DynamicType() == STANDARD_TYPE(Geom_SphericalSurface);

        if (!isFillet)
            continue;

        for (TopExp_Explorer eexp(face, TopAbs_EDGE); eexp.More(); eexp.Next())
        {
            builder.Add(result, eexp.Current());
        }
    }

    return result;
}


// Generic function: replaces one shape inside a compound with a modified version
void ReplaceShapeInCompound(
    TopoDS_Compound& compound,
    TopoDS_Shape& targetShape,
    const std::function<TopoDS_Shape(const TopoDS_Shape&)>& modifier
) {
    BRep_Builder builder;
    TopoDS_Compound newCompound;
    builder.MakeCompound(newCompound);

    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        const TopoDS_Shape& currentShape = it.Value();

        if (currentShape.IsSame(targetShape)) {
            TopoDS_Shape newShape = modifier(currentShape);
            if (!newShape.IsNull()) {
                builder.Add(newCompound, newShape);
                targetShape = newShape; // update reference
            } else {
                builder.Add(newCompound, currentShape); // fallback
            }
        } else {
            builder.Add(newCompound, currentShape);
        }
    }

    compound = newCompound;
}


TopoDS_Shape FixShape(const TopoDS_Shape& s) {
    ShapeFix_Shape fixer(s);
    fixer.Perform();
    return fixer.Shape();
}
TopoDS_Shape CleanShape(const TopoDS_Shape& s) {
    ShapeUpgrade_UnifySameDomain unify(s, true, true, true);
    unify.Build();
    return unify.Shape();
}

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

		// Helper: compute perpendicular vector
		gp_Vec2d Perpendicular(const gp_Vec2d& v) { return gp_Vec2d(-v.Y(), v.X()); }



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
void rotateShapeByMouse(Handle(AIS_Shape) toRotate, Handle(AIS_InteractiveContext) context, const gp_Pnt& center, const gp_Dir& axisDir, float dx, float dy) {
	if (toRotate.IsNull() || context.IsNull()) return;

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
std::string lua_error_with_line(lua_State* L, const std::string& msg) {
    lua_Debug ar;

    // level 1 = caller of this function
    if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "Sl", &ar)) {
        std::ostringstream oss;
        oss << msg;

        const char* src = ar.short_src && ar.short_src[0] ? ar.short_src : ar.source;

        if (ar.currentline > 0) {
            oss << " (Lua: " << src << ":" << ar.currentline << ")";
        } else {
            oss << " (Lua: " << src << ")";
        }

        return oss.str();
    }

    return msg;
}


std::vector<TopoDS_Solid> ExtractSolids(const TopoDS_Shape& shape) {
    std::vector<TopoDS_Solid> solids;
    for (TopExp_Explorer exp(shape, TopAbs_SOLID); exp.More(); exp.Next()) {
        solids.push_back(TopoDS::Solid(exp.Current()));
    }
    return solids;
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

// char tests
static inline bool is_space(char c) {
  return c==' '||c=='\t'||c=='\r';
}
static inline bool is_ident_start(char c) {
  return (c=='_') || (c>='A'&&c<='Z') || (c>='a'&&c<='z');
}
static inline bool is_ident_char(char c) {
  return is_ident_start(c) || (c>='0'&&c<='9');
}
static inline char to_lower(char c) {
  return (c>='A'&&c<='Z') ? char(c - 'A' + 'a') : c;
}
static std::string lower_sv(std::string_view sv) {
  std::string s; s.reserve(sv.size());
  for (char c : sv) s.push_back(to_lower(c));
  return s;
}
static inline char sh_tolower(char c) { return (c>='A'&&c<='Z') ? char(c - 'A' + 'a') : c; }
static std::string sh_lower(std::string_view sv) {
    std::string s; s.resize(sv.size());
    for (size_t i=0;i<sv.size();++i) s[i]=sh_tolower(sv[i]);
    return s;
}

std::string translate_shorthand(std::string_view src,
								const std::unordered_set<std::string>& S,  // single-string commands
								const std::unordered_set<std::string>& A   // arg-first commands
) {
	std::string out;
	out.reserve(src.size() + src.size() / 16 + 32);

	size_t i = 0, n = src.size();
	while (i < n) {
		size_t lineEnd = src.find('\n', i);
		if (lineEnd == std::string_view::npos) lineEnd = n;
		std::string_view line = src.substr(i, lineEnd - i);

		// skip blank/comment
		size_t p = 0;
		while (p < line.size() && is_space(line[p])) ++p;
		if (p >= line.size() || (line[p] == '-' && p + 1 < line.size() && line[p + 1] == '-')) {
			out.append(line.data(), line.size());
			if (lineEnd < n) out.push_back('\n');
			i = (lineEnd == n ? n : lineEnd + 1);
			continue;
		}

		// extract command name
		size_t nameStart = p;
		while (p < line.size() && is_ident_char(line[p])) ++p;
		std::string_view name = line.substr(nameStart, p - nameStart);
		std::string lname = lower_sv(name);

		// skip spaces to argument start
		size_t q = p;
		while (q < line.size() && is_space(line[q])) ++q;

		bool handled = false;

		// if already parentheses, pass through
		if (q < line.size() && line[q] == '(') {
			out.append(line.data(), line.size());
			handled = true;
		}
		// single-string commands → wrap entire tail in quotes
		else if (!handled && S.count(lname) && q < line.size()) {
			out.append(line.data() + nameStart, name.size());
			out.push_back('(');
			out.push_back('"');
			for (size_t t = q; t < line.size(); ++t) {
				char c = line[t];
				if (c == '"' || c == '\\') out.push_back('\\');
				out.push_back(c);
			}
			out.push_back('"');
			out.push_back(')');
			handled = true;
		}
		// arg-first commands → split tail on spaces and join by commas
		else if (!handled && A.count(lname) && q < line.size()) {
			out.append(line.data() + nameStart, name.size());
			out.push_back('(');
			// collect tokens
			std::vector<std::string_view> toks;
			size_t r = q;
			while (r < line.size()) {
				while (r < line.size() && is_space(line[r])) ++r;
				if (r >= line.size()) break;
				size_t e = r;
				while (e < line.size() && !is_space(line[e])) ++e;
				toks.emplace_back(line.data() + r, e - r);
				r = e;
			}
			for (size_t ti = 0; ti < toks.size(); ++ti) {
				if (ti) out.push_back(',');
				out.append(toks[ti].data(), toks[ti].size());
			}
			out.push_back(')');
			handled = true;
		}

		if (!handled) {
			out.append(line.data(), line.size());
		}
		if (lineEnd < n) out.push_back('\n');
		i = (lineEnd == n ? n : lineEnd + 1);
	}

	return out;
}

std::string translate_shorthand(std::string_view src){
	std::vector<std::string> vSstring = {"Part","Pl","Move","Rotate"};
	std::vector<std::string> vAstring = {"Offset","Extrude","Fuse","Clone","Intersect","Revolution"};

	std::unordered_set<std::string> S, A;
	S.reserve(vSstring.size()); A.reserve(vAstring.size());
	for (auto& s : vSstring) S.insert(sh_lower(s));
	for (auto& s : vAstring) A.insert(sh_lower(s));

	std::string code = translate_shorthand(src, S, A); 
	return code; 
}

static int shorthand_searcher(lua_State* L) {
    const char* modname = luaL_checkstring(L, 1);

    // filepath = package.searchpath(modname, package.path)
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchpath");
    lua_pushvalue(L, 1);                 // modname
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");         // package.path
    lua_call(L, 2, 1);                    // -> filepath | nil+errmsg
    const char* filepath = lua_tostring(L, -1);

    if (!filepath) {
        // Return the aggregated "not found" message string for require()
        return 1; // leave the error message on the stack
    }

    // Read file
    std::ifstream f(filepath, std::ios::binary);
    if (!f) {
        lua_pushfstring(L, "\n\tno file '%s'", filepath);
        return 1;
    }
    std::string src((std::istreambuf_iterator<char>(f)), {});

 
    std::string code = translate_shorthand(src);

    // Load translated chunk
    if (luaL_loadbuffer(L, code.data(), code.size(), filepath) != LUA_OK) {
        return lua_error(L);
    }
    return 1; // return the loaded function
}

void install_shorthand_searcher(lua_State* L) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers"); // Lua 5.2+ ("loaders" on 5.1)
    lua_pushcfunction(L, shorthand_searcher);
    lua_rawseti(L, -2, 2);            // replace the Lua file searcher
    lua_pop(L, 2);                    // pop searchers, package
}

//region globals
void gopart(const std::string& str);

#define flwindow Fl_Window

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

//region slidercfg
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <vector>
#include <string>

//--------------------------------------
// Slider config
//--------------------------------------
struct SliderCfg {
    std::string label;
    double value;
    double min;
    double max;
	bool called=0;
};

//--------------------------------------
// Build window with sliders from vector
//--------------------------------------
    std::vector<SliderCfg> sliders;// = {
    //     {"Angle",   0.2, 0.01, 15.0},
    //     {"Intensity",  5.0,  1.0, 40},
    //     {"Concentration", 0.0, 0.0, 20.0},
    //     {"Offset", 0.1, 0.1, 0.9},
    //     {"Metallic", 0.5, 0.01, 1.0},
    //     {"Roughness", 0.5, 0.01, 1.0},
    //     {"Ambient", 0.5, 0.01, 3.0},
    //     {"I_top", 10.5, 0.1, 9000.0}, 
    //     {"I_side", 10.5, 0.1, 9000.0}, 
    //     {"I_bottom", 10.5, 0.1, 9000.0}, 
    // };
void saveSliders(const std::string& filename) {
    std::ofstream f(filename);
    if (!f) return;

    f << "{\n  \"sliders\": [\n";

    for (size_t i = 0; i < sliders.size(); ++i) {
        const auto& s = sliders[i];
        f << "    {\"label\":\"" << s.label
          << "\", \"value\":" << s.value
          << ", \"min\":" << s.min
          << ", \"max\":" << s.max << "}";
        if (i + 1 < sliders.size()) f << ",";
        f << "\n";
    }

    f << "  ]\n}";
}

void loadSliders(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) return;

    std::string json((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());

    for (auto& s : sliders) {
        // procura o bloco do slider pelo label
        std::string key = "\"label\":\"" + s.label + "\"";
        size_t pos = json.find(key);
        if (pos == std::string::npos) continue;

        // value
        size_t vpos = json.find("\"value\":", pos);
        if (vpos != std::string::npos)
            s.value = atof(json.c_str() + vpos + 8);

        // min
        size_t mpos = json.find("\"min\":", pos);
        if (mpos != std::string::npos)
            s.min = atof(json.c_str() + mpos + 6);

        // max
        size_t xpos = json.find("\"max\":", pos);
        if (xpos != std::string::npos)
            s.max = atof(json.c_str() + xpos + 6);
    }
}
std::unordered_map<std::string, SliderCfg> cfgindex;
double strcfg(const std::string& label,double min,double max) {
    // static std::unordered_map<std::string, double> index;
    // static std::unordered_map<std::string, int> index;

    // build once
    // if (index.empty()) {
	// 	loadSliders("msvcad.cfg");
    //     for (int i = 0; i < sliders.size(); ++i)
    //         index[sliders[i].label] = i;
    // }

    auto it = cfgindex.find(label);
    if (it == cfgindex.end()){
		cfgindex[label]={label,(max-min)/2.0,min,max,1};
		sliders.push_back(cfgindex[label]);
	}
    return cfgindex[label].value;

 
}

//--------------------------------------
// Callback updates struct value
//--------------------------------------
flwindow *floccv;
// Handle(V3d_View) m_view;
std::vector<Handle(AIS_Shape)> flshapes;
void slider_cb(Fl_Widget *w, void *userdata) {
    auto *cfg = static_cast<SliderCfg*>(userdata);
    cfg->value = static_cast<Fl_Slider*>(w)->value();

	lop(i, 0, flshapes.size()) {
		auto shape=flshapes[i];
Graphic3d_MaterialAspect mat =
    shape->Attributes()->ShadingAspect()->Material();

Graphic3d_PBRMaterial pbr = mat.PBRMaterial();

// std::cout << "Metallic:  " << pbr.Metallic()  << "\n";
// std::cout << "Roughness: " << pbr.Roughness() << "\n";
// std::cout << "Emission:  " << pbr.Emission()  << "\n";

pbr.SetMetallic(strcfg("Metallic"));
pbr.SetRoughness(strcfg("Roughness"));
// mat.SetSpecular(1.0);

mat.SetPBRMaterial(pbr);
shape->SetMaterial(mat);
// // 4. Reaplicar o PBR ao material
// mat.SetPBRMaterial(pbr);

// // 5. Reaplicar o material ao shape
// shape->SetMaterial(mat);

	}

	saveSliders("msvcad.cfg");
	floccv->redraw();
}
#include <tuple>
#include <FL/Fl_Value_Input.H>
struct SliderPack {
    Fl_Slider* slider;
    Fl_Value_Input* minI;
    Fl_Value_Input* maxI;
    SliderCfg* cfg; // <-- replace with actual type
};
int slidercfg() {
    Fl_Group::current(nullptr);

    static Fl_Window* wsl =
        new Fl_Window(win->x() + win->w() / 4, 0,
                      win->w() / 2, win->h(),
                      "Help");

    wsl->set_modal();

    // --- SCROLL AREA ---
    Fl_Scroll* scroll = new Fl_Scroll(0, 0, wsl->w(), wsl->h()-22*2);
    scroll->type(Fl_Scroll::VERTICAL_ALWAYS);

    int y = 10;

for (auto& cfg : cfgindex) {
    int x = 20;
    int width = wsl->w() - 40;

    Fl_Box* label = new Fl_Box(x, y, width, 20, cfg.first.c_str());

    Fl_Value_Input* minInput = new Fl_Value_Input(x, y + 22, 60, 25);
    minInput->value(cfg.second.min);

    Fl_Value_Input* maxInput = new Fl_Value_Input(x + width - 60, y + 22, 60, 25);
    maxInput->value(cfg.second.max);

    Fl_Slider* s = new Fl_Slider(x + 70, y + 22, width - 140, 25);
    s->type(FL_HOR_NICE_SLIDER);
    s->bounds(cfg.second.min, cfg.second.max);
    s->value(cfg.second.value);

    // allocate shared state
    SliderPack* pack = new SliderPack{ s, minInput, maxInput, &cfg.second };

    // --- slider ---
    s->callback([](Fl_Widget* w, void* data) {
        auto* p = static_cast<SliderPack*>(data);
        p->cfg->value = ((Fl_Slider*)w)->value();


		lop(i, 0, flshapes.size()) {
		auto shape=flshapes[i];
Graphic3d_MaterialAspect mat =
    shape->Attributes()->ShadingAspect()->Material();

Graphic3d_PBRMaterial pbr = mat.PBRMaterial();

// std::cout << "Metallic:  " << pbr.Metallic()  << "\n";
// std::cout << "Roughness: " << pbr.Roughness() << "\n";
// std::cout << "Emission:  " << pbr.Emission()  << "\n";

pbr.SetMetallic(strcfg("Metallic"));
pbr.SetRoughness(strcfg("Roughness"));
// mat.SetSpecular(1.0);

mat.SetPBRMaterial(pbr);
shape->SetMaterial(mat);
		}
		addViewportLights(m_view,m_view->Viewer());
		floccv->redraw();
    }, pack);

    // --- min ---
    minInput->callback([](Fl_Widget* w, void* data) {
        auto* p = static_cast<SliderPack*>(data);

        double minVal = p->minI->value();
        double maxVal = p->maxI->value();
        if (minVal >= maxVal) return;

        p->slider->bounds(minVal, maxVal);
        p->cfg->min = minVal;

        if (p->slider->value() < minVal)
            p->slider->value(minVal);

        p->slider->redraw();
    }, pack);

    // --- max ---
    maxInput->callback([](Fl_Widget* w, void* data) {
        auto* p = static_cast<SliderPack*>(data);

        double minVal = p->minI->value();
        double maxVal = p->maxI->value();
        if (minVal >= maxVal) return;

        p->slider->bounds(minVal, maxVal);
        p->cfg->max = maxVal;

        if (p->slider->value() > maxVal)
            p->slider->value(maxVal);

        p->slider->redraw();
    }, pack);

    y += 70;
}
    // Expand scrollable area to fit all sliders
    scroll->end();
    wsl->end();
    wsl->show();

    return 0;
}

int slidercfg_() {
Fl_Group::current(nullptr);	
static Fl_Window* wsl = new Fl_Window(win->x() + win->w() / 4, 0, win->w() / 2, win->h() , "Help");
			wsl->set_modal(); 
    int y = 10;
    for (auto &cfg : sliders) {
        auto *s = new Fl_Slider(20, y, 220, 30, cfg.label.c_str());
        s->type(FL_HOR_NICE_SLIDER);
        s->bounds(cfg.min, cfg.max);
        s->value(cfg.value);
        s->callback(slider_cb, &cfg);
        y += 60;
    }
wsl->end();
wsl->show(); 
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
		hints={"Part"};
			lua_str(filename,1);
			win->label(filename.c_str());
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
		lua_str(filename,1);
		return 1;
	}


	if(e==FL_UNFOCUS)SendEditor(SCI_AUTOCCANCEL);


	int ret= fl_scintilla::handle(e);
	if(e == FL_KEYDOWN){
		lua_str_realtime(getalltext());
	}
	return ret;
}


scint* editor;

void scint_init(int x,int y,int w,int h){ 
	editor = new scint(x,y,w,h);
} 

void gopart(const std::string& str) {
    if (str.empty()) return;

    const int docLen = editor->SendEditor(SCI_GETTEXTLENGTH);
    if (docLen <= 0) return;

    // Enable regex search
    editor->SendEditor(SCI_SETSEARCHFLAGS, SCFIND_REGEXP);

    // Search the entire document
    editor->SendEditor(SCI_SETTARGETSTART, 0);
    editor->SendEditor(SCI_SETTARGETEND, docLen);

    // Build regex: allow spaces/tabs, then Part, then optional space/paren, then your text
    std::string pattern = "^[ \\t]*Part[ \\t]*" + str;

    // Perform regex search
    const int pos = editor->SendEditor(
        SCI_SEARCHINTARGET,
        pattern.size(),
        (sptr_t)pattern.c_str()
    );

    if (pos == -1) return;  // no match

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

//region occv

struct OCC_Viewer : public flwindow {
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

	TopLoc_Location Origin;

	Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 0.2);
	Handle(Prs3d_LineAspect) edgeAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.5);
	Handle(Prs3d_LineAspect) highlightaspect = new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 5.0);
	Handle(Prs3d_Drawer) customDrawerp = new Prs3d_Drawer();

	bool show_fillets=0;

	struct luadraw; 

	OCC_Viewer(int X, int Y, int W, int H, const char* L = 0) : flwindow(X, Y, W, H, L) {
		
		Fl::add_timeout(10, idle_refresh_cb, 0);
	}
	static void idle_refresh_cb(void*) {
		// clear gpu usage each 10 secs
		glFlush();
		glFinish();
		Fl::repeat_timeout(10, idle_refresh_cb, 0);
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
			Handle(Prs3d_LineAspect) thickGreenAspect = new Prs3d_LineAspect(Quantity_NOC_GREEN, Aspect_TOL_SOLID, 5.0);

			// 4. ***Crucially, set the Line Aspect for the highlight presentation***
			//    This tells the Drawer what line properties to use when drawing the highlighted shape.
			hl->SetLineAspect(thickGreenAspect);

			// 5. Ensure the shape is highlighted in a *Wireframe* mode,
			//    as shaded highlighting (the default) might only show the face color.
			hl->SetDisplayMode(0);	// Display mode 0 typically refers to Wireframe/Edges

			// 6. Apply the style
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Dynamic, hl);
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_Selected, hl);
			ctx->SetHighlightStyle(Prs3d_TypeOfHighlight_LocalSelected, hl);

			// 7. Update the viewer
			ctx->UpdateCurrentViewer();
		}
	}

	void initialize_opencascade() {
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

		m_display_connection = new Aspect_DisplayConnection(display);
		Handle(Xw_Window) wind = new Xw_Window(m_display_connection, win);

#endif
		m_graphic_driver = new OpenGl_GraphicDriver(m_display_connection);

		m_viewer = new V3d_Viewer(m_graphic_driver);
		m_viewer->SetDefaultLights();
		m_viewer->SetLightOn();
		m_context = new AIS_InteractiveContext(m_viewer);
		ctx=m_context;
		m_view = m_viewer->CreateView();
		m_view->SetWindow(wind);
		InitializeCustomWireframeAspects(m_context);

		m_view->SetImmediateUpdate(Standard_False);




Handle(V3d_DirectionalLight) l1 = new V3d_DirectionalLight(V3d_XnegYnegZneg, Quantity_NOC_WHITE, Standard_True);
Handle(V3d_DirectionalLight) l2 = new V3d_DirectionalLight(V3d_XposYposZpos, Quantity_NOC_GRAY80, Standard_True);
m_view->Viewer()->AddLight(l1);
m_view->Viewer()->SetLightOn(l1);
m_view->Viewer()->AddLight(l2);
m_view->Viewer()->SetLightOn(l2);

l1->SetIntensity(9.5f);
l2->SetIntensity(9.5f);






		SetupHighlightLineType(m_context);

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

		// SetupHighlightLineType(m_context);

		m_view->ChangeRenderingParams().IsTransparentShadowEnabled = Standard_False;
		m_view->ChangeRenderingParams().ToEnableDepthPrepass = Standard_True;

		m_view->SetAutoZFitMode(Standard_True, 1);
		redraw();
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
			std::cout << "GL_VENDOR: " << (const char*)glGetString(GL_VENDOR) << "\n";
std::cout << "GL_RENDERER: " << (const char*)glGetString(GL_RENDERER) << "\n";
// std::cout << "GL_EXTENSIONS: " << (const char*)glGetString(GL_EXTENSIONS) << "\n";
// GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
// std::cout << "FBO status: " << status << std::endl;

		}

		// toggle_shaded_transp(currentMode);

		Fl::add_timeout(
			1.4,
			[](void* d) {
				auto l = (Fl_Help_View*)d;
				l->value("");
			},
			helpv);
		m_initialized = true;
	}
void lettherebelight(){
		auto view=m_view;
		auto viewer=view->Viewer();
	// Define these as class members or static handles so they persist
static Handle(V3d_SpotLight) mySpots[4];
static bool lightsInitialized = false;

try {
    if (!lightsInitialized) {
        for (int i = 0; i < 4; ++i) {
            mySpots[i] = new V3d_SpotLight(gp_Pnt(0,0,0), gp_Dir(0,0,1));
            mySpots[i]->SetIntensity(3000000.0);
            mySpots[i]->SetAngle(0.7);
            viewer->AddLight(mySpots[i]);
            viewer->SetLightOn(mySpots[i]);
        }
        lightsInitialized = true;
    }

    // Now, ONLY update the properties, don't "AddLight" again
    gp_Pnt eye = view->Camera()->Eye();
    gp_Pnt center = view->Camera()->Center();
    double offset = 100.0; 

       gp_Pnt targets[4] = {
        gp_Pnt(center.X() + offset, center.Y(), center.Z()),
        gp_Pnt(center.X() - offset, center.Y(), center.Z()),
        gp_Pnt(center.X(), center.Y() + offset, center.Z()),
        gp_Pnt(center.X(), center.Y() - offset, center.Z())
    };


    for (int i = 0; i < 4; ++i) {
        mySpots[i]->SetPosition(eye);
        mySpots[i]->SetDirection(gp_Dir(gp_Vec(eye, targets[i])));
    }

    // This tells the GPU the lights moved
    viewer->UpdateLights(); 
    // view->Redraw();

} catch (...) { /* ... */ }
}
void lettherebelight31() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        // Ensure PBR is active
        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // 1. Clear existing lights
        // 1. Clear existing lights
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights()) {
            lights.Append(viewer->ActiveLight());
        }
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));
        }

        // 2. Setup Ambient Light (The "Base" fill)
        // Keep it low to allow shadows and highlights to pop.
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_Color(0.2, 0.2, 0.25, Quantity_TOC_RGB));
        amb->SetIntensity(0.5f);
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);

        // 3. Directional "Key" Light (The "Sun")
        // Positioned slightly above and to the side for depth.
        gp_Dir sunDir(-1.0, -1.5, -1.0); 
        Handle(V3d_DirectionalLight) sunLight = new V3d_DirectionalLight(sunDir, Quantity_NOC_WHITE);
        sunLight->SetIntensity(22.5f);
        sunLight->SetCastShadows(Standard_True); // Critical for realism
        viewer->AddLight(sunLight);
        viewer->SetLightOn(sunLight);

        // 4. Soft "Fill" Light
        // Coming from the opposite side to soften shadows.
        gp_Dir fillDir(1.0, 0.5, 0.2);
        Handle(V3d_DirectionalLight) fillLight = new V3d_DirectionalLight(fillDir, Quantity_NOC_GRAY80);
        fillLight->SetIntensity(20.8f);
        viewer->AddLight(fillLight);
        viewer->SetLightOn(fillLight);

        // 5. High-End PBR Tweaks
        // This enables the "glossy" reflection of the environment.
        // view->SetTransitionFF(Standard_True); // Smooth transitions

        // Optional: Environment Background (The "Secret Sauce")
        // If you have a Cubemap/HDR file, loading it here makes PBR look 10x better.
        // view->SetBackgroundCubeMap(myCubeMap);

        viewer->UpdateLights();
        // view->Redraw();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

//region lettherebelight
void lettherebelight77() {
    auto view = m_view;
    auto viewer = view->Viewer();
try{
    if (viewer.IsNull() || view.IsNull()) return;

    // 1. Get Camera Vectors
    // This allows us to define "Up" and "Right" relative to your current screen
    gp_Dir eyeDir = view->Camera()->Direction(); // Direction camera is looking
    gp_Dir upDir  = view->Camera()->Up();        // Screen "Up"
    gp_Dir sideDir = eyeDir ^ upDir;   // Screen "Right" (Cross product)

    // 2. Define the Diagonal Direction
    // We want a vector that goes Right (+side) and Up (+up) 
    // We also add a bit of 'eyeDir' so it points 'into' the screen
    gp_XYZ diagXYZ = sideDir.XYZ() + upDir.XYZ() + (eyeDir.XYZ() * -1.0);
    gp_Dir finalDir(diagXYZ);

    // 3. Create the Directional Light
    Handle(V3d_DirectionalLight) aDiagLight = new V3d_DirectionalLight(finalDir);

    // 4. Set as Headlight
    // This is the "magic" - it locks this diagonal to your screen.
    // No matter how you rotate the model, the light stays 
    // pointing from bottom-left to top-right of your monitor.
    // aDiagLight->SetHeadlight(Standard_True);
    
    aDiagLight->SetIntensity(strcfg("Intensity"));
    aDiagLight->SetColor(Quantity_NOC_WHITE);

    // --- CLEANUP AND APPLY ---
    viewer->SetLightOff();
    
    // Add a soft ambient light to fill the "back" side so it's not pitch black
    Handle(V3d_AmbientLight) ambient = new V3d_AmbientLight(Quantity_NOC_WHITE);
    ambient->SetIntensity(0.2);
    
    viewer->AddLight(ambient);
    viewer->AddLight(aDiagLight);
    
    viewer->SetLightOn(ambient);
    viewer->SetLightOn(aDiagLight);

    viewer->UpdateLights();
}catch(...){}
    // view->Redraw();
}
void lettherebelight772() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        // Use Phong or Pbr - Phong is often smoother for standard CAD colors
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- 1. CLEAR EXISTING LIGHTS ---
        viewer->SetLightOff(); // Quick way to disable all
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

        // // --- 2. AMBIENT LIGHT (The "Gradient" Maker) ---
        // This ensures no face is ever pitch black.
        Handle(V3d_AmbientLight) ambient = new V3d_AmbientLight();
        ambient->SetColor(Quantity_NOC_WHITE);
        ambient->SetIntensity(0.3); // Low intensity just to fill shadows
        viewer->AddLight(ambient);
        viewer->SetLightOn(ambient);

        // // --- 3. MAIN DIRECTIONAL LIGHT (Fixed in Space) ---
        // // By setting Headlight to FALSE, the light stays put while you rotate the part.
        // Handle(V3d_DirectionalLight) mainLight = new V3d_DirectionalLight(gp_Dir(-1.0, -1.0, -1.0));
        // mainLight->SetColor(Quantity_NOC_WHITE);
        // mainLight->SetIntensity(1.0);
        // mainLight->SetHeadlight(Standard_False); // Crucial for "movement" feel
        // viewer->AddLight(mainLight);
        // viewer->SetLightOn(mainLight);

        // // --- 4. THE "HEADLIGHT" (Soft Fill) ---
        // // A softer light that follows the camera to highlight front-facing details.
        // Handle(V3d_DirectionalLight) headLight = new V3d_DirectionalLight(gp_Dir(0.0, 0.0, -1.0));
        // headLight->SetColor(Quantity_NOC_WHITE);
        // headLight->SetIntensity(0.5);
        // headLight->SetHeadlight(Standard_True); 
        // viewer->AddLight(headLight);
        // viewer->SetLightOn(headLight);

		// 1. Define the Direction (from 0,0,0 towards 1,1,0)
// 1. Define Position and Direction
gp_Pnt aOrigin(0.0, 0.0, 0.0);
gp_Dir aDirection(1.0, 1.0, 0.0); // Points from 0,0,0 to 1,1,0

// 2. Create the Spotlight
Handle(V3d_SpotLight) aSpot = new V3d_SpotLight(aOrigin, aDirection);

// 3. Make it "Thin" (Narrow the Beam)
// Angle is in radians. Standard_PI / 12.0 is 15 degrees. 
// Smaller = thinner beam.
aSpot->SetAngle(3.14 / strcfg("Angle")); 

// 4. Intensity and Focus
aSpot->SetIntensity(strcfg("Intensity"));
aSpot->SetConcentration(1.0); // Higher values make the beam sharper at the center

// 5. Vital: Keep it in the world, not on the camera
aSpot->SetHeadlight(0);

viewer->AddLight(aSpot);
viewer->SetLightOn(aSpot);

        viewer->UpdateLights();
        view->Redraw();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}
void lettherebelight771(){
	    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

// 1. Criar a luz direcional
// Direção: x=1.0 (direita), y=1.0 (cima), z=1.0 (frente ao objeto)
Handle(V3d_DirectionalLight) aLight = new V3d_DirectionalLight(gp_Dir(1.0, 1.0, 1.0));

// 2. Definir como "Headlight" (Fixa na câmera)
aLight->SetHeadlight(Standard_True);

// 3. Ajustar intensidade e cor (opcional)
aLight->SetIntensity(strcfg("Intensity"));
aLight->SetColor(Quantity_NOC_WHITE);

    viewer->AddLight(aLight);
    viewer->SetLightOn(aLight);


viewer->UpdateLights();


    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight7()
{
    auto view   = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // Clear all existing lights
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

        Handle(Graphic3d_Camera) cam = view->Camera();

        gp_Dir forward = cam->Direction();        // looking direction (from eye to center)
        gp_Dir up      = cam->Up();
        gp_Dir right   = forward ^ up;            // should be normalized already

        gp_Pnt eye     = cam->Eye();

        double strength   = strcfg("DiagonalStrength", 0.8, 3.0);
        double spotAngle  = strcfg("SpotAngle", 0.6, 1.2);        // larger cone (~35-70°)
        // double penumbra   = strcfg("SpotPenumbra", 0.15, 0.4);
        double intensity  = strcfg("Intensity", 800.0, 9000.0);   // much higher for PBR

        // ===================================================================
        // Diagonal direction in VIEWPORT space: bottom-left → top-right
        // ===================================================================
        // "Bottom-left" on screen ≈ -right - up  (in camera local coords)
        // "Top-right"  on screen ≈ +right + up
        gp_Vec diagVec = gp_Vec(right) * strength + gp_Vec(up) * strength;
        if (diagVec.SquareMagnitude() < gp::Resolution())
            diagVec = gp_Vec(right) + gp_Vec(up);   // fallback

        gp_Dir diagDir(diagVec);

        // Place light far behind the camera so the cone illuminates the whole scene
        gp_Pnt lightPos = eye.Translated( gp_Vec(forward) * (-50.0) );   // adjust distance if needed

        // Main diagonal spot light
        Handle(V3d_SpotLight) mainLight = new V3d_SpotLight(
            lightPos,
            diagDir,
            Quantity_NOC_WHITE   // slightly warm
            // Quantity_Color(1.0, 0.98, 0.92, Quantity_TOC_RGB)   // slightly warm
        );

        mainLight->SetIntensity(intensity);
        mainLight->SetAngle(spotAngle);
        // if (penumbra > 0.0)
        //     mainLight->SetPenumbraAngle(penumbra);   // may not exist in very old OCCT
        // mainLight->SetRange(0.0);                    // 0 = infinite range
        // mainLight->SetAttenuation(1.0, 0.0, 0.0); // optional

        viewer->AddLight(mainLight);
        viewer->SetLightOn(mainLight);

        // Optional softer fill from opposite diagonal (cooler tone)
        gp_Vec fillVec = gp_Vec(right) * (-0.6 * strength) + gp_Vec(up) * (0.8 * strength);
        if (fillVec.SquareMagnitude() > gp::Resolution())
        {
            Handle(V3d_SpotLight) fillLight = new V3d_SpotLight(
                lightPos,
                gp_Dir(fillVec),
                Quantity_Color(0.75, 0.82, 1.0, Quantity_TOC_RGB)
            );
            fillLight->SetIntensity(intensity * 0.45);
            fillLight->SetAngle(spotAngle * 1.3);
            // fillLight->SetRange(0.0);
            viewer->AddLight(fillLight);
            viewer->SetLightOn(fillLight);
        }

        // Strong directional light as main "sun" (often works better with PBR than spots)
        Handle(V3d_DirectionalLight) dirLight = new V3d_DirectionalLight(
            diagDir,
            Quantity_Color(1.0, 1.0, 0.95, Quantity_TOC_RGB)
        );
        dirLight->SetIntensity(intensity * 0.6);
        viewer->AddLight(dirLight);
        viewer->SetLightOn(dirLight);

        // Ambient (keep subtle)
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient", 0.15, 1.5));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);

        viewer->UpdateLights();
        // view->Redraw();   // force redraw

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight74()
{
    auto view   = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

        // --- VIEW METRICS ---
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH);

        double t = strcfg("Offset");
        double halfW = viewW * 0.5;
        double halfH = viewH * 0.5;

        double offsetW = t * halfW;
        double offsetH = t * halfH;

        double dynamicIntensity = (viewW * viewH) * strcfg("Intensity");

        gp_Pnt center = view->Camera()->Center();
        gp_Pnt eye    = view->Camera()->Eye();

        // ============================================================
        // 6‑LIGHT STUDIO RIG
        // ============================================================

        gp_Pnt targetTop    (center.X(), center.Y() + offsetH, center.Z());
        gp_Pnt targetBottom (center.X(), center.Y() - offsetH, center.Z());

        gp_Pnt targetRight  (center.X() + offsetW, center.Y(), center.Z());
        gp_Pnt targetLeft   (center.X() - offsetW, center.Y(), center.Z());

        gp_Pnt targetFront  (center.X(), center.Y(), center.Z() + offsetH);
        gp_Pnt targetBack   (center.X(), center.Y(), center.Z() - offsetH);

        // --- COLORS ---
        Quantity_Color colTop    (0.70, 0.72, 0.85, Quantity_TOC_RGB); // cool sky
        Quantity_Color colBottom (0.95, 0.85, 0.70, Quantity_TOC_RGB); // warm bounce

        Quantity_Color colRight  (0.80, 0.80, 0.90, Quantity_TOC_RGB);
        Quantity_Color colLeft   (0.80, 0.80, 0.90, Quantity_TOC_RGB);

        Quantity_Color colFront  (0.95, 0.95, 0.95, Quantity_TOC_RGB); // key
        Quantity_Color colBack   (0.75, 0.80, 0.95, Quantity_TOC_RGB); // rim

        // --- INTENSITIES ---
        double I_top    = dynamicIntensity * strcfg("I_top");
        double I_bottom = dynamicIntensity * strcfg("I_bottom");

        double I_right  = dynamicIntensity * strcfg("I_right");
        double I_left   = dynamicIntensity * strcfg("I_left");

        double I_front  = dynamicIntensity * strcfg("I_front");
        double I_back   = dynamicIntensity * strcfg("I_back");

        double angle = strcfg("Angle");
        double conc  = strcfg("Concentration");

        // --- LIGHT CREATION ---
        auto addSpot = [&](const gp_Pnt& tgt, const Quantity_Color& col, double intensity)
        {
            gp_Vec vec(eye, tgt);
            if (vec.SquareMagnitude() <= gp::Resolution()) return;

            Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), col);
            spot->SetIntensity(intensity);
            spot->SetAngle(angle);
            spot->SetConcentration(conc);
            spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        };

        // --- ADD ALL 6 LIGHTS ---
        addSpot(targetTop,    colTop,    I_top);
        addSpot(targetBottom, colBottom, I_bottom);

        addSpot(targetRight,  colRight,  I_right);
        addSpot(targetLeft,   colLeft,   I_left);

        addSpot(targetFront,  colFront,  I_front);
        addSpot(targetBack,   colBack,   I_back);

        // --- AMBIENT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient"));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);

        viewer->UpdateLights();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight61() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));


			// --- VIEW METRICS ---
Standard_Real viewW, viewH;
view->Size(viewW, viewH);

double t = strcfg("Offset");
double dynamicIntensity = (viewW * viewH) * strcfg("Intensity");

gp_Pnt center = view->Camera()->Center();
gp_Pnt eye    = view->Camera()->Eye();

// --- CAMERA BASIS ---
gp_Dir forward = view->Camera()->Direction();
gp_Dir up      = view->Camera()->Up();

gp_Dir right(forward.Crossed(up));
gp_Dir left = -right;

// Convert to vectors
gp_Vec vRight(right);
gp_Vec vLeft(left);
gp_Vec vUp(up);
gp_Vec vDown = -vUp;

// Offset distance based on view size
Standard_Real d = t * Max(viewW, viewH);

// Light origins
gp_Pnt eye_right  = eye.Translated(vRight * d);
gp_Pnt eye_left   = eye.Translated(vLeft  * d);
gp_Pnt eye_top    = eye.Translated(vUp    * d);
gp_Pnt eye_bottom = eye.Translated(vDown  * d);

// Light targets (center shifted in each direction)
gp_Pnt targetRight  = center.Translated(vRight * d);
gp_Pnt targetLeft   = center.Translated(vLeft  * d);
gp_Pnt targetTop    = center.Translated(vUp    * d);
gp_Pnt targetBottom = center.Translated(vDown  * d);

// Colors
Quantity_Color colTop    (0.68627, 0.68627, 0.77647, Quantity_TOC_RGB);
Quantity_Color colBottom (0.68627, 0.68627, 0.77647, Quantity_TOC_RGB);
Quantity_Color colRight  (0.68627, 0.68627, 0.77647, Quantity_TOC_RGB);
Quantity_Color colLeft   (0.68627, 0.68627, 0.77647, Quantity_TOC_RGB);

// Intensities
double I_top    = dynamicIntensity * strcfg("I_top");
double I_bottom = dynamicIntensity * strcfg("I_bottom");
double I_right  = dynamicIntensity * strcfg("I_side");
double I_left   = dynamicIntensity * strcfg("I_side");

double angle = strcfg("Angle");
double conc  = strcfg("Concentration");

// --- LIGHT CREATION ---
auto addSpot = [&](const gp_Pnt& origin, const gp_Pnt& tgt,
                   const Quantity_Color& col, double intensity)
{
    gp_Vec vec(origin, tgt);
    if (vec.SquareMagnitude() <= gp::Resolution()) return;

    Handle(V3d_SpotLight) spot = new V3d_SpotLight(origin, gp_Dir(vec), col);
    spot->SetIntensity(intensity);
    spot->SetAngle(angle);
    spot->SetConcentration(conc);
    spot->SetCastShadows(Standard_False);

    viewer->AddLight(spot);
    viewer->SetLightOn(spot);
};

// --- ADD 4 CAMERA-ALIGNED LIGHTS ---
addSpot(eye_top,    targetTop,    colTop,    I_top);
addSpot(eye_bottom, targetBottom, colBottom, I_bottom);
addSpot(eye_right,  targetRight,  colRight,  I_right);
addSpot(eye_left,   targetLeft,   colLeft,   I_left);

// --- AMBIENT ---
Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
amb->SetIntensity(strcfg("Ambient"));
viewer->AddLight(amb);
viewer->SetLightOn(amb);

viewer->UpdateLights();


    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}
// SIMPLE VERTICAL BEAM
void addVerticalBeam(const Handle(V3d_View)& view,
                     const Handle(V3d_Viewer)& viewer)
{
    gp_Pnt eye = view->Camera()->Eye();
    gp_Dir forward = view->Camera()->Direction();
    gp_Dir up = view->Camera()->Up();
    gp_Dir right = forward.Crossed(up);

    gp_Vec vForward(forward);
    gp_Vec vRight(right);
    gp_Vec vUp(up);

    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);

    // camera FOV (vertical)
    double fovY = view->Camera()->FOVy(); // radians

    // distance from camera
    double d = 300.0;

    // world units per pixel at depth d
    double worldPerPixel = (2.0 * d * tan(fovY * 0.5)) / viewH;

    // beam horizontal position in pixels
    double beamX_px = viewW * 0.8;

    // convert horizontal pixel to world offset
    double nx_px = beamX_px - (viewW * 0.5);
    gp_Vec baseOffset = vRight * (nx_px * worldPerPixel);

    int segments = 12;

    for (int i = 0; i < segments; ++i)
    {
        double y_px = (viewH * i) / (segments - 1);
        double ny_px = y_px - (viewH * 0.5);

        gp_Vec verticalOffset = vUp * (ny_px * worldPerPixel);

        gp_Pnt origin =
            eye.Translated(vForward * d)
               .Translated(baseOffset)
               .Translated(verticalOffset);

        Handle(V3d_SpotLight) spot =
            new V3d_SpotLight(origin, forward, Quantity_NOC_WHITE);

        spot->SetIntensity(300000000.0 / segments);
        spot->SetAngle(0.5);
        spot->SetConcentration(2.0);

        viewer->AddLight(spot);
        viewer->SetLightOn(spot);
    }
}
#include <vector>

void updateVerticalBeam(const Handle(V3d_View)& view,
                        const Handle(V3d_Viewer)& viewer)
{
    static std::vector<Handle(V3d_SpotLight)> beamLights;
    static bool initialized = false;
    
    // Initialize lights if not already done
    if (!initialized)
    {
        // Clear any existing lights by turning them off and removing from viewer
        for (auto& light : beamLights)
        {
            viewer->SetLightOff(light);
        }
        beamLights.clear();
        
        int segments = 20; // More segments for smoother beam
        
        // Temporary position for initialization (will be updated in each frame)
        gp_Pnt tempPos(0, 0, 0);
        gp_Dir tempDir(0, 0, 1);
        Quantity_Color tempColor(Quantity_NOC_WHITE);
        
        for (int i = 0; i < segments; ++i)
        {
            Handle(V3d_SpotLight) spot = new V3d_SpotLight(tempPos, tempDir, tempColor);
            spot->SetIntensity(500000000.0 / segments);
            spot->SetAngle(0.3); // Narrow beam angle
            spot->SetConcentration(3.0);
            
            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
            beamLights.push_back(spot);
        }
        
        initialized = true;
    }
    
    if (beamLights.empty()) return;
    
    gp_Pnt eye = view->Camera()->Eye();
    gp_Dir forwardDir = view->Camera()->Direction();
    gp_Dir upDir = view->Camera()->Up();
    gp_Dir rightDir = forwardDir.Crossed(upDir);
    
    // Convert to vectors for arithmetic operations
    gp_Vec forward(forwardDir);
    gp_Vec up(upDir);
    gp_Vec right(rightDir);
    
    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);
    
    // Calculate world units at a reasonable distance (adjust as needed)
    double distance = 500.0;
    
    // Get FOV to calculate world coordinates from pixel coordinates
    double fovY = view->Camera()->FOVy();
    double worldHeightAtDistance = 2.0 * distance * tan(fovY * 0.5);
    double worldPerPixelY = worldHeightAtDistance / viewH;
    
    // For a vertical beam at a fixed X position (e.g., 20% from left)
    double beamX_px = viewW * 0.2; // Adjust this value (0.0 to 1.0) for horizontal position
    
    // Calculate horizontal offset in world space
    double nx_px = beamX_px - (viewW * 0.5);
    double worldOffsetX = nx_px * worldPerPixelY * (viewW / viewH); // Account for aspect ratio
    
    // Position lights along the vertical axis
    int numLights = beamLights.size();
    for (int i = 0; i < numLights; ++i)
    {
        // Map from 0 to 1 along the vertical screen space
        double t = (double)i / (numLights - 1);
        
        // Convert to pixel Y coordinate (0 = top, viewH = bottom)
        double y_px = t * viewH;
        
        // Convert to offset from screen center
        double ny_px = y_px - (viewH * 0.5);
        
        // Convert to world space offset
        double worldOffsetY = ny_px * worldPerPixelY;
        
        // Calculate position in world space
        gp_Pnt lightPos = eye.Translated(forward * distance)
                              .Translated(right * worldOffsetX)
                              .Translated(up * worldOffsetY);
        
        // Update light position and direction
        beamLights[i]->SetPosition(lightPos);
        beamLights[i]->SetDirection(forwardDir);
    }
}
void addViewportLightsno(const Handle(V3d_View)& view,
                       const Handle(V3d_Viewer)& viewer)
{
    // --- CLEAR EXISTING LIGHTS ---
    TColStd_ListOfTransient lights;
    for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
        lights.Append(viewer->ActiveLight());
    for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
        viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);

    // ✔ Zoom factor (critical!)
    double zoom = view->Camera()->Scale();

    // ✔ Base distance (tweakable)
    double baseD = strcfg("Distance",1,99999);

    // ✔ Final distance that stays consistent with zoom
    double d = baseD * zoom;

    int cols = 6;
    int rows = 10;
	        // --- LIGHT COLOR and conversion to linear
        auto srgbToLinear = [](double c)->double {
            if (c <= 0.04045) return c / 12.92;
            return pow((c + 0.055) / 1.055, 2.4);
        };

        Quantity_Color srgb(0.54118,0.54118,0.61569, Quantity_TOC_RGB);
        Quantity_Color linear(
            srgbToLinear(srgb.Red()),
            srgbToLinear(srgb.Green()),
            srgbToLinear(srgb.Blue()),
            Quantity_TOC_RGB
        );

    for (int iy = 0; iy < rows; ++iy)
    {
        for (int ix = 0; ix < cols; ++ix)
        {
            int px = int((viewW * ix) / (cols - 1));
            int py = int((viewH * iy) / (rows - 1));

            Standard_Real x, y, z;
            Standard_Real vx, vy, vz;

            // Pixel → world point + ray direction
            view->ConvertWithProj(px, py, x, y, z, vx, vy, vz);

            gp_Pnt P(x, y, z);
            gp_Vec dir(vx, vy, vz);
            dir.Normalize();

            // ✔ Distance now scales with zoom
            gp_Pnt origin = P.Translated(dir );
            // gp_Pnt origin = P.Translated(dir * d);

            Handle(V3d_SpotLight) spot =
                new V3d_SpotLight(origin, gp_Dir(dir), linear);
                // new V3d_SpotLight(origin, gp_Dir(dir), Quantity_NOC_WHITE);

            spot->SetIntensity(90000000000.0 / (rows * cols));
            spot->SetAngle(3.0);
            spot->SetConcentration(3.0);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }
    }

    // Ambient
    Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
    amb->SetIntensity(strcfg("Ambient",0.1,4));
    viewer->AddLight(amb);
    viewer->SetLightOn(amb);

    viewer->UpdateLights();
}

void addViewportLightsneeds_to_add_spot_too(const Handle(V3d_View)& view,const Handle(V3d_Viewer)& viewer){
	// Clear existing lights
TColStd_ListOfTransient lights;
for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
    lights.Append(viewer->ActiveLight());
for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
    viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

// 1. KEY LIGHT: Primary bright light (Front-Top-Right)
Handle(V3d_DirectionalLight) keyLight = new V3d_DirectionalLight(V3d_XposYnegZpos, Quantity_NOC_WHITE);
keyLight->SetIntensity(strcfg("Intensity1",0.1,24.0)); // Adjust based on your scene scale
viewer->AddLight(keyLight);
viewer->SetLightOn(keyLight);

// 2. FILL LIGHT: Softer light to fill in shadows (Front-Left)
Handle(V3d_DirectionalLight) fillLight = new V3d_DirectionalLight(V3d_XnegYnegZpos, Quantity_NOC_WHITE);
fillLight->SetIntensity(strcfg("Intensity2",0.1,24.0)); 
viewer->AddLight(fillLight);
viewer->SetLightOn(fillLight);

// 3. BACK/RIM LIGHT: Adds a sharp edge to make shapes pop out of the background (Back-Top)
Handle(V3d_DirectionalLight) rimLight = new V3d_DirectionalLight(V3d_YposZpos, Quantity_NOC_WHITE);
rimLight->SetIntensity(strcfg("Intensity3",0.1,24.0)); 
viewer->AddLight(rimLight);
viewer->SetLightOn(rimLight);

// AMBIENT LIGHT: Keep this very low, just to prevent pitch-black shadows
Handle(V3d_AmbientLight) ambLight = new V3d_AmbientLight(Quantity_NOC_WHITE);
ambLight->SetIntensity(0.2);
viewer->AddLight(ambLight);
viewer->SetLightOn(ambLight);

viewer->UpdateLights();
}


void addViewportLightsbah(const Handle(V3d_View)& view,
                           const Handle(V3d_Viewer)& viewer)
{
    // --- CLEAR EXISTING LIGHTS ---
    TColStd_ListOfTransient lights;
    for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
        lights.Append(viewer->ActiveLight());
    for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
        viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

    // 1. Get view size in WORLD units (Used for scaling distance)
    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);

    // 2. Get window size in PIXELS (Used for screen projection)
    Standard_Integer winW, winH;
    view->Window()->Size(winW, winH);

    int cols = 6;
    int rows = 10;

    // --- ZOOM SCALING MATH ---
    // Make the distance dynamically scale with the view's current world size.
    // 1.5 is a multiplier to keep the lights behind the camera. Adjust if needed.
    double d = std::max(viewW, viewH) * 1.5; 

    // Scale intensity using the Inverse-Square Law based on your original d=300 baseline
    double baseIntensity =strcfg("Intensity",0.1, 900000000000.0) / (rows * cols);
    double distanceRatio = d / 300.0;
    double finalIntensity = baseIntensity * (distanceRatio * distanceRatio);

    for (int iy = 0; iy < rows; ++iy)
    {
        for (int ix = 0; ix < cols; ++ix)
        {
            // Calculate screen positions using WINDOW PIXELS, not world units
            int px = int((winW * ix) / (cols - 1));
            int py = int((winH * iy) / (rows - 1));

            Standard_Real x, y, z;
            Standard_Real vx, vy, vz;

            // Convert pixel → world point AND ray direction
            view->ConvertWithProj(px, py, x, y, z, vx, vy, vz);

            gp_Pnt P(x, y, z);
            gp_Vec dir(vx, vy, vz);
            dir.Normalize();

            // Distance 'd' now scales perfectly with your zoom level
            gp_Pnt origin = P.Translated(dir * d);

            Handle(V3d_SpotLight) spot =
                new V3d_SpotLight(origin, gp_Dir(dir), Quantity_NOC_WHITE);

            // Apply the dynamically scaled intensity
            spot->SetIntensity(finalIntensity);
            spot->SetAngle(strcfg("Angle",0.1,22.0));
            spot->SetConcentration(3.0);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }
    }

    // --- AMBIENT LIGHT ---
    Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
    // Assuming strcfg is a custom macro/function in your codebase
    amb->SetIntensity(strcfg("Ambient",0.1,4)); 
    viewer->AddLight(amb);
    viewer->SetLightOn(amb);
    viewer->UpdateLights();
}
void addViewportLightsnice(const Handle(V3d_View)& view,
                       const Handle(V3d_Viewer)& viewer)
{

	        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));
    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);

    int cols = 6;
    int rows = 10;

    for (int iy = 0; iy < rows; ++iy)
    {
        for (int ix = 0; ix < cols; ++ix)
        {
            int px = int((viewW * ix) / (cols - 1));
            int py = int((viewH * iy) / (rows - 1));

            // world point on view plane
            Standard_Real x, y, z;
            Standard_Real vx, vy, vz;

            // Convert pixel → world point AND ray direction
            view->ConvertWithProj(px, py, x, y, z, vx, vy, vz);

            gp_Pnt P(x, y, z);
            gp_Vec dir(vx, vy, vz);
            dir.Normalize();

            // choose distance along ray
            double d = 300.0;
            gp_Pnt origin = P.Translated(dir * d);

            Handle(V3d_SpotLight) spot =
                new V3d_SpotLight(origin, gp_Dir(dir), Quantity_NOC_WHITE);

            spot->SetIntensity(90000000000.0 / (rows * cols));
            spot->SetAngle(3.0);
            spot->SetConcentration(1.0);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }
    }

        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient",0.1,4));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);
		 viewer->UpdateLights();
}

void addViewportLights2(const Handle(V3d_View)& view,
                       const Handle(V3d_Viewer)& viewer)
{

	        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

    gp_Pnt eye      = view->Camera()->Eye();
gp_Dir forward = view->Camera()->Direction();
gp_Dir up      = view->Camera()->Up();

// Cross product gives gp_Vec, not gp_Dir
gp_Vec vRightVec = forward.Crossed(up);
vRightVec.Normalize();
gp_Dir right(vRightVec);

// Recompute up to ensure orthogonality
gp_Vec vUpVec = right.Crossed(forward);
vUpVec.Normalize();
gp_Dir up2(vUpVec);

// Convert to gp_Vec for offsets
gp_Vec vForward(forward);
gp_Vec vRight(right);
gp_Vec vUp(up2);


    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);

    double d    = 300.0;                 // depth from camera
    double fovY = view->Camera()->FOVy(); // radians

    double worldPerPixelY = (2.0 * d * tan(fovY * 0.5)) / viewH;
    double worldPerPixelX = worldPerPixelY * (viewW / viewH);

    int cols = 6;
    int rows = 10;

    for (int iy = 0; iy < rows; ++iy)
    {
        double y_px = (viewH * iy) / (rows - 1);
        double dy   = (y_px - viewH * 0.5) * worldPerPixelY;

        for (int ix = 0; ix < cols; ++ix)
        {
            double x_px = (viewW * ix) / (cols - 1);
            double dx   = (x_px - viewW * 0.5) * worldPerPixelX;

            gp_Vec offset = vRight * dx + vUp * dy;

            gp_Pnt origin =
                eye.Translated(vForward * d)
                   .Translated(offset);

            Handle(V3d_SpotLight) spot =
                new V3d_SpotLight(origin, forward, Quantity_NOC_WHITE);

            spot->SetIntensity(500000000.0 / (rows * cols));
            spot->SetAngle(3.0);
            spot->SetConcentration(1.0);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }
    }

        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient",0.1,4));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);
	 viewer->UpdateLights();
}

void addViewportLights1(const Handle(V3d_View)& view,
                       const Handle(V3d_Viewer)& viewer)
{
	        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

    gp_Pnt eye = view->Camera()->Eye();
    gp_Dir forward = view->Camera()->Direction();
    gp_Dir up = view->Camera()->Up();
    gp_Dir right = forward.Crossed(up);

    gp_Vec vForward(forward);
    gp_Vec vRight(right);
    gp_Vec vUp(up);

    Standard_Real viewW, viewH;
    view->Size(viewW, viewH);

    double d = 300.0;        // distance from camera
    int cols = 6;            // number of lights horizontally
    int rows = 10;           // number of lights vertically

    for (int iy = 0; iy < rows; ++iy)
    {
        double y_px = (viewH * iy) / (rows - 1);
        double ny = (y_px / viewH) - 0.5;

        for (int ix = 0; ix < cols; ++ix)
        {
            double x_px = (viewW * ix) / (cols - 1);
            double nx = (x_px / viewW) - 0.5;

            gp_Vec offset =
                vRight * (nx * d) +
                vUp    * (ny * d);

            gp_Pnt origin =
                eye.Translated(vForward * d)
                   .Translated(offset);

            Handle(V3d_SpotLight) spot =
                new V3d_SpotLight(origin, forward, Quantity_NOC_WHITE);

            spot->SetIntensity(50000000.0 / (rows * cols));
            spot->SetAngle(3.0);
            spot->SetConcentration(3.0);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }
    }

        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient",0.1,4));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);
	 viewer->UpdateLights();
}

void lettherebelight6() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

			{
        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient",0.1,4));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);}

        // --- VIEW METRICS ---
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH);

addVerticalBeam(view, viewer );          // narrow angle

        viewer->UpdateLights();
		return;



        double t = strcfg("Offset"); // normalized offset in [-1..1] expected
        double halfW = viewW * 0.5;
        double halfH = viewH * 0.5;

        // normalized offsets in screen space [-1..1]
        double offsetW_norm = t; // positive = right, negative = left
        double offsetH_norm = 0.0; // keep beam centered vertically; change if needed

        double dynamicIntensity = (viewW * viewH) * strcfg("Intensity",1,999);

        gp_Pnt center = view->Camera()->Center();
        gp_Pnt eye    = view->Camera()->Eye();

        // camera basis
        gp_Dir forward = view->Camera()->Direction(); // points from eye into scene
        gp_Dir up      = view->Camera()->Up();
        gp_Dir right(forward.Crossed(up));
        // normalize just in case
        // right.Normalize();
        // up.Normalize();
        // forward.Normalize();

        gp_Vec vForward(forward);
        gp_Vec vRight(right);
        gp_Vec vUp(up);

        // distance from eye where we place the light origin (near plane-ish)
        Standard_Real nearDist = strcfg("NearDist", 10.0, 10000.0); // tweakable via config
        // scale factor to convert normalized screen offset to world offset at nearDist
        // a simple heuristic: worldOffset = normalizedOffset * nearDist * scaleFactor
        // scaleFactor controls how many world units correspond to full-screen offset
        Standard_Real scaleFactor = strcfg("ScreenToWorld", 0.5, 10.0);

        gp_Vec worldOffset = vRight * (offsetW_norm * nearDist * scaleFactor)
                           + vUp    * (offsetH_norm * nearDist * scaleFactor);

        // --- LIGHT COLOR and conversion to linear
        auto srgbToLinear = [](double c)->double {
            if (c <= 0.04045) return c / 12.92;
            return pow((c + 0.055) / 1.055, 2.4);
        };

        Quantity_Color srgb(0.54118,0.54118,0.61569, Quantity_TOC_RGB);
        Quantity_Color linear(
            srgbToLinear(srgb.Red()),
            srgbToLinear(srgb.Green()),
            srgbToLinear(srgb.Blue()),
            Quantity_TOC_RGB
        );

        // --- INTENSITIES FROM SLIDERS ---
        double I_core = dynamicIntensity * strcfg("I_core",1,999);
        double I_glow = dynamicIntensity * strcfg("I_glow",0.2,999);

        double angle_core = strcfg("Angle",1,10) * (M_PI/180.0); // degrees -> radians if needed
        double conc_core  = strcfg("Concentration",0,2);

        // Helper to add a camera‑relative spotlight centered at a screen position
        auto addCameraSpot = [&](double offsetX_norm, double offsetY_norm,
                                 const Quantity_Color& col, double intensity,
                                 double angleDeg, double concentration)
        {
            // compute origin near the camera and offset in camera space
            gp_Vec ofs = vRight * (offsetX_norm * nearDist * scaleFactor)
                       + vUp    * (offsetY_norm * nearDist * scaleFactor);

            gp_Pnt origin = eye.Translated(vForward * nearDist).Translated(ofs);

            // direction is camera forward (beam projects into scene)
            gp_Dir dir(forward);

            // create spot
            Handle(V3d_SpotLight) spot = new V3d_SpotLight(origin, dir, col);
            spot->SetIntensity(intensity);
            spot->SetAngle(angleDeg);
            spot->SetConcentration(concentration);
            // spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        };

        // --- ADD CORE NARROW BEAM centered at normalized offset t horizontally ---
        // addCameraSpot(offsetW_norm, offsetH_norm, linear, I_core, angle_core, conc_core);
// Beam parameters
double beamX = strcfg("BeamX",  -1.0, 1.0);   // horizontal position in screen space
double beamHeight = 1.0;                          // full vertical span
int segments = 6;                                  // number of vertical lights

for (int i = 0; i < segments; ++i)
{
    double yNorm = -beamHeight * 0.5 + beamHeight * (double(i) / (segments - 1));

    gp_Vec ofs = vRight * (beamX * nearDist * scaleFactor)
               + vUp    * (yNorm * nearDist * scaleFactor);

    gp_Pnt origin = eye.Translated(vForward * nearDist).Translated(ofs);

    Handle(V3d_SpotLight) spot = new V3d_SpotLight(origin, forward, linear);
    spot->SetIntensity(I_core / segments);
    spot->SetAngle(angle_core);          // very small angle
    spot->SetConcentration(conc_core);   // high concentration

    viewer->AddLight(spot);
    viewer->SetLightOn(spot);
}

        // --- OPTIONAL: add a wider, softer glow behind the core for visual richness ---
        // addCameraSpot(offsetW_norm, offsetH_norm, linear, I_glow, angle_core * 3.0, conc_core * 0.2);

        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient",0.1,4));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);

        viewer->UpdateLights();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}


void lettherebelight611() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

        // --- VIEW METRICS ---
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH);

        double t = strcfg("Offset");
        double halfW = viewW * 0.5;
        double halfH = viewH * 0.5;

        double offsetW = t * halfW;
        double offsetH = t * halfH;

        double dynamicIntensity = (viewW * viewH) * strcfg("Intensity",1,999);

        gp_Pnt center = view->Camera()->Center();
        gp_Pnt eye    = view->Camera()->Eye();


gp_Dir forward = view->Camera()->Direction();
gp_Dir up      = view->Camera()->Up();

// Build perpendicular basis
gp_Dir right(forward.Crossed(up));
gp_Dir left = -right;

// Convert to vectors (gp_Dir cannot be multiplied by scalars)
gp_Vec vRight(right);
gp_Vec vLeft(left);
gp_Vec vUp(up);
gp_Vec vDown = -vUp;

// Offset distance
Standard_Real d = 1000.0;

// Compute points
gp_Pnt eye_right  = eye.Translated(vRight * d);
gp_Pnt eye_left   = eye.Translated(vLeft  * d);
gp_Pnt eye_top    = eye.Translated(vUp    * d);
gp_Pnt eye_bottom = eye.Translated(vDown  * d);



        // ============================================================
        // 3-LIGHT RIG (REALISTIC):
        //  - TOP LIGHT: coming from +Y (sky)
        //  - SIDE RIGHT: coming from +X
        //  - SIDE LEFT: coming from -X
        // ============================================================

        gp_Pnt targetTop    (center.X(), center.Y() + offsetH, center.Z());
        gp_Pnt targetRight  (center.X() + offsetW, center.Y(), center.Z());
        gp_Pnt targetLeft   (center.X() - offsetW, center.Y(), center.Z());

        // --- COLORS (sky / neutral / warm) ---
        Quantity_Color colTop   (Quantity_Color(0.68627, 0.68627, 0.77647, Quantity_TOC_RGB));
        Quantity_Color colRight (Quantity_Color(0.68627, 0.68627, 0.77647, Quantity_TOC_RGB));
        Quantity_Color colLeft  (Quantity_Color(0.68627, 0.68627, 0.77647, Quantity_TOC_RGB));
        // Quantity_Color colTop   (Quantity_NOC_GOLD);
        // Quantity_Color colRight (Quantity_NOC_WHITE);
        // Quantity_Color colLeft  (Quantity_NOC_RED);
			auto srgbToLinear = [](double c)->double {
    if (c <= 0.04045) return c / 12.92;
    return pow((c + 0.055) / 1.055, 2.4);
};

Quantity_Color srgb(0.54118,0.54118,0.61569, Quantity_TOC_RGB);
Quantity_Color linear(
    srgbToLinear(srgb.Red()),
    srgbToLinear(srgb.Green()),
    srgbToLinear(srgb.Blue()),
    Quantity_TOC_RGB
);

        // --- INTENSITIES FROM SLIDERS ---
        double I_top    = dynamicIntensity * strcfg("I_top",1,999);
        double I_right  = dynamicIntensity * strcfg("I_side",1,999);
        double I_left   = dynamicIntensity * strcfg("I_bottom",1,999);

        double angle = strcfg("Angle",1,10);
        double conc  = strcfg("Concentration",0,2);

        // --- CREATE LIGHT FUNCTION ---
        auto addSpot = [&](const gp_Pnt& tgt, const Quantity_Color& col, double intensity)
        { 
            gp_Vec vec(eye_right, tgt);
            // gp_Vec vec(eye, tgt);
            if (vec.SquareMagnitude() <= gp::Resolution()) return;

            Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye_right, gp_Dir(vec), col);
            // Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), col);
            spot->SetIntensity(intensity);
            spot->SetAngle(angle);
            spot->SetConcentration(conc);
            // spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        };

        // --- ADD 3 GRADIENT LIGHTS ---
        addSpot(targetTop,   linear,   I_top);
        addSpot(targetRight, linear, I_right);
        addSpot(targetLeft,  linear,  I_left);
        // addSpot(targetTop,   colTop,   I_top);
        // addSpot(targetRight, colRight, I_right);
        // addSpot(targetLeft,  colLeft,  I_left);

        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient",0.1,4));
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);

        viewer->UpdateLights();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight5() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);

        // --- CLEAR EXISTING LIGHTS ---
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights())
            lights.Append(viewer->ActiveLight());
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next())
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));

        // --- VIEW METRICS ---
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH);

        double t = strcfg("Offset");
        double halfW = viewW * 0.5;
        double halfH = viewH * 0.5;

        double offsetW = t * halfW;
        double offsetH = t * halfH;

        double dynamicIntensity = (viewW * viewH) * strcfg("Intensity");

        gp_Pnt center = view->Camera()->Center(); 
        gp_Pnt eye    = view->Camera()->Eye();



        // --- 3-LIGHT GRADIENT RIG TARGETS ---
        gp_Pnt targetTop    (center.X(), center.Y() + offsetH, center.Z());
        gp_Pnt targetSide   (center.X() + offsetW, center.Y(), center.Z());
        gp_Pnt targetBottom (center.X(), center.Y() - offsetH, center.Z());

        // --- COLORS (sky / neutral / warm) ---
        Quantity_Color colTop   (Quantity_NOC_GOLD);
        Quantity_Color colSide  (Quantity_NOC_WHITE);
        Quantity_Color colBottom(Quantity_NOC_RED);

        // --- INTENSITIES FROM SLIDERS ---
        double I_top    = dynamicIntensity * strcfg("I_top");   // slider 6
        double I_side   = dynamicIntensity * strcfg("I_side");   // slider 7
        double I_bottom = dynamicIntensity * strcfg("I_bottom");   // slider 8

        double angle = strcfg("Angle");;
        double conc  = strcfg("Concentration");;

        // --- CREATE LIGHT FUNCTION ---
        auto addSpot = [&](const gp_Pnt& tgt, const Quantity_Color& col, double intensity)
        {
            gp_Vec vec(eye, tgt);
            if (vec.SquareMagnitude() <= gp::Resolution()) return;

            Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), col);
            spot->SetIntensity(intensity);
            spot->SetAngle(angle);
            spot->SetConcentration(conc);
            spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        };

        // --- ADD 3 GRADIENT LIGHTS ---
        addSpot(targetTop,    colTop,    I_top);
        addSpot(targetSide,   colSide,   I_side);
        addSpot(targetBottom, colBottom, I_bottom);

        // --- AMBIENT LIGHT ---
        Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
        amb->SetIntensity(strcfg("Ambient")); // ambient slider
        viewer->AddLight(amb);
        viewer->SetLightOn(amb);

        viewer->UpdateLights();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight4() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

    viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        // 1. Clear existing lights
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights()) {
            lights.Append(viewer->ActiveLight());
        }
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));
        }

        // 2. Calculate View Metrics
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH); 
        
        // // Offset: 30% of the view width/height to spread them out
        // double offsetW = viewW * 0.3;
        // double offsetH = viewH * 0.3; 
// Slider value in [0.01, 0.99]
double t = sliders[3].value;

// Convert to normalized offset: 0 → center, 1 → corners
// t = 0.01 → 0.01 of half-view
// t = 0.99 → 0.99 of half-view
double nx = t;
double ny = t;

// Half extents of the view in world units
double halfW = viewW * 0.5;
double halfH = viewH * 0.5;

// Final offsets
double offsetW = nx * halfW;
double offsetH = ny * halfH;

// Intensity scaling
double viewArea = viewW * viewH;
double dynamicIntensity = viewArea * sliders[1].value;

gp_Pnt center = view->Camera()->Center();

        gp_Pnt eye = view->Camera()->Eye();
// Targets spread to corners
gp_Pnt targets[4] = {
    gp_Pnt(center.X() + offsetW, center.Y() + offsetH, center.Z()),
    gp_Pnt(center.X() - offsetW, center.Y() + offsetH, center.Z()),
    gp_Pnt(center.X() - offsetW, center.Y() - offsetH, center.Z()),
    gp_Pnt(center.X() + offsetW, center.Y() - offsetH, center.Z())
};

vector<Quantity_NameOfColor> vcolors={Quantity_NOC_BLUE,Quantity_NOC_GRAY90,Quantity_NOC_WHITE,Quantity_NOC_GRAY50};
        for (int i = 0; i < 4; ++i) {
            gp_Vec vec(eye, targets[i]);
            if (vec.SquareMagnitude() <= gp::Resolution()) continue;

            // Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), Quantity_NOC_BLUE);
            // Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), Quantity_NOC_GRAY90);
            Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), vcolors[i]);
            // Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), Quantity_NOC_WHITE);

            // --- DISPERSION SETTINGS ---
            spot->SetIntensity(dynamicIntensity);
            
            // Set Angle very wide (PI/2 is 90 degrees, total cone 180)
            // 2.5 radians provides a very wide, dispersed beam.
            // spot->SetAngle(1.1); 
            spot->SetAngle(sliders[0].value); 
			// cotm(sliders[0].value);
            // spot->SetAngle(2.5); 
            
            // Concentration 0.0 means the light doesn't "fade" toward the edges of the cone
            spot->SetConcentration(sliders[2].value); 

            spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }

    // Luz ambiente
    Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
    amb->SetIntensity(sliders[6].value);
    // amb->SetIntensity(0.4f);
    viewer->AddLight(amb);
    viewer->SetLightOn(amb);
        viewer->UpdateLights(); 

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight3() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

    viewer->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
        // 1. Clear existing lights
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights()) {
            lights.Append(viewer->ActiveLight());
        }
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));
        }

        // 2. Calculate View Metrics
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH); 
        
        // // Offset: 30% of the view width/height to spread them out
        // double offsetW = viewW * 0.3;
        // double offsetH = viewH * 0.3; 
        double offsetW = viewW * sliders[3].value;
        double offsetH = viewH * sliders[3].value;

        // Intensity: Scale based on the area of the view
        // We use a base multiplier. Adjust 5000.0 if it's too bright/dim.
        double viewArea = viewW * viewH;
        double dynamicIntensity = viewArea * sliders[1].value;
        // double dynamicIntensity = viewArea * 19.0;

        gp_Pnt eye = view->Camera()->Eye();
        gp_Pnt center = view->Camera()->Center();

        // 3. Targets: Pushing them further apart for dispersion
        gp_Pnt targets[4] = {
            gp_Pnt(center.X() + offsetW, center.Y() + offsetH, center.Z()),
            gp_Pnt(center.X() - offsetW, center.Y() + offsetH, center.Z()),
            gp_Pnt(center.X() - offsetW, center.Y() - offsetH, center.Z()),
            gp_Pnt(center.X() + offsetW, center.Y() - offsetH, center.Z())
        };

        for (int i = 0; i < 4; ++i) {
            gp_Vec vec(eye, targets[i]);
            if (vec.SquareMagnitude() <= gp::Resolution()) continue;

            Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), Quantity_NOC_GRAY90);
            // Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, gp_Dir(vec), Quantity_NOC_WHITE);

            // --- DISPERSION SETTINGS ---
            spot->SetIntensity(dynamicIntensity);
            
            // Set Angle very wide (PI/2 is 90 degrees, total cone 180)
            // 2.5 radians provides a very wide, dispersed beam.
            // spot->SetAngle(1.1); 
            spot->SetAngle(sliders[0].value); 
			// cotm(sliders[0].value);
            // spot->SetAngle(2.5); 
            
            // Concentration 0.0 means the light doesn't "fade" toward the edges of the cone
            spot->SetConcentration(sliders[2].value); 

            spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }

    // Luz ambiente
    Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
    amb->SetIntensity(sliders[6].value);
	cotm(sliders[6].value);
    // amb->SetIntensity(0.4f);
    viewer->AddLight(amb);
    viewer->SetLightOn(amb);
        viewer->UpdateLights(); 

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}

void lettherebelight2() {
    auto view = m_view;
    auto viewer = view->Viewer();

    try {
        if (viewer.IsNull() || view.IsNull()) return;

        // 1. Clean up existing lights safely
        TColStd_ListOfTransient lights;
        for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights()) {
            lights.Append(viewer->ActiveLight());
        }
        for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
            viewer->DelLight(Handle(V3d_Light)::DownCast(it.Value()));
        }

        // 2. DETECT VIEW SCALE / DIMENSIONS
        Standard_Real viewW, viewH;
        view->Size(viewW, viewH); // Returns width/height in world units

        // We use 20% of the smaller dimension as our offset
        // This ensures lights are always visible within the current frame
        double offset = (viewW < viewH ? viewW : viewH) * 0.2;

        gp_Pnt eye = view->Camera()->Eye();
        gp_Pnt center = view->Camera()->Center();

        // 3. Define 4 distinct targets based on dynamic offset
        gp_Pnt targets[4] = {
            gp_Pnt(center.X() + offset, center.Y(), center.Z()),
            gp_Pnt(center.X() - offset, center.Y(), center.Z()),
            gp_Pnt(center.X(), center.Y() + offset, center.Z()),
            gp_Pnt(center.X(), center.Y() - offset, center.Z())
        };

        for (int i = 0; i < 4; ++i) {
            gp_Vec vec(eye, targets[i]);
            if (vec.SquareMagnitude() <= gp::Resolution()) continue;

            gp_Dir dir(vec);
            Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, dir, Quantity_NOC_WHITE);

            // Settings
            spot->SetIntensity(300000000.0);
            spot->SetAngle(0.2); 
            spot->SetCastShadows(Standard_False);

            viewer->AddLight(spot);
            viewer->SetLightOn(spot);
        }

        viewer->UpdateLights();
        // view->Redraw();

    } catch (const Standard_Failure& e) {
        std::cout << "OCCT Exception: " << e.GetMessageString() << std::endl;
    }
}
void lettherebelight1(){
// 	#include <iostream>
// #include <Standard_Failure.hxx> // Required for OCCT exceptions

		auto view=m_view;
		auto viewer=view->Viewer();
try {
    // 1. Safety Check: Are the pointers valid?
    if (viewer.IsNull() || view.IsNull()) {
        std::cout << "Error: Viewer or View handle is null!" << std::endl;
        return;
    }

    // viewer->SetLightOff();
	// for (V3d_ListOfLightIterator itL(viewer->DefinedLightIterator()); itL.More(); itL.Next()) {
    //     viewer->SetLightOff(itL.Value()); 
    // }

	// Desliga todas as luzes existentes
TColStd_ListOfTransient lights;

viewer->InitActiveLights();
while (viewer->MoreActiveLights()) {
    lights.Append(viewer->ActiveLight());
    viewer->NextActiveLights();
}

for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
    Handle(V3d_Light) L = Handle(V3d_Light)::DownCast(it.Value());
    viewer->DelLight(L);
}

    gp_Pnt eye = view->Camera()->Eye();
    gp_Pnt center = view->Camera()->Center();
    double offset = 100.0; 

    // Define 4 distinct targets
    gp_Pnt targets[4] = {
        gp_Pnt(center.X() + offset, center.Y(), center.Z()),
        gp_Pnt(center.X() - offset, center.Y(), center.Z()),
        gp_Pnt(center.X(), center.Y() + offset, center.Z()),
        gp_Pnt(center.X(), center.Y() - offset, center.Z())
    };

    for (int i = 0; i < 4; ++i) {
        gp_Vec vec(eye, targets[i]);
        
        // 2. Safety Check: Direction cannot be zero length
        if (vec.SquareMagnitude() <= gp::Resolution()) {
            std::cout << "Warning: Light " << i << " has zero-length direction vector. Skipping." << std::endl;
            continue;
        }

        gp_Dir dir(vec);
        Handle(V3d_SpotLight) spot = new V3d_SpotLight(eye, dir, Quantity_NOC_WHITE);

        spot->SetIntensity(300000000.0);
        spot->SetAngle(1.7); // ~40 degrees
        spot->SetCastShadows(Standard_False);

        viewer->AddLight(spot);
        viewer->SetLightOn(spot);
    }

    viewer->UpdateLights();
    // view->MustBeResized(); // Often helps refresh the buffer
    // view->Redraw();

    std::cout << "Successfully added 4 spotlights." << std::endl;

} catch (const Standard_Failure& e) {
    // This catches OpenCascade specific errors
    std::cout << "OpenCascade Exception: " << e.GetMessageString() << std::endl;
} catch (const std::exception& e) {
    // This catches standard C++ errors (like bad_alloc)
    std::cout << "Standard Exception: " << e.what() << std::endl;
} catch (...) {
    std::cout << "An unknown, non-standard error occurred." << std::endl;
}
}
	void draw() {
		if (!m_initialized) {
			glViewport(0, 0, w(), h());
			initialize_opencascade();
			addViewportLights(m_view,m_viewer);
		}
		if (!m_initialized) return;
		// addViewportLights
		// lettherebelight6();
		// updateVerticalBeam(m_view,m_viewer);
		addViewportLights(m_view,m_viewer);
		{ 
		// auto view=m_view;
		// auto viewer=view->Viewer();
	// Remove all default lights
// 	try{
// viewer->SetLightOff();

// gp_Pnt eye    = view->Camera()->Eye();
// gp_Pnt center = view->Camera()->Center();
// gp_Dir dir(center.XYZ() - eye.XYZ());

// static Handle(V3d_SpotLight) spot =
//     new V3d_SpotLight(eye, dir, Quantity_NOC_WHITE);

// spot->SetIntensity(3000000.0);
// // spot->SetIntensity(3000000000.0);
// spot->SetAngle(2.12);          // 70 degrees
// // spot->SetConcentration(100.0);
// // spot->SetAttenuation(1.0, 0.0);
// spot->SetCastShadows(Standard_False);
// // spot->SetHeadlight(Standard_True);


//         spot->SetPosition(eye);
//         spot->SetDirection(dir);

// viewer->AddLight(spot);
// viewer->SetLightOn(spot);
// cotm(999);
//         // viewer->UpdateLights();
// 	}catch(...){ }
// }
// try {

// 		auto view=m_view;
// 		auto viewer=view->Viewer();
//     // 1. Clear existing lights if you want a clean slate
//     viewer->SetLightOff(); 

//     // 2. Define the source (Eye)
//     gp_Pnt eye = view->Camera()->Eye();
    
//     // 3. Define 4 different targets (e.g., corners of a box around the center)
//     // You can adjust these offsets to point the lights where you need.
//     gp_Pnt center = view->Camera()->Center();
//     double offset = 50.0; // Adjust based on your model scale

//     gp_Pnt targets[4] = {
//         gp_Pnt(center.X() + offset, center.Y() + offset, center.Z()),
//         gp_Pnt(center.X() - offset, center.Y() + offset, center.Z()),
//         gp_Pnt(center.X() - offset, center.Y() - offset, center.Z()),
//         gp_Pnt(center.X() + offset, center.Y() - offset, center.Z())
//     };

//     // 4. Create and configure the array of 4 lights
//     Handle(V3d_SpotLight) mySpots[4];

//     for (int i = 0; i < 4; ++i) {
//         // Calculate direction from eye to the specific target
//         gp_Dir dir(gp_Vec(eye, targets[i]));

//         mySpots[i] = new V3d_SpotLight(eye, dir, Quantity_NOC_WHITE);
        
//         // Settings
//         mySpots[i]->SetIntensity(30000000.0); 
//         mySpots[i]->SetAngle(1.5); // Narrower angle (radians) for better "spot" effect
//         mySpots[i]->SetCastShadows(Standard_False);
        
//         // Add to viewer
//         viewer->AddLight(mySpots[i]);
//         viewer->SetLightOn(mySpots[i]);
//     }
// cotm(99999);
//     // 5. Update the visual state
//     // viewer->UpdateLights();
//     // view->Redraw();

// } catch (...) {
//     // Handle potential OCCT exceptions
// }
		}
    if (0 && !sun.IsNull())
    { 
        // posição da câmara
        gp_Pnt eye = m_view->Camera()->Eye();
		// eye.Reverse();

        // direção da câmara
        gp_Dir look = m_view->Camera()->Direction();
		// look.Reverse();
        // spotlight presa à vista
        sun->SetPosition(eye);
        sun->SetDirection(look);

        // obrigatório para PBR
        m_viewer->UpdateLights();

		std::cout << "Look: " 
          << look.X() << " " 
          << look.Y() << " " 
          << look.Z() << std::endl;
std::cout << "Range: " << sun->Range() << std::endl;
std::cout << "Lights: " << m_viewer->ActiveLights().Extent() << std::endl;

    }
		m_view->Redraw();

    
// if (!sun.IsNull())
// {
// gp_Dir worldDir = m_view->Camera()->Direction();
// worldDir.Reverse();
// sun->SetDirection(worldDir);
// sun->SetHeadlight(Standard_True);
// m_view->Viewer()->SetLightOn(sun);
// }

		return;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthRange(0.0, 1.0);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-3, -3);
		m_view->Redraw();

		glDisable(GL_POLYGON_OFFSET_LINE);
	}

	void resize(int X, int Y, int W, int H) override {
		static bool in_resize = 0;
		if (in_resize) return;	// Prevent recursion
		in_resize = true;

		Fl_Window::resize(X, Y, W, window()->h() - 22 - 25);
		if (m_initialized) {
			m_view->MustBeResized();
			setbar5per();
		}
		in_resize = false;
	}

	int handle(int event) override {
		if (!m_initialized) return Fl_Window::handle(event);
		static auto last_event = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event);

		constexpr auto frame_interval = std::chrono::milliseconds(1000 / 30);
		// constexpr auto frame_interval = std::chrono::milliseconds(((int)(1000 / 8.0)));

		if (event == FL_DRAG && isRotating && m_initialized) { 
			if (elapsed > frame_interval) { 
				m_view->Rotation(Fl::event_x(), Fl::event_y());
				projectAndDisplayWithHLR(vshapes, 1); 
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
			rotateShapeByMouse(ulua[help.pname]->ashape, m_context, pickcenterforstudyrotation,
													 pickcenteraxisDir, Fl::event_x(), Fl::event_y());
			study_trg = ulua[help.pname]->ashape;
		}

		if (event == FL_PUSH && (Fl::event_state() & FL_CTRL)) {
			if (help.pname != "") {
				cotm(help.pname) 
				gopart(help.pname);
				editor->take_focus();
				// ld->occv->FitViewToShape(ld->occv->m_view, ld->shape);
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
			ev_highlight();

			// return 1;
		}

		if (event == FL_PUSH) {
			// cotm("")
			OnMouseClick(mousex, mousey, m_context, m_view);
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
							m_view->Rotate(0, 0,
										   angle);	// Rotate around Z-axis
							// perf("vnormal");
							// projectAndDisplayWithHLR(vshapes); 
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
				if (isPanning && m_initialized && Fl::event_button() == FL_RIGHT_MOUSE) {
					int dx = Fl::event_x() - lastX;
					int dy = Fl::event_y() - lastY; 
					m_view->Pan(dx, -dy);
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

					redraw();

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
	int mousex = 0;
	int mousey = 0;

	void ev_highlight() {
		if (!m_initialized) return; 

		static Handle(AIS_Shape) highlight;

		if (!highlight.IsNull()) m_context->Remove(highlight, 1);

		clearHighlight();

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
		m_context->MoveTo(mousex, mousey, m_view, Standard_False);
		m_context->SelectDetected(AIS_SelectionScheme_Replace);

		Handle(SelectMgr_EntityOwner) anOwner = m_context->DetectedOwner();
		if (anOwner.IsNull()) {
			clearHighlight();
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			redraw();
			return;
		}

		
		// Continue with your existing code using `anOwner`

		luadraw* ldd = lua_detected(anOwner);
		if (ldd) {
			std::string pname = ldd->name;
			// if (pname != help.pname) //to optimize speed
			{
				help.pname = pname;
				GProp_GProps systemProps;
				// Density is not directly handled here — mass is proportional to volume.
				// If you want real mass, multiply by material density later.
				BRepGProp::VolumeProperties(ldd->shape, systemProps);

				// Get the mass (volume if density=1)
				Standard_Real mass = systemProps.Mass();
				double massa = mass / 1000;
				help.mass = " Mass:" + fmt(massa, 0) + "cm³" + " Petg50%:~" + fmt(massa * 1.27 * 0.75, 0) + "g" + " Steel:~" + fmt(massa * 0.007850, 2) + "kg";
				// std::cout << "Mass (assuming density=1): " << mass << std::endl;
				help.upd();
			}
		} else {
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			return;
		}

		Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast(anOwner);
		if (brepOwner.IsNull()) {
			clearHighlight();
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			redraw();
			return;
		}

		TopoDS_Shape detected = brepOwner->Shape();
		if (detected.IsNull()) {
			clearHighlight();
			if (m_context->IsDisplayed(visible_)) visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
			redraw();
			return;
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
					redraw(); 
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
					redraw();
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
				clearHighlight();
				break;
		}
		redraw();
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
	void highlightVertex(const TopoDS_Vertex& aVertex, luadraw* ldd = 0) {
		clearHighlight();  // Clear any existing highlight first
 
		float ratio = GetViewportAspectRatio()[0]; 

		gp_Pnt vertexPntL = BRep_Tool::Pnt(aVertex);
		gp_Pnt vertexPnt = vertexPntL;
		if (ldd) vertexPnt = vertexPntL.Transformed(ldd->Origin);

		// check here
		//  if(!IsVertexVisible(vertexPnt,m_view,m_context))return;

		// Create a small red sphere at the vertex location
		Standard_Real sphereRadius = 5 * ratio;	 // Small radius for the highlight ball
		TopoDS_Shape sphereShape = BRepPrimAPI_MakeSphere(vertexPnt, sphereRadius).Shape();
		myHighlightedPointAIS = new AIS_Shape(sphereShape);
		myHighlightedPointAIS->SetColor(Quantity_NOC_GREEN);
		myHighlightedPointAIS->SetDisplayMode(AIS_Shaded);
		// myHighlightedPointAIS->SetTransparency(0.2f);			   // Slightly transparent




// 1. Criar o material PBR
Graphic3d_PBRMaterial aPbrMat;
// aPbrMat.SetColor(Quantity_Color(Quantity_NOC_BLUE));  <----- aqui nao faz nada
aPbrMat.SetMetallic(0.5f);  // Quase metal para testar brilho
aPbrMat.SetRoughness(0.5f); // Bem polido

// 2. Configurar o MaterialAspect (CRUCIAL: definir a cor aqui também)
Graphic3d_MaterialAspect aMat(Graphic3d_NameOfMaterial_UserDefined);
aMat.SetPBRMaterial(aPbrMat);
aMat.SetColor(Quantity_Color(Quantity_NOC_GREEN)); // Sincroniza Albedo com Diffuse
aMat.SetShininess(1.0f);
// aMat.SetCastShadows(false);
// aMat.SetReceiveShadows(false);

// 3. Aplicar diretamente ao AIS_Shape (Método mais limpo)
myHighlightedPointAIS->SetMaterial(aMat); 
myHighlightedPointAIS->SetDisplayMode(1);

// 4. Forçar o Shading Model no Aspecto do objeto
Handle(Prs3d_ShadingAspect) aShading = myHighlightedPointAIS->Attributes()->ShadingAspect();
aShading->Aspect()->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
aShading->Aspect()->SetInteriorStyle(Aspect_IS_SOLID);

// // Criar material PBR
// Graphic3d_MaterialAspect mat(Graphic3d_NOM_UserDefined);

// // Cor emissiva forte (autoiluminado)
// mat.SetEmissiveColor(Quantity_Color(0.0, 1.0, 0.0, Quantity_TOC_RGB));

// // Opcional: remover influência de sombras
// mat.SetCastShadows(false);
// mat.SetReceiveShadows(false);

// // Aplicar ao AIS
// myHighlightedPointAIS->Attributes()->SetMaterial(mat, false);
// myHighlightedPointAIS->Redisplay(true);

myHighlightedPointAIS->Attributes()->SetShadingModel(Graphic3d_TOSM_NONE);





		myHighlightedPointAIS->SetZLayer(Graphic3d_ZLayerId_Top);  // Ensure it's drawn on top

		m_context->Display(myHighlightedPointAIS, Standard_True);
		myLastHighlightedVertex = aVertex;
		redraw();
		// Fl::wait();
		if (!IsWorldPointGreen(m_view, vertexPnt)) return;
		// if(!IsPixelQuantityGreen(m_view,mousex,mousey))return;

		help.point = "";
		help.upd();
		// if(!IsVisible(myHighlightedPointAIS,m_view))return;
		// if(!IsShapeVisible(myHighlightedPointAIS,m_view,m_context))return;

		// cotm("Highlighted Vertex:", vertexPnt.X(), vertexPnt.Y(), vertexPnt.Z());
		help.point = "Point: " + fmt(vertexPnt.X()) + "," + fmt(vertexPnt.Y()) + "," + fmt(vertexPnt.Z());
		if (ldd && !ldd->Origin.IsIdentity()) {
			help.point += " Pointl: " + fmt(vertexPntL.X()) + "," + fmt(vertexPntL.Y()) + "," + fmt(vertexPntL.Z());
		}
		help.upd();
	}
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
			// cotm("an1")
			if (wrapper->is_setview) {
				// cotm(1);
				// m_view->SetProj(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
				// cotm(2)
				// m_view->SetUp(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz);

				occv->end_proj_global = gp_Vec(wrapper->v.dx, wrapper->v.dy, wrapper->v.dz);
				occv->end_up_global = gp_Vec(wrapper->v.ux, wrapper->v.uy, wrapper->v.uz); 
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

//region animation
gp_Vec end_proj_global;
gp_Vec end_up_global;
Handle(AIS_AnimationCamera) CurrentAnimation;

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

			// cotm2("an11")
    // Pivot = center of currently visible shapes in viewport (robust)
    gp_Pnt center = computeVisibleCenter(occv);
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


//spin
	bool IsSpinning = false;
	gp_Pnt SpinPivot;
	gp_Ax1 SpinAxis;
	gp_Dir CurrentDir;
	double SpinStep = 0.02;

	// New tracking variables
	double TargetRotation = 0;	 // 0 = infinity
	double CurrentRotation = 0;	 // Accumulated radians

	void start_continuous_rotation(double spins_amount = 1) {
		if (IsSpinning) {
			IsSpinning = false;
			return;
		}

		IsSpinning = true;
		SpinPivot = computeVisibleCenter(this);

		// Initialize rotation tracking
		CurrentRotation = 0;
		// 1 spin = 2 * PI. If amount is 0, TargetRotation stays 0 (infinite)
		TargetRotation = spins_amount * (2.0 * M_PI);

		CurrentDir = gp_Dir(0, 1, 0);
		SpinAxis = gp_Ax1(SpinPivot, CurrentDir);

		Fl::add_timeout(1.0 / 60.0, SpinCallback, this);
	}

	static void SpinCallback(void* userdata) {
		auto* occv = static_cast<OCC_Viewer*>(userdata);

		// 1. Stop conditions (Manual toggle or Escape)
		if (!occv->IsSpinning || Fl::event_key() == FL_Escape) {
			occv->stop_spinning();
			return;
		}

		// 2. Limit Check: If not infinite, check if we've reached the goal
		if (occv->TargetRotation > 0) {
			occv->CurrentRotation += std::abs(occv->SpinStep);
			if (occv->CurrentRotation >= occv->TargetRotation) {
				occv->stop_spinning();
				return;
			}
		}

		// 3. Axis change logic (Alt + X, Y, or Z)
		int state = Fl::event_state();
		if (state & FL_ALT) {
			int key = Fl::event_key();
			gp_Dir targetDir = occv->CurrentDir;
			if (key == 'x')
				targetDir = gp_Dir(1, 0, 0);
			else if (key == 'y')
				targetDir = gp_Dir(0, 1, 0);
			else if (key == 'z')
				targetDir = gp_Dir(0, 0, 1);

			if (!targetDir.IsEqual(occv->CurrentDir, 1e-6)) {
				occv->CurrentDir = targetDir;
				occv->SpinAxis = gp_Ax1(occv->SpinPivot, targetDir);
			}
		}

		// 4. Transformation
		Handle(Graphic3d_Camera) cam = occv->m_view->Camera();
		gp_Trsf trsf;
		trsf.SetRotation(occv->SpinAxis, occv->SpinStep);

		cam->SetCenter(occv->SpinPivot);
		cam->SetEye(cam->Eye().Transformed(trsf));
		cam->SetUp(cam->Up().Transformed(trsf));

		occv->projectAndDisplayWithHLR(occv->vshapes);
		occv->redraw();
		Fl::repeat_timeout(1.0 / 60.0, SpinCallback, userdata);
	}

	// Helper to handle cleanup
	void stop_spinning() {
		IsSpinning = false;
		projectAndDisplayWithHLR(vshapes);
		redraw();
	}
	//region sbt
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


void SetSideBarBackground(const Handle(V3d_View)& view)
{
    Standard_Integer width, height;
    view->Window()->Size(width, height);

    // Criar imagem como HANDLE
    Handle(Image_PixMap) img = new Image_PixMap();
    img->InitTrash(Image_Format_RGB, width, height);

    // Pintar tudo branco
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            img->SetPixelColor(x, y, Quantity_Color(1,1,1,Quantity_TOC_RGB));
        }
    }

    // Barra rosa a 5%
    int barStart = width * 0.95;
    for (int y = 0; y < height; ++y)
    {
        for (int x = barStart; x < width; ++x)
        {
            img->SetPixelColor(x, y, Quantity_Color(1.0, 0.4, 0.7, Quantity_TOC_RGB));
        }
    }

    // Criar textura a partir do Handle(Image_PixMap)
    Handle(Graphic3d_Texture2D) tex = new Graphic3d_Texture2D(img);

    // Aplicar como background
    view->SetBackgroundImage(tex, Aspect_FM_STRETCH, Standard_True);
    view->Redraw();
}

void setbar5per(){
	SetSideBarBackground(m_view);
// 	Handle(Image_PixMap) img = new Image_PixMap();
// img->InitTrash(Image_Format_RGB, width, height);

// // pinta tudo branco
// img->Fill(Quantity_Color(1,1,1,Quantity_TOC_RGB));

// // pinta barra rosa
// for (int x = width*0.95; x < width; ++x)
//   for (int y = 0; y < height; ++y)
//     img->SetPixelColor(x, y, Quantity_Color(1.0, 0.4, 0.7, Quantity_TOC_RGB));

// m_view->SetBackgroundImage(img, Aspect_FM_STRETCH, Standard_True);

}

	void setbar5perv1() {
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
	}

	void fillvectopo() {
		vshapes.clear();
		// vshapes is not in sync vector
		//  cotm(vaShape.size(), vshapes.size());
		for (int i = 0; i < vaShape.size(); i++) {
			if (!m_context->IsDisplayed(vaShape[i]) || !vlua[i]->visible_hardcoded) continue;

			TopoDS_Shape s = vaShape[i]->Shape();
			if (!vlua[i]->Origin.IsIdentity()) {
				s = s.Located(TopLoc_Location(vlua[i]->Origin.Transformation()));
			}
			vshapes.push_back(s);  // not in sync

			// vshapes.push_back(vaShape[i]->Shape());
		}
		// cotm(vaShape.size(), vshapes.size());
	}
//region toggle_shaded
void InitializeCustomWireframeAspects(const Handle(AIS_InteractiveContext)& ctx)
{
    if (ctx.IsNull()) return;

    const Handle(Prs3d_Drawer)& drawer = ctx->DefaultDrawer();

    Handle(Prs3d_LineAspect) wireAsp =
        new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.0);

    Handle(Prs3d_IsoAspect) isoAsp =
        new Prs3d_IsoAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 1.0, 1);
    // isoAsp->SetNumber(0);

    drawer->SetWireAspect(wireAsp);
    drawer->SetLineAspect(wireAsp);

    drawer->SetUIsoAspect(isoAsp);
    drawer->SetVIsoAspect(isoAsp);

    drawer->SetSeenLineAspect(wireAsp);

    drawer->SetFaceBoundaryDraw(Standard_False);
}

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


				// m_context->Remove(aShape, 0); //debug

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

				m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);

				// aShape->SetColor(Quantity_NOC_GRAY70); 
				aShape->SetColor(Quantity_NOC_WHITE);
if(1){
				// aShape->Attributes()->SetFaceBoundaryDraw(1);//////////////////
				aShape->Attributes()->SetFaceBoundaryDraw(!vlua[i]->ttfillet);//////////////////
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

if(0){
 
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



// 			Handle(AIS_Shape)& highlight = vaShape[i];
// Handle(Prs3d_LineAspect) la =
//     new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 2.0);

// highlight->Attributes()->SetLineAspect(la);
// highlight->Attributes()->SetWireAspect(la);
// highlight->Attributes()->SetFreeBoundaryAspect(la);
// highlight->Attributes()->SetUnFreeBoundaryAspect(la);
// highlight->Attributes()->SetFaceBoundaryAspect(la);
// highlight->Attributes()->SetHiddenLineAspect(la);
// highlight->Attributes()->SetVectorAspect(la);
// highlight->Attributes()->SetSectionAspect(la);
// highlight->Attributes()->SetSeenLineAspect(la);





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
void toggle_shaded_transpvn(Standard_Integer fromcurrentMode = AIS_WireFrame) {

auto viewer=m_view->Viewer();

// Desliga todas as luzes existentes
// TColStd_ListOfTransient lights;

// viewer->InitActiveLights();
// while (viewer->MoreActiveLights()) {
//     lights.Append(viewer->ActiveLight());
//     viewer->NextActiveLights();
// }

// for (TColStd_ListIteratorOfListOfTransient it(lights); it.More(); it.Next()) {
//     Handle(V3d_Light) L = Handle(V3d_Light)::DownCast(it.Value());
//     viewer->DelLight(L);
// }



// viewer->SetLightOff();
m_view->Viewer()->SetDefaultShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
m_view->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
// // Ambient: strong, soft base
// Handle(V3d_AmbientLight) amb = new V3d_AmbientLight(Quantity_NOC_WHITE);
// amb->SetIntensity(1.2f);
// viewer->AddLight(amb);
// viewer->SetLightOn(amb);

// // Key light: subtle
// gp_Dir sunDir(-1.0, -1.0, -1.0);
// Handle(V3d_DirectionalLight) sun = new V3d_DirectionalLight(sunDir, Quantity_NOC_WHITE);
// sun->SetIntensity(1.0f);   // was 22.5
// viewer->AddLight(sun);
// viewer->SetLightOn(sun);

// // Fill light: very soft
// gp_Dir fillDir(1.0, 0.5, 0.2);
// Handle(V3d_DirectionalLight) fill = new V3d_DirectionalLight(fillDir, Quantity_NOC_WHITE);
// fill->SetIntensity(0.6f);  // was 20.8
// viewer->AddLight(fill);
// viewer->SetLightOn(fill);

// #include <V3d_DirectionalLight.hxx> 
// for (viewer->InitActiveLights(); viewer->MoreActiveLights(); viewer->NextActiveLights()) {
//     viewer->SetLightOff(viewer->ActiveLight());
// }

// 1. Criar a luz direcional
// Direção: x=1.0 (direita), y=1.0 (cima), z=1.0 (frente ao objeto)
// Handle(V3d_DirectionalLight) aLight = new V3d_DirectionalLight(gp_Dir(1.0, 1.0, 1.0));

// // 2. Definir como "Headlight" (Fixa na câmera)
// aLight->SetHeadlight(Standard_True);

// // 3. Ajustar intensidade e cor (opcional)
// aLight->SetIntensity(10000.0);
// aLight->SetColor(Quantity_NOC_WHITE);


// viewer->AddLight(aLight);
// viewer->SetLightOn(aLight);

// lettherebelight77();


		for (std::size_t i = 0; i < vaShape.size(); ++i) {
			Handle(AIS_Shape) shape = vaShape[i];
			if (shape.IsNull()) continue;

		{
			shape->UnsetAttributes();	// limpa materiais, cor, largura, etc.

	auto srgbToLinear = [](double c)->double {
    if (c <= 0.04045) return c / 12.92;
    return pow((c + 0.055) / 1.055, 2.4);
};

Quantity_Color srgb(0.54118,0.54118,0.61569, Quantity_TOC_RGB);
Quantity_Color linear(
    srgbToLinear(srgb.Red()),
    srgbToLinear(srgb.Green()),
    srgbToLinear(srgb.Blue()),
    Quantity_TOC_RGB
);

// Graphic3d_MaterialAspect mat(Graphic3d_NameOfMaterial_UserDefined);
// mat.SetDiffuseColor(Quantity_Color(0.2,0.2,0.2, Quantity_TOC_RGB));
// mat.SetSpecularColor(Quantity_NOC_WHITE);
// mat.SetShininess(80.0f); // higher = tighter highlight
// // mat.SetReflectionModeOn(Graphic3d_TypeOfReflection_Specular);
// mat.SetTransparency(0.0f);



// pbr1.Albedo = linear;

    // Graphic3d_MaterialAspect mat1(Graphic3d_NameOfMaterial_Steel);
    // Graphic3d_MaterialAspect mat1(Graphic3d_NameOfMaterial_ShinyPlastified);
    Graphic3d_MaterialAspect mat1(Graphic3d_NameOfMaterial_UserDefined);
	    Graphic3d_PBRMaterial pbr1;
    pbr1.SetMetallic(strcfg("Metallic",0,1));
    pbr1.SetRoughness(strcfg("Roughness",0,1)); 
    // pbr1.SetMetallic(0.94f);
    // pbr1.SetRoughness(0.44f);
    mat1.SetPBRMaterial(pbr1);
	// mat1.SetColor(Quantity_NOC_RED);
	mat1.SetColor(linear);
	mat1.SetEmissiveColor(Quantity_NOC_RED);
	mat1.SetShininess(1.0f);
	mat1.SetSpecularColor(Quantity_NOC_WHITE);
    shape->SetMaterial(mat1); 

	BRepMesh_IncrementalMesh mesher1(shape->Shape(), 0.05, Standard_False, 0.1, Standard_True);
ApplyFakeBevelNormals(shape->Shape(), gp_Pnt(5000.0, 5000.0, 5000.0));

    shape->Attributes()->ShadingAspect()->Aspect()
    //     ->SetShadingModel(Graphic3d_TypeOfShadingModel_DEFAULT);
        ->SetShadingModel(Graphic3d_TypeOfShadingModel_Pbr);
    // REALÇAR ARESTAS
    shape->Attributes()->SetFaceBoundaryDraw(Standard_True);
    shape->Attributes()->SetFaceBoundaryAspect(
        new Prs3d_LineAspect(Quantity_Color(0,0.0,0.0,Quantity_TOC_RGB), Aspect_TOL_SOLID, 2.0)
        // new Prs3d_LineAspect(Quantity_Color(1,0.35,0.35,Quantity_TOC_RGB), Aspect_TOL_SOLID, 2.0)
    );
			Handle(Prs3d_LineAspect) wireAsp = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 0.2);
			shape->Attributes()->SetUnFreeBoundaryAspect(wireAsp);
		} 
			if (m_context->IsDisplayed(shape))
						m_context->Remove(shape, Standard_False); 

			if (fromcurrentMode == AIS_Shaded) {
				hlr_on = 1;
				shape->SetDisplayMode(AIS_WireFrame);

			}else{
				hlr_on = 0;
				shape->SetDisplayMode(AIS_Shaded);

			}
			m_context->Display(shape, 0);
		}

		if (hlr_on == 1) { 
				projectAndDisplayWithHLR(vshapes); 
		} else {
			if (!visible_.IsNull()) {
				m_context->Remove(visible_, 0);
				visible_.Nullify();
			}
		currentMode = fromcurrentMode;
	}
}

void toggle_shaded_transpv2_nw(Standard_Integer fromcurrentMode = AIS_WireFrame)
{
    perf1();

    // Aspects created once – good (zero isolines kills gray fillet lines)
    Handle(Prs3d_IsoAspect) wireAsp = new Prs3d_IsoAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.0, 0);
    Handle(Prs3d_LineAspect) invisAsp = new Prs3d_LineAspect(Quantity_NOC_WHITE, Aspect_TOL_SOLID, 0.0);
    // Alternative for truly invisible hidden lines: Quantity_NOC_TRANSPARENT if your OCCT supports it

    for (std::size_t i = 0; i < vaShape.size(); ++i)
    {
        Handle(AIS_Shape) aShape = vaShape[i];
        if (aShape.IsNull()) continue;

        if (fromcurrentMode == AIS_WireFrame)
        {
            hlr_on = 1;

            // 1. Switch mode
            m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_False);

            // 2. Critical: force presentation computation NOW → initializes drawer internals
            m_context->Redisplay(aShape, Standard_False);

            // 3. Now create & link fresh drawer (safe after Redisplay)
            Handle(Prs3d_Drawer) newDrawer = new Prs3d_Drawer();
            if (!m_context->DefaultDrawer().IsNull())
            {
                newDrawer->Link(m_context->DefaultDrawer());
            }

            // 4. Assign it
            aShape->SetAttributes(newDrawer);

            // 5. Now safe to set aspects
            newDrawer->SetLineAspect(wireAsp);
            newDrawer->SetWireAspect(wireAsp);
            newDrawer->SetUIsoAspect(wireAsp);
            newDrawer->SetVIsoAspect(wireAsp);
            newDrawer->SetSeenLineAspect(wireAsp);
            newDrawer->SetHiddenLineAspect(invisAsp);

            // Clean up fillet/iso garbage
            newDrawer->SetFaceBoundaryDraw(Standard_False);
            newDrawer->SetTypeOfHLR(Prs3d_TOH_NotSet); // or Prs3d_TOH_PolyAlgo if you want approximate HLR
        }
        else  // back to shaded
        {
            hlr_on = 0;

            // Safe here
            // aShape->UnsetAttributes();

            m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);
            aShape->SetColor(Quantity_NOC_GRAY70);

            // Restore visible boundaries if needed
            Handle(Prs3d_Drawer) dr = aShape->Attributes();
            if (!dr.IsNull())
            {
                dr->SetFaceBoundaryDraw(Standard_True);
                dr->SetFaceBoundaryAspect(
                    new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.0)
                );
            }
        }

        // Redisplay again to reflect changes
        m_context->Redisplay(aShape, Standard_False);
    }

    // HLR handling
    if (hlr_on == 1)
    {
        projectAndDisplayWithHLR(vshapes);
    }
    else
    {
        if (!visible_.IsNull())
        {
            m_context->Remove(visible_, Standard_False);
            visible_.Nullify();
        }
    }

    m_context->UpdateCurrentViewer();

    currentMode = fromcurrentMode;

    perf1("toggle_shaded_transp");
}


void toggle_shaded_transpv1(Standard_Integer fromcurrentMode = AIS_WireFrame) {
		perf1(); 
		for (std::size_t i = 0; i < vaShape.size(); ++i) {
			Handle(AIS_Shape) aShape = vaShape[i];
			if (aShape.IsNull()) continue;

			if (fromcurrentMode == AIS_Shaded) {
				hlr_on = 1;
				// Mudar para modo wireframe
				// // aShape->UnsetColor();
				// aShape->UnsetAttributes();	// limpa materiais, cor, largura, etc.
				// m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_False);

				// aShape->Attributes()->SetFaceBoundaryDraw(Standard_False);
				// aShape->Attributes()->SetLineAspect(wireAsp);
				// aShape->Attributes()->SetSeenLineAspect(wireAsp);
				// aShape->Attributes()->SetWireAspect(wireAsp);
				// aShape->Attributes()->SetUnFreeBoundaryAspect(wireAsp);
				// aShape->Attributes()->SetFreeBoundaryAspect(wireAsp);
				// aShape->Attributes()->SetFaceBoundaryAspect(wireAsp);
// aShape->UnsetAttributes(); 

// // 1. Set the display mode
// m_context->SetDisplayMode(aShape, AIS_WireFrame, Standard_False);

// 2. Apply your custom Aspect to ALL wire-related categories
// auto drawer = aShape->Attributes();
// drawer->SetLineAspect(wireAsp);
// drawer->SetWireAspect(wireAsp);
// drawer->SetUnFreeBoundaryAspect(wireAsp);
// drawer->SetFreeBoundaryAspect(wireAsp);
// drawer->SetFaceBoundaryAspect(wireAsp);
// drawer->SetSeenLineAspect(wireAsp);

// // Crucial: Fillets often rely on IsoAspects in Wireframe mode
// // Prs3d_IsoAspect(Color, Type, Width, NumberOfIsolines)
// Handle(Prs3d_IsoAspect) wireAsp = new Prs3d_IsoAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 2.0, 1);

// // Now these will all work without errors:
// // aShape->Attributes()->SetLineAspect(wireAsp);   // Works (Upcast)
// aShape->Attributes()->SetUIsoAspect(wireAsp);   // Works (Exact match)
// aShape->Attributes()->SetVIsoAspect(wireAsp);   // Works (Exact match)
// // drawer->SetUIsoAspect(wireAsp);
// // drawer->SetVIsoAspect(wireAsp);

// // 3. Disable Face Boundaries if you only want the wireframe
// drawer->SetFaceBoundaryDraw(Standard_False);

// 4. Tell the context to push these changes to the GPU
// m_context->Redisplay(aShape, Standard_False); // Standard_False avoids an immediate viewer update
// m_context->UpdateCurrentViewer();             // Updates everything at once
			


// 1. Create a brand new, fresh Drawer
Handle(Prs3d_Drawer) newDrawer = new Prs3d_Drawer();

// 2. Link it to the context's defaults (optional, but prevents other crashes)
newDrawer->Link(m_context->DefaultDrawer());

// 3. Create your Aspect (Color, Type, Width, IsolineCount)
// Setting '0' here is what fixes the gray lines on fillets!
Handle(Prs3d_IsoAspect) visibleAsp = new Prs3d_IsoAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 1.0, 0);

// 4. Populate the aspects (Order doesn't matter now)
newDrawer->SetLineAspect(visibleAsp);
newDrawer->SetWireAspect(visibleAsp);
newDrawer->SetUIsoAspect(visibleAsp);
newDrawer->SetVIsoAspect(visibleAsp);
newDrawer->SetSeenLineAspect(visibleAsp);

// 5. Hide hidden lines without messy flags
Handle(Prs3d_LineAspect) hiddenAsp = new Prs3d_LineAspect(Quantity_NOC_WHITE, Aspect_TOL_SOLID, 0.0);
newDrawer->SetHiddenLineAspect(hiddenAsp);

// 6. Set HLR and disable Face Boundaries
newDrawer->SetTypeOfHLR(Prs3d_TOH_PolyAlgo);
newDrawer->SetFaceBoundaryDraw(Standard_False);

// 7. SWAP the drawer on the shape
aShape->SetAttributes(newDrawer);

// 8. Update
m_context->Redisplay(aShape, Standard_True);




			} else {
				hlr_on = 0;
				// 			// Mudar para modo sombreado
				aShape->UnsetAttributes();	// limpa materiais, cor, largura, etc.
				if (1) {
					m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False);

					aShape->SetColor(Quantity_NOC_GRAY70);
					aShape->Attributes()->SetFaceBoundaryDraw(1);  //////////////////
					// aShape->Attributes()->SetFaceBoundaryDraw(!vlua[i]->ttfillet);//////////////////
					aShape->Attributes()->SetFaceBoundaryAspect(edgeAspect);
					aShape->Attributes()->SetSeenLineAspect(edgeAspect);  //
				}
				 

				if (0) { 
					if (vlua[i]->name == "framev") {
						aShape->SetZLayer(Graphic3d_ZLayerId_Top);
					} else {
						// aShape->SetZLayer(Graphic3d_ZLayerId_Bot);
					}

					m_context->SetDisplayMode(aShape, AIS_Shaded, Standard_False); 
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
					objDrawer->SetFaceBoundaryAspect(new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID,
																		  1.8));  // 1.0 a 1.5 costuma ficar bom

					// Crucial: suprime as boundaries com continuidade alta (seams suaves, como em cilindros ou fillets)
					drawer->SetFaceBoundaryUpperContinuity(
						GeomAbs_C2);  // ou GeomAbs_C2 para esconder ainda mais seams suaves
					// GeomAbs_C0 = mostra tudo (arestas "duras" só)
					// GeomAbs_C1 = esconde seams tangentes (bom para cilindros)
					// GeomAbs_C2 = esconde mais (fillets suaves)

					// Desativa isoparamétricas (linhas U/V da triangulação)
					drawer->SetIsoOnTriangulation(Standard_False); 
					if (0 && vlua[i]->name == "framev") { 
 
						Graphic3d_ZLayerId myEdgeLayer;
						m_viewer->AddZLayer(myEdgeLayer);  // retorna o ID

						// Obtém as settings da layer para modificar
						Graphic3d_ZLayerSettings layerSettings = m_viewer->ZLayerSettings(myEdgeLayer);

 
						m_viewer->SetZLayerSettings(myEdgeLayer, layerSettings);

						// Atribui ao teu objeto (ou só às edges se separares)
						m_context->SetZLayer(aShape, myEdgeLayer);
						// m_context->Redisplay(aShape, Standard_True);
					}
				}

				Handle(AIS_Shape) & highlight = vaShape[i];
				Handle(Prs3d_LineAspect) la = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 2.0);
				highlight->Attributes()->SetLineAspect(la);
				highlight->Attributes()->SetWireAspect(la);
				highlight->Attributes()->SetFreeBoundaryAspect(la);
				highlight->Attributes()->SetUnFreeBoundaryAspect(la);
				highlight->Attributes()->SetFaceBoundaryAspect(la);
				highlight->Attributes()->SetHiddenLineAspect(la);
				highlight->Attributes()->SetVectorAspect(la);
				highlight->Attributes()->SetSectionAspect(la);
				highlight->Attributes()->SetSeenLineAspect(la);
			}
			m_context->Redisplay(aShape, 0); 
		}

		perf1("toggle_shaded_transp");
		if (hlr_on == 1) { 
				projectAndDisplayWithHLR(vshapes); 
		} else { 
			if (!visible_.IsNull()) {
				m_context->Remove(visible_, 0);
				visible_.Nullify();
			}
		}
		currentMode = fromcurrentMode;
	}
	Standard_Integer currentMode = AIS_WireFrame;
	void projectAndDisplayWithHLR(const std::vector<TopoDS_Shape>& shapes, bool isDragonly = false){
		if (!hlr_on)return;
		perf2(); 
		projectAndDisplayWithHLR_lp(shapes,isDragonly); 
		perf2("p2 hlr");
	}
	void projectAndDisplayWithHLR_lp(const std::vector<TopoDS_Shape>& shapes, bool isDragonfly = false) {
		if (!hlr_on || m_context.IsNull() || m_view.IsNull()) return;

		if (visible_) m_context->Remove(visible_, 0);

		// 1. Camera transformation setup (kept your working version)
		const Handle(Graphic3d_Camera) & camera = m_view->Camera();
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

		for (auto& s : shapes) {
			if (!s.IsNull()) {
				BRepTools::Clean(s);

				// Optional: skip if already meshed sufficiently (your commented check)
				// For maximum speed, you can force remesh or skip check
				BRepMesh_IncrementalMesh(s, deflection, Standard_False, angularDeflection, Standard_True);
			}
		}

		// 4. HLR computation (sequential - no parallelism available in OCCT HLR)
		Handle(HLRBRep_PolyAlgo) algo = new HLRBRep_PolyAlgo();

		for (const auto& s : shapes) {
			if (!s.IsNull()) {
				algo->Load(s);
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
		m_context->Display(visible_, false);
		visible_->SetZLayer(Graphic3d_ZLayerId_Topmost);
		m_context->Deactivate(visible_);
	}

	//region luadraw
	// struct luadraw; 
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
		string  from_sketch="";
		float Extrude_val=0;
		int clone_qtd=0;

		int type=0; 
		bool editing=0; 
		bool ttfillet=0;

		// build it using BRep_Builder
		BRep_Builder builder; 
		string name = "";
		bool visible_hardcoded = 1; 
		TopoDS_Compound cshape;
		TopoDS_Shape shape;
		TopoDS_Shape fshape; 
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

		luadraw(string _name = "test", OCC_Viewer* p = 0) : occv(p), name(_name) {
			// regen
			if (occv->vaShape.size() > 0) {
				OCC_Viewer::luadraw* ld = occv->getluadraw_from_ashape(occv->vaShape.back());
				// occv
				ld->redisplay();
			}

			builder.MakeCompound(TopoDS::Compound(cshape));

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

			Origin = occv->Origin;

			ashape->SetLocalTransformation(Origin.Transformation());
		}
		void redisplay() {
			update_placement();
			if (visible_hardcoded) {
				if (occv->m_context->IsDisplayed(ashape)) {
					occv->m_context->Redisplay(ashape, false);
				} else {
					occv->m_context->Display(ashape, false);
				}
			} else {
				// Only erase if it is displayed
				if (occv->m_context->IsDisplayed(ashape)) {
					cotm("notvisible", name) occv->m_context->Erase(ashape, Standard_False);
				}
			}
		}
		void update_placement() {
			if (needsplacementupdate == 0) return;
			shape = FuseAndRefineWithAPI(cshape);
			ashape->Set(cshape);
			needsplacementupdate = 0;
		}
		TopoDS_Shape FuseAndRefineWithAPI(const TopoDS_Shape& inputShape) {
			auto solids = ExtractSolids(inputShape);
			if (solids.empty()) {
				// std::cerr << "⚠️ No solids found in input shape\n";
				// cotm(name, "⚠️ No solids found in input shape");
				// lua_error_with_line(L, "No solids found in input shape\n");
				return shape; 
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
		void clone(luadraw* toclone, bool copy_placement = false) {
			// return;
			if (!toclone) {
				throw std::runtime_error(lua_error_with_line(L, "Something went wrong"));
				return;
			}

			this->vpoints = toclone->vpoints;

			if (!toclone->shape.IsNull()) {
				toclone->needsplacementupdate = 1;	// verify better why this is needed
				toclone->redisplay();				// verify better why this is needed

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
				bool contain = Contains(toclone->name, "sketch");

				if (this->from_sketch == "" && contain) {
					this->from_sketch = toclone->name;
					// toclone->clone_qtd++;
				}

				if (toclone->clone_qtd == 0) clone_qtd = 1;
				// this->clone_qtd+=toclone->clone_qtd;

				if (this->from_sketch == "" && !contain) {
					this->from_sketch = toclone->from_sketch;
					// this->clone_qtd=toclone->clone_qtd;
					// return;
				}
				this->clone_qtd += toclone->clone_qtd;
				// this->clone_qtd++;
				this->Extrude_val = toclone->Extrude_val;
			}
		}

		vector<std::vector<gp_Vec2d>> vpoints;

		void CreateWire(const std::vector<gp_Vec2d>& points, bool closed = false) {
			vpoints.push_back(points);
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
				mergeShape(cshape, face);
				shape = face;
			} else {
				mergeShape(cshape, wire);
			}
		}
		void createOffset(double distance) {
			vector<gp_Pnt2d> ppoints;
			ConvertVec2dToPnt2d(vpoints.back(), ppoints);

			bool closed = ((vpoints.back()[0].X() == vpoints.back().back().X()) &&
						   (vpoints.back()[0].Y() == vpoints.back().back().Y()));
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
		void mergeShape(TopoDS_Compound& target, const TopoDS_Shape& toAdd) {
			shape = toAdd;	// last added
			// cotm(ShapeTypeName(shape));
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

		return nullptr;
	}
};
OCC_Viewer* occv = 0;
OCC_Viewer::luadraw* current_part = nullptr;

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
	// if(occv->vshapes.empty())return;
	perf();
	static int binit = 0; 
	if (!binit) {
		binit = 1;
		fbm->setCallback([](void* data, int code, void* fbm_) {
			OCC_Viewer::luadraw* ld = (OCC_Viewer::luadraw*)data;
			fl_browser_msv* fbm=(fl_browser_msv*)fbm_;
			std::cout << "Callback data_ptr=" << ld->name << ", code=" << code << std::endl;

			if (code == 2 && !fbm->isrightclick) {
				gopart(ld->name); 
				return;
			}
			if (code == 2 && fbm->isrightclick) {
				FitViewToShape(ld->occv->m_view, ld->shape);
				// ld->occv->FitViewToShape(ld->occv->m_view, ld->shape);
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
	// if(occv->vlua.size()>0)occv->vlua.back()->redisplay(); //regen
	lop(ij, 0, occv->vaShape.size()) {
		OCC_Viewer::luadraw* ld = occv->vlua[ij]; 

		fbm->addnew({"H", "S", ld->name});
		fbm->data(fbm->size(), (void*)ld);
		int line = fbm->size();

		ld->visible_hardcoded = 1;

		if (mhide[ld->name] == 1) {
			ld->visible_hardcoded = 0;
			fbm->toggleon(line, 0, 1, 0);
		} 
 
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
	occv->toggle_shaded_transp(occv->currentMode);
	occv->redraw();
	perf("fillbrowser");
}
//region sol
 
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

 

lua.set_exception_handler([](lua_State* L,
                             sol::optional<const std::exception&> maybe_ex,
                             sol::string_view description) -> int 
{
    // Handles C++ exceptions thrown inside Lua
    std::string msg = maybe_ex ? maybe_ex->what() : std::string(description);
    std::string enriched = lua_error_with_line(L, msg);
    lua_pushlstring(L, enriched.c_str(), enriched.size());
    return 1;
});

// lua.set_panic([](lua_State* L) -> int {
//     const char* description = lua_tostring(L, -1);
//     std::string msg = description ? description : "unknown panic";

//     std::string enriched = lua_error_with_line(L, msg);

//     lua_pushlstring(L, enriched.c_str(), enriched.size());
//     return 1;
// });

 

	lua.set_function("Origin", [occv](Standard_Real x = 0, Standard_Real y = 0, Standard_Real z = 0) {
		if (current_part) current_part->redisplay();
		if (x == 0 && y == 0 && z == 0) {
			occv->Origin = TopLoc_Location();
			return;
		}
		gp_Trsf tr;
		tr.SetTranslation(gp_Vec(x, y, z));
		occv->Origin = TopLoc_Location(tr);
	});

	lua.set_function("Part", [occv, &lua](const std::string& name) {
		auto* obj = new OCC_Viewer::luadraw(name, occv);
		lua[name] = obj;  // same as: name = obj in Lua, now  part names are luadraw
		current_part = obj;
		return obj;
	});
	lua.set_function("Pl", [&, parse_coords](const std::string& coords) {
		if (!current_part) luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");
		current_part->CreateWire(parse_coords(coords), false);
	});
	lua.set_function("Offset", [&](double val) {
		if (!current_part) luaL_error(lua.lua_state(), "No current part.");
		current_part->createOffset(val);
	});

lua.set_function("Extrude", [&](float val=0){
	if(val==0)luaL_error(lua.lua_state(), "Extrude must have a value, other than 0.");
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
    current_part->Extrude_val=val;
    // current_part->from_sketch=current_part->name;


    current_part->mergeShape(current_part->cshape, extruded);
    current_part->shape = extruded;
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
            // cotm("Movel");
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
   
lua.set_function("Clone", sol::protect([&](OCC_Viewer::luadraw* val,sol::optional<int> copy_placement){
    if (!current_part) luaL_error(lua.lua_state(), "No current part."); 
    current_part->clone(val,copy_placement.value_or(0)); 
})); 


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


lua.set_function("Mirrorbaked", [&](OCC_Viewer::luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){    
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    if (!original || original->cshape.IsNull()) {
        luaL_error(lua.lua_state(), "Invalid original shape.");
    }

    TopoDS_Shape origShape = original->cshape;

    // 1. Get the Bounding Box of the specific shape
    Bnd_Box bbox;
    BRepBndLib::Add(origShape, bbox);
    bbox.SetGap(0.0);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    // Calculate the geometric center of the shape
    Standard_Real cx = (xmin + xmax) * 0.5;
    Standard_Real cy = (ymin + ymax) * 0.5;
    Standard_Real cz = (zmin + zmax) * 0.5;

    // 2. Define Mirror Normal and Plane Origin
    gp_Dir normal;
    gp_Pnt planeOrigin(cx, cy, cz); // Start at the shape's center

    if (x != 0) {
        normal = gp::DX();
        // Move the plane relative to the center along X
        planeOrigin.SetX(cx + offset); 
    }
    else if (y != 0) {
        normal = gp::DY();
        planeOrigin.SetY(cy + offset);
    }
    else if (z != 0) {
        normal = gp::DZ();
        planeOrigin.SetZ(cz + offset);
    }

    // 3. Apply Transformation
    gp_Ax2 mirrorPlane(planeOrigin, normal);
    gp_Trsf trsf;
    trsf.SetMirror(mirrorPlane);

    BRepBuilderAPI_Transform transformer(origShape, trsf, true);
    if (!transformer.IsDone()) {
        luaL_error(lua.lua_state(), "Mirror failed.");
    }

    TopoDS_Shape mirrored = transformer.Shape();
    
    // Optional: Ensure the shape is a clean copy with corrected orientation
    mirrored = BRepBuilderAPI_Copy(mirrored).Shape();

    // 4. Create result
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);
    builder.Add(newCompound, mirrored); // Add original here too if you want both

    current_part->cshape = newCompound;
	current_part->shape = mirrored; ///for location to work
    current_part->Extrude_val = original->Extrude_val;
    current_part->from_sketch = original->from_sketch;
    current_part->clone_qtd += 1;
});
lua.set_function("Mirrorv1", [&](OCC_Viewer::luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){    
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    if (!original || original->cshape.IsNull()) {
        luaL_error(lua.lua_state(), "Invalid original shape.");
    }

    // 1. Get the original shape
    TopoDS_Shape origShape = original->cshape;

    // Calculate Bounding Box for center
    Bnd_Box bbox;
    BRepBndLib::Add(origShape, bbox);
    bbox.SetGap(0.0);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    Standard_Real cx = (xmin + xmax) * 0.5;
    Standard_Real cy = (ymin + ymax) * 0.5;
    Standard_Real cz = (zmin + zmax) * 0.5;

    // 2. Define Mirror Normal and Plane Origin
    gp_Dir normal;
    gp_Pnt planeOrigin(cx, cy, cz);

    if (x != 0) {
        normal = gp::DX();
        planeOrigin.SetX(cx + offset); 
    }
    else if (y != 0) {
        normal = gp::DY();
        planeOrigin.SetY(cy + offset);
    }
    else if (z != 0) {
        normal = gp::DZ();
        planeOrigin.SetZ(cz + offset);
    }

    // 3. Create the Transformation
    gp_Ax2 mirrorPlane(planeOrigin, normal);
    gp_Trsf trsf;
    trsf.SetMirror(mirrorPlane);

    // 4. Apply as a Location (Non-baked)
    // We create a new shape object that points to the same geometry but has a new location
    // Note: We multiply the new transformation with the existing one to be safe
    TopLoc_Location loc(trsf);
    TopoDS_Shape mirrored = origShape.Moved(loc); 

    // 5. Create result compound
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);
    
    // Add the mirrored instance to the compound
    builder.Add(newCompound, mirrored);

    current_part->cshape = newCompound;
	current_part->shape = mirrored; ///for location to work
    current_part->Extrude_val = original->Extrude_val;
    current_part->from_sketch = original->from_sketch;
    current_part->clone_qtd += 1;
});
lua.set_function("Mirrorv2", [&](OCC_Viewer::luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){    
    if (!current_part || !original || original->cshape.IsNull()) {
        luaL_error(lua.lua_state(), "Invalid part or original shape.");
    }

    // 1. Calculate the mirror transformation (gp_Trsf)
    Bnd_Box bbox;
    BRepBndLib::Add(original->cshape, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    if (x != 0) { normal = gp::DX(); planeOrigin.SetX(planeOrigin.X() + offset); }
    else if (y != 0) { normal = gp::DY(); planeOrigin.SetY(planeOrigin.Y() + offset); }
    else { normal = gp::DZ(); planeOrigin.SetZ(planeOrigin.Z() + offset); }

    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // 2. Apply to the Location, NOT the geometry
    // We take the original geometry and set its location to the mirror transformation.
    // If the original already had a location, we multiply them:
    TopLoc_Location combinedLoc = original->cshape.Location() * TopLoc_Location(mirrorTrsf);
    
    // Create a new shape handle pointing to the SAME TShape but with NEW Location
    TopoDS_Shape mirrored = original->cshape.Located(combinedLoc);

    // 3. Wrap in a Compound
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);
    builder.Add(newCompound, mirrored);

    current_part->cshape = newCompound;
	current_part->shape = mirrored; ///for location to work
    
    // Sync metadata
    current_part->Extrude_val = original->Extrude_val;
    current_part->from_sketch = original->from_sketch;
});
lua.set_function("Mirror", [&](OCC_Viewer::luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){    
    if (!current_part || !original || original->cshape.IsNull()) {
        luaL_error(lua.lua_state(), "Invalid part or original shape.");
    }
	original->needsplacementupdate = 1;	// verify better why this is needed
				original->redisplay();

    // 1. Get the current global bounding box of the original
    Bnd_Box bbox;
    BRepBndLib::Add(original->cshape, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    // Calculate the center of the bounding box
    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

    // Adjust plane origin based on the user's offset
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

    // 2. Create the Mirror Transformation
    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // 3. APPLY TO LOCATION ONLY
    // Use 'Moved' to multiply the existing location by the mirror transformation.
    // This keeps the underlying geometry (TShape) exactly the same as the original.
    TopoDS_Shape mirrored = original->cshape.Moved(TopLoc_Location(mirrorTrsf));

    // 4. Assign to your part
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);
    builder.Add(newCompound, mirrored);

    current_part->cshape = newCompound;
    current_part->shape = mirrored; 
    
    // Metadata sync
    current_part->Extrude_val = original->Extrude_val;
    current_part->from_sketch = original->from_sketch;
    current_part->clone_qtd += 1;
});
lua.set_function("Mirrorv4", [&](OCC_Viewer::luadraw* original, float offset = 0.0f, int x = 0, int y = 0, int z = 0){    
    if (!current_part || !original || original->cshape.IsNull()) {
        luaL_error(lua.lua_state(), "Invalid part or original shape.");
    }

    // 1. Get global bounding box (respects current Location)
    Bnd_Box bbox;
    BRepBndLib::Add(original->cshape, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    gp_Pnt planeOrigin((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
    gp_Dir normal;

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

    // 2. Mirror transformation (global)
    gp_Trsf mirrorTrsf;
    mirrorTrsf.SetMirror(gp_Ax2(planeOrigin, normal));

    // 3. Apply mirror to local geometry (this always produces duplicated geometry + identity location because mirror has det=-1)
    TopoDS_Shape localShape = original->cshape.Located(TopLoc_Location());

    BRepBuilderAPI_Transform transformer(mirrorTrsf);
    transformer.Perform(localShape, Standard_True);   // force geometry copy

    if (!transformer.IsDone()) {
        luaL_error(lua.lua_state(), "Mirror transformation failed.");
    }

    TopoDS_Shape mirroredGeom = transformer.Shape();   // geometry is mirrored, location is usually identity

    // 4. CORRECT COMPOSITION: new_location = mirrorTrsf * original_location
    //    This is the key fix for correct position in all axes
    gp_Trsf originalTrsf = original->cshape.Location().Transformation();
    gp_Trsf newTrsf = mirrorTrsf;
    newTrsf.Multiply(originalTrsf);                    // mirror first, then original (order matters!)

    TopoDS_Shape mirrored = mirroredGeom.Located(TopLoc_Location(newTrsf));

    // 5. Assign to current part
    TopoDS_Compound newCompound;
    BRep_Builder builder;
    builder.MakeCompound(newCompound);
    builder.Add(newCompound, mirrored);

    current_part->cshape = newCompound;
    current_part->shape  = mirrored; 

    gp_Trsf finalTrsf = mirrored.Location().Transformation();
std::cout << "Final mirrored location form: " << finalTrsf.Form() << std::endl;
// or dump values:
std::cout << "Translation: " << finalTrsf.TranslationPart().X() << ", " 
          << finalTrsf.TranslationPart().Y() << ", " 
          << finalTrsf.TranslationPart().Z() << std::endl;
    // Metadata sync
    current_part->Extrude_val = original->Extrude_val;
    current_part->from_sketch = original->from_sketch;
    current_part->clone_qtd += 1;
});
lua.set_function("Mirrorvn", [&](OCC_Viewer::luadraw* original, float offset, int x, int y, int z){    
    if (!current_part || !original || original->cshape.IsNull()) {
        luaL_error(lua.lua_state(), "Invalid part or original shape.");
    }

    // 1. Get the original part's coordinate system
    gp_Trsf origTrsf = original->cshape.Location().Transformation();
    gp_Ax3 origAx3(gp::Origin(), gp::DZ(), gp::DX()); // Local CS
    origAx3.Transform(origTrsf); // Move to World CS

    // 2. Define the Mirror Plane
    gp_Pnt p(0, 0, 0);
    gp_Dir n;
    if (x != 0)      { p.SetX(offset); n = gp::DX(); }
    else if (y != 0) { p.SetY(offset); n = gp::DY(); }
    else             { p.SetZ(offset); n = gp::DZ(); }
    gp_Ax2 mirrorPlane(p, n);

    // 3. Mirror the Coordinate System (The Placement)
    // This calculates exactly where the origin and the axes go
    gp_Ax3 mirroredAx3 = origAx3;
    mirroredAx3.Mirror(mirrorPlane);

    // 4. Create a CLEAN Rigid Transformation from the mirrored CS
    // SetTransformation(From, To) creates a right-handed move
    gp_Trsf finalTrsf;
    finalTrsf.SetTransformation(mirroredAx3, gp_Ax3()); 
    finalTrsf.Invert(); // Convert from "move origin to mirroredAx3" to "placement"

    // 5. Mirror the GEOMETRY locally
    // We flip the shape at (0,0,0) so it's physically a mirror image
    gp_Trsf localMirror;
    if (x != 0)      localMirror.SetMirror(gp::YOZ());
    else if (y != 0) localMirror.SetMirror(gp_Ax2(gp::Origin(), gp::DY()));
    else             localMirror.SetMirror(gp::XOY());

    BRepBuilderAPI_Transform baker(original->cshape.Located(TopLoc_Location()), localMirror, Standard_True);
    TopoDS_Shape bakedGeom = baker.Shape();

    // 6. Apply placement and build Compound
    current_part->shape = bakedGeom.Located(TopLoc_Location(finalTrsf));
    
    BRep_Builder builder;
    builder.MakeCompound(current_part->cshape);
    builder.Add(current_part->cshape, current_part->shape);

    // Sync Metadata
    current_part->Extrude_val = original->Extrude_val;
    current_part->from_sketch = original->from_sketch;
    current_part->clone_qtd += 1;
});
lua.set_function("Rotatelx", [&](float angleDegrees = 0.0f) {
  // 1. Get the function from the global Lua table (using the sol::state object 'lua')
  sol::protected_function Rotatel = lua["Rotatel"];

  // 2. Call the function with the desired arguments
  // sol::protected_function::operator() returns a sol::protected_function_result
  Rotatel(angleDegrees, 1,0,0);
});
lua.set_function("Rotately", [&](float angleDegrees = 0.0f) { 
  sol::protected_function Rotatel = lua["Rotatel"]; 
  Rotatel(angleDegrees, 0,1,0);
});
lua.set_function("Rotatelz", [&](float angleDegrees = 0.0f) { 
  sol::protected_function Rotatel = lua["Rotatel"]; 
  Rotatel(angleDegrees, 0,0,1);
});
lua.set_function("Copy_placement", sol::protect( [&](OCC_Viewer::luadraw* val) {
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
}));
lua.set_function("Fuse", [&]() {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

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
        throw std::runtime_error(lua_error_with_line(
            L, "Need at least two solids, two wires, or two edges to fuse."));
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
            throw std::runtime_error(lua_error_with_line(
                L, "3D fuse operation failed."));
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
            throw std::runtime_error(lua_error_with_line(L, "Wire 1 is not closed."));
        if (!BRep_Tool::IsClosed(w2))
            throw std::runtime_error(lua_error_with_line(L, "Wire 2 is not closed."));

        TopoDS_Face f1 = BRepBuilderAPI_MakeFace(w1, true);
        TopoDS_Face f2 = BRepBuilderAPI_MakeFace(w2, true);

        BRepAlgoAPI_Fuse fuseOp(f1, f2);
        fuseOp.Build();
        if (!fuseOp.IsDone()) {
            throw std::runtime_error(lua_error_with_line(
                L, "2D fuse operation (wires) failed."));
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
            throw std::runtime_error(lua_error_with_line(
                L, "Failed to build wire from edge 1."));
        TopoDS_Wire w1 = mw1.Wire();

        BRepBuilderAPI_MakeWire mw2;
        mw2.Add(e2);
        if (!mw2.IsDone())
            throw std::runtime_error(lua_error_with_line(
                L, "Failed to build wire from edge 2."));
        TopoDS_Wire w2 = mw2.Wire();

        if (!BRep_Tool::IsClosed(w1))
            throw std::runtime_error(lua_error_with_line(L, "Edge 1 wire is not closed."));
        if (!BRep_Tool::IsClosed(w2))
            throw std::runtime_error(lua_error_with_line(L, "Edge 2 wire is not closed."));

        TopoDS_Face f1 = BRepBuilderAPI_MakeFace(w1, true);
        TopoDS_Face f2 = BRepBuilderAPI_MakeFace(w2, true);

        BRepAlgoAPI_Fuse fuseOp(f1, f2);
        fuseOp.Build();
        if (!fuseOp.IsDone()) {
            throw std::runtime_error(lua_error_with_line(
                L, "2D fuse operation (edges) failed."));
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
    TopoDS_Compound result;
    TopoDS_Builder builder;
    builder.MakeCompound(result);

    for (TopExp_Explorer ex(c, TopAbs_SHAPE); ex.More(); ex.Next()) {
        const TopoDS_Shape& s = ex.Current();

        if (has3D && s.ShapeType() == TopAbs_SOLID) {
            const TopoDS_Solid sol = TopoDS::Solid(s);
            if (sol.IsSame(solids.back()) || sol.IsSame(solids[solids.size() - 2]))
                continue;
        }

        if (has2D_wires && s.ShapeType() == TopAbs_WIRE) {
            const TopoDS_Wire w = TopoDS::Wire(s);
            if (w.IsSame(wires.back()) || w.IsSame(wires[wires.size() - 2]))
                continue;
        }

        if (has2D_edges && s.ShapeType() == TopAbs_EDGE) {
            const TopoDS_Edge e = TopoDS::Edge(s);
            if (e.IsSame(edges.back()) || e.IsSame(edges[edges.size() - 2]))
                continue;
        }

        builder.Add(result, s);
    }

    builder.Add(result, fused);

    current_part->cshape = result;
    current_part->shape  = fused;
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
lua.set_function("DebugShapes", [&]() {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    const TopoDS_Compound& c = current_part->cshape;

    int solids = 0, wires = 0, edges = 0, faces = 0;

    for (TopExp_Explorer ex(c, TopAbs_SOLID); ex.More(); ex.Next()) solids++;
    for (TopExp_Explorer ex(c, TopAbs_WIRE);  ex.More(); ex.Next()) wires++;
    for (TopExp_Explorer ex(c, TopAbs_EDGE);  ex.More(); ex.Next()) edges++;
    for (TopExp_Explorer ex(c, TopAbs_FACE);  ex.More(); ex.Next()) faces++;

    std::cout << "=== DebugShapes ===\n";
    std::cout << "Solids: " << solids << "\n";
    std::cout << "Wires:  " << wires  << "\n";
    std::cout << "Edges:  " << edges  << "\n";
    std::cout << "Faces:  " << faces  << "\n";
});
lua.set_function("Circlev1", [&](float radius) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

    if (radius <= 0)
        luaL_error(lua.lua_state(), "Circle radius must be > 0.");

    // 1. Determine the center point based on current shape location
    // gp_Pnt center(0, 0, 0); // Default origin

    // Get the current transformation of the part
    gp_Trsf partTrsf = current_part->shape.Location().Transformation();

    // Compute the global position of the part's local (0,0,0)
    gp_Pnt center = gp_Pnt(0, 0, 0).Transformed(partTrsf);

    // 2. Define circle in XY plane at that center
    gp_Ax2 axis(center, gp::DZ());  // Normal = Z axis
    gp_Circ circ(axis, radius);

    // 3. Build the geometry
    try {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circ); 
        TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge); 
        TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

        // 4. Add to current part
        current_part->mergeShape(current_part->cshape, face);
    }
    catch (Standard_Failure& e) {
        luaL_error(lua.lua_state(), "Failed to create circle face.");
    }
});
lua.set_function("Circle", [&](float radius, float x, float y) {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

    if (radius <= 0)
        luaL_error(lua.lua_state(), "Circle radius must be > 0.");
current_part->needsplacementupdate = 1;	// verify better why this is needed
				current_part->redisplay();
    // Define center point
    // gp_Pnt center(x, y, 0.0);
	    // Get the current transformation of the part
    gp_Trsf partTrsf = current_part->shape.Location().Transformation();

    // Compute the global position of the part's local (0,0,0)
    gp_Pnt center = gp_Pnt(0, 0, 0).Transformed(partTrsf);

// 3. Compute the global direction of the local Z-axis
// This ensures that if the part was mirrored/flipped, the circle flips too!
gp_Dir localZ = gp::DZ().Transformed(partTrsf);

// 4. Define circle using the dynamic direction
gp_Ax2 axis(center, localZ);
    // // Define circle in XY plane
    // gp_Ax2 axis(center, gp::DZ());  // Normal = Z axis

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
lua.set_function("Dup", [&](){
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");

    TopoDS_Shape baseShape = current_part->shape;
    if (baseShape.IsNull())
        luaL_error(lua.lua_state(), "Current part has no shape.");
	current_part->mergeShape(current_part->cshape, baseShape);
});
lua.set_function("Subtract", [&]() {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

    TopoDS_Compound& compound = current_part->cshape;
    if (compound.IsNull()) {
        throw std::runtime_error("Current part's compound is null.");
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

    bool is3D = !solids.empty();
    bool is2D = !is3D && !faces.empty();

    size_t count = is3D ? solids.size() : faces.size();
    if (count < 2) {
        throw std::runtime_error("Subtract requires at least two objects (solids or faces).");
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
        throw std::runtime_error("Boolean subtract (cut) failed.");
    }

    TopoDS_Shape result = cutOp.Shape();
    if (result.IsNull()) {
        throw std::runtime_error("Boolean subtract produced null shape.");
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

    // Add all except the last two
    for (size_t i = 0; i < shapeVec.size() - 2; ++i) {
        builder.Add(newCompound, shapeVec[i]);
    } 

    // Add the result
    builder.Add(newCompound, result);

    // Update current part
    current_part->cshape = newCompound;
    current_part->shape  = result;  // last result is the "active" shape
});
lua.set_function("Common", [&]() {
    if (!current_part)
        luaL_error(lua.lua_state(), "No current part. Call Part(name) first.");

    TopoDS_Compound& compound = current_part->cshape;
    if (compound.IsNull()) {
        throw std::runtime_error("Current part's compound is null.");
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

    bool is3D = !solids.empty();
    bool is2D = !is3D && !faces.empty();

    size_t count = is3D ? solids.size() : faces.size();
    if (count < 2) {
        throw std::runtime_error("Common requires at least two objects (solids or faces).");
    }

    TopoDS_Shape object;
    TopoDS_Shape tool;

    if (is3D) {
        object = solids[solids.size() - 2];
        tool   = solids.back();
    } else {
        object = faces[faces.size() - 2];
        tool   = faces.back();
    }

    // Boolean common (intersection)
    BRepAlgoAPI_Common commonOp(object, tool);

    // Same fuzzy tolerance as Subtract for numerical robustness
    commonOp.SetFuzzyValue(1e-6);

    commonOp.Build();

    if (!commonOp.IsDone()) {
        throw std::runtime_error("Boolean common (intersection) failed.");
    }

    TopoDS_Shape result = commonOp.Shape();
    if (result.IsNull()) {
        throw std::runtime_error("Boolean common produced null shape.");
    }

    // Cleanup: unify coincident edges/vertices
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

    // Add all except the last two
    for (size_t i = 0; i < shapeVec.size() - 2; ++i) {
        builder.Add(newCompound, shapeVec[i]);
    }

    // Add the result
    builder.Add(newCompound, result);

    // Update current part
    current_part->cshape = newCompound;
    current_part->shape  = result;
}); 
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
lua.set_function("FilletToAllEdges_v0", [&](double val)
{

    if (!current_part)
        luaL_error(lua.lua_state(), "No current part.");
	if(!current_part->occv->show_fillets)return;

    current_part->ttfillet = 1;

    ReplaceShapeInCompound(
        current_part->cshape,
        current_part->shape,
        [&](const TopoDS_Shape& s)
        {
            TopoDS_Shape clean = CleanShape(s);

            BRepFilletAPI_MakeFillet fillet(clean);

            // add all edges with same radius
            for (TopExp_Explorer exp(clean, TopAbs_EDGE); exp.More(); exp.Next())
            {
                TopoDS_Edge e = TopoDS::Edge(exp.Current());
                fillet.Add(val, e);
            }

            fillet.Build();

            if (!fillet.IsDone())
                return s;

            TopoDS_Shape result = fillet.Shape();

            // remove internal seam edges between tangent faces
            ShapeUpgrade_UnifySameDomain unify(result);
            unify.Build();
            result = unify.Shape();

            // fix tolerances and small geometry issues
            ShapeFix_Shape fix(result);
            fix.Perform();
            result = fix.Shape();

            // refine triangulation (important for smooth display)
            BRepMesh_IncrementalMesh(result, 0.05);

            return FixShape(result);
        }
    );

    // if (current_part->ashape)
    //     current_part->ashape->Attributes()->SetFaceBoundaryDraw(false);
});
}
//region lua
nmutex lua_mtx("lua_mtx", 1);
void luainit() {
    if (G){
		G.reset();
	} 
    // if (G) return;
    if (!occv) cotm("occv not init");

    G = std::make_unique<sol::state>();
    auto& lua = *G;
    // lua.open_libraries(sol::lib::base, sol::lib::package);
	lua.open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::math,
		sol::lib::table,
		sol::lib::string
	);
    lua["package"]["path"] = std::string("./lua/?.lua;") + std::string(lua["package"]["path"]);
    L = lua.lua_state();

    bind_luadraw(lua, occv); 
    install_shorthand_searcher(lua.lua_state());
}
void clearAll(OCC_Viewer* occv){ 
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
	occv->vshapes.clear();
	occv->vaShape.clear();
	occv->ulua.clear();
	occv->vlua.clear();
 
	occv->Origin = TopLoc_Location(); //reset 
}
void lua_str(const string &str, bool isfile) { 
        lua_mtx.lock(); 
		perf();
        luainit();

		clearAll(occv); 
		
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
			// cotm(code.data())
            status = luaL_loadbuffer(L, code.data(), code.size(), str.c_str());
			// auto result = G->safe_script(code, sol::script_pass_on_error);
        } else {
            code = translate_shorthand(str);
			// code+="\nrobot1()";
            status = luaL_loadbuffer(L, code.data(), code.size(), "chunk");
        }

        if (status == LUA_OK) {
            if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
                std::cerr << "runtimer error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1); 
            }
			perf("lua");

            Fl::awake(fillbrowser);
 
        } else {
            std::cerr << "Load error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
        lua_mtx.unlock(); 
}
void lua_str_realtime(const string &str){ 
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
			occv->m_view->FitAll();
			occv->redraw();
		},occv, 0);


	menu->add(
		"View/Show fillets", FL_ALT + 'i',
		[](Fl_Widget* mnu, void* ud) {
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();	// This gets the actually clicked item
			
			if (!item->value()) { 
				occv->show_fillets=0;
			} else {
				occv->show_fillets=1;
			}
			lua_str(currfilename,1); 
			occv->redraw(); 
		},
		0, FL_MENU_TOGGLE);

	menu->add(
		"View/Animation", FL_ALT + 'a',
		[](Fl_Widget* mnu, void* ud) {
			occv->start_continuous_rotation();
		},occv, 0);

	menu->add(
		"View/Transparent", FL_ALT + 't',
		[](Fl_Widget* mnu, void* ud) {
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();	// This gets the actually clicked item
			//  occv->fillvectopo();
			if (!item->value()) {
				occv->toggle_shaded_transp(AIS_WireFrame);
			} else {
				occv->toggle_shaded_transp(AIS_Shaded);
			}
			occv->redraw(); 
		},
		0, FL_MENU_TOGGLE);

	menu->add(
		"View/Nice", FL_ALT + 'n',
		[](Fl_Widget* mnu, void* ud) {
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();	// This gets the actually clicked item
			// NiceSteelStandard(occv->m_view, occv->vaShape );
			NiceSteel(occv->m_view, occv->vaShape, item->value());
			occv->redraw(); 
		},
		0, FL_MENU_TOGGLE);





	menu->add(
		"Tools/List of profiles with length and quantity", 0,
		[](Fl_Widget*, void* ud) {
			OCC_Viewer* v = (OCC_Viewer*)(ud);

			stringstream strm;
			// string nm = "sketch_profile";

			strm << "<html><body>";

			for (int ji = 0; ji < occv->vlua.size(); ++ji) {
				auto jo = occv->vlua[ji];
				if (!Contains(jo->name, "sketch")) continue;

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

				struct EntryAgg {
					std::string first_name;
					int total_qtty = 0;
				};

				std::unordered_map<float, EntryAgg> agg;

				for (int i = 0; i < occv->vlua.size(); ++i) {
					auto o = occv->vlua[i];
					cotm(o->from_sketch,nm);
					if (o->from_sketch != nm) continue;
					if (!o->visible_hardcoded) continue;

					float val = std::abs(o->Extrude_val);
					int qtty = o->clone_qtd;

					auto& slot = agg[val];
					// cotm2(o->from_sketch, o->name)
					if (slot.total_qtty == 0) {
						slot.first_name = o->name;
					}
					slot.total_qtty += qtty;
				}

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

			for (int i = 0; i < v->vaShape.size(); i++) {
				if (!v->m_context->IsDisplayed(v->vaShape[i]) || !v->vlua[i]->visible_hardcoded) continue;

				TopoDS_Shape s = v->vlua[i]->cshape;
				// TopoDS_Shape s = v->vaShape[i]->Shape();
				if (s.IsNull()) continue;

				if (!v->vlua[i]->Origin.IsIdentity()) {
					s = s.Located(TopLoc_Location(v->vlua[i]->Origin.Transformation()));
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

			for (int i = 0; i < v->vaShape.size(); i++) {
				if (!v->m_context->IsDisplayed(v->vaShape[i]) || !v->vlua[i]->visible_hardcoded) continue;

				TopoDS_Shape s = v->vlua[i]->cshape;
				if (s.IsNull()) continue;

				// Apply origin transform if needed
				if (!v->vlua[i]->Origin.IsIdentity()) {
					s = s.Located(TopLoc_Location(v->vlua[i]->Origin.Transformation()));
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


	
	menu->add(
		"Options/Tune view ", 0,
		[](Fl_Widget* mnu, void* ud) {
			floccv=occv;
			Fl_Menu_* menu = static_cast<Fl_Menu_*>(mnu);
			const Fl_Menu_Item* item = menu->mvalue();
			flshapes=occv->vaShape;
			m_view=occv->m_view;
			slidercfg(); 
		},
		0, 0);

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
 
	win->position(0, 0);  
	// int x, y, _w, _h;
	// Fl::screen_work_area(x, y, _w, _h);
	// win->resize(x, y+22, _w, _h-22); 
	// win->maximize();
	win->show();  
	Fl::flush();  // make sure everything gets drawn
	win->flush();	 
	woccbtn->flush(); 

	Fl::add_timeout(0.7,[](void* d) {lua_str(currfilename,1);},0);				
	Fl::add_timeout(0.9,[](void* d) {occv->m_view->FitAll();},0);				
	Fl::add_timeout(1.5,[](void* d) {occv->sbt[6].occbtn->do_callback(); },0);				
	Fl::add_timeout(2.4,[](void* d) {occv->m_view->FitAll(); },0);		 
	return Fl::run();
}