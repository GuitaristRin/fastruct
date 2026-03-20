# Fastruct - 快速项目结构查看工具

> 🌐 Language | [中文](#中文) | [English](#english)

## 中文
<a id="中文"></a>
<a id="top"></a>

# Fastruct - 快速项目结构查看工具

Fastruct 是一个轻量级的命令行工具，用于以清晰的树状结构展示目录内容。它会根据文件类型使用不同颜色进行标记，并显示每个文件的完整路径。

### 识别的源代码文件类型

支持多种编程语言的源代码文件：
- C/C++ 家族 (`.cpp`, `.c`, `.h`, `.hpp`, `.cc`, `.cxx`)
- Python (`.py`)
- Java (`.java`)
- JavaScript (`.js`)
- Rust (`.rs`)
- Go (`.go`)
- Ruby (`.rb`)
- PHP (`.php`)
- Perl (`.pl`)
- Shell 脚本 (`.sh`, `.bash`, `.zsh`)

### 可执行文件识别

- **Windows**: 根据扩展名识别 (`.exe`, `.bat`, `.cmd`, `.com`)
- **Unix-like**: 根据文件权限识别（具有执行权限的文件）

## 安装方法

### Arch Linux (AUR)

通过AUR来安装：

```bash
git clone https://aur.archlinux.org/fastruct.git
cd fastruct
makepkg -si
```

### Windows

1. 从 [Releases](https://github.com/yourusername/fastruct/releases) 页面下载最新版本的 `fastruct-1.0.0-windows-x86_64.exe`
2. 运行安装包，安装完成后，您需要设置PATH环境变量

### 从源码编译

#### 依赖项
- C++17 兼容的编译器
- GCC/G++ （若可能，建议）

#### 编译步骤

```bash
git clone https://github.com/GuitaristRin/fastruct.git
cd fastruct

# Linux
g++ -std=c++17 -o fastruct fastruct.cpp

# Windows (MinGW)
g++ -std=c++17 -o fastruct.exe fastruct.cpp
```

## 使用方法

### 基本语法

```bash
fastruct [选项]
```

### 选项说明

| 选项 | 描述 |
|------|------|
| `-d <路径>` | 指定起始目录（默认为当前目录） |
| `--depth <数值>` | 设置最大递归深度（默认为无限） |

### 使用示例

#### 1. 查看当前目录结构

```bash
fastruct
```

输出示例：
```
project/
-subdir1
-file.txt /home/user/project/file.txt
-script.sh /home/user/project/script.sh
-main.cpp /home/user/project/main.cpp
```

#### 2. 查看指定目录

```bash
fastruct -d /path/to/directory
```

#### 3. 限制遍历深度

```bash
# 只显示当前目录内容（不进入子目录）
fastruct --depth 0

# 显示两层结构
fastruct --depth 2
```

#### 4. 组合使用

```bash
fastruct -d /home/user/projects --depth 3
```

### 输出格式说明

每一行代表一个文件或目录：
- **目录**以 `-` 开头，后跟黄色的目录名
- **文件**以 `-` 开头，后跟彩色文件名，最后是灰色的完整路径

## 注意事项

- 在 Windows 上使用时，确保终端支持 ANSI 颜色代码（Windows 10 及以上版本默认支持）
- 如果遇到权限不足的目录，程序会显示错误信息并继续处理其他目录
- 符号链接会被跳过，以避免循环引用

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。
---





<p align="right"><a href="#top">🔝 返回顶部</a></p>

---

## English
<a id="english"></a>

# Fastruct - Fast Project Structure Viewer

Fastruct is a lightweight command-line tool for displaying directory contents in a clear tree structure. It uses different colors to mark files based on their type and displays the full path of each file.

### Recognized Source Code File Types

Supports source code files for multiple programming languages:
- C/C++ Family (`.cpp`, `.c`, `.h`, `.hpp`, `.cc`, `.cxx`)
- Python (`.py`)
- Java (`.java`)
- JavaScript (`.js`)
- Rust (`.rs`)
- Go (`.go`)
- Ruby (`.rb`)
- PHP (`.php`)
- Perl (`.pl`)
- Shell Scripts (`.sh`, `.bash`, `.zsh`)

### Executable File Detection

- **Windows**: Recognized by file extension (`.exe`, `.bat`, `.cmd`, `.com`)
- **Unix-like**: Recognized by file permissions (files with execute permission)

## Installation

### Arch Linux (AUR)

Install via AUR:

```bash
git clone https://aur.archlinux.org/fastruct.git
cd fastruct
makepkg -si
```

### Windows

1. Download the latest `fastruct-1.0.0-windows-x86_64.exe` from the [Releases](https://github.com/yourusername/fastruct/releases) page
2. Run setup,after the installation,you may need to add the PATH

### Build from Source

#### Dependencies
- C++17 compatible compiler
- GCC/G++ (recommended if possible)

#### Build Steps

```bash
git clone https://github.com/GuitaristRin/fastruct.git
cd fastruct

# Linux
g++ -std=c++17 -o fastruct fastruct.cpp

# Windows (MinGW)
g++ -std=c++17 -o fastruct.exe fastruct.cpp
```

## Usage

### Basic Syntax

```bash
fastruct [options]
```

### Options

| Option | Description |
|--------|-------------|
| `-d <path>` | Specify the starting directory (defaults to current directory) |
| `--depth <value>` | Set the maximum recursion depth (defaults to unlimited) |

### Usage Examples

#### 1. View Current Directory Structure

```bash
fastruct
```

Sample Output:
```
project/
-subdir1
-file.txt /home/user/project/file.txt
-script.sh /home/user/project/script.sh
-main.cpp /home/user/project/main.cpp
```

#### 2. View a Specified Directory

```bash
fastruct -d /path/to/directory
```

#### 3. Limit Traversal Depth

```bash
# Show only the current directory contents (do not enter subdirectories)
fastruct --depth 0

# Show two levels of structure
fastruct --depth 2
```

#### 4. Combined Usage

```bash
fastruct -d /home/user/projects --depth 3
```

### Output Format Description

Each line represents a file or directory:
- **Directory** starts with `-` followed by the directory name in yellow
- **File** starts with `-` followed by the colored filename, ending with the full path in gray

## Notes

- When using on Windows, ensure your terminal supports ANSI color codes (Windows 10 and above support this by default)
- If a directory with insufficient permissions is encountered, the program will display an error message and continue processing other directories
- Symbolic links are skipped to avoid circular references

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.


<p align="right"><a href="#top">🔝 Back to top</a></p>


