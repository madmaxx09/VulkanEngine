#include "tinygltfloader.h"
#define TINYGLTF3_ENABLE_FS

#include "model.h"
#include "./external/tinygltf/tiny_gltf_v3.h"

#include <iostream>

static bool checkScene(tg3_model &modelToValidate)
{
    if (modelToValidate.scenes_count > 1)
        return false;
    if (modelToValidate.meshes_count == 0)
        return false;

    return true;
}

bool loadModel(const std::string &pathToFile, Model &toLoadIn)
{

    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model tg_model;

    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    tg3_error_code err = tg3_parse_file(&tg_model, &errors, pathToFile.c_str(), pathToFile.size(), &opts);
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count; i++) {
            fprintf(stderr, "[%d] %s\n", (int)errors.entries[i].severity,
                    errors.entries[i].message ? errors.entries[i].message : "(null)");
        }
    }
    if (!checkScene(tg_model))
    {
        tg3_model_free(&tg_model);
        tg3_error_stack_free(&errors);
        return false ;
    }

    for (uint32_t meshIndex = 0; meshIndex < tg_model.meshes_count; meshIndex++)
    {
        const tg3_mesh &mesh = tg_model.meshes[meshIndex];

        for (uint32_t primitiveIndex = 0; primitiveIndex < mesh.primitives_count; primitiveIndex++)
        {
            const tg3_primitive &primitive = mesh.primitives[primitiveIndex];
            
            for (uint32_t attrIndex = 0; attrIndex < primitive.attributes_count; attrIndex++)
            {
                const auto& attr = primitive.attributes[attrIndex];
                std::cout << std::string(attr.key.data, attr.key.len) << std::endl;;
            }
        }
    }


    tg3_model_free(&tg_model);
    tg3_error_stack_free(&errors);


    return true;
}