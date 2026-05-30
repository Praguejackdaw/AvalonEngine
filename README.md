![image](https://github.com/Praguejackdaw/AvalonEngine/blob/main/demo.gif)

本引擎支持跨平台编译，您可以使用以下 CMake 命令在本地一键构建并运行项目：
```bash
# 1. 克隆代码仓库与子模块
git clone https://github.com/Praguejackdaw/AvalonEngine.git
cd AvalonEngine
# 2. 生成 CMake 构建配置
cmake -B build
# 3. 编译并构建 Release 版本
cmake --build build --config Release
