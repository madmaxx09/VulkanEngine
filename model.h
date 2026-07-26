#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include "mesh_component.h"
#include "tinygltfloader.h"

#include "renderer.h"

class Model
{
    public:
        Model(std::string name) : name(std::move(name)) {

        };
        ~Model() {

        };

    private:
        const std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

};

class ModelLoader
{
    public:
        ModelLoader(Renderer *renderer) {
            if (!init(renderer))
                throw std::runtime_error("Model loader not initialised, renderer unavailable");    
        };

        ~ModelLoader() {

        };

        Model *loadGltfModel(const std::string pathToFile);

        //bool loadSceneTroughGltf();     

        std::vector<std::string> getLoadedModelsNames() {
            std::vector<std::string> names;

            for (auto &model : models)
            {
                names.push_back(model.first);
            }
            return names;
        };
        
    
    private:
        bool init(Renderer *renderer) 
        {
            if (!renderer)
                return false;
            _renderer = renderer;
            return true;
        }



        std::unordered_map<std::string, std::unique_ptr<Model>> models;

        Renderer *_renderer = nullptr;

};