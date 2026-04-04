#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.h>
#include <shader.h>
#include <model.h>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <files.h>
#include <light_settings.h>


void writeScreenShot(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;
static float rotation = 0.0f;
bool cursorEnabled = true;
bool useBanding = false;
int bandLevels = 3;
static int current_model = 0;
static std::vector<std::string> models;
glm::vec4 bgColor = glm::vec4(0.427f, 0.506f, 0.588f, 1.0f);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec4 rotatedPos(0.0f, 0.0f, 0.0f, 0.0f);
ImVec4 ambientLightColor = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
/*struct LightSettings {
    bool isEnabled = true;
    float rotationY = 0.0f;  
    float rotationX = 0.0f;  
    float distance  = 3.0f;  // how far from the model
    glm::vec3 diffuse  = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
};*/

struct LightPreset {
    std::string name;
    std::vector<LightSettings> lights;
};

std::vector<LightPreset> presets = {
    {
        "Rembrandt", {
            { true, glm::radians(45.0f),  glm::radians(30.0f), 3.0f, glm::vec3(0.5f, 0.45f, 0.4f), glm::vec3(1.0f), 1.0f },
        }
    },
    {
        "Three Point", {
            { true, glm::radians(45.0f),  glm::radians(30.0f), 3.0f, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f), 1.0f  }, // key
            { true, glm::radians(-45.0f), glm::radians(15.0f), 4.0f, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f), 0.5f  }, // fill
            { true, glm::radians(180.0f), glm::radians(20.0f), 3.0f, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f), 0.75f }, // rim
        }
    },
    {
        "Top Down", {
            { true, glm::radians(0.0f), glm::radians(90.0f), 3.0f, glm::vec3(0.5f), glm::vec3(1.0f), 1.0f },
        }
    },
    {
        "Split", {
            { true, glm::radians(90.0f),  glm::radians(30.0f), 3.0f, glm::vec3(1.0f), glm::vec3(1.0f), 1.0f },
            { true, glm::radians(-90.0f), glm::radians(30.0f), 3.0f, glm::vec3(1.0f), glm::vec3(1.0f), 1.0f },
        }
    },
};

glm::vec3 getLightPosition(const LightSettings& light);
std::vector<LightSettings> lights;

// Grid Lines
bool showGrid = false;
glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH,
                             0.0f, (float)SCR_HEIGHT);
std::vector<float> gridLines = {
    // vertical lines
    SCR_WIDTH/3.0f, 0.0f,
    SCR_WIDTH/3.0f, SCR_HEIGHT,

    2*SCR_WIDTH/3.0f, 0.0f,
    2*SCR_WIDTH/3.0f, SCR_HEIGHT,

    // horizontal lines
    0.0f, SCR_HEIGHT/3.0f,
    SCR_WIDTH, SCR_HEIGHT/3.0f,

    0.0f, 2*SCR_HEIGHT/3.0f,
    SCR_WIDTH, 2*SCR_HEIGHT/3.0f
};
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Painter's Book", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    stbi_set_flip_vertically_on_load(true);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("../shaders/colors.vs", "../shaders/colors.fs");
    Shader lightCubeShader("../shaders/light_cube.vs", "../shaders/light_cube.fs");
    Shader shaderGrid("../shaders/grid.vs", "../shaders/grid.fs");

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};
    unsigned int VBO, cubeVAO, gridVAO, gridVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridLines.size() * sizeof(float), &gridLines[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    models = loadModelFiles("../resources");

    Model ourModel("../resources/AsaroHead.obj");

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // lock to the right side
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 300, 0));
    ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y));

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    {
        ImGuiStyle& style = ImGui::GetStyle();

        // rounding
        style.WindowRounding = 8.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.TabRounding = 4.0f;

        // padding and spacing
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(8, 4);
        style.ItemSpacing = ImVec2(8, 6);

        // colors - dark warm theme, friendlier for artists
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]         = ImVec4(0.13f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_FrameBg]          = ImVec4(0.20f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.30f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_TitleBgActive]    = ImVec4(0.25f, 0.22f, 0.22f, 1.00f);
        colors[ImGuiCol_Button]           = ImVec4(0.35f, 0.28f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonHovered]    = ImVec4(0.45f, 0.36f, 0.36f, 1.00f);
        colors[ImGuiCol_SliderGrab]       = ImVec4(0.70f, 0.45f, 0.35f, 1.00f);
        colors[ImGuiCol_CheckMark]        = ImVec4(0.70f, 0.45f, 0.35f, 1.00f);
        colors[ImGuiCol_Header]           = ImVec4(0.35f, 0.28f, 0.28f, 1.00f);
        colors[ImGuiCol_HeaderHovered]    = ImVec4(0.45f, 0.36f, 0.36f, 1.00f);
    }

    //push 1 main light to the scene
    lights.push_back(LightSettings());
    lights[0].rotationY = glm::radians(45.0f);  lights[0].rotationX = glm::radians(30.0f);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        // -----
        processInput(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render
        // ------
        glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setInt("numLights", lights.size());

        for (int i = 0; i < lights.size(); i++)
        {
            string name = "lights[" + std::to_string(i) + "]";
            glm::vec3 position = getLightPosition(lights[i]);
            glm::vec3 diffuse = lights[i].diffuse;
            glm::vec3 specular = lights[i].specular;
            float intensity = lights[i].intensity;
            lightingShader.setBool(name + ".isEnabled", lights[i].isEnabled);
            lightingShader.setVec3(name + ".position", position);
            lightingShader.setVec3(name + ".diffuse", diffuse);
            lightingShader.setVec3(name + ".specular", specular);
            lightingShader.setVec3(name + ".ambient", 0.1f, 0.1f, 0.1f);
        }

        lightingShader.setVec3("viewPos", camera.Position); 
        lightingShader.setBool("useBanding", useBanding);
        lightingShader.setInt("bandLevels", bandLevels);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        lightingShader.setMat4("model", model);
        ourModel.Draw(lightingShader); 

        for (int i = 0; i < lights.size(); i++) {
            if (!lights[i].isEnabled){
                continue;
            }
            glm::vec3 position = getLightPosition(lights[i]);
        
            lightCubeShader.use();
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);
        
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::scale(model, glm::vec3(0.2f));
            lightCubeShader.setMat4("model", model);
        
            glBindVertexArray(lightCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (showGrid) {
            glDisable(GL_DEPTH_TEST); // overlay
            shaderGrid.use();
            shaderGrid.setMat4("projection", ortho);
            shaderGrid.setVec3("gridColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white lines

            glBindVertexArray(gridVAO);
            glDrawArrays(GL_LINES, 0, gridLines.size()/2);
            glEnable(GL_DEPTH_TEST);
        }
        
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Painter's Book", nullptr,
                            ImGuiWindowFlags_NoResize   |
                            ImGuiWindowFlags_NoMove     |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoBringToFrontOnFocus
            );                         
            if (!cursorEnabled){
                ImGui::Text("Light Mode");
            }else{
                ImGui::Text("Camera Mode");
            }
            ImGui::Text("Model Settings");
            if (ImGui::BeginCombo("Select Model", models[current_model].c_str())) {
                for (int n = 0; n < models.size(); n++) {
                    bool is_selected = (current_model == n);
                    if (ImGui::Selectable(models[n].c_str(), is_selected)) {
                        current_model = n;
                        ourModel = Model(models[current_model]);
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SliderFloat("model rotation", &rotation, 0.0f, 2.0f * M_PI);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::Separator();
            ImGui::Text("Light Settings");
            if (ImGui::Button("Add Light") && lights.size() < 8) {
                lights.push_back(LightSettings());
            }
            for (int i = 0; i < lights.size(); i++) {
                ImGui::PushID(i); // critical - makes each section unique

                std::string header = "Light " + std::to_string(i);
                if (ImGui::CollapsingHeader(header.c_str())) {
                    ImGui::Checkbox("Enabled", &lights[i].isEnabled);

                    ImGui::SliderFloat("Horizontal", &lights[i].rotationY, 0.0f, 2.0f * M_PI);
                    ImGui::SliderFloat("Elevation",  &lights[i].rotationX, -M_PI * 0.5f, M_PI * 0.5f);
                    ImGui::SliderFloat("Distance",   &lights[i].distance,  0.5f, 10.0f);

                    ImGui::ColorEdit3("Diffuse",  glm::value_ptr(lights[i].diffuse));
                    ImGui::ColorEdit3("Specular", glm::value_ptr(lights[i].specular));
                    ImGui::SliderFloat("Intensity", &lights[i].intensity, 0.0f, 2.0f);

                    if (ImGui::Button("Key"))  { lights[i].rotationY = glm::radians(45.0f);  lights[i].rotationX = glm::radians(30.0f); }
                    ImGui::SameLine();
                    if (ImGui::Button("Fill")) { lights[i].rotationY = glm::radians(-45.0f); lights[i].rotationX = glm::radians(15.0f); }
                    ImGui::SameLine();
                    if (ImGui::Button("Rim"))  { lights[i].rotationY = glm::radians(180.0f); lights[i].rotationX = glm::radians(20.0f); }
                
                    if (ImGui::Button("Remove")) {
                        lights.erase(lights.begin() + i);
                        i--; // step back so we don't skip the next light
                    }
                }
            
                ImGui::PopID();
            }

            ImGui::Text("Lighting Presets");
            for (int i = 0; i < presets.size(); i++) {
                if (ImGui::Button(presets[i].name.c_str())) {
                    lights = presets[i].lights; // replace current lights with preset
                }
                if (i < presets.size() - 1) ImGui::SameLine();
            }
            ImGui::Separator();

            ImGui::Text("Banding Settings");
            ImGui::SliderInt("Band Levels", &bandLevels, 1, 6);
            if (ImGui::Button("Toggle Banding Mode")) {
                useBanding = !useBanding;
            }
            ImGui::Text("Misc");
            if (ImGui::Button("Toggle Grid")) {
                showGrid = !showGrid;
            }
            if (ImGui::Button("Take Screenshot")) {
                writeScreenShot(window);
            }
            ImGui::ColorEdit4("Background Color",glm::value_ptr(bgColor));
            if (ImGui::Button("Reset BG Color")){
                bgColor = glm::vec4(0.427f, 0.506f, 0.588f, 1.0f);
            }
            
            ImGui::Separator();
            ImGui::Text("Save / Load");

            static char filename[64] = "my_setup";
            ImGui::InputText("##filename", filename, sizeof(filename));

            if (ImGui::Button("Save")) {
                saveLightSetup(lights, std::string(filename) + ".json");
            }
            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                std::vector<LightSettings> loaded = loadLightSetup(std::string(filename) + ".json");
                if (!loaded.empty()) lights = loaded;
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
    if (cursorEnabled){
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (cursorEnabled){
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_E && action == GLFW_PRESS){
        cursorEnabled = !cursorEnabled;
        glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void writeScreenShot(GLFWwindow* window) {
    // Get the current framebuffer size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Create a buffer to hold the pixel data
    std::vector<unsigned char> pixels(width * height * 3); // 3 channels (RGB)

    // Read the pixels from the framebuffer
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip the image vertically (OpenGL's origin is bottom-left)
    std::vector<unsigned char> flippedPixels(width * height * 3);
    for (int y = 0; y < height; ++y) {
        std::copy(pixels.begin() + y * width * 3,
                  pixels.begin() + (y + 1) * width * 3,
                  flippedPixels.begin() + (height - 1 - y) * width * 3);
    }

    // Save the image using stb_image_write
    stbi_write_png("screenshot.png", width, height, 3, flippedPixels.data(), width * 3);
}

glm::vec3 getLightPosition(const LightSettings& light) {
    float x = light.distance * cos(light.rotationX) * sin(light.rotationY);
    float y = light.distance * sin(light.rotationX);
    float z = light.distance * cos(light.rotationX) * cos(light.rotationY);
    return glm::vec3(x, y, z);
}