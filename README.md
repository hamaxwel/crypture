# cpppass

A tiny, fast, offline C++ password manager (CLI).

## Features
- Securely store credentials (site, username, password, notes)
- AES-256-GCM encryption (OpenSSL)
- Master password (PBKDF2 key derivation)
- All data in a single encrypted file (`secrets.enc`)
- CLI: add, get, list, delete, search, export, import
- Interactive mode (REPL) with command aliases and suggestions
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

### Interactive Mode
Run with no arguments to enter interactive mode:
```
./build/cpppass
```
You will see a prompt:
```
Master Password: <your-master-password>
Welcome to cpppass (v1.0.0) - Encrypted Credential Manager
Type 'help' for commands, 'exit' to quit.
cpppass>
```

### Commands (in interactive mode or as arguments)
- `add`                Add a new credential
- `get`                Retrieve a credential by site
- `list` or `ls`       List all sites
- `delete` or `rm`     Delete a credential by site
- `search <term>` or `s <term>`  Search credentials by site, username, or notes
- `export <file>`      Export credentials to an encrypted file
- `import <file>`      Import credentials from an encrypted file
- `help` or `h`        Show help message
- `version`            Show version info
- `exit` or `quit`     Exit interactive mode

You can also run commands directly:
```
./build/cpppass add
./build/cpppass list
./build/cpppass search gmail
./build/cpppass export backup.enc
```

### Example Session
```
cpppass> add
cpppass> list
cpppass> search facebook
cpppass> export mybackup.enc
cpppass> import mybackup.enc
cpppass> help
cpppass> exit
```

## Security Notes
- Master password never stored on disk
- All credentials encrypted with AES-256-GCM
- Key derived with PBKDF2 (100,000 rounds)
- Salt and IV are static for MVP (improve for production)

## License
MIT 