#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ================= GLOBAL VARIABLES =================
float g_yaw = 0.0f;
float g_pitch = 0.0f; // Start looking horizontally (In-plane view for 2+1D)
float g_fov = 60.0f;
float g_dist = 6.0f;

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  static double lastX = 0, lastY = 0;
  static bool firstMouse = true;

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = (float)(xpos - lastX);
  float yoffset = (float)(lastY - ypos);
  lastX = xpos;
  lastY = ypos;

  g_yaw += xoffset * 0.003f;
  g_pitch += yoffset * 0.003f;

  if (g_pitch > 1.55f)
    g_pitch = 1.55f;
  if (g_pitch < -1.55f)
    g_pitch = -1.55f;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  g_dist -= (float)yoffset * 0.5f;
  if (g_dist < 1.0f)
    g_dist = 1.0f;
  if (g_dist > 50.0f)
    g_dist = 50.0f;
}

// ================= HELPER: FIND FILE =================
std::string findFile(const std::string &relativePath) {
  std::string path = relativePath;
  for (int i = 0; i < 5; i++) {
    std::ifstream f(path);
    if (f.good())
      return path;
    path = "../" + path;
  }
  return "";
}

// ================= SHADER LOADER =================
GLuint LoadShaders(const std::string &vertex_rel,
                   const std::string &fragment_rel) {
  // Resolve Paths
  std::string vertex_path = findFile(vertex_rel);
  std::string fragment_path = findFile(fragment_rel);

  if (vertex_path.empty() || fragment_path.empty()) {
    std::cerr << "CRITICAL: Could not find shader files!" << std::endl;
    std::cerr << "Searched for: " << vertex_rel << " and " << fragment_rel
              << std::endl;
    return 0;
  }

  std::cout << "Loading shaders from: " << std::endl;
  std::cout << "  Vertex: " << vertex_path << std::endl;
  std::cout << "  Fragment: " << fragment_path << std::endl;

  // 1. Retrieve the vertex/fragment source code from filePath
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  // ensure ifstream objects can throw exceptions:
  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    // open files
    vShaderFile.open(vertex_path);
    fShaderFile.open(fragment_path);
    std::stringstream vShaderStream, fShaderStream;

    // read file's buffer contents into streams
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    // close file handlers
    vShaderFile.close();
    fShaderFile.close();

    // convert stream into string
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  } catch (std::ifstream::failure &e) {
    std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << vertex_path
              << " or " << fragment_path << std::endl;
    return 0;
  }

  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  // 2. Compile shaders
  GLuint vertex, fragment;
  int success;
  char infoLog[512];

  // Vertex Shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // Fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // Shader Program
  GLuint ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);
  glLinkProgram(ID);

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return ID;
}

// ================= MAIN =================
int main() {
  // 1. Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 800, "Black Hole BTZ (2+1D AdS)", NULL,
                       NULL); // Square aspect ratio for 2D feel
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // 2. Initialize GLEW
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  // 3. Setup Quad
  float quadVertices[] = {// positions   // texCoords
                          -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                          0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                          1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  // 4. Load Shader (USING BTZ SHADER)
  GLuint shaderProgram =
      LoadShaders("shaders/vertex.glsl", "shaders/fragment_btz.glsl");
  if (shaderProgram == 0) {
    return -1;
  }

  // 5. Load Texture
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  std::string texPath = findFile("data/ciel.png");
  if (texPath.empty()) {
    texPath = findFile("data/ciel.jpg");
  }

  if (!texPath.empty()) {
    unsigned char *data = stbi_load(texPath.c_str(), &width, &height,
                                    &nrChannels, 3); // force RGB
    if (data) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      glGenerateMipmap(GL_TEXTURE_2D);
      stbi_image_free(data);
      std::cout << "Texture loaded successfully: " << texPath << std::endl;
    } else {
      std::cerr << "Failed to load texture: " << texPath << std::endl;
    }
  }

  glUseProgram(shaderProgram);
  glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);

  // 6. Main Loop
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // Render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Update Uniforms
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glUniform2f(glGetUniformLocation(shaderProgram, "u_resolution"), (float)w,
                (float)h);

    // Camera Magic
    float camX = g_dist * cos(g_pitch) * sin(g_yaw);
    float camY = g_dist * sin(g_pitch);
    float camZ = g_dist * cos(g_pitch) * cos(g_yaw);
    glm::vec3 camPos(camX, camY, camZ);

    glm::mat4 view =
        glm::lookAt(camPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 invView = glm::inverse(view);

    glUniform3fv(glGetUniformLocation(shaderProgram, "u_camPos"), 1,
                 &camPos[0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_invView"), 1,
                       GL_FALSE, &invView[0][0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "u_fov"), g_fov);

    // Draw Quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();
  return 0;
}
