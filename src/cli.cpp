#include "cli.hpp"
#include "storage.hpp"
#include <iostream>
#include <vector>
#include <limits>

static std::vector<Credential> g_entries;

void add_entry() {
    Credential c;
    std::cout << "Site: ";
    std::getline(std::cin, c.site);
    std::cout << "Username: ";
    std::getline(std::cin, c.username);
    std::cout << "Password: ";
    std::getline(std::cin, c.password);
    std::cout << "Notes: ";
    std::getline(std::cin, c.notes);
    g_entries.push_back(c);
    if (save_entries(g_entries))
        std::cout << "Entry added.\n";
    else
        std::cout << "Failed to save entry!\n";
}

void get_entry() {
    std::string site;
    std::cout << "Site to retrieve: ";
    std::getline(std::cin, site);
    Credential* c = find_entry(g_entries, site);
    if (c) {
        std::cout << "Username: " << c->username << "\n";
        std::cout << "Password: " << c->password << "\n";
        std::cout << "Notes: " << c->notes << "\n";
    } else {
        std::cout << "Not found.\n";
    }
}

void list_entries() {
    if (g_entries.empty()) {
        std::cout << "No entries.\n";
        return;
    }
    std::cout << "Sites:\n";
    for (const auto& c : g_entries) {
        std::cout << "- " << c.site << "\n";
    }
}

void delete_entry() {
    std::string site;
    std::cout << "Site to delete: ";
    std::getline(std::cin, site);
    if (delete_entry(g_entries, site)) {
        save_entries(g_entries);
        std::cout << "Deleted.\n";
    } else {
        std::cout << "Not found.\n";
    }
}

int handle_cli(int argc, char* argv[]) {
    if (!load_entries(g_entries)) {
        std::cout << "Failed to load entries.\n";
        return 1;
    }
    if (argc < 2) {
        std::cout << "Usage: cpppass [add|get|list|delete]\n";
        return 0;
    }
    std::string command = argv[1];
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush leftover input
    if (command == "add") add_entry();
    else if (command == "get") get_entry();
    else if (command == "list") list_entries();
    else if (command == "delete") delete_entry();
    else std::cout << "Unknown command\n";
    return 0;
} 