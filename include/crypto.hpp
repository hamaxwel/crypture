#pragma once
#include <string>

bool initialize_crypto(const std::string& masterPassword);
std::string encrypt(const std::string& plain);
std::string decrypt(const std::string& cipher);
std::string prompt_for_master_password(); 