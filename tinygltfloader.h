#pragma once

#include <string>


class Model;

bool loadModel(const std::string& pathToFile, Model& toLoadIn);
