#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <obj.h>
#include <shader.h>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width,
                             uint32_t height);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void process_input(GLFWwindow *window);

static uint32_t ss_id = 0;

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

int main()
{
  // initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Facial Expressions", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global OpenGL state
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // build and compile shader program
  Shader shader("shaders/shader.vs", "shaders/shader.fs");

  // load floor obj
  Obj floor("asset/floor.obj");
  std::vector<tinyobj::shape_t> shapes = floor.getShapes();
  std::vector<tinyobj::real_t> vertices = floor.getVertices();
  std::vector<tinyobj::real_t> normals = floor.getNormals();
  std::vector<tinyobj::real_t> texcoords = floor.getTexCoords();

  std::vector<tinyobj::real_t> vbuffer, nbuffer, tbuffer;
  for (auto id : shapes[0].mesh.indices)
  {
    int vid = id.vertex_index;
    int nid = id.normal_index;
    int tid = id.texcoord_index;

    // vertex positions
    vbuffer.push_back(vertices[vid * 3]);
    vbuffer.push_back(vertices[vid * 3 + 1]);
    vbuffer.push_back(vertices[vid * 3 + 2]);

    // normal positions
    nbuffer.push_back(normals[nid * 3]);
    nbuffer.push_back(normals[nid * 3 + 1]);
    nbuffer.push_back(normals[nid * 3 + 2]);

    // texture coordinates
    tbuffer.push_back(texcoords[tid * 2]);
    tbuffer.push_back(texcoords[tid * 2 + 1]);
  }

  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load("asset/floor.jpeg", &width, &height, &nrChannels, 0);

  if (!data)
  {
    std::cout << "Failed to load texture" << std::endl;
    return -1;
  }

  unsigned int textureT;
  glGenTextures(1, &textureT);
  glBindTexture(GL_TEXTURE_2D, textureT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);

  GLuint VAO, VBO_vertices, VBO_normals, VBO_texcoords;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // bind vertex array to vertex buffer
  glGenBuffers(1, &VBO_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
  glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(tinyobj::real_t), &vbuffer[0],
               GL_STATIC_DRAW);

  // position attribute
  // GLuint vertex_loc = shader.getAttribLocation("aPos");
  glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(0);

  // bind normal array to normal buffer
  glGenBuffers(1, &VBO_normals);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
  glBufferData(GL_ARRAY_BUFFER, nbuffer.size() * sizeof(tinyobj::real_t), &nbuffer[0],
               GL_STATIC_DRAW);

  // normal attribute
  glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(1);

  // bind texture coordinate array to texture coordinate buffer
  glGenBuffers(1, &VBO_texcoords);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_texcoords);
  glBufferData(GL_ARRAY_BUFFER, tbuffer.size() * sizeof(tinyobj::real_t), &tbuffer[0],
               GL_STATIC_DRAW);

  // texture coordinate attribute
  glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(2);

  // load bucket obj
  Obj bucket("asset/bucket.obj");
  shapes = bucket.getShapes();
  vertices = bucket.getVertices();
  normals = bucket.getNormals();
  texcoords = bucket.getTexCoords();

  std::vector<tinyobj::real_t> vbuffer1, nbuffer1, tbuffer1;
  for (auto id : shapes[0].mesh.indices)
  {
    int vid = id.vertex_index;
    int nid = id.normal_index;
    int tid = id.texcoord_index;

    // vertex positions
    vbuffer1.push_back(vertices[vid * 3]);
    vbuffer1.push_back(vertices[vid * 3 + 1]);
    vbuffer1.push_back(vertices[vid * 3 + 2]);

    // normal positions
    nbuffer1.push_back(normals[nid * 3]);
    nbuffer1.push_back(normals[nid * 3 + 1]);
    nbuffer1.push_back(normals[nid * 3 + 2]);

    // texture coordinates
    tbuffer1.push_back(texcoords[tid * 2]);
    tbuffer1.push_back(texcoords[tid * 2 + 1]);
  }

  int width1, height1, nrChannels1;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data1 = stbi_load("asset/bucket.jpg", &width1, &height1, &nrChannels1, 0);

  if (!data1)
  {
    std::cout << "Failed to load texture" << std::endl;
    return -1;
  }

  unsigned int textureT1;
  glGenTextures(1, &textureT1);
  glBindTexture(GL_TEXTURE_2D, textureT1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
  stbi_image_free(data1);

  GLuint VAO1, VBO_vertices1, VBO_normals1, VBO_texcoords1;
  glGenVertexArrays(1, &VAO1);
  glBindVertexArray(VAO1);

  // bind vertex array to vertex buffer
  glGenBuffers(1, &VBO_vertices1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices1);
  glBufferData(GL_ARRAY_BUFFER, vbuffer1.size() * sizeof(tinyobj::real_t), &vbuffer1[0],
               GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(0);

  // bind normal array to normal buffer
  glGenBuffers(1, &VBO_normals1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_normals1);
  glBufferData(GL_ARRAY_BUFFER, nbuffer1.size() * sizeof(tinyobj::real_t), &nbuffer1[0],
               GL_STATIC_DRAW);

  // normal attribute
  glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(1);

  // bind texture coordinate array to texture coordinate buffer
  glGenBuffers(1, &VBO_texcoords1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_texcoords1);
  glBufferData(GL_ARRAY_BUFFER, tbuffer1.size() * sizeof(tinyobj::real_t), &tbuffer1[0],
               GL_STATIC_DRAW);

  // texture coordinate attribute
  glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(tinyobj::real_t),
                        (void *)0);
  glEnableVertexAttribArray(2);

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(50, 100, 200), glm::vec3(0, 80, 0),
                               glm::vec3(0, 1, 0));
  glm::mat4 proj =
      glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

  // render loop
  while (!glfwWindowShouldClose(window))
  {
    process_input(window);

    // background color
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate shader
    shader.use();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);

    // render container
    glBindTexture(GL_TEXTURE_2D, textureT);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vbuffer.size() / 3);

    glBindTexture(GL_TEXTURE_2D, textureT1);
    glBindVertexArray(VAO1);
    glDrawArrays(GL_TRIANGLES, 0, vbuffer1.size() / 3);

    // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
void process_input(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // press p to capture screen
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
  {
    std::cout << "Capture Window " << ss_id << std::endl;
    int buffer_width, buffer_height;
    glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
    dump_framebuffer_to_ppm("tmp", buffer_width, buffer_height);
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void dump_framebuffer_to_ppm(std::string prefix, uint32_t width,
                             uint32_t height)
{
  int pixelChannel = 3;
  int totalPixelSize = pixelChannel * width * height * sizeof(GLubyte);
  GLubyte *pixels = new GLubyte[totalPixelSize];
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  std::string fileName = prefix + std::to_string(ss_id) + ".ppm";
  std::filesystem::path filePath = std::filesystem::current_path() / fileName;
  std::ofstream fout(filePath.string());

  fout << "P3\n"
       << width << " " << height << "\n"
       << 255 << std::endl;
  for (size_t i = 0; i < height; i++)
  {
    for (size_t j = 0; j < width; j++)
    {
      size_t cur = pixelChannel * ((height - i - 1) * width + j);
      fout << (int)pixels[cur] << " " << (int)pixels[cur + 1] << " "
           << (int)pixels[cur + 2] << " ";
    }
    fout << std::endl;
  }

  ss_id++;

  delete[] pixels;
  fout.flush();
  fout.close();
}
