#include <iostream>

#include <glbinding/gl/gl.h>
#include <globjects/Texture.h>
#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

#include "Model.h"

using namespace std;
using namespace globjects;
using namespace gl;

Model::Model(string path, bool normalize) {
  this->loadModel(path, normalize);
}

void Model::draw(ref_ptr<Program> shader) {
  for (auto &mesh : this->meshes) {
    mesh.draw(shader);
  }
}

void Model::loadModel(string path, bool normalize) {
  // Some obj files contain material information.
  // We'll ignore them for this assignment.
  Assimp::Importer import;
  import.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, normalize);
  aiScene const *scene =
      import.ReadFile(path,
                      aiProcess_Triangulate
                      | aiProcess_GenSmoothNormals
                      | aiProcess_PreTransformVertices);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
    return;
  }

  // retrieve the directory of the filepath
  this->directory = path.substr(0, path.find_last_of('/'));

  // process ASSIMP's root node recursively
  this->processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, aiScene const *scene) {
  for (int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    this->meshes.push_back(this->processMesh(mesh, scene));
  }
  for (int i = 0; i < node->mNumChildren; i++) {
    this->processNode(node->mChildren[i], scene);
  }
}

Mesh Model::processMesh(aiMesh *mesh, aiScene const *scene) {

  // Data to fill
  vector<glm::vec3> positions;
  vector<glm::vec3> normals;
  vector<glm::vec2> texCoords;
  vector<int> indices;
  vector<ref_ptr<Texture>> textures;

  for (int i = 0; i < mesh->mNumVertices; i++) {
    auto aiVec = mesh->mVertices[i];
    positions.push_back(glm::vec3(aiVec.x, aiVec.y, aiVec.z));

    aiVec = mesh->mNormals[i];
    normals.push_back(glm::vec3(aiVec.x, aiVec.y, aiVec.z));

    if (mesh->mTextureCoords[0]) {
      aiVec = mesh->mTextureCoords[0][i];
      glm::vec2 vector;
      vector.x = aiVec.x;
      vector.y = aiVec.y;
      texCoords.push_back(vector);
    } else {
      texCoords.push_back(glm::vec2(0.0f));
    }
  }

  // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
  for (int i = 0; i < mesh->mNumFaces; i++) {
    auto face = mesh->mFaces[i];
    // Retrieve all indices of the face and store them in the indices vector
    for (int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  auto material = scene->mMaterials[mesh->mMaterialIndex];
  // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
  // Same applies to other texture as the following list summarizes:
  // Diffuse: texture_diffuseN
  // Specular: texture_specularN
  // Normal: texture_normalN

  // 1. Diffuse maps
  vector<ref_ptr<Texture>> diffuseMaps = this->loadMaterialTextures(material,
                                                                    aiTextureType_DIFFUSE,
                                                                    "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

  // 2. Specular maps
  vector<ref_ptr<Texture>> specularMaps = this->loadMaterialTextures(material,
                                                                     aiTextureType_SPECULAR,
                                                                     "texture_specular");
  textures.insert(textures.cend(), specularMaps.begin(), specularMaps.end());

  return Mesh(positions, normals, texCoords, indices, textures);
}

vector<ref_ptr<Texture>> Model::loadMaterialTextures(aiMaterial *mat,
                                                     aiTextureType type,
                                                     string const typeName) {
  vector<ref_ptr<Texture>> textures;

  for (int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    auto stdStr = string(str.C_Str());
    // cache textures loaded so we don't load from file twice, an optimization in case multiple
    // meshes in a model use the same texture.
    if (textures_loaded.count(stdStr) > 0) {
      // texture already loaded
      textures.push_back(textures_loaded[stdStr]);
    } else {
      auto texture = TextureFromFile(str.C_Str(), this->directory);
      // give the texture the name of it's type for later binding
      texture->setName(typeName);
      textures_loaded[stdStr] = texture;
      textures.push_back(texture);
    }
  }

  return textures;
}

void Model::addTexture(aiString const path, string const typeName, bool tile) {

  auto stdPath = string(path.C_Str());

  if (textures_loaded.count(stdPath) > 0) {
    return;
  } else {
    auto texture = TextureFromFile(path.C_Str(), this->directory, tile);
    texture->setName(typeName);
    textures_loaded[stdPath] = texture;
    for (auto &mesh : meshes) {
      mesh.addTexture(texture);
    }
  }

}

ref_ptr<Texture> TextureFromFile(const char *path, string directory, bool tile) {
  //Generate texture ID and load texture data
  string filename = directory + '/' + string(path);
  ref_ptr<Texture> texture = make_ref<Texture>();
  ilInit();
  ilutRenderer(ILUT_OPENGL);
  int imageID = ilGenImage();
  ilBindImage(imageID);
  ilLoadImage(filename.c_str());

  ILinfo info;
  iluGetImageInfo(&info);
  if (info.Format == IL_BGR) {
    ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
  }

  auto dimensions = glm::vec2(info.Width, info.Height);

  texture->bind();

  texture->image2D(0, GL_RGB8, dimensions, 0, GL_RGB, GL_UNSIGNED_BYTE, info.Data);

  texture->generateMipmap();

  if (tile) {
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
  } else {
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texture->unbind();

  ilDeleteImage(imageID);

  return texture;
}
