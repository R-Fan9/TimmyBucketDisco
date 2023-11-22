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

const std::vector<std::string> obj_paths = {"asset/timmy.obj", "asset/bucket.obj", "asset/floor.obj"};
const std::vector<std::string> img_paths = {"asset/timmy.png", "asset/bucket.jpg", "asset/floor.jpeg"};
std::vector<GLuint> VAOs(obj_paths.size());
std::vector<GLuint> VBOs(obj_paths.size() * 3);
std::vector<unsigned int> textures(obj_paths.size());
std::vector<unsigned int> vbuffer_sizes(obj_paths.size());

void setup_objs(std::vector<Obj> objs, std::vector<std::string> imgs);

std::vector<Obj> load_objs(std::vector<std::string> obj_paths);

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
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Timmy and the Bucket are at Disco", NULL, NULL);
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

  std::vector<Obj> objs = load_objs(obj_paths);
  setup_objs(objs, img_paths);

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(50, 100, 200), glm::vec3(0, 80, 0),
                               glm::vec3(0, 1, 0));
  glm::mat4 proj =
      glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

  float theta = 0.0f;

  // render loop
  while (!glfwWindowShouldClose(window))
  {
    process_input(window);

    // background color
    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // point light position
    glm::vec3 lightPos(0, 200, 0);
    glm::vec3 spotDirR(50 + sin(theta) * 50, -200, -50 + cos(theta) * 50);
    glm::vec3 spotDirG(-50 + sin(theta) * 50, -200, -50 + cos(theta) * 50);
    glm::vec3 spotDirB(0 + sin(theta) * 50, -200, 50 + cos(theta) * 50);
    theta += 0.05f;

    // activate shader
    shader.use();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);

    shader.setVec3("lights[0].position", lightPos);
    shader.setVec3("lights[0].direction", spotDirR);
    shader.setFloat("lights[0].cutOff", glm::cos(M_PI / 6.0f));
    shader.setVec3("lights[0].ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("lights[0].diffuse", 1.0f, 0.0f, 0.0f);
    shader.setFloat("lights[0].constant", 1.0f);
    shader.setFloat("lights[0].linear", 0.35e-4f);
    shader.setFloat("lights[0].quadratic", 0.44e-4);

    shader.setVec3("lights[1].position", lightPos);
    shader.setVec3("lights[1].direction", spotDirG);
    shader.setFloat("lights[1].cutOff", glm::cos(M_PI / 6.0f));
    shader.setVec3("lights[1].ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("lights[1].diffuse", 0.0f, 1.0f, 0.0f);
    shader.setFloat("lights[1].constant", 1.0f);
    shader.setFloat("lights[1].linear", 0.35e-4f);
    shader.setFloat("lights[1].quadratic", 0.44e-4);

    shader.setVec3("lights[2].position", lightPos);
    shader.setVec3("lights[2].direction", spotDirB);
    shader.setFloat("lights[2].cutOff", glm::cos(M_PI / 6.0f));
    shader.setVec3("lights[2].ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("lights[2].diffuse", 0.0f, 0.0f, 1.0f);
    shader.setFloat("lights[2].constant", 1.0f);
    shader.setFloat("lights[2].linear", 0.35e-4f);
    shader.setFloat("lights[2].quadratic", 0.44e-4);

    // render container
    for (size_t i = 0; i < objs.size(); i++)
    {
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      glBindVertexArray(VAOs[i]);
      glDrawArrays(GL_TRIANGLES, 0, vbuffer_sizes[i] / 3);
    }

    // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

std::vector<Obj> load_objs(std::vector<std::string> obj_paths)
{
  std::vector<Obj> objs;

  for (auto path : obj_paths)
  {
    Obj obj(path.c_str());
    objs.push_back(obj);
  }

  return objs;
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

void setup_objs(std::vector<Obj> objs, std::vector<std::string> imgs)
{
  const int num_objs = objs.size();
  glGenVertexArrays(num_objs, &VAOs[0]);
  glGenBuffers(num_objs * 3, &VBOs[0]);
  glGenTextures(num_objs, &textures[0]);

  for (size_t i = 0; i < num_objs; i++)
  {
    std::vector<tinyobj::shape_t> shapes = objs[i].getShapes();
    std::vector<tinyobj::real_t> vertices = objs[i].getVertices();
    std::vector<tinyobj::real_t> normals = objs[i].getNormals();
    std::vector<tinyobj::real_t> texcoords = objs[i].getTexCoords();

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

    vbuffer_sizes[i] = vbuffer.size();

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(imgs[i].c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
      std::cout << "Failed to load texture" << std::endl;
      exit(-1);
    }

    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glBindVertexArray(VAOs[i]);

    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[i * 3]);
    glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(tinyobj::real_t), &vbuffer[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // normals
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[i * 3 + 1]);
    glBufferData(GL_ARRAY_BUFFER, nbuffer.size() * sizeof(tinyobj::real_t), &nbuffer[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(tinyobj::real_t),
                          (void *)0);
    glEnableVertexAttribArray(1);

    // texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[i * 3 + 2]);
    glBufferData(GL_ARRAY_BUFFER, tbuffer.size() * sizeof(tinyobj::real_t), &tbuffer[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(tinyobj::real_t),
                          (void *)0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
  }
}