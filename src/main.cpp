#include "cli.hpp"
#include "crypto.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string masterPassword = prompt_for_master_password();
    if (!initialize_crypto(masterPassword)) {
        std::cerr << "Failed to initialize crypto. Exiting.\n";
        return 1;
    }
    return handle_cli(argc, argv);
} 