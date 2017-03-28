/* Base code for texture mapping lab */
/* includes three images and three meshes - Z. Wood 2016 */
#include <iostream>

#include "MatrixStack.h"
#include "Model.h"
#include <SFML/Window.hpp>
#include <glbinding/gl/gl.h>
#include <globjects/globjects.h>
#include <globjects/Shader.h>
#include <globjects/State.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <globjects/Texture.h>

#define FRAMERATE_LIMIT 60

using namespace std;
using namespace glm;
using namespace globjects;
using namespace gl;

string RESOURCE_DIR = ""; // Where the resources are loaded from
ref_ptr<Program> prog0;
ref_ptr<Program> prog1;
shared_ptr<Model> ground;
shared_ptr<Model> dog;
shared_ptr<Model> globe;

// OpenGL handle to texture data

float cTheta = 0;
float cHeight = 0;

static void initialize() {
  globjects::init(Shader::IncludeImplementation::Fallback);
  // Enable z-buffer test.
  auto currentState = State::currentState();
  currentState->enable(GL_DEPTH_TEST);
  currentState->clearColor(.5f, .5f, 1.f, 1.0f);
  currentState->apply();
  DebugMessage::enable();

  // Initialize mesh.
  dog = make_shared<Model>(RESOURCE_DIR + "dog.obj");
  dog->addTexture(aiString("fur.jpg"), "texture_diffuse");

  globe = make_shared<Model>(RESOURCE_DIR + "sphere.obj");
  globe->addTexture(aiString("world.bmp"), "texture_diffuse");

  ground = make_shared<Model>(RESOURCE_DIR + "ground_plane.obj", false);
  ground->addTexture(aiString("grass.bmp"), "texture_diffuse", true);

  // Initialize the GLSL programs
  prog0 = make_ref<Program>();
  prog0->attach(Shader::fromFile(GL_VERTEX_SHADER, RESOURCE_DIR + "tex_vert.glsl"),
                Shader::fromFile(GL_FRAGMENT_SHADER, RESOURCE_DIR + "tex_frag0.glsl"));

  prog1 = make_ref<Program>();
  prog1->attach(Shader::fromFile(GL_VERTEX_SHADER, RESOURCE_DIR + "tex_vert.glsl"),
                Shader::fromFile(GL_FRAGMENT_SHADER, RESOURCE_DIR + "tex_frag1.glsl"));

}

/****DRAW
This is the most important function in your program - this is where you
will actually issue the commands to draw any geometry you have set up to
draw
********/
static void render(sf::Window &window) {
  // Get current frame buffer size.
  auto windowSize = window.getSize();
  float aspect = (float) windowSize.x / windowSize.y;

  // Clear framebuffer.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  // Create the matrix stacks
  auto P = make_shared<MatrixStack>();
  auto MV = make_shared<MatrixStack>();
  P->pushMatrix();
  P->perspective(45.0f, aspect, 0.01f, 100.0f);

  //draw the dog mesh
  prog0->setUniform("P", P->topMatrix());

  MV->pushMatrix();

  MV->loadIdentity();
  MV->translate(vec3(0, cHeight, -5));

  MV->pushMatrix();
  MV->translate(vec3(-1, 0, 0));
  MV->rotate(cTheta, vec3(0, 1, 0));
  prog0->setUniform("MV", MV->topMatrix());

  dog->draw(prog0);

  MV->popMatrix();

  //draw the world sphere
  prog1->setUniform("P", P->topMatrix());

  MV->pushMatrix();
  MV->translate(vec3(1, 0, 0));
  MV->rotate(cTheta, vec3(0, 1, 0));
  prog1->setUniform("MV", MV->topMatrix());

  globe->draw(prog1);

  MV->popMatrix();

  //draw the ground plane
  prog0->setUniform("P", P->topMatrix());
  prog0->setUniform("MV", MV->topMatrix());

  ground->draw(prog0);

  // Pop matrix stacks.
  MV->popMatrix();
  P->popMatrix();
}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Please specify the resource directory." << std::endl;
    return 0;
  }
  RESOURCE_DIR = argv[1];

  /* your main will always include a similar set up to establish your window
    and GL context, etc. */

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

  sf::Window window(sf::VideoMode(g_width, g_height),
                    "Andrew Kennedy",
                    sf::Style::Default,
                    settings);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FRAMERATE_LIMIT);

  initialize();

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  // Initialize scene.
  // Loop until the user closes the window.
  auto running = true;
  while (running) {
    // Event handling
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:running = false;
          break;
        case sf::Event::KeyPressed:
          switch (event.key.code) {
            case sf::Keyboard::Escape:running = false;
              break;
            case sf::Keyboard::A:cTheta += 5;
              break;
            case sf::Keyboard::D:cTheta -= 5;
              break;
            case sf::Keyboard::S:cHeight += 0.5;
              break;
            case sf::Keyboard::W:cHeight -= 0.5;
              break;
            default: break;
          }
          break;
        case sf::Event::Resized:glViewport(0, 0, event.size.width, event.size.height);
          break;
        default: break;
      }
    }

    // Activate window for openGL rendering
    window.setActive();

    render(window);

    // end current frame and display to window.
    window.display();
  }

  window.close();
  return 0;

}
