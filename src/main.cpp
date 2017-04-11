#include <iostream>

#include "MatrixStack.h"
#include "Model.h"
#include "Camera.h"
#include <SFML/Window.hpp>
#include <glbinding/gl/gl.h>
#include <globjects/State.h>

#include <globjects/Texture.h>

#define FRAMERATE_LIMIT 60
#define ROTATION_SPEED 60
#define LIGHT_PAN_SPEED 20

using namespace std;
using namespace glm;
using namespace globjects;
using namespace gl;

string RESOURCE_DIR = ""; // Where the resources are loaded from
vector<ref_ptr<Program>> g_shaders;
vector<ref_ptr<AbstractUniform>> g_uniformList;
unordered_map<string, ref_ptr<AbstractUniform>> g_uniforms;
ref_ptr<Program> activeShader;
vector<Model> models;

Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
sf::Window window;
sf::Time g_time;

bool running = true;

bool firstMouse = true;

float lastMouseX = 800;
float lastMouseY = 600;

float deltaTime = 0.0f;
float model_rotation = 0.0f;
int active_shader = 0;
int active_material = 0;
glm::vec3 g_lightpos(1.0f);

void handleKeyboard();
void handleMouse();
void handleWindowEvents();

static void updateGlobalUniforms();
static void initialize() {
  globjects::init(Shader::IncludeImplementation::Fallback);
  // Enable z-buffer test.
  auto currentState = State::currentState();
  currentState->enable(GL_DEPTH_TEST);
  currentState->clearColor(.15f, .15f, 0.15f, 1.0f);
  currentState->apply();
  DebugMessage::enable();

  // Initialize mesh.

  models.push_back(Model(RESOURCE_DIR + "bunny_gold.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_gold.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_chrome.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_chrome.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_cyan.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_cyan.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_ruby.obj", false));
  models.push_back(Model(RESOURCE_DIR + "bunny_ruby.obj", false));


  auto bunnyM = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  bunnyM = glm::translate(bunnyM, glm::vec3(0.0f, 0.0f, -1.0f));
  models[0].setUniform(new Uniform<glm::mat4>("model", bunnyM));
  models[2].setUniform(new Uniform<glm::mat4>("model", bunnyM));
  models[4].setUniform(new Uniform<glm::mat4>("model", bunnyM));
  models[6].setUniform(new Uniform<glm::mat4>("model", bunnyM));

  bunnyM = glm::translate(glm::mat4(1.0f), glm::vec3(1.0, 0.0f, 0.0f));
  models[1].setUniform(new Uniform<glm::mat4>("model", bunnyM));
  models[3].setUniform(new Uniform<glm::mat4>("model", bunnyM));
  models[5].setUniform(new Uniform<glm::mat4>("model", bunnyM));
  models[7].setUniform(new Uniform<glm::mat4>("model", bunnyM));

  // Initialize the GLSL programs
  auto gouradShader = make_ref<Program>();
  gouradShader->attach(Shader::fromFile(GL_VERTEX_SHADER, RESOURCE_DIR + "gourad_vert.glsl"),
                Shader::fromFile(GL_FRAGMENT_SHADER, RESOURCE_DIR + "gourad_frag.glsl"));
  g_shaders.push_back(gouradShader);

  auto phongShader = make_ref<Program>();
  phongShader->attach(Shader::fromFile(GL_VERTEX_SHADER, RESOURCE_DIR + "phong_vert.glsl"),
                Shader::fromFile(GL_FRAGMENT_SHADER, RESOURCE_DIR + "phong_frag.glsl"));
  g_shaders.push_back(phongShader);

  auto silhouetteShader = make_ref<Program>();
  silhouetteShader->attach(Shader::fromFile(GL_VERTEX_SHADER, RESOURCE_DIR + "silhouette_vert.glsl"),
                      Shader::fromFile(GL_FRAGMENT_SHADER, RESOURCE_DIR + "silhouette_frag.glsl"));
  g_shaders.push_back(silhouetteShader);

  auto normalShader = make_ref<Program>();
  normalShader->attach(Shader::fromFile(GL_VERTEX_SHADER, RESOURCE_DIR + "normal_vert.glsl"),
                           Shader::fromFile(GL_FRAGMENT_SHADER, RESOURCE_DIR + "normal_frag.glsl"));
  g_shaders.push_back(normalShader);

}

void initWindowContext() {
  int g_width = 1600;
  int g_height = 1200;
  // Create a windowed mode window and its OpenGL context.
  sf::ContextSettings settings;
  settings.majorVersion = 4;
  settings.minorVersion = 1;
  settings.depthBits = 24;
  settings.antialiasingLevel = 2;
  settings.stencilBits = 8;
  settings.attributeFlags = sf::ContextSettings::Attribute::Core;

  window.create(sf::VideoMode(g_width, g_height),
                "Andrew Kennedy",
                sf::Style::Default,
                settings);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FRAMERATE_LIMIT);
  window.setMouseCursorGrabbed(true);
  window.setMouseCursorVisible(false);
}

static void initUniforms() {
  g_uniforms["projection"] = new Uniform<glm::mat4>("projection");
  g_uniforms["model"] = new Uniform<glm::mat4>("model");
  g_uniforms["view"] = new Uniform<glm::mat4>("view");
  g_uniforms["u_viewPosition"] = new Uniform<glm::vec3>("u_viewPosition");
  g_uniforms["u_resolution"] = new Uniform<glm::vec2>("u_resolution");
  g_uniforms["u_time"] = new Uniform<float>("u_time");
  g_uniforms["u_light.position"] = new Uniform<glm::vec3>("u_light.position");
  g_uniforms["u_light.ambient"] = new Uniform<glm::vec3>("u_light.ambient");
  g_uniforms["u_light.diffuse"] = new Uniform<glm::vec3>("u_light.diffuse");
  g_uniforms["u_light.specular"] = new Uniform<glm::vec3>("u_light.specular");
  g_uniforms["u_nRotation"] = new Uniform<glm::mat3>("u_nRotation");
  g_uniforms["u_normal"] = new Uniform<glm::mat3>("u_normal");
  g_uniforms["u_rotation"] = new Uniform<glm::mat4>("u_rotation");


  g_uniforms["u_light.position"]->as<glm::vec3>()->set(glm::vec3(1.0f));
  g_uniforms["u_light.ambient"]->as<glm::vec3>()->set(glm::vec3(1.0f));
  g_uniforms["u_light.diffuse"]->as<glm::vec3>()->set(glm::vec3(1.0f));
  g_uniforms["u_light.specular"]->as<glm::vec3>()->set(glm::vec3(1.0f));
  g_uniforms["u_nRotation"]->as<glm::mat3>()->set(glm::mat3(1.0f));
  g_uniforms["u_normal"]->as<glm::mat3>()->set(glm::mat3(glm::transpose(glm::inverse(glm::mat4(1.0f)))));
  g_uniforms["u_rotation"]->as<glm::mat4>()->set(glm::mat4(1.0f));
}

/****DRAW
This is the most important function in your program - this is where you
will actually issue the commands to draw any geometry you have set up to
draw
********/
static void render() {
  // Clear framebuffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  updateGlobalUniforms();
  // Create the matrix stacks

  // update shaders with current top level uniforms
  for (auto &s : g_shaders) {
    for (auto &u : g_uniforms) {
      s->addUniform(u.second);
    }
  }

  models[active_material].draw(g_shaders[active_shader]);
  models[active_material + 1].draw(g_shaders[active_shader]);


}
static void updateGlobalUniforms() {
  auto windowSize = window.getSize();
  float aspect = (float) windowSize.x / windowSize.y;
  auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(model_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

  g_uniforms["projection"]->as<glm::mat4>()->set(glm::perspective(glm::radians(camera.Zoom), aspect, 0.01f, 1000.0f));
  g_uniforms["model"]->as<glm::mat4>()->set(glm::mat4(1.0f));
  g_uniforms["view"]->as<glm::mat4>()->set(camera.GetViewMatrix());
  g_uniforms["u_viewPosition"]->as<glm::vec3>()->set(camera.Position);
  g_uniforms["u_time"]->as<float>()->set(g_time.asSeconds());
  g_uniforms["u_resolution"]->as<glm::vec2>()->set(glm::vec2(windowSize.x, windowSize.y));
  g_uniforms["u_rotation"]->as<glm::mat4>()->set(rotation);
  g_uniforms["u_nRotation"]->as<glm::mat3>()->set(glm::mat3(rotation));
  g_uniforms["u_light.position"]->as<glm::vec3>()->set(g_lightpos);

}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Please specify the resource directory." << std::endl;
    return 0;
  }
  RESOURCE_DIR = argv[1];

  initWindowContext();

  initialize();

  initUniforms();

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  // Initialize scene.
  // Loop until the user closes the window.
  sf::Clock frameTimer;
  while (running) {
    // Event handling
    deltaTime = frameTimer.restart().asSeconds();

    handleWindowEvents();
    if (window.hasFocus()) {
      handleKeyboard();
      handleMouse();
    }


    // Activate window for openGL rendering
    window.setActive();

    render();

    // end current frame and display to window.
    window.display();
  }

  window.close();
  return 0;

}

void handleMouse() {
  auto mouseLoc = sf::Mouse::getPosition(window);

  if (firstMouse) {
    lastMouseX = mouseLoc.x;
    lastMouseY = mouseLoc.y;
    firstMouse = false;
  }

  auto xOffset = mouseLoc.x - lastMouseX;
  auto yOffset = mouseLoc.y - lastMouseY;

  camera.ProcessMouseMovement(xOffset, -yOffset);


  // Handle wrapping mouse movement around the window
  if (mouseLoc.x >= window.getSize().x - 2) {
    sf::Mouse::setPosition(sf::Vector2i(1, mouseLoc.y), window);
  } else if (mouseLoc.x <= 0) {
    sf::Mouse::setPosition(sf::Vector2i(window.getSize().x - 3, mouseLoc.y), window);
  }
  lastMouseX = sf::Mouse::getPosition(window).x;
  lastMouseY = sf::Mouse::getPosition(window).y;
}

void handleKeyboard() {
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
    camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
    camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
    camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
    camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
    running = false;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
    model_rotation = glm::mod(model_rotation + (ROTATION_SPEED * deltaTime), 360.0f);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
    model_rotation = glm::mod(model_rotation - (ROTATION_SPEED * deltaTime), 360.0f);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
    g_lightpos.x -= LIGHT_PAN_SPEED * deltaTime;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
    g_lightpos.x += LIGHT_PAN_SPEED * deltaTime;
  }
}

void handleWindowEvents() {
  sf::Event event;
  while (window.pollEvent(event)) {
    switch (event.type) {
      case sf::Event::Closed:running = false;
        break;
      case sf::Event::Resized:glViewport(0, 0, event.size.width, event.size.height);
        break;
      case sf::Event::MouseWheelScrolled:
        camera.ProcessMouseScroll(event.mouseWheelScroll.delta);
        break;
      case sf::Event::KeyReleased:
        if (event.key.code == sf::Keyboard::Key::P) {
          active_shader = (active_shader + 1) % g_shaders.size();
        } else if (event.key.code == sf::Keyboard::Key::M) {
          active_material = (active_material + 2) % (models.size());
        }
        break;
      default: break;
    }
  }
}
