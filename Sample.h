#pragma once
#include <string>
#include "json.hpp"

struct Sample {
    int         id    = 0;
    std::string name;
    std::string spec;
    int         stock = 0;
};

inline void to_json(nlohmann::json& j, const Sample& s) {
    j = { {"id", s.id}, {"name", s.name}, {"spec", s.spec}, {"stock", s.stock} };
}

inline void from_json(const nlohmann::json& j, Sample& s) {
    j.at("id").get_to(s.id);
    j.at("name").get_to(s.name);
    j.at("spec").get_to(s.spec);
    j.at("stock").get_to(s.stock);
}
