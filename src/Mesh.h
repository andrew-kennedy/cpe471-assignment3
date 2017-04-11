#pragma once

#include <string>
#include <vector>
#include <memory>
#include <globjects/globjects.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace globjects;
using namespace std;

class Mesh {
public:
  Mesh(vector<glm::vec3> positions,
       vector<glm::vec3> normals,
       vector<glm::vec2> texCoords,
       vector<int> indices,
       vector<ref_ptr<Texture>> textures);
  void draw(ref_ptr<Program> shader) const;
  void addTexture(ref_ptr<Texture> tex);
  void setUniforms(unordered_map<string, ref_ptr<AbstractUniform>> uniforms);

private:
  void init();
  vector<int> indices;
  vector<glm::vec3> positions;
  vector<glm::vec3> normals;
  vector<glm::vec2> texCoords;
  vector<ref_ptr<Texture>> textures;
  unordered_map<string, ref_ptr<AbstractUniform>> uniforms;
  ref_ptr<Buffer> elemBuf;
  ref_ptr<Buffer> posBuf;
  ref_ptr<Buffer> normBuf;
  ref_ptr<Buffer> texCoordBuf;
  ref_ptr<VertexArray> vao;
};

