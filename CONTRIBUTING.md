# 贡献指南

## 构建问题解决方案

### Qt Creator 构建卡死问题

如果遇到Qt Creator构建卡死的情况，请按以下步骤解决：

#### 1. 立即解决
```bash
# Windows
taskkill /F /IM qtcreator.exe /T

# 或者直接使用任务管理器结束Qt Creator进程
```

#### 2. 清理构建文件
```bash
# 删除所有构建目录
rm -rf build/
rm -rf debug/
rm -rf release/
rm -rf *Qt*build*/

# 删除Qt Creator用户设置文件
rm *.pro.user*
```

#### 3. 使用命令行构建
推荐使用提供的构建脚本：

```bash
# 使用CMake构建
./build_cmake.bat

# 或使用qmake构建  
./build_qmake.bat
```

#### 4. Qt Creator 重新配置
1. 重新打开Qt Creator
2. 选择 "文件" -> "打开文件或项目"
3. 选择 `CMakeLists.txt` 或 `musicbrainz-qt.pro`
4. 重新配置Qt Kit
5. 选择 "构建" -> "清理项目"
6. 选择 "构建" -> "重新构建项目"

#### 5. 常见原因及解决方案

**原因1：编译器路径问题**
- 确保MinGW或MSVC编译器路径正确
- 检查Qt Kit配置中的编译器设置

**原因2：文件锁定**
- 某些文件被其他进程锁定
- 重启计算机后重试

**原因3：内存不足**
- 关闭其他占用内存的应用程序
- 减少并行编译线程数

**原因4：项目文件损坏**
- 检查代码中是否有语法错误
- 验证CMakeLists.txt或.pro文件完整性

#### 6. 预防措施
- 定期清理构建文件
- 避免在构建过程中修改源代码
- 使用版本控制确保代码完整性
- 定期更新Qt Creator和编译器

## 其他构建选项

### 使用命令行编译
```bash
# CMake方式
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# qmake方式
qmake musicbrainz-qt.pro
make
```

### 并行编译加速
```bash
# 使用多线程编译
make -j4  # 使用4个线程
mingw32-make -j4  # Windows MinGW
```

### 调试构建问题
```bash
# 详细输出构建过程
make VERBOSE=1
cmake --build . --verbose
```
