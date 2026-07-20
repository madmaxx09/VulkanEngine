#include "model.h"

Model *ModelLoader::loadGltfModel(const std::string pathToFile)
{
    const std::string modelName = std::filesystem::path(pathToFile).stem().string();

    auto it = models.find(modelName);

    if (it != models.end()) {
        std::cout << "model already loaded right ?" << std::endl;
        return it->second.get();
    }

    auto newModel = std::make_unique<Model>(modelName);

    if (!loadModel(pathToFile, *newModel))
        return nullptr;

    models[modelName] = std::move(newModel);

    return models[modelName].get();
}