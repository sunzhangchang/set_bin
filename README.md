# set_bin


### 简介

将一个可执行文件包装成 `.bat` 文件扔到已有的环境变量里（Windows）（就像 linux 的 `/usr/bin`）
再也不用添加新的环境变量（单个文件）

### 用法

Usage: set_bin "bin_path" "env_dir"

#### 参数
 + 第一个参数 ： 要添加的可执行文件（目前支持".exe"，".bat"，".cmd"，".com"，".ps1"?）
 + 第二个参数（可选）： 要添加进的环境变量的目录

### 示例

sublime text命令行
`set_bin "sublime text的安装根目录/sublime_text.exe" "环境变量目录"`

即可用 `sublime_text "file"` 运行 sublime text 编辑/查看文件

