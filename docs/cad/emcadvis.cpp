#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>

#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
#endif

// --- VARIÁVEIS GLOBAIS ---
Model model = { 0 };
Shader customShader = { 0 };
Camera camera = { 0 };

// Estado da Câmera estilo Órbita CAD
float cameraAngleX = 45.0f;
float cameraAngleY = 30.0f;
float cameraDistance = 40.0f;
Vector3 cameraTarget = { 0.0f, 0.0f, 0.0f };

Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };
float modelScale = 1.0f;
bool modelLoaded = false;

// Shaders GLSL ES 100 para iluminação de estúdio 3D
const char* vsSource = R"(
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

varying vec3 fragNormal;

uniform mat4 mvp;
uniform mat4 matModel;

void main() {
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

const char* fsSource = R"(
precision mediump float;

varying vec3 fragNormal;
uniform vec4 colDiffuse;

void main() {
    vec3 normal = normalize(fragNormal);
    
    vec3 lightDir1 = normalize(vec3(0.5, 1.0, 0.8));
    vec3 lightDir2 = normalize(vec3(-0.6, -0.4, -0.5));
    
    float diff1 = max(dot(normal, lightDir1), 0.0);
    float diff2 = max(dot(normal, lightDir2), 0.0) * 0.35;
    
    vec3 ambient = vec3(0.22, 0.25, 0.30);
    vec3 lightColor = ambient + vec3(0.85) * diff1 + vec3(0.4) * diff2;
    
    gl_FragColor = vec4(colDiffuse.rgb * lightColor, colDiffuse.a);
}
)";

// --- CARREGADOR BINÁRIO STL ---
Model LoadSTLBinary(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return { 0 };

    char header[80];
    file.read(header, 80);

    uint32_t numTriangles = 0;
    file.read(reinterpret_cast<char*>(&numTriangles), 4);

    if (numTriangles == 0) return { 0 };

    Mesh mesh = { 0 };
    mesh.triangleCount = numTriangles;
    mesh.vertexCount = numTriangles * 3;

    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals  = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    for (uint32_t i = 0; i < numTriangles; i++) {
        float normal[3];
        file.read(reinterpret_cast<char*>(normal), 12);

        float v0[3], v1[3], v2[3];
        file.read(reinterpret_cast<char*>(v0), 12);
        file.read(reinterpret_cast<char*>(v1), 12);
        file.read(reinterpret_cast<char*>(v2), 12);

        if (normal[0] == 0.0f && normal[1] == 0.0f && normal[2] == 0.0f) {
            Vector3 vec0 = { v0[0], v0[1], v0[2] };
            Vector3 vec1 = { v1[0], v1[1], v1[2] };
            Vector3 vec2 = { v2[0], v2[1], v2[2] };
            Vector3 e1 = Vector3Subtract(vec1, vec0);
            Vector3 e2 = Vector3Subtract(vec2, vec0);
            Vector3 fn = Vector3Normalize(Vector3CrossProduct(e1, e2));
            normal[0] = fn.x; normal[1] = fn.y; normal[2] = fn.z;
        }

        float verts[3][3] = {
            { v0[0], v0[1], v0[2] },
            { v1[0], v1[1], v1[2] },
            { v2[0], v2[1], v2[2] }
        };

        for (int v = 0; v < 3; v++) {
            int idx = (i * 3 + v) * 3;
            mesh.vertices[idx + 0] = verts[v][0];
            mesh.vertices[idx + 1] = verts[v][1];
            mesh.vertices[idx + 2] = verts[v][2];

            mesh.normals[idx + 0] = normal[0];
            mesh.normals[idx + 1] = normal[1];
            mesh.normals[idx + 2] = normal[2];
        }

        uint16_t attr;
        file.read(reinterpret_cast<char*>(&attr), 2);
    }

    UploadMesh(&mesh, false);
    Model m = LoadModelFromMesh(mesh);
    m.materials[0].shader = customShader;
    return m;
}

void SetupLoadedModel(const char* filepath) {
    if (modelLoaded) {
        UnloadModel(model);
        modelLoaded = false;
    }

    model = LoadSTLBinary(filepath);

    if (model.meshCount > 0) {
        modelLoaded = true;
        BoundingBox bounds = GetModelBoundingBox(model);

        Vector3 size = {
            bounds.max.x - bounds.min.x,
            bounds.max.y - bounds.min.y,
            bounds.max.z - bounds.min.z
        };

        float maxDim = fmaxf(size.x, fmaxf(size.y, size.z));
        if (maxDim > 0.0f) modelScale = 20.0f / maxDim;

        Vector3 center = {
            (bounds.min.x + bounds.max.x) / 2.0f,
            (bounds.min.y + bounds.max.y) / 2.0f,
            (bounds.min.z + bounds.max.z) / 2.0f
        };

        modelPosition = (Vector3){ -center.x * modelScale, -center.y * modelScale, -center.z * modelScale };
        cameraTarget = (Vector3){ 0.0f, 0.0f, 0.0f };
        cameraDistance = 35.0f;
    }
}

// --- CONTROLE DE CÂMERA ÓRBITA CAD SUAVE ---
void UpdateOrbitCamera() {
    Vector2 currentMousePos = GetMousePosition();
    static Vector2 previousMousePos = currentMousePos;
    
    Vector2 mouseDelta = { 
        currentMousePos.x - previousMousePos.x, 
        currentMousePos.y - previousMousePos.y 
    };
    previousMousePos = currentMousePos;

    // Zoom com scroll
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        cameraDistance -= wheel * (cameraDistance * 0.1f);
        if (cameraDistance < 1.0f) cameraDistance = 1.0f;
        if (cameraDistance > 500.0f) cameraDistance = 500.0f;
    }

    // Órbita (Botão Esquerdo)
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float sensitivity = 0.3f;
        cameraAngleX -= mouseDelta.x * sensitivity;
        cameraAngleY -= mouseDelta.y * sensitivity;

        if (cameraAngleY > 89.0f) cameraAngleY = 89.0f;
        if (cameraAngleY < -89.0f) cameraAngleY = -89.0f;
    }

    // Pan / Mover (Botão Direito ou Meio)
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

        float panSpeed = cameraDistance * 0.0015f;
        cameraTarget = Vector3Add(cameraTarget, Vector3Scale(right, -mouseDelta.x * panSpeed));
        cameraTarget = Vector3Add(cameraTarget, Vector3Scale(up, mouseDelta.y * panSpeed));
    }

    float radX = cameraAngleX * DEG2RAD;
    float radY = cameraAngleY * DEG2RAD;

    camera.position.x = cameraTarget.x + cameraDistance * cosf(radY) * sinf(radX);
    camera.position.y = cameraTarget.y + cameraDistance * sinf(radY);
    camera.position.z = cameraTarget.z + cameraDistance * cosf(radY) * cosf(radX);
    camera.target = cameraTarget;
}

// --- RENDER LOOP ---
void UpdateDrawFrame() {
    // Permite arrastar novos arquivos STL se desejar
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".stl")) {
            SetupLoadedModel(droppedFiles.paths[0]);
        }
        UnloadDroppedFiles(droppedFiles);
    }

    UpdateOrbitCamera();

    BeginDrawing();
        ClearBackground((Color){ 24, 26, 30, 255 }); // Fundo estúdio limpo

        BeginMode3D(camera);
            if (modelLoaded) {
                DrawModel(model, modelPosition, modelScale, (Color){ 80, 160, 230, 255 });
            }
        EndMode3D();
    EndDrawing();
}

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <fstream>
#include <cstdint>

#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
#endif

// ... [Manter variáveis globais, LoadSTLBinary e SetupLoadedModel iguais] ...

// Callback executado quando o download do test.stl via HTTP terminar
void OnSTLLoaded(const char* filename) {
    std::cout << "SUCCESS: " << filename << " baixado via HTTP!" << std::endl;
    SetupLoadedModel(filename);
}

void OnSTLError(const char* filename) {
    std::cout << "ERRO: Nao foi possivel baixar " << filename << " do servidor HTTP." << std::endl;
}

int main() {
    InitWindow(800, 600, "WebCAD STL Viewer");

    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    customShader = LoadShaderFromMemory(vsSource, fsSource);

#if defined(__EMSCRIPTEN__)
    // Solicita o download assíncrono do test.stl direto do seu servidor local (npx http-server)
    emscripten_async_wget("https://missionsave.github.io/cad/test.stl", "test.stl", OnSTLLoaded, OnSTLError);
    
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    if (FileExists("test.stl")) {
        SetupLoadedModel("test.stl");
    }

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    if (modelLoaded) UnloadModel(model);
    UnloadShader(customShader);
    CloseWindow();
    return 0;
}