#include "crypto.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <vector>
#include <cstring>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {
static std::vector<unsigned char> g_key(32);
static std::vector<unsigned char> g_iv(12); // 96 bits for GCM
static const int PBKDF2_ITER = 100000;
static const unsigned char g_salt[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; // TODO: randomize and store
}

std::string prompt_for_master_password() {
    std::string pwd;
    std::cout << "Master Password: ";
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b' && !pwd.empty()) {
            pwd.pop_back();
            std::cout << "\b \b";
        } else if (ch != '\b') {
            pwd.push_back(ch);
            std::cout << '*';
        }
    }
#else
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, pwd);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    std::cout << std::endl;
    return pwd;
}

bool initialize_crypto(const std::string& masterPassword) {
    // Derive key
    if (PKCS5_PBKDF2_HMAC(masterPassword.c_str(), masterPassword.size(), g_salt, sizeof(g_salt), PBKDF2_ITER, EVP_sha256(), g_key.size(), g_key.data()) != 1)
        return false;
    // Generate IV
    if (RAND_bytes(g_iv.data(), g_iv.size()) != 1)
        return false;
    return true;
}

std::string encrypt(const std::string& plain) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> ciphertext(plain.size() + 16);
    int len, ciphertext_len;
    std::vector<unsigned char> tag(16);
    std::string out;
    if (!ctx) return "";
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) goto error;
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, g_key.data(), g_iv.data()) != 1) goto error;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (const unsigned char*)plain.data(), plain.size()) != 1) goto error;
    ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) goto error;
    ciphertext_len += len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) goto error;
    EVP_CIPHER_CTX_free(ctx);
    // Output: [IV][CIPHERTEXT][TAG]
    out.assign((char*)g_iv.data(), g_iv.size());
    out.append((char*)ciphertext.data(), ciphertext_len);
    out.append((char*)tag.data(), tag.size());
    return out;
error:
    EVP_CIPHER_CTX_free(ctx);
    return "";
}

std::string decrypt(const std::string& cipher) {
    if (cipher.size() < g_iv.size() + 16) return "";
    const unsigned char* iv = (const unsigned char*)cipher.data();
    const unsigned char* ciphertext = (const unsigned char*)cipher.data() + g_iv.size();
    int ciphertext_len = cipher.size() - g_iv.size() - 16;
    const unsigned char* tag = (const unsigned char*)cipher.data() + cipher.size() - 16;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> plain(ciphertext_len + 1);
    int len, plain_len;
    if (!ctx) return "";
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) goto error;
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, g_key.data(), iv) != 1) goto error;
    if (EVP_DecryptUpdate(ctx, plain.data(), &len, ciphertext, ciphertext_len) != 1) goto error;
    plain_len = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)tag) != 1) goto error;
    if (EVP_DecryptFinal_ex(ctx, plain.data() + len, &len) != 1) goto error;
    plain_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return std::string((char*)plain.data(), plain_len);
error:
    EVP_CIPHER_CTX_free(ctx);
    return "";
} 