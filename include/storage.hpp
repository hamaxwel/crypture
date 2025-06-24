#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Credential {
    std::string site;
    std::string username;
    std::string password;
    std::string notes;
};

// nlohmann::json serialization for Credential
inline void to_json(nlohmann::json& j, const Credential& c) {
    j = nlohmann::json{{"site", c.site}, {"username", c.username}, {"password", c.password}, {"notes", c.notes}};
}
inline void from_json(const nlohmann::json& j, Credential& c) {
    j.at("site").get_to(c.site);
    j.at("username").get_to(c.username);
    j.at("password").get_to(c.password);
    j.at("notes").get_to(c.notes);
}

bool load_entries(std::vector<Credential>& entries);
bool save_entries(const std::vector<Credential>& entries);
Credential* find_entry(std::vector<Credential>& entries, const std::string& site);
bool delete_entry(std::vector<Credential>& entries, const std::string& site); 