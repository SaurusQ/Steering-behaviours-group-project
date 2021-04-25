#include "mainConfig.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "3D/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <math.h>
#include <thread>
#include <mutex>
#include <algorithm>

#include "3D/camera.hpp"
#include "3D/shader.hpp"

#include "core/boid.hpp"
#include "core/world.hpp"



extern void mainCore(World &gameWorld); //Form core.hpp
extern void consoleCore(World &gameWorld);
extern float worldSize;

//Funcion prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);

float worldSize = 50.0f;

#define BOX_T 0.02f //Box thicness

//Calculations for the base model
#define BOID_WIDE 0.4f
#define BOID_LEN 1.0f
const float cz = BOID_WIDE;
const float cy = 0.0f;
const float bz = BOID_WIDE * cos( glm::radians(120.0f) );
const float by = BOID_WIDE * sin( glm::radians(120.0f) );
const float az = BOID_WIDE * cos( glm::radians(240.0f) );
const float ay = BOID_WIDE * sin( glm::radians(240.0f) );
const float trix = -BOID_LEN/2.0f;
const float nose = BOID_LEN/2.0f;

int              g_srceenWitdth = 800;
unsigned int     g_screenHeight = 600;
float            g_deltaTime = 0;

//Camera
static Cam::Camera *camera; //TODO smart pointer

static bool firstMouse = true;
static float lastX =  (float)g_srceenWitdth / 2.0f;
static float lastY =  (float)g_screenHeight / 2.0f;
static float lastFrame = 0.0f; // Time of last frame

std::mutex g_pause; //Mutex to halt coreThread

int main() {
    //Generate wordl
    World gameWorld(worldSize);

    //Generate core thread
    std::thread coreThread(mainCore, std::ref(gameWorld));

    //Create window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Version numbers
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Don't use backwards-compatible features
    GLFWwindow* window = glfwCreateWindow(g_srceenWitdth, g_screenHeight, "Steering Behaviours 3D", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    //Set
    glViewport(0, 0, g_srceenWitdth, g_screenHeight);//Create rendering window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //GLFW to capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Create camera
    camera = new Cam::FPS(); //Start with FPS type camera

    //Generate shader from shader files
    std::string root = __FILE__;
    std::replace(root.begin(), root.end(), '\\', '/');
    char c = ' ';
    while(c != '/') {
        c = root.back();
        root.pop_back();
    }
    std::string bv = root + "/3D/shaders/basic.vert";
    std::string bf = root + "/3D/shaders/basic.frag";
    std::string lv = root + "/3D/shaders/lightSrc.vert";
    std::string lf = root + "/3D/shaders/lightSrc.frag";
    std::string cv = root + "/3D/shaders/bound.vert";
    std::string cf = root + "/3D/shaders/bound.frag";
    std::string sv = root + "/3D/shaders/cubemap.vert";
    std::string sf = root + "/3D/shaders/cubemap.frag";
    
    std::cout << root << std::endl;
    Shader basicShader(bv.c_str(), bf.c_str());
    //Shader lightShader(lv.c_str(), lf.c_str());
    //Shader boundShader(cv.c_str(), cf.c_str());
    Shader skyboxShader(sv.c_str(), sf.c_str());

    //Set up vertex data
    float boidVert[] = {
        trix, cy, cz,  -1.0f, 0.0f,  0.0f,
        trix, by, bz,  -1.0f, 0.0f,  0.0f,
        trix, ay, az,  -1.0f, 0.0f,  0.0f,

        nose, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        trix, cy, cz,      -1.0f, 0.0f,  0.0f,
        trix, by, bz,      -1.0f, 0.0f,  0.0f,

        nose, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        trix, by, bz,      -1.0f, 0.0f,  0.0f,
        trix, ay, az,      -1.0f, 0.0f,  0.0f,
        
        nose, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        trix, cy, cz,      -1.0f, 0.0f,  0.0f,
        trix, ay, az,      -1.0f, 0.0f,  0.0f
    };

    float boundVert[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  -1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  -1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  -1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  -1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  -1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  -1.0f,  0.0f
    };

    float boxVert[] = {
        -0.5f, -0.5f, -0.5f,         0.0f, 1.0f, 0.0f,
        -0.5f + BOX_T, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f + BOX_T, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f + BOX_T, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,          0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,         0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f + BOX_T, -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f + BOX_T, 0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f + BOX_T, 0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f
    };

    //Calculate rorations for world bounds
    glm::quat quatRot = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4x4 rot1 = glm::mat4_cast(quatRot);
    quatRot = glm::angleAxis(glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4x4 rot2 = glm::mat4_cast(quatRot);
    quatRot = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4x4 rot3 = glm::mat4_cast(quatRot);

    //Calculate normal vectors
    for(int i = 0; i < sizeof(boidVert) / sizeof(boidVert[0]);) {
        glm::vec3 v1(boidVert[i], boidVert[i + 1], boidVert[i + 2]);
        i += 6;
        glm::vec3 v2(boidVert[i], boidVert[i + 1], boidVert[i + 2]);
        i += 6;
        glm::vec3 v3(boidVert[i], boidVert[i + 1], boidVert[i + 2]);
        i += 6;
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v2, v1 - v3));
        if(normal[0] < 0 && !(i == 3 * 6))
            normal = -normal;
        for(int j = i - (3 * 6); j < i; j += 6) {
            boidVert[j + 3] = normal.x;
            boidVert[j + 4] = normal.y;
            boidVert[j + 5] = normal.z;
        }
    }

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    //Boid
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boidVert), boidVert, GL_STATIC_DRAW);
    //Vertex attributes
    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Normal vector
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Box
    unsigned int boxVAO, boxVBO;
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);
    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVert), boxVert, GL_STATIC_DRAW);
    //Vertex attributes
    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Normal vector
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //World bounds
    unsigned int boundVBO, boundVAO;
    glGenVertexArrays(1, &boundVAO);
    glGenBuffers(1, &boundVBO);
    glBindVertexArray(boundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundVert), boundVert, GL_STATIC_DRAW);
    //Vertex attributes
    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Normal vector
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //Load textures
    std::vector<std::string> faces
    {
        root + "/3D/textures/right.jpg",
        root + "/3D/textures/left.jpg",
        root + "/3D/textures/top.jpg",
        root + "/3D/textures/bot.jpg",
        root + "/3D/textures/front.jpg",
        root + "/3D/textures/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.set("skybox", 0);

    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightDirection(0.5f, 0.5f, 0.5f);
    glm::vec3 boundColor(0.6f, 0.6f, 1.0f);
    glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse(0.5f, 0.5f, 0.5f);
    glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);
    glm::vec3 matSpecular(0.5f, 0.5f, 0.5f);

    //Create transformations
    glm::mat4 model         = glm::mat4(1.0f); //Initialize matrises to identity matrix
    glm::mat4 view          = glm::mat4(1.0f);
    glm::mat4 projection    = glm::mat4(1.0f);

    //Start console thread
    std::thread consoleThread(consoleCore, std::ref(gameWorld));

    //Render loop
    while(!glfwWindowShouldClose(window)) {
        //per frame logic
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        camera->setDeltaTime(deltaTime); //Camera needs information about deltatime

        //Input
        processInput(window);

        //Clear color buffer The possible bits we can set are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT and GL_STENCIL_BUFFER_BIT
        glDepthMask(GL_TRUE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Rendering

        glDisable(GL_DEPTH_TEST);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        glm::mat4 view2 = glm::mat4(glm::mat3(camera->getViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", glm::value_ptr(view2));
        skyboxShader.setMat4("projection", glm::value_ptr(projection));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glEnable(GL_DEPTH_TEST);


        //Set shader variables
        basicShader.use();
        basicShader.set("alpha", 1.0f);
        basicShader.setVec3("lightColor", glm::value_ptr(lightColor));
        basicShader.setVec3("light.direction", glm::value_ptr(lightDirection));
        glm::vec3 camPos = camera->getPos();
        basicShader.setVec3("viewPos", glm::value_ptr(camPos));
        basicShader.setVec3("material.specular", glm::value_ptr(matSpecular));
        basicShader.setVec3("light.ambient", glm::value_ptr(lightAmbient));
        basicShader.setVec3("light.diffuse", glm::value_ptr(lightDiffuse));
        basicShader.setVec3("light.specular", glm::value_ptr(lightSpecular)); 
        basicShader.set("material.shininess", 32.0f);

        //Pass projection matrix to shader
        projection = glm::perspective(glm::radians(camera->getFov()), static_cast<float>(g_srceenWitdth) / static_cast<float>(g_screenHeight), 0.1f, 1000.0f);
        basicShader.setMat4("projection", glm::value_ptr(projection));
        //Pass view matrix to shader
        view = camera->getViewMatrix();
        basicShader.setMat4("view", glm::value_ptr(view));

        //Render boids
        glBindVertexArray(VAO);
        //Get boids and lock the vector
        auto boids = gameWorld.getBoids();
        for(auto it = boids.begin(); it < boids.end(); it++) {
            //Calculate orientation based on velocity
            glm::vec3 velocity = it->getVelocity();
            glm::mat4 orientation(1.0f);
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            glm::vec3 ax = glm::normalize(glm::cross(velocity, up));
            orientation[0] = glm::vec4(glm::normalize(velocity), 0.0f);
            orientation[1] = glm::vec4(ax, 0.0f);
            orientation[2] = glm::vec4(glm::normalize(glm::cross(velocity, ax)), 0.0f);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, it->getPos()) * orientation;
            
            //Draw boid 
            basicShader.setMat4("model", glm::value_ptr(model));
            const float *objectColor = glm::value_ptr(it->getColor());
            basicShader.setVec3("objectColor", objectColor);
            basicShader.setVec3("material.ambient", objectColor);
            basicShader.setVec3("material.diffuse", objectColor);
            glDrawArrays(GL_TRIANGLES, 0, 12);
        }
        //Free the lock
        gameWorld.returnBoids();

        //Scale boids based on the world size
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(gameWorld.getLimits()[0] * 2));

        /*//Draw bounds
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDepthMask(GL_FALSE);
        basicShader.use();
        basicShader.set("alpha", 0.2f);
        basicShader.setMat4("model", glm::value_ptr(model));
        basicShader.setVec3("objectColor", glm::value_ptr(boundColor));
        basicShader.setVec3("material.ambient", glm::value_ptr(boundColor));
        basicShader.setVec3("material.diffuse", glm::value_ptr(boundColor));
        //Draw the object
        glBindVertexArray(boundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnd();
        glDepthMask(GL_TRUE);*/
        
        //Draw box
        const glm::vec3 boxColor(1.0f, 0.0f, 0.0f);
        basicShader.use();
        basicShader.set("alpha", 1.0f);
        basicShader.setMat4("model", glm::value_ptr(model));
        basicShader.setVec3("objectColor", glm::value_ptr(boxColor));
        basicShader.setVec3("material.ambient", glm::value_ptr(boxColor));
        basicShader.setVec3("material.diffuse", glm::value_ptr(boxColor));
        //Draw the object
        glBindVertexArray(boxVAO);

        for(int i = 0; i < 2; i++) {
            for(int j = 0; j < 4; j++) {
                model *= rot1;
                basicShader.setMat4("model", glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, sizeof(boxVert) / sizeof(float) / 6);
            }
            model *= rot2 * rot2;
        }
        model *= rot2;
        for(int i = 0; i < 4; i++) {
            model *= rot3;
            basicShader.setMat4("model", glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, sizeof(boxVert) / sizeof(float) / 6);
        }

        //Check and call event and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents(); //Keyboard input / mouse movement / window resize
    }

    //Terminate all
    gameWorld.stop();

    //Wait for thread to join
    //consoleThread.join();
    coreThread.join();

    delete camera;

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &boundVAO);
    glDeleteVertexArrays(1, &boxVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &boundVBO);
    glDeleteBuffers(1, &boxVBO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

//Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_srceenWitdth = width;
    g_screenHeight = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    //Window
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Rendering
    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); 
    
    static bool cameraSpeedup = false;

    //Camera movement
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->processKeyboard(Cam::Key::UP);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->processKeyboard(Cam::Key::DOWN);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->processKeyboard(Cam::Key::LEFT);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->processKeyboard(Cam::Key::RIGHT);
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera->processKeyboard(Cam::Key::DROP);
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera->processKeyboard(Cam::Key::RISE);
    if(!cameraSpeedup && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera->setWalkSens(10.0f * camera->getWalkSens());
        cameraSpeedup = true;
    }
    else if(cameraSpeedup && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
        camera->setWalkSens(camera->getWalkSens() / 10.0f);
        cameraSpeedup = false;
    }

    static bool isFullScreen = false;
    static bool ready = true;
    static int wndPos[2];
    static int wndSize[2];

    if((glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)) {
        ready = true;
    }

    if((glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) && ready) {
        ready =  false;
        if(!isFullScreen) {
            isFullScreen = true;
            glfwGetWindowPos(window, &wndPos[0], &wndPos[1] );
            glfwGetWindowSize(window, &wndSize[0], &wndSize[1] );
            // get resolution of monitor
            const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            // switch to full screen
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, 0 );
        } else {
            isFullScreen = false;
            // restore last window size and position
            glfwSetWindowMonitor(window, nullptr,  wndPos[0], wndPos[1], wndSize[0], wndSize[1], 0 );    
        }
    }
    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        g_pause.try_lock();
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        g_pause.unlock();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera->processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(xoffset, yoffset);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
