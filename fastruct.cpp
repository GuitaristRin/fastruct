#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <clocale>
#include <locale>
#include <sstream>

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
const std::string BOLD        = "\033[1m";
const std::string CYAN        = "\033[36m";

const std::vector<std::string> source_ext = {
    ".cpp", ".c", ".h", ".hpp", ".cc", ".cxx",
    ".py", ".java", ".js", ".rs", ".go", ".rb",
    ".php", ".pl", ".sh", ".bash", ".zsh"
};

#ifdef _WIN32
const std::vector<std::string> exec_ext = {
    ".exe", ".bat", ".cmd", ".com"
};
#endif

bool no_path = false;
std::string save_path = "";
std::ofstream output_file;
bool output_to_file = false;
int depth_warning_threshold = 5;
bool depth_limit_exceeded = false;

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
    std::string ext = toLower(p.extension().string());
    return std::find(exec_ext.begin(), exec_ext.end(), ext) != exec_ext.end();
    #else
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

void writeOutput(const std::string& text) {
    std::cout << text;
    if (output_to_file && output_file.is_open()) {
        std::string clean_text = text;
        size_t pos = 0;
        while ((pos = clean_text.find("\033", pos)) != std::string::npos) {
            size_t end = clean_text.find("m", pos);
            if (end != std::string::npos) {
                clean_text.erase(pos, end - pos + 1);
            }
        }
        output_file << clean_text;
    }
}

int estimateMaxDepth(const fs::path& start, int max_check = 1000) {
    int max_depth = 0;
    std::vector<std::pair<fs::path, int>> stack;
    stack.push_back({start, 0});
    int checked = 0;

    while (!stack.empty() && checked < max_check) {
        auto [path, depth] = stack.back();
        stack.pop_back();
        if (depth > max_depth) max_depth = depth;
        if (max_depth > depth_warning_threshold) return max_depth;

        try {
            if (fs::is_directory(path)) {
                for (const auto& entry : fs::directory_iterator(path)) {
                    if (fs::is_directory(entry.status())) {
                        stack.push_back({entry.path(), depth + 1});
                        checked++;
                        if (checked >= max_check) break;
                    }
                }
            }
        } catch (...) {
            continue;
        }
    }
    return max_depth;
}

bool confirmDeepDirectory(int max_depth) {
    if (max_depth <= depth_warning_threshold) return true;

    std::string prompt = BOLD + YELLOW + "[WARNING] " + RESET +
    "The directory depth (" + std::to_string(max_depth) +
    ") exceeds the warning threshold (" +
    std::to_string(depth_warning_threshold) +
    "). Are you sure you want to continue? (y/N): ";
    std::cout << prompt;
    std::string answer;
    std::getline(std::cin, answer);

    if (answer.empty() || (answer[0] != 'y' && answer[0] != 'Y')) {
        std::cout << YELLOW << "Operation cancelled by user." << RESET << std::endl;
        return false;
    }

    depth_limit_exceeded = true;
    return true;
}

std::string getPrefix(int depth) {
    if (depth_limit_exceeded && depth > depth_warning_threshold) {
        return std::string(depth_warning_threshold, '-');
    }
    return std::string(depth, '-');
}

void traverse(const fs::path& currentAbs, const std::string& relPath,
              const std::string& startName, int depth, int maxDepth) {
    std::string displayPath;
    if (relPath.empty()) {
        displayPath = startName;
        if (!displayPath.empty() && displayPath.back() != '/')
            displayPath += '/';
    } else {
        displayPath = startName + "/" + relPath + "/";
    }

    std::string dirPrefix = getPrefix(depth);
    if (depth > 0) {
        writeOutput(dirPrefix);
    }
    writeOutput(GREEN + displayPath + RESET + "\n");

    std::vector<fs::directory_entry> entries;
    try {
        for (const auto& entry : fs::directory_iterator(currentAbs))
            entries.push_back(entry);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error reading directory " << currentAbs << ": " << e.what() << std::endl;
        return;
    }

    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename() < b.path().filename();
              });

    std::vector<fs::directory_entry> subdirs;

    for (const auto& entry : entries) {
        const auto& path = entry.path();

        std::string prefix = getPrefix(depth + 1);

        if (fs::is_directory(entry.status()) && !fs::is_symlink(path)) {
            writeOutput(prefix);
            writeOutput(YELLOW + path.filename().string() + RESET + "\n");
            subdirs.push_back(entry);
        } else {
            std::string colour = fileColour(path);
            writeOutput(prefix);
            writeOutput(colour + path.filename().string() + RESET);

            if (!no_path) {
                writeOutput(" " + GRAY + fs::absolute(path).generic_string() + RESET);
            }
            writeOutput("\n");
        }
    }

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

                  fs::path startPath = fs::current_path();
                  int maxDepth = -1;

                  for (int i = 1; i < argc; ++i) {
                      std::string arg = argv[i];
                      if (arg == "-d" && i + 1 < argc) {
                          startPath = fs::absolute(fs::path(argv[++i]));
                      }
                      else if (arg == "--depth" && i + 1 < argc) {
                          try {
                              maxDepth = std::stoi(argv[++i]);
                              if (maxDepth < 0) {
                                  std::cerr << "Error: --depth must be non-negative." << std::endl;
                                  return 1;
                              }
                          } catch (...) {
                              std::cerr << "Error: --depth requires an integer argument." << std::endl;
                              return 1;
                          }
                      }
                      else if (arg == "--nopath") {
                          no_path = true;
                      }
                      else if (arg == "--saveto") {
                          if (i + 1 < argc && argv[i+1][0] != '-') {
                              save_path = argv[++i];
                              output_to_file = true;
                          } else {
                              save_path = fs::current_path().string();
                              output_to_file = true;
                          }
                      }
                      else {
                          std::cerr << "Unknown argument: " << arg << std::endl;
                          return 1;
                      }
                  }

                  if (!fs::exists(startPath)) {
                      std::cerr << "Error: Path does not exist: " << startPath << std::endl;
                      return 1;
                  }
                  if (!fs::is_directory(startPath)) {
                      std::cerr << "Error: Path is not a directory: " << startPath << std::endl;
                      return 1;
                  }

                  if (output_to_file) {
                      fs::path save_file;
                      if (save_path.empty()) {
                          save_file = fs::current_path() / "fastruct_output.txt";
                      } else {
                          save_file = fs::path(save_path);
                          if (fs::is_directory(save_file)) {
                              save_file /= "fastruct_output.txt";
                          }
                      }

                      output_file.open(save_file);
                      if (!output_file.is_open()) {
                          std::cerr << "Error: Cannot open file for writing: " << save_file << std::endl;
                          return 1;
                      }

                      std::cout << "Output will be saved to: " << save_file.generic_string() << std::endl;
                  }

                  int estimated_max_depth = estimateMaxDepth(startPath);
                  if (!confirmDeepDirectory(estimated_max_depth)) {
                      if (output_to_file) output_file.close();
                      return 0;
                  }

                  fs::path absStart = fs::absolute(startPath);
                  std::string startName = absStart.filename().string();
                  if (startName.empty()) {
                      startName = absStart.generic_string();
                      if (!startName.empty() && startName.back() == '/')
                          startName.pop_back();
                  }

                  traverse(absStart, "", startName, 0, maxDepth);

                  if (output_to_file && output_file.is_open()) {
                      output_file.close();
                      std::cout << GREEN << "Output saved successfully." << RESET << std::endl;
                  }

                  return 0;
              }
