#include "cli.hpp"
#include "storage.hpp"
#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <map>
#include <string>

static std::vector<Credential> g_entries;
#define CPPPASS_VERSION "1.0.0"

// Helper: lowercase a string
std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

// Helper: suggest closest command
std::string suggest_command(const std::string& input, const std::vector<std::string>& commands) {
    int min_dist = 1000;
    std::string best;
    for (const auto& cmd : commands) {
        int dist = 0;
        for (size_t i = 0; i < std::min(input.size(), cmd.size()); ++i)
            if (input[i] != cmd[i]) ++dist;
        dist += std::abs((int)input.size() - (int)cmd.size());
        if (dist < min_dist) { min_dist = dist; best = cmd; }
    }
    return best;
}

void add_entry() {
    Credential c;
    std::cout << "Site (required): ";
    std::getline(std::cin, c.site);
    std::cout << "Username: ";
    std::getline(std::cin, c.username);
    std::cout << "Password: ";
    std::getline(std::cin, c.password);
    std::cout << "Notes (optional): ";
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
    std::cout << "Are you sure you want to delete '" << site << "'? (y/N): ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (to_lower(confirm) != "y" && to_lower(confirm) != "yes") {
        std::cout << "Delete cancelled.\n";
        return;
    }
    if (delete_entry(g_entries, site)) {
        save_entries(g_entries);
        std::cout << "Deleted.\n";
    } else {
        std::cout << "Not found.\n";
    }
}

void search_entries(const std::string& term) {
    bool found = false;
    for (const auto& c : g_entries) {
        if (c.site.find(term) != std::string::npos ||
            c.username.find(term) != std::string::npos ||
            c.notes.find(term) != std::string::npos) {
            std::cout << "Site: " << c.site << "\nUsername: " << c.username << "\nPassword: " << c.password << "\nNotes: " << c.notes << "\n---\n";
            found = true;
        }
    }
    if (!found) std::cout << "No matching entries found.\n";
}

void export_entries(const std::string& filename) {
    nlohmann::json j = g_entries;
    std::string plain = j.dump();
    std::string cipher = encrypt(plain);
    if (cipher.empty()) {
        std::cout << "Encryption failed.\n";
        return;
    }
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cout << "Failed to open file for writing.\n";
        return;
    }
    out.write(cipher.data(), cipher.size());
    std::cout << "Exported encrypted credentials to " << filename << "\n";
}

void import_entries(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cout << "Failed to open file for reading.\n";
        return;
    }
    std::string cipher((std::istreambuf_iterator<char>(in)), {});
    std::string plain = decrypt(cipher);
    if (plain.empty()) {
        std::cout << "Decryption failed or file is empty.\n";
        return;
    }
    auto j = nlohmann::json::parse(plain, nullptr, false);
    if (j.is_discarded()) {
        std::cout << "Failed to parse JSON.\n";
        return;
    }
    std::vector<Credential> imported = j.get<std::vector<Credential>>();
    g_entries.insert(g_entries.end(), imported.begin(), imported.end());
    save_entries(g_entries);
    std::cout << "Imported " << imported.size() << " credentials from " << filename << "\n";
}

void print_help() {
    std::cout << "cpppass - Encrypted Credential Manager\n";
    std::cout << "\nUsage:\n";
    std::cout << "  cpppass add                Add a new credential\n";
    std::cout << "  cpppass get                Retrieve a credential by site\n";
    std::cout << "  cpppass list               List all sites\n";
    std::cout << "  cpppass delete             Delete a credential by site\n";
    std::cout << "  cpppass search <term>      Search credentials by site, username, or notes\n";
    std::cout << "  cpppass export <file>      Export credentials to an encrypted file\n";
    std::cout << "  cpppass import <file>      Import credentials from an encrypted file\n";
    std::cout << "  cpppass help, --help       Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  cpppass add\n";
    std::cout << "  cpppass get\n";
    std::cout << "  cpppass list\n";
    std::cout << "  cpppass search gmail\n";
    std::cout << "  cpppass export backup.enc\n";
    std::cout << "  cpppass import backup.enc\n";
}

void print_version() {
    std::cout << "cpppass version " << CPPPASS_VERSION << "\n";
}

void repl() {
    std::cout << "Welcome to cpppass (v" << CPPPASS_VERSION << ") - Encrypted Credential Manager\n";
    std::cout << "Type 'help' for commands, 'exit' to quit.\n";
    std::string line;
    std::map<std::string, std::string> aliases = {
        {"ls", "list"}, {"rm", "delete"}, {"s", "search"}, {"h", "help"}
    };
    std::vector<std::string> valid_cmds = {"add","get","list","delete","search","export","import","help","--help","ls","rm","s","h","exit","quit","version","--version"};
    while (true) {
        std::cout << "cpppass> ";
        if (!std::getline(std::cin, line)) {
            std::cout << "\nGoodbye!\n";
            break;
        }
        if (line.empty()) continue;
        std::string cmd, arg;
        auto pos = line.find(' ');
        if (pos != std::string::npos) {
            cmd = to_lower(line.substr(0, pos));
            arg = line.substr(pos + 1);
        } else {
            cmd = to_lower(line);
        }
        if (aliases.count(cmd)) cmd = aliases[cmd];
        if (cmd == "exit" || cmd == "quit") break;
        if (cmd == "help" || cmd == "--help") { print_help(); continue; }
        if (cmd == "version" || cmd == "--version") { print_version(); continue; }
        else if (cmd == "add") add_entry();
        else if (cmd == "get") get_entry();
        else if (cmd == "list") list_entries();
        else if (cmd == "delete") delete_entry();
        else if (cmd == "search") {
            if (arg.empty()) std::cout << "Error: Usage: search <term>\n";
            else search_entries(arg);
        }
        else if (cmd == "export") {
            if (arg.empty()) std::cout << "Error: Usage: export <file>\n";
            else export_entries(arg);
        }
        else if (cmd == "import") {
            if (arg.empty()) std::cout << "Error: Usage: import <file>\n";
            else import_entries(arg);
        }
        else {
            std::cout << "Error: Unknown command. ";
            std::string suggestion = suggest_command(cmd, valid_cmds);
            if (!suggestion.empty()) std::cout << "Did you mean '" << suggestion << "'?\n";
            print_help();
        }
    }
    std::cout << "Exiting interactive mode.\n";
}

int handle_cli(int argc, char* argv[]) {
    if (!load_entries(g_entries)) {
        std::cout << "Error: Failed to load entries.\n";
        return 1;
    }
    std::map<std::string, std::string> aliases = {
        {"ls", "list"}, {"rm", "delete"}, {"s", "search"}, {"h", "help"}
    };
    std::vector<std::string> valid_cmds = {"add","get","list","delete","search","export","import","help","--help","ls","rm","s","h","version","--version"};
    if (argc < 2) {
        repl();
        return 0;
    }
    std::string command = to_lower(argv[1]);
    if (aliases.count(command)) command = aliases[command];
    if (command == "version" || command == "--version") { print_version(); return 0; }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush leftover input
    if (command == "help" || command == "--help") { print_help(); return 0; }
    else if (command == "add") add_entry();
    else if (command == "get") get_entry();
    else if (command == "list") list_entries();
    else if (command == "delete") delete_entry();
    else if (command == "search" && argc > 2) search_entries(argv[2]);
    else if (command == "export" && argc > 2) export_entries(argv[2]);
    else if (command == "import" && argc > 2) import_entries(argv[2]);
    else {
        std::cout << "Error: Unknown command. ";
        std::string suggestion = suggest_command(command, valid_cmds);
        if (!suggestion.empty()) std::cout << "Did you mean '" << suggestion << "'?\n";
        print_help();
    }
    return 0;
} 