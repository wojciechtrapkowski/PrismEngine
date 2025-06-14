#include "loaders/mesh_loader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>

namespace Prism::Loaders {
    namespace {
        constexpr std::string_view MODELS_DIR = "models/";
        constexpr unsigned int MODELS_LOADING_FLAGS =
            aiProcess_Triangulate | aiProcess_FlipUVs;

        using MeshDescriptor = Resources::MeshResource::MeshDescriptor;
        using Vertex = Resources::MeshResource::Vertex;
        using Index = Resources::MeshResource::Index;

        std::optional<MeshDescriptor> loadModel(Assimp::Importer &importer,
                                                const std::string &path) {
            const aiScene *scene = importer.ReadFile(
                std::string(MODELS_DIR) + path, MODELS_LOADING_FLAGS);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
                !scene->mRootNode) {

                std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString()
                          << std::endl;

                return std::nullopt;
            }

            std::vector<Vertex> vertices;
            std::vector<Index> indices;

            size_t vertexOffset = 0;

            for (size_t m = 0; m < scene->mNumMeshes; ++m) {
                aiMesh *mesh = scene->mMeshes[m];

                for (size_t v = 0; v < mesh->mNumVertices; ++v) {
                    Vertex vert;
                    vert.position = {mesh->mVertices[v].x, mesh->mVertices[v].y,
                                     mesh->mVertices[v].z};
                    vertices.push_back(std::move(vert));

                    // TODO: In the future add parsing normals as well.
                }

                // Collect indices (faces)
                for (size_t f = 0; f < mesh->mNumFaces; ++f) {
                    const aiFace &face = mesh->mFaces[f];
                    for (size_t i = 0; i < face.mNumIndices; ++i) {
                        Index idx{};
                        idx.idx = face.mIndices[i] + vertexOffset;
                        indices.push_back(std::move(idx));
                    }
                }

                vertexOffset += mesh->mNumVertices;
            }

            return MeshDescriptor{.vertices = std::move(vertices),
                                  .indices = std::move(indices)};
        }
    } // namespace

    MeshLoader::result_type
    MeshLoader::operator()(const std::string &path) const {
        Assimp::Importer importer;

        auto loadedModelDescriptorOpt = loadModel(importer, path);
        if (!loadedModelDescriptorOpt) {
            return std::nullopt;
        }

        auto &loadedModelDescriptor = *loadedModelDescriptorOpt;
        Resources::MeshResource meshResource{std::move(loadedModelDescriptor)};

        return {
            std::make_unique<Resources::MeshResource>(std::move(meshResource))};
    }; // namespace Prism::Loaders
} // namespace Prism::Loaders