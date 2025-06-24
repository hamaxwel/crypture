# cpppass

A tiny, fast, offline C++ password manager (CLI).

## Features
- Securely store credentials (site, username, password, notes)
- AES-256-GCM encryption (OpenSSL)
- Master password (PBKDF2 key derivation)
- All data in a single encrypted file (`secrets.enc`)
- CLI: add, get, list, delete
- Cross-platform (Windows/Linux)

## Build

Requirements:
- CMake 3.10+
- OpenSSL
- Git (for nlohmann/json)

```
git clone <repo-url>
cd cpppass
cmake -B build
cmake --build build
```

## Usage

```
./cpppass [add|get|list|delete]
```

## Security Notes
- Master password never stored on disk
- All credentials encrypted with AES-256-GCM
- Key derived with PBKDF2 (100,000 rounds)
- Salt and IV are static for MVP (improve for production)

## License
MIT 