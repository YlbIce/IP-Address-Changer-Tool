# IP Address Changer Tool

一个使用Qt C++开发的Windows网卡IP地址快速切换工具。

## 功能特点

- 快速切换指定网卡IP地址信息
- 本地存储常用IP地址配置
- 支持添加、编辑、删除IP配置
- 支持DHCP和静态IP模式
- 自动检测并选择网络适配器
- 显示当前IP地址状态

## 系统要求

- Windows 10/11
- Qt 6.x
- 需要管理员权限运行（修改网络设置需要）

## 编译说明

### 方法1: 使用Qt Creator
1. 打开Qt Creator
2. File -> Open File or Project
3. 选择CMakeLists.txt
4. 配置项目并编译

### 方法2: 使用命令行
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目（MinGW）
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# 或（MSVC）
cmake -G "Visual Studio 17 2022" -A x64 ..

# 编译
cmake --build . --config Release
```

### 方法3: 使用批处理脚本
直接运行 `build.bat` 自动完成构建和打包

## 使用说明

1. 以管理员身份运行程序
2. 在下拉列表中选择要修改的网络适配器
3. 从列表中选择要应用的IP配置，或点击"Add"添加新配置
4. 点击"Apply Selected"应用选中的IP配置

### 添加IP配置
1. 点击"Add"按钮
2. 输入配置名称
3. 选择使用DHCP或手动配置IP地址
4. 如果选择静态IP，填写：
   - IP地址（如：192.168.1.100）
   - 子网掩码（如：255.255.255.0）
   - 网关（如：192.168.1.1）
   - DNS服务器（如：8.8.8.8）

### 编辑/删除配置
- 在列表中选中配置后，点击"Edit"编辑或"Delete"删除

## 数据存储

IP配置保存在：`%APPDATA%\IPTool\ip_configs.json`

## 注意事项

- 本程序需要管理员权限才能修改网络设置
- 修改IP地址可能需要几秒钟才能生效
- 建议在使用前备份当前网络配置

## 开源协议

MIT License
