#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 12.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isSpotlightActived = false;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    // shaders
    Shader objectShader("resources/shaders/object.vs", "resources/shaders/object.fs");
    Shader floorShader("resources/shaders/floor.vs", "resources/shaders/floor.fs");
    Shader lightShader("resources/shaders/light_source.vs", "resources/shaders/light_source.fs");

    // models
    Model tableModel(FileSystem::getPath("resources/objects/dining_table/dining_table.obj"));
    Model cakeModel(FileSystem::getPath("resources/objects/slice_of_cake/cake.obj"));
    Model lightModel(FileSystem::getPath("resources/objects/light/light.obj"));

    // floor
    float floorVertices[] = {
            // positions          //normals            // texture coords
            1.0f,  0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 10.0f,
            1.0f, 0.0f, -1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 0.0f,
            -1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 10.0f,
            -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,    0.0f, 0.0f
    };
    unsigned int floorIndices[] = {
            0, 1, 3, // first triangle
            0, 2, 3  // second triangle
    };
    unsigned int floorVBO, floorVAO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int floorDiffTexture = TextureFromFile("floor_diffuse.png", "resources/objects/floor");
    unsigned int floorSpecTexture = TextureFromFile("floor_specular2.png", "resources/objects/floor");

    glm::vec3 pointLightPositions[3];
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        objectShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        // table
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
        objectShader.setMat4("model", model);
        tableModel.Draw(objectShader);

        // TODO: Napraviti funkciju za cake i light
        // cake
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f,-2.15f, 3.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, 3.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f,-2.15f, 0.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, 0.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f,-2.15f, -3.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, -3.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, 0.0f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);


        // light
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, -3.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
        model = glm::rotate(model, glm::radians((float)(10.0 * sin(1.0 + 2*glfwGetTime()))),
                            glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.32f, 0.0f));
        pointLightPositions[0] = glm::vec3(model * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
        model = glm::rotate(model, glm::radians((float)(10.0 * sin(2*glfwGetTime()))),
                            glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.32f, 0.0f));
        pointLightPositions[1] = glm::vec3(model * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 3.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
        model = glm::rotate(model, glm::radians((float)(10.0 * sin(2.0 + 2*glfwGetTime()))),
                            glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.32f, 0.0f));
        pointLightPositions[2] = glm::vec3(model * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        //floor
        floorShader.use();
        floorShader.setMat4("projection", projection);
        floorShader.setMat4("view", view);

        float pointLightLinear = 0.09;
        float pointLightQuadratic = 0.032;
        // point light 1
        floorShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        floorShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        floorShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        floorShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        floorShader.setFloat("pointLights[0].constant", 1.0f);
        floorShader.setFloat("pointLights[0].linear", pointLightLinear);
        floorShader.setFloat("pointLights[0].quadratic", pointLightQuadratic);
        // point light 2
        floorShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        floorShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        floorShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        floorShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        floorShader.setFloat("pointLights[1].constant", 1.0f);
        floorShader.setFloat("pointLights[1].linear", pointLightLinear);
        floorShader.setFloat("pointLights[1].quadratic", pointLightQuadratic);
        // point light 3
        floorShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        floorShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        floorShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        floorShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        floorShader.setFloat("pointLights[2].constant", 1.0f);
        floorShader.setFloat("pointLights[2].linear", pointLightLinear);
        floorShader.setFloat("pointLights[2].quadratic", pointLightQuadratic);
        // spotLight
        floorShader.setVec3("spotLight.position", camera.Position);
        floorShader.setVec3("spotLight.direction", camera.Front);
        if(isSpotlightActived){
            floorShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            floorShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
            floorShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        }
        else{ // All to 0.
            floorShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            floorShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
            floorShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
        }
        floorShader.setFloat("spotLight.constant", 1.0f);
        floorShader.setFloat("spotLight.linear", 0.09);
        floorShader.setFloat("spotLight.quadratic", 0.032);
        floorShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(2.5f)));
        floorShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(22.0f)));

        floorShader.setVec3("viewPos", camera.Position);
        floorShader.setFloat("material.shininess", 128.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorDiffTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorSpecTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));
        floorShader.setMat4("model", model);
        glBindVertexArray(floorVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &floorEBO);

    glfwTerminate();
    return 0;
}

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        isSpotlightActived = !isSpotlightActived;
    }
}