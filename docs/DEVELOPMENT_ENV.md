# TIP输入法 开发环境搭建指南

## 必要条件

TIP输入法 是一款 Windows 桌面拼音输入法，完整编译与调试需要 Windows 环境。业务引擎层（`src/engine/`、`src/utils/`）为纯 C++ 代码，可在 Linux / macOS 上编译并运行单元测试；TSF 内核层（`src/core/`）与 UI 层（`src/ui/`）依赖 Windows SDK，仅在 Windows 上构建。

## Windows 开发环境

1. **Visual Studio 2022**
   - 工作负载：「使用 C++ 的桌面开发」
   - 组件：Windows 10/11 SDK、CMake工具、C++ CMake工具

2. **Git**
   - 推荐安装 Git for Windows

3. **CMake**
   - 建议 3.20 或更高版本

### Windows 编译步骤

```powershell
git clone https://github.com/SpadyDong/tip-ime.git
cd tip-ime
cmake -B build -S . -A x64
cmake --build build --config Release
```

### Windows 运行单元测试

```powershell
ctest --test-dir build -C Release --output-on-failure
```

## Linux / macOS 开发环境

用于验证业务引擎层的跨平台正确性。

```bash
sudo apt-get install cmake g++ make    # Debian/Ubuntu
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## 目录说明

- `src/core/` - TSF COM DLL（Windows only）
- `src/engine/` - 拼音引擎、词库、配置、状态机（跨平台）
- `src/ui/` - 候选栏、设置面板、托盘图标（Windows only）
- `src/utils/` - 字符串工具、日志（跨平台）
- `tests/unit/` - 单元测试（跨平台）
- `tests/integration/` - 集成测试（Windows only）
