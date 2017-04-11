#include <iostream>
#include <glbinding/gl/gl.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/globjects.h>
#include <globjects/Texture.h>
#include <glm/vec3.hpp>

#include "Mesh.h"

using namespace std;
using namespace gl;
using namespace globjects;

Mesh::Mesh(vector<glm::vec3> positions,
           vector<glm::vec3> normals,
           vector<glm::vec2> texCoords,
           vector<int> indices,
           vector<ref_ptr<Texture>> textures) :
    elemBuf(make_ref<Buffer>()),
    posBuf(make_ref<Buffer>()),
    normBuf(make_ref<Buffer>()),
    texCoordBuf(make_ref<Buffer>()),
    vao(globjects::make_ref<VertexArray>()) {
  this->positions = positions;
  this->normals = normals;
  this->texCoords = texCoords;
  this->indices = indices;
  this->textures = textures;

  this->init();
}

void Mesh::init() {

  elemBuf->setData(indices, GL_STATIC_DRAW);
  vao->bind();
  elemBuf->bind(GL_ELEMENT_ARRAY_BUFFER);

  auto binding = vao->binding(0);
  posBuf->setData(positions, GL_STATIC_DRAW);
  binding->setAttribute(0);
  binding->setBuffer(posBuf, 0, sizeof(float) * 3);
  binding->setFormat(3, GL_FLOAT, GL_TRUE);
  vao->enable(0);

  if (!normals.empty()) {
    normBuf->setData(normals, GL_STATIC_DRAW);
    auto binding1 = vao->binding(1);
    binding1->setAttribute(1);
    binding1->setBuffer(normBuf, 0, sizeof(float) * 3);
    binding1->setFormat(3, GL_FLOAT, GL_TRUE);
    vao->enable(1);
  }

  if (!texCoords.empty()) {
    texCoordBuf->setData(texCoords, GL_STATIC_DRAW);
    auto binding2 = vao->binding(2);
    binding2->setAttribute(2);
    binding2->setBuffer(texCoordBuf, 0, sizeof(float) * 2);
    binding2->setFormat(2, GL_FLOAT);
    vao->enable(2);
  }

  vao->unbind();

}

void Mesh::draw(ref_ptr<Program> shader) const {
  shader->use();

  int diffuseNr = 1;
  int specularNr = 1;
  for (int i = 0; i < this->textures.size(); i++) {
    string number;
    auto name = this->textures[i]->name();
    if (name == "texture_diffuse") {
      number = std::to_string(diffuseNr++);
    } else if (name == "texture_specular") {
      number = std::to_string(specularNr++);
    }
    shader->setUniform(name + number, i);
    this->textures[i]->bindActive(i);
  }

  // set all uniforms
  // this allows for overriding previously defined uniforms with ones
  // from the model/mesh
  for (auto &u : uniforms) {
    shader->addUniform(u.second);
  }



  vao->bind();
  elemBuf->bind(GL_ELEMENT_ARRAY_BUFFER);
  vao->drawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT);
  vao->unbind();
  elemBuf->unbind(GL_ELEMENT_ARRAY_BUFFER);

  for (int i = 0; i < this->textures.size(); i++) {
    this->textures[i]->unbindActive(i);
  }

  shader->release();
}

void Mesh::addTexture(ref_ptr<Texture> tex) {
  textures.push_back(tex);
}
void Mesh::setUniforms(unordered_map<string, ref_ptr<AbstractUniform>> uniforms) {
  this->uniforms = uniforms;
}

