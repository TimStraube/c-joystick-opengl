#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "./shader.h"
#include "./obj-loader.h"

// globale variables
unsigned int width = 600;
unsigned int heigth = 600;
// prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow * initGraficsLibrary(void);

int main() {
    // local variables
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
    unsigned int vertexBufferObject;
    unsigned int vertexArrayObject;
    unsigned int vertexElementObject;
    float s;
    const float* axes;
    float vertices[200000];
    float normals[200000];
    int count;
    // vertices = (float *) calloc(120000, sizeof(float));
    int success;
    int k;
    int l;
    GLFWwindow * window;
    objl::Loader loader;
    bool mainBody = loader.LoadFile("./src/model/plane.obj");
    if (mainBody) {
        // size of vertices vector
        s = loader.LoadedVertices.size();
        for (int i = 0; i < s; i++) {
            l = 3 * i;
            k = 3 * i;
            vertices[l]     = loader.LoadedVertices[i].Position.X;
            vertices[l + 1] = loader.LoadedVertices[i].Position.Y;
            vertices[l + 2] = loader.LoadedVertices[i].Position.Z;
            vertices[l + 3] = loader.LoadedVertices[i].TextureCoordinate.X;
            vertices[l + 4] = loader.LoadedVertices[i].TextureCoordinate.Y;
            normals[k]      = loader.LoadedVertices[i].Normal.X;
            normals[k + 1]  = loader.LoadedVertices[i].Normal.Y;
            normals[k + 2]  = loader.LoadedVertices[i].Normal.Z;
        }
    } else {
        return -1;
    }
    // init Window
    window = initGraficsLibrary();
    // check window
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // load glut
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // joystick
    GLint present = glfwJoystickPresent(GLFW_JOYSTICK_1);
    // shader
    Shader shader1("./src/shader/shader.vertex", "./src/shader/shader.fragment");
    // graphics pipeline, transforming 3d vertices to 2d screen and colorizing them
    // 
    // generate vertex array object names (1)
    glGenVertexArrays(1, &vertexArrayObject);
    // bind vertex array object
    glBindVertexArray(vertexArrayObject);
    // generate buffer object names, used for storing vertex data on gpu at buffer id
    glGenBuffers(1, &vertexBufferObject);
    // vertexBufferObject as active array buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    // copy vertex data into buffer memory, static draw leads to data being stored in memory that has slower write time
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    // array for vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) 0);
    // enables a generic vertex attribute array at index 0
    glEnableVertexAttribArray(0);
    // array for texture data
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));
    // enables a generic vertex attribute array at index 1
    glEnableVertexAttribArray(1);
    // test
    glBindVertexArray(0);
    // wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // z buffer enabled
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        // user input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        // get joystick data
        axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
        {
            // transformation math
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, glm::vec3(axes[0], axes[2], axes[1]));
            transform = glm::rotate(transform, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            transform = glm::rotate(transform, glm::radians(100.0f * axes[4]), glm::vec3(0.0f, 1.0f, 0.0f));
            transform = glm::rotate(transform, glm::radians(100.0f * axes[3]), glm::vec3(0.0f, 0.0f, 1.0f));
            // transformation matrix to gpu via uniform
            int modelLocTransform = glGetUniformLocation(shader1.ID, "model");
            glUniformMatrix4fv(modelLocTransform, 1, GL_FALSE, glm::value_ptr(transform));
            // view math
            glm::mat4 view = glm::mat4(1.0f);
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
            // view matrix to gpu via uniform
            int modelLocView = glGetUniformLocation(shader1.ID, "view");
            glUniformMatrix4fv(modelLocView, 1, GL_FALSE, glm::value_ptr(view));
            // projection math
            glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float) width / (float) heigth, 0.1f, 100.0f);
            // projection matrix to gpu via uniform
            int modelLocProjection = glGetUniformLocation(shader1.ID, "projection");
            glUniformMatrix4fv(modelLocProjection, 1, GL_FALSE, glm::value_ptr(projection));
        }
        // render
        // 
        // clear viewport with color
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use shader program
        shader1.use();
        glBindVertexArray(vertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices));
        // update Window
        glfwSwapBuffers(window);
        // poll events
        glfwPollEvents();
    }
    // vertices array speicher freigeben
    // free(vertices);
    // delete vertex array object
    glDeleteVertexArrays(1, &vertexArrayObject);
    // delete vertex buffer object
    glDeleteBuffers(1, &vertexBufferObject);
    // delete element buffer
    glDeleteBuffers(1, &vertexElementObject);
    // delete shader program
    glDeleteProgram(shaderProgram);
    // terminate glfw
    glfwTerminate();
    return 0;
}

// adjust viewport size to window size for transforming normalized data to screensize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// init window
GLFWwindow * initGraficsLibrary(void) {
    // init graphics library framework
    glfwInit();
    // set supported opengl version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // printf("%d\n", GL_MAX_VERTEX_UNIFORM_COMPONENTS);
    // printf("%ld\n", sizeof(GLint));
    // create window
    GLFWwindow* window = glfwCreateWindow(width, heigth, "Plane", NULL, NULL);
    glfwMakeContextCurrent(window);
    return window;
}

