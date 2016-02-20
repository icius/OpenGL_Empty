// Std. Includes
#include <string>
#include <iomanip>
#include <iostream>

#include "gl_core_4_3.h"

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GL includes
//#include "Shader.h"
#include "glutils.h"
#include "glslprogram.h"
#include "Camera.h"
#include "Text.h"

// Other Libs
#include <SOIL.h>

using namespace std;

// Properties
GLuint screenWidth = 1024, screenHeight = 768;

// Function prototypes
void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                  int mode);

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void doMovement();
GLuint loadTexture(GLchar* path);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat frameRate = 1000.0f;

// Light attributes
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// The MAIN function, from here we start our application and run our Game loop
int main()
{

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight,
                                          "LearnOpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    int loaded = ogl_LoadFunctions();
    if(loaded == ogl_LOAD_FAILED) {
        //Destroy the context and abort
        return 0;
    }
    int num_failed = loaded - ogl_LOAD_SUCCEEDED;
    printf("Number of functions that failed to load: %i.\n",num_failed);

    // Set the required callback functions
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW to setup the OpenGL Function pointers
    //glewExperimental = GL_TRUE;
    //glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);

   // Setup some OpenGL options
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDebugMessageCallback(GLUtils::debugCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

    // Draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLSLProgram lampShader, floorShader, textShader;

    try {
       lampShader.compileShader("shaders/lamp.vert");
       lampShader.compileShader("shaders/lamp.frag");
       lampShader.link();
       lampShader.validate();
    }
    catch( GLSLProgramException &e ) {
        cerr << e.what() << endl;   exit(EXIT_FAILURE);
    }

    try {
       floorShader.compileShader("shaders/floor.vert");
       floorShader.compileShader("shaders/floor.frag");
       floorShader.link();
       floorShader.validate();
    }
    catch( GLSLProgramException &e ) {
        cerr << e.what() << endl;   exit(EXIT_FAILURE);
    }

    try {
       textShader.compileShader("shaders/text.vert");
       textShader.compileShader("shaders/text.frag");
       textShader.link();
       textShader.validate();
    }
    catch( GLSLProgramException &e ) {
        cerr << e.what() << endl;   exit(EXIT_FAILURE);
    }


    GLfloat planeVertices[] = {
        // Positions          // Normals         // Texture Coords
         7.5f, -1.0f,  7.5f,  0.0f, 1.0f, 0.0f,  5.5f, 0.0f,
        -7.5f, -1.0f,  7.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        -7.5f, -1.0f, -7.5f,  0.0f, 1.0f, 0.0f,  0.0f, 5.5f,

         7.5f, -1.0f,  7.5f,  0.0f, 1.0f, 0.0f,  5.5f, 0.0f,
        -7.5f, -1.0f, -7.5f,  0.0f, 1.0f, 0.0f,  0.0f, 5.5f,
         7.5f, -1.0f, -7.5f,  0.0f, 1.0f, 0.0f,  5.5f, 5.5f
    };

    GLfloat cubeVertices[] = {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left
    };

    // Setup plane VAO
    GLuint planeVAO, planeVBO;

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);



    // Setup cube VAO
    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);


    // Variables for the Frame Rate Display
    GLint frameRateCounterTarget = 4;
    GLint frameRateCounter = frameRateCounterTarget;
    string frameRateString;

    Text frameRateText(textShader, "fonts/Arial.ttf", 48, screenWidth,
                       screenHeight);

    // Load textures
    GLuint floorTexture = loadTexture((char *)"textures/wood.png");

    // Game loop
    while(!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        // Setting up the text for the Frame Rate display
        if(frameRateCounter == frameRateCounterTarget)
        {
            frameRate = deltaTime * 1000;
            frameRateString = to_string(frameRate);
            frameRateCounter = 0;
        }

        ++frameRateCounter;

        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();
        doMovement();

        // Clear the colorbuffer
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)screenWidth/
                                                (float)screenHeight, 0.1f,
                                                100.0f);

        glm::mat4 view = camera.GetViewMatrix();


        //------ Setup and Render the Lamp ------


        lampShader.use();

        lampShader.setUniform("view", view);
        lampShader.setUniform("projection", projection);

        glm::mat4 model;
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        lampShader.setUniform("model", model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);


        //------ Render the Framerate Text ------

        frameRateText.render(textShader, frameRateString, screenWidth - 130.0f,
                             screenHeight - 30.0f, 0.5f,
                             glm::vec3(0.2f, 0.6f, 0.2f));


        //------ Setup and Render the Floor ------

        floorShader.use();

        floorShader.setUniform("view", view);
        floorShader.setUniform("projection", projection);
        floorShader.setUniform("lightPos", lightPos);
        floorShader.setUniform("viewPos", camera.Position);

        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);


        glfwSwapBuffers(window);
    }

    glfwTerminate();


    return 0;
}


// This function loads a texture from file. Note: texture loading functions like these are usually
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio).
// For learning purposes we'll just define it as a utility function.

GLuint loadTexture(GLchar* path)
{
    // Generate texture ID and load texture data
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0,
                                           SOIL_LOAD_RGB);

    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_LINEAR );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;

}

// Moves/alters the camera positions based on user input
void doMovement()
{
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                  int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                   GLsizei length, const GLchar * message, const void * param)
{
    // Convert GLenum parameters to strings
    printf("%d:%d[%d](%d): %s\n", source, type, severity, id, message);
}
