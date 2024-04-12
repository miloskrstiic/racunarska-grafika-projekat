#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

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
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1100;
const unsigned int SCR_HEIGHT = 850;

// camera
Camera camera(glm::vec3(-34.0f, 2.0f, -9.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

bool blinn = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
PointLight initPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
                         float constant, float linear, float quadratic);
void setPointLight(std::string name, PointLight pointLight, Shader shader);

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

};

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
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
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces {
        FileSystem::getPath("resources/textures/skybox/right.tga"),
        FileSystem::getPath("resources/textures/skybox/left.tga"),
        FileSystem::getPath("resources/textures/skybox/top.jpg"),
        FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
        FileSystem::getPath("resources/textures/skybox/front.tga"),
        FileSystem::getPath("resources/textures/skybox/back.tga")
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    // shader configuration
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // ################################################# MODELS #################################################
    Model cube("resources/objects/cube/cube.obj");
    cube.SetShaderTextureNamePrefix("material.");
    Shader cubeShader("resources/shaders/model_loading.vs", "resources/shaders/model_loading.fs");
    // load models
    Model village("resources/objects/village/VolgarStreet.obj");
    village.SetShaderTextureNamePrefix("material.");
    Shader villageShader("resources/shaders/model_loading.vs", "resources/shaders/model_loading.fs");

    Model nissan("resources/objects/nissan/source/SA5HLA5LO5H1RQJ42KKT685IS.obj");
    nissan.SetShaderTextureNamePrefix("material.");
    Shader nissanShader("resources/shaders/model_loading.vs", "resources/shaders/nissan.fs");

    Model mercedes("resources/objects/mercedes/9IGEYFTP0J6AQ1IDGYCN823X7.obj");
    mercedes.SetShaderTextureNamePrefix("material.");
    Shader mercedesShader("resources/shaders/model_loading.vs", "resources/shaders/mercedes.fs");

    Model porsche("resources/objects/porsche/N17ARA9C0GT5W7X12AGMQ0F88.obj");
    porsche.SetShaderTextureNamePrefix("material.");
    Shader porscheShader("resources/shaders/model_loading.vs", "resources/shaders/porsche.fs");

    Model lamppost("resources/objects/lamppost/Wooden Lantern.obj");
    lamppost.SetShaderTextureNamePrefix("material.");
    Shader lamppostShader("resources/shaders/model_loading.vs", "resources/shaders/lamppost.fs");

    // lighting info
    // ----------------------------
    // directional light
    DirLight directional;
    glm::vec3 vec1 = glm::vec3(-50.0f, 20.0f, 20.0f);
    glm::vec3 vec2 = glm::vec3(-45.0f, 17.0f, 15.0f);
    directional.direction = vec2 - vec1;
    directional.ambient = glm::vec3(0.09f);
    directional.diffuse = glm::vec3(0.4f);
    directional.specular = glm::vec3(0.5f);

    // spotlight
    SpotLight spotlight;
    spotlight.position = camera.Position;
    spotlight.direction = camera.Front;
    spotlight.ambient = glm::vec3(0.2f);
    spotlight.diffuse = glm::vec3(1.0f, 0.894f, 0.627f);
    spotlight.specular = glm::vec3(1.0f, 0.894f, 0.627f);
    spotlight.cutOff = glm::cos(glm::radians(13.0f));
    spotlight.outerCutOff = glm::cos(glm::radians(16.5f));

    // pointlights
    PointLight pointLight = initPointLight(glm::vec3(-15.0f, -0.6f, 3.83f),
                                                 glm::vec3(5.5f, 3.7f, 1.0f),
                                           glm::vec3(5.5f, 3.7f, 1.0f),
                                           glm::vec3(5.5f, 3.7f, 1.0f),
                                                1.0f, 0.09f, 0.032f);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        villageShader.use();
        model = glm::mat4(1.0f);
        villageShader.setMat4("projection", projection);
        villageShader.setMat4("view", view);
//        villageShader.setMat4("model", model);
        villageShader.setVec3("viewPos", camera.Position);
        villageShader.setFloat("material.shininess", 16.0f);
        //Directional light
        villageShader.setVec3("directional.direction", directional.direction);
        villageShader.setVec3("directional.ambient", directional.ambient);
        villageShader.setVec3("directional.diffuse", directional.diffuse);
        villageShader.setVec3("directional.specular", directional.specular);
        //Spotlight
        villageShader.setVec3("spotlight.position", camera.Position);
        villageShader.setVec3("spotlight.direction", camera.Front);
        villageShader.setVec3("spotlight.ambient", spotlight.ambient);
        villageShader.setVec3("spotlight.diffuse", spotlight.diffuse);
        villageShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        villageShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);
        // Pointlight
        // TODO: lamppost -> pointlight
        setPointLight("pointlight.", pointLight, villageShader);
        // blinn
        villageShader.setBool("blinn", blinn);
        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -4.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f));	// it's a bit too big for our scene, so scale it down
        villageShader.setMat4("model", model);
        village.Draw(villageShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-15.0f, -0.6f, 3.83f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f));	// it's a bit too big for our scene, so scale it down
        cubeShader.setMat4("model", model);
        cube.Draw(cubeShader);


        lamppostShader.use();
        model = glm::mat4(1.0f);
        lamppostShader.setMat4("projection", projection);
        lamppostShader.setMat4("view", view);
//        villageShader.setMat4("model", model);
        lamppostShader.setVec3("viewPos", camera.Position);
        lamppostShader.setFloat("material.shininess", 16.0f);
        //Directional light
        lamppostShader.setVec3("directional.direction", directional.direction);
        lamppostShader.setVec3("directional.ambient", directional.ambient);
        lamppostShader.setVec3("directional.diffuse", directional.diffuse);
        lamppostShader.setVec3("directional.specular", directional.specular);
        //Spotlight
        lamppostShader.setVec3("spotlight.position", camera.Position);
        lamppostShader.setVec3("spotlight.direction", camera.Front);
        lamppostShader.setVec3("spotlight.ambient", spotlight.ambient);
        lamppostShader.setVec3("spotlight.diffuse", spotlight.diffuse);
        lamppostShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        lamppostShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);
        // Pointlight
        // TODO: lamppost -> pointlight
        setPointLight("pointlight.", pointLight, lamppostShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-15.0f, -4.0f, 6.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.47f));	// it's a bit too big for our scene, so scale it down
        lamppostShader.setMat4("model", model);
        lamppost.Draw(lamppostShader);



        nissanShader.use();
        model = glm::mat4(1.0f);
        nissanShader.setMat4("projection", projection);
        nissanShader.setMat4("view", view);
//        nissanShader.setMat4("model", model);
        nissanShader.setVec3("viewPos", camera.Position);
        nissanShader.setFloat("material.shininess", 16.0f);
        //Directional light
        nissanShader.setVec3("directional.direction", directional.direction);
        nissanShader.setVec3("directional.ambient", directional.ambient);
        nissanShader.setVec3("directional.diffuse", directional.diffuse);
        nissanShader.setVec3("directional.specular", directional.specular);
        //Spotlight
        nissanShader.setVec3("spotlight.position", camera.Position);
        nissanShader.setVec3("spotlight.direction", camera.Front);
        nissanShader.setVec3("spotlight.ambient", spotlight.ambient);
        nissanShader.setVec3("spotlight.diffuse", spotlight.diffuse);
        nissanShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        nissanShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);
        // blinn
        nissanShader.setBool("blinn", blinn);
        // Pointlight
        // TODO: lamppost -> pointlight
        setPointLight("pointlight.", pointLight, nissanShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-20.0f, -2.75f, 2.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(3.0f));	// it's a bit too big for our scene, so scale it down
        nissanShader.setMat4("model", model);
        nissan.Draw(nissanShader);



        mercedesShader.use();
        model = glm::mat4(1.0f);
        mercedesShader.setMat4("projection", projection);
        mercedesShader.setMat4("view", view);
//        mercedesShader.setMat4("model", model);
        mercedesShader.setVec3("viewPos", camera.Position);
        mercedesShader.setFloat("material.shininess", 16.0f);
        //Directional light
        mercedesShader.setVec3("directional.direction", directional.direction);
        mercedesShader.setVec3("directional.ambient", directional.ambient);
        mercedesShader.setVec3("directional.diffuse", directional.diffuse);
        mercedesShader.setVec3("directional.specular", directional.specular);
        //Spotlight
        mercedesShader.setVec3("spotlight.position", camera.Position);
        mercedesShader.setVec3("spotlight.direction", camera.Front);
        mercedesShader.setVec3("spotlight.ambient", spotlight.ambient);
        mercedesShader.setVec3("spotlight.diffuse", spotlight.diffuse);
        mercedesShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        mercedesShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);
        // blinn
        mercedesShader.setBool("blinn", blinn);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(7.0f, -2.69f, -2.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(3.0f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, (float)glm::radians(180.0), glm::vec3(0.0f, 1.0f, 0.0f));
        mercedesShader.setMat4("model", model);
        mercedes.Draw(mercedesShader);



        porscheShader.use();
        model = glm::mat4(1.0f);
        porscheShader.setMat4("projection", projection);
        porscheShader.setMat4("view", view);
//        porscheShader.setMat4("model", model);
        porscheShader.setVec3("viewPos", camera.Position);
        porscheShader.setFloat("material.shininess", 16.0f);
        //Directional light
        porscheShader.setVec3("directional.direction", directional.direction);
        porscheShader.setVec3("directional.ambient", directional.ambient);
        porscheShader.setVec3("directional.diffuse", directional.diffuse);
        porscheShader.setVec3("directional.specular", directional.specular);
        //Spotlight
        porscheShader.setVec3("spotlight.position", camera.Position);
        porscheShader.setVec3("spotlight.direction", camera.Front);
        porscheShader.setVec3("spotlight.ambient", spotlight.ambient);
        porscheShader.setVec3("spotlight.diffuse", spotlight.diffuse);
        porscheShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        porscheShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);
        // blinn
        porscheShader.setBool("blinn", blinn);
        // Pointlight
        // TODO: lamppost -> pointlight
//        setPointLight("pointlight.", , porscheShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-7.0f, -2.69f, -2.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(3.0f));	// it's a bit too big for our scene, so scale it down
        porscheShader.setMat4("model", model);
        porsche.Draw(porscheShader);


        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
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
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
unsigned int loadCubemap(vector<std::string> faces)
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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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

PointLight initPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
                         float constant, float linear, float quadratic) {
    PointLight pointLight;
    pointLight.position = position;
    pointLight.ambient = ambient;
    pointLight.diffuse = diffuse;
    pointLight.specular = specular;
    pointLight.constant = constant;
    pointLight.linear = linear;
    pointLight.quadratic = quadratic;
    return pointLight;
}

void setPointLight(std::string name, PointLight pointLight, Shader shader) {
    shader.setVec3(name + "position", pointLight.position);
    shader.setVec3(name + "ambient", pointLight.ambient * 0.3f);
    shader.setVec3(name + "diffuse", pointLight.diffuse * 2.0f);
    shader.setVec3(name + "specular", pointLight.specular * 0.5f);
    shader.setFloat(name + "constant", pointLight.constant);
    shader.setFloat(name + "linear", pointLight.linear);
    shader.setFloat(name + "quadratic", pointLight.quadratic);
    shader.setVec3("viewPos", camera.Position);
    shader.setFloat("material.shininess", 32.0f);
}