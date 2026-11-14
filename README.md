# XXCC 编译驱动

XXCC 调用 Clang 和链接器，为 XJ380 应用加入固定编译参数、SDK 头文件、启动对象及运行库。驱动自身使用宿主 C++17 标准库，应用仍默认使用 C11 / C++11。

## 一键构建全部 8 个命令

需要 GNU Make，以及支持 C++17 和 `std::filesystem` 的宿主 C++ 编译器：

```sh
make
# 或并行构建
make -j4
```

Linux 产物在 `release/linux/`，Windows 产物在 `release/windows/`（带 `.exe`）。公共实现只编译一次，再链接出以下 8 个入口：

| 命令 | 默认语言 | 启动对象 | 默认输出 |
| --- | --- | --- | --- |
| xxcc | C++11 | lib/x86_64/start.o | a.epf |
| xxcc-- | C11 | lib/x86_64/start.o | a.epf |
| xxcc-elf | C++11 | lib/x86_64/start.o | a.elf |
| xxcc-elf-- | C11 | lib/x86_64/start.o | a.elf |
| xxccte | C++11 | lib/x86_64/constart.o | a.epf |
| xxccte-- | C11 | lib/x86_64/constart.o | a.epf |
| xxccte-elf | C++11 | lib/x86_64/constart.o | a.elf |
| xxccte-elf-- | C11 | lib/x86_64/constart.o | a.elf |

保留 `make linux`、`make win`、`make xj380` 目标。这些目标选择输出目录和扩展名；编译器必须能生成所选宿主平台的程序，Windows/Linux 配置与编译器不匹配时会明确报错。例如，在 Windows 的 MinGW/GNU Make 环境运行 `make win`，或在 Linux 使用完整的 MinGW 交叉工具链：

```sh
make win CXX=x86_64-w64-mingw32-g++
```

`make xj380` 保留原有输出命名，但需要另行提供具备 C++17 标准库、文件系统和 POSIX 进程接口的目标工具链；本阶段未验证驱动在 XJ380 内自举运行。

可通过 `CXX`、`CPPFLAGS`、`CXXFLAGS`、`LDFLAGS`、`LDLIBS` 自定义驱动构建。清理当前平台产物：

```sh
make clean
```

## 使用

应用编译需要 `clang` / `clang++`，Linux 链接使用 `ld`，Windows 使用 `lld` 的 GNU ELF 模式。

```sh
# 可以从自己的项目目录调用；选项支持出现在源文件前后
/path/to/XXCC-suite/release/linux/xxcc -O2 main.cpp -o app.epf
/path/to/XXCC-suite/release/linux/xxccte-elf-- main.c -I "my headers" -o app.elf

# 多源文件、已有目标文件及静态库
xxcc src/main.cpp src/helper.cpp helper.o custom.a -o app.epf

# 分阶段构建
xxcc -c main.cpp -MMD -o main.o
xxcc -S main.cpp -o main.s
xxcc-- -E main.c

# 查看实际命令或保留中间文件
xxcc -v main.cpp -o app.epf
xxcc --xxcc-save-temps main.cpp -o app.epf
```

省略 `-o` 时按入口使用 `a.epf` 或 `a.elf`；显式指定 `-o` 时原样保留文件名。两种后缀目前沿用相同的 ELF 链接布局，没有额外 EPF 格式转换。输出目录需要事先存在。

支持常用的 `-I/-D/-U/-include/-isystem/-x` 等参数及其独立参数值，支持 `-L/-l/-Wl,/-Xlinker` 链接参数。用户对象和库的相对顺序保留。首次修复尚未提供完整的 Clang/GCC 驱动参数兼容；`@response-file` 会明确报错。GCC 后端适配留待后续阶段。

`--help` 显示帮助，`--version`、`-version`、单独的 `-v` 显示版本。

## SDK 和工具路径

SDK 查找优先级为：

1. `--xxcc-sdk=PATH` 或 `--xxcc-sdk PATH`。
2. 环境变量 `XXCC_SDK_ROOT`。
3. 可执行文件所在目录及其上三级目录，兼容本仓库的 `release/linux`、`release/windows` 布局。

SDK 根目录使用 XACT CRT 布局：

```
include/
lib/x86_64/
    crt0.o
    start.o
    constart.o
    libxact.a
    libxgui.a
    libxnetapi.a
    libxnet.a
    libxmedia.a
    liballoc.a
```

`xxcc` / `xxcc-elf` 等 GUI 入口链接 `start.o` + `libxgui.a`；
`xxccte` / `xxccte-elf` 等终端入口链接 `constart.o`，不链接 `libxgui.a`。
显式 SDK 路径无效时直接报错，不回退到其他 SDK；查找不依赖调用者的工作目录。
SDK 头文件和运行库本身由操作系统项目（XJ380-SDK）维护。

```sh
xxcc --xxcc-sdk="/path/to/SDK" main.cpp -o app.epf
```

通过环境变量指定工具的**单个可执行文件路径**，支持路径中的空格；不要填写带额外参数的 Shell 命令：

- `XXCC_CC`：C 编译器，默认 `clang`。
- `XXCC_CXX`：C++ 编译器，默认 `clang++`。
- `XXCC_LD`：链接器，Linux 默认 `ld`，Windows 默认 `lld`。

链接前检查选定运行时的启动对象、`arch/x86_64/crt0.S.o` 和 `liballoc-x86_64.a`；包含 TLS 对象时要求 `libxnet.a` 存在。已提供的 `libxnet.a`、`libxmedia.a` 会显式加入链接。仅编译、汇编、预处理时不要求运行库存在。

应用保留原有的 `-ffreestanding -fno-builtin -m64 -fno-stack-protector -fno-exceptions -fshort-wchar -nostdinc` 等默认参数、`__XJ380_OS__` 宏、启动对象选择和 `0x200000` 文本地址。Windows 继续使用 `--target=x86_64-unknown-elf`；Linux 保留原有目标选择行为。

## 失败处理与验证

任一编译失败后立即停止，不再链接；后端的非零退出码会传回调用者，缺少工具返回 127。中间对象位于每次调用独占的临时目录，同名源文件和并行构建互不覆盖。

链接和单文件输出先写同目录下的独立临时位置，成功后才替换目标文件。失败时保留已有输出。默认自动清理自己创建的临时文件，`--xxcc-save-temps` 可保留链接前的中间对象。

```sh
make test
```

测试需要 Python 3。Linux 下包含参数/路径/失败状态/并发等回归测试；安装 Clang 和 GNU ld 时，还会实际构建全部 8 个入口的最小应用、同名多源文件及仓库示例，并检查 ELF 架构、入口与文本地址。测试只生成 XJ380 程序，不在宿主上执行它们。Windows 执行层和 XJ380 内实际运行仍需各自环境验证。
