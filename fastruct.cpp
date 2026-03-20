#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <clocale>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

namespace fs = std::filesystem;

const std::string GREEN       = "\033[32m";
const std::string YELLOW      = "\033[33m";
const std::string BLUE        = "\033[34m";
const std::string BRIGHT_BLUE = "\033[94m"; 
const std::string RED         = "\033[31m";
const std::string GRAY        = "\033[90m";
const std::string RESET       = "\033[0m";

// Source file extensions (lowercase)
const std::vector<std::string> source_ext = {
    ".cpp", ".c", ".h", ".hpp", ".cc", ".cxx",
    ".py", ".java", ".js", ".rs", ".go", ".rb",
    ".php", ".pl", ".sh", ".bash", ".zsh"
};

#ifdef _WIN32
// Executable file extensions on Windows
const std::vector<std::string> exec_ext = {
    ".exe", ".bat", ".cmd", ".com"
};
#endif

void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    _setmode(_fileno(stdout), _O_BINARY);
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#else
    std::setlocale(LC_ALL, "en_US.UTF-8");
#endif
    
    std::locale::global(std::locale(""));
}

std::string toLower(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) result += std::tolower(static_cast<unsigned char>(c));
    return result;
}

bool isSource(const fs::path& p) {
    std::string ext = toLower(p.extension().string());
    return std::find(source_ext.begin(), source_ext.end(), ext) != source_ext.end();
}

bool isExecutable(const fs::path& p) {
    fs::file_status s = fs::status(p);
    if (!fs::is_regular_file(s)) return false;
#ifdef _WIN32
    // On Windows, check the file extension
    std::string ext = toLower(p.extension().string());
    return std::find(exec_ext.begin(), exec_ext.end(), ext) != exec_ext.end();
#else
    // On Unix-like systems, check the execute permission bits
    auto perms = s.permissions();
    return (perms & fs::perms::owner_exec) != fs::perms::none ||
           (perms & fs::perms::group_exec) != fs::perms::none ||
           (perms & fs::perms::others_exec) != fs::perms::none;
#endif
}

std::string fileColour(const fs::path& p) {
    if (isExecutable(p)) return RED;
    if (isSource(p))      return BRIGHT_BLUE; 
    return BLUE;
}

// Recursive directory traversal
void traverse(const fs::path& currentAbs, const std::string& relPath,
              const std::string& startName, int depth, int maxDepth) {
    // Build the path to display for this directory
    std::string displayPath;
    if (relPath.empty()) {
        displayPath = startName;
        if (!displayPath.empty() && displayPath.back() != '/')
            displayPath += '/';
    } else {
        displayPath = startName + "/" + relPath + "/";
    }

    // Print directory header in green
    std::cout << GREEN << displayPath << RESET << std::endl;

    // Collect all entries
    std::vector<fs::directory_entry> entries;
    try {
        for (const auto& entry : fs::directory_iterator(currentAbs))
            entries.push_back(entry);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error reading directory " << currentAbs << ": " << e.what() << std::endl;
        return;
    }

    // Sort entries by filename
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename() < b.path().filename();
              });

    // Separate subdirectories for later recursion
    std::vector<fs::directory_entry> subdirs;

    // Output entries in the current directory
    for (const auto& entry : entries) {
        const auto& path = entry.path();

        if (fs::is_directory(entry.status()) && !fs::is_symlink(path)) {
            // Directory: print name in yellow
            std::cout << "-" << YELLOW << path.filename().string() << RESET << std::endl;
            subdirs.push_back(entry);
        } else {
            // File or other: determine colour
            std::string colour = fileColour(path);
            std::cout << "-" << colour << path.filename().string() << RESET
                      << " " << GRAY << fs::absolute(path).generic_string() << RESET << std::endl;
        }
    }

    // Recurse into subdirectories if depth allows
    if (maxDepth == -1 || depth + 1 <= maxDepth) {
        for (const auto& subdir : subdirs) {
            std::string subName = subdir.path().filename().string();
            std::string newRelPath = relPath.empty() ? subName : relPath + "/" + subName;
            traverse(subdir.path(), newRelPath, startName, depth + 1, maxDepth);
        }
    }
}

int main(int argc, char* argv[]) {
    setupConsole();

    fs::path startPath = fs::current_path();   // default
    int maxDepth = -1;                           // -1 means unlimited

    // Parse command‑line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-d" && i + 1 < argc) {
            startPath = fs::absolute(fs::path(argv[++i]));
        } else if (arg == "--depth" && i + 1 < argc) {
            try {
                maxDepth = std::stoi(argv[++i]);
                if (maxDepth < 0) {
                    std::cerr << "Error: --depth must be non‑negative." << std::endl;
                    return 1;
                }
            } catch (...) {
                std::cerr << "Error: --depth requires an integer argument." << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    // Validate the starting directory
    if (!fs::exists(startPath)) {
        std::cerr << "Error: Path does not exist: " << startPath << std::endl;
        return 1;
    }
    if (!fs::is_directory(startPath)) {
        std::cerr << "Error: Path is not a directory: " << startPath << std::endl;
        return 1;
    }

    // Absolute path and its base name for display
    fs::path absStart = fs::absolute(startPath);
    std::string startName = absStart.filename().string();
    if (startName.empty()) {                 // e.g. root directory
        startName = absStart.generic_string();
        if (!startName.empty() && startName.back() == '/')
            startName.pop_back();             // remove trailing slash, we will add it later
    }

    // Begin traversal
    traverse(absStart, "", startName, 0, maxDepth);

    return 0;
}
