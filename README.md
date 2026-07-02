# TIP输入法（tip-ime）

一款轻量、开源、高性能的 Windows 桌面拼音输入法，基于微软官方 **TSF（Text Services Framework）** 现代框架实现。

## 核心特性

- 全拼音输入、简拼支持、基础模糊音适配
- 本地词库支持 10000+ 词条，Trie 前缀树索引
- 跟随光标候选栏、数字选词、翻页
- 中英文切换、全半角切换、中英文标点切换
- 独立设置面板、自定义短语导入导出
- 标准安装包，支持一键注册 / 卸载输入法

## 快速开始

```powershell
git clone https://github.com/SpadyDong/tip-ime.git
cd tip-ime
cmake -B build -S . -A x64
cmake --build build --config Release
```

详细环境搭建说明请参阅 [docs/DEVELOPMENT_ENV.md](docs/DEVELOPMENT_ENV.md)。

## 开发计划

项目采用 AI Agent 辅助开发，分阶段推进。完整计划请参阅 [docs/DEVELOPMENT_PLAN.md](docs/DEVELOPMENT_PLAN.md)。

## 开源协议

本项目采用 [MIT 协议](LICENSE) 开源。
