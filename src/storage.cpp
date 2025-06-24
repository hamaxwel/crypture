#include "storage.hpp"
#include "crypto.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

const char* SECRETS_FILE = "secrets.enc";

using json = nlohmann::json;

bool load_entries(std::vector<Credential>& entries) {
    std::ifstream in(SECRETS_FILE, std::ios::binary);
    if (!in) return true; // No file yet, treat as empty
    std::string cipher((std::istreambuf_iterator<char>(in)), {});
    std::string plain = decrypt(cipher);
    if (plain.empty()) return false;
    auto j = json::parse(plain, nullptr, false);
    if (j.is_discarded()) return false;
    entries = j.get<std::vector<Credential>>();
    return true;
}

bool save_entries(const std::vector<Credential>& entries) {
    json j = entries;
    std::string plain = j.dump();
    std::string cipher = encrypt(plain);
    if (cipher.empty()) return false;
    std::ofstream out(SECRETS_FILE, std::ios::binary);
    out.write(cipher.data(), cipher.size());
    return true;
}

Credential* find_entry(std::vector<Credential>& entries, const std::string& site) {
    for (auto& c : entries) {
        if (c.site == site) return &c;
    }
    return nullptr;
}

bool delete_entry(std::vector<Credential>& entries, const std::string& site) {
    auto it = std::remove_if(entries.begin(), entries.end(), [&](const Credential& c) { return c.site == site; });
    if (it == entries.end()) return false;
    entries.erase(it, entries.end());
    return true;
}

// nlohmann::json serialization
void to_json(json& j, const Credential& c) {
    j = json{{"site", c.site}, {"username", c.username}, {"password", c.password}, {"notes", c.notes}};
}
void from_json(const json& j, Credential& c) {
    j.at("site").get_to(c.site);
    j.at("username").get_to(c.username);
    j.at("password").get_to(c.password);
    j.at("notes").get_to(c.notes);
} 