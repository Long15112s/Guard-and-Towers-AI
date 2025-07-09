#pragma once
#include "AI.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>    // für is_constructible_v

using AICreator = std::function<std::unique_ptr<ai::AI>(int depth)>;

struct AIInfo {
  std::string name;
  AICreator  create;
};

inline std::vector<AIInfo>& AIRegistry() {
  static std::vector<AIInfo> impls;
  return impls;
}

#define REGISTER_AI(ClassName)                                            \
  namespace {                                                             \
    static const bool _ai_registered_##__COUNTER__ = [](){                \
      AIRegistry().push_back({#ClassName,                                 \
        [](int d)->std::unique_ptr<ai::AI>{                               \
          if constexpr(std::is_constructible_v<ClassName,int>)           \
            return std::make_unique<ClassName>(d);                        \
          else                                                             \
            return std::make_unique<ClassName>();                         \
        }                                                                 \
      });                                                                  \
      return true;                                                         \
    }();                                                                   \
  }
