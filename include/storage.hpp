#pragma once
#include <string>
#include <vector>

struct Credential {
    std::string site;
    std::string username;
    std::string password;
    std::string notes;
};

bool load_entries(std::vector<Credential>& entries);
bool save_entries(const std::vector<Credential>& entries);
Credential* find_entry(std::vector<Credential>& entries, const std::string& site);
bool delete_entry(std::vector<Credential>& entries, const std::string& site); 