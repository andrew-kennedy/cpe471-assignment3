#pragma once

#include <string>
#include "Mesh.h"

using namespace std;

// returns a texture loaded from a file.
ref_ptr<Texture> TextureFromFile(char const *path, string directory, bool tile = false);

class Model {
public:
  Model(string path, bool normalize = true);
  void draw(ref_ptr<Program> shader);
  void addTexture(aiString const path, string const typeName, bool tile = false);
private:
  vector<Mesh> meshes;
  string directory;
  unordered_map<string, ref_ptr<Texture>> textures_loaded;

  void loadModel(string path, bool normalize);
  void processNode(aiNode *node, aiScene const *scene);
  Mesh processMesh(aiMesh *mesh, aiScene const *scene);
  vector<ref_ptr<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};


