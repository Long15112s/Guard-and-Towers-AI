#pragma once
#include <string>
bool validateFENPlacement(const std::string& placement);
std::string padPlacement(const std::string& placement);
bool hasSoldierWithoutHeight(const std::string& placement);
