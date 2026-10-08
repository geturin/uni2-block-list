# UNI2 黑名单

[English](README.md)

为 **UNDER NIGHT IN-BIRTH II Sys:Celes** 的 Steam 匹配加入玩家屏蔽功能的 Windows 工具。

[**下载 Windows 运行包**](https://github.com/geturin/uni2-block-list/releases/download/v0.1.0-rc.1/UNI2-Block-List-0.1.0-rc.1-win32.zip) · [全部版本](https://github.com/geturin/uni2-block-list/releases)

## 功能

- 显示最近观察到的匹配玩家、名称、估计延迟和网络类型。
- 选中玩家拉黑，或从保存的黑名单中解除拉黑。
- **屏蔽 Wi-Fi 玩家**，无需逐一加入黑名单。
- 查看跳过、拒绝、拦截和丢弃的请求计数。
- 支持中英文切换。首次启动按 Windows 界面语言选择：中文系统使用中文，其余使用英文。

## 使用方法

需要 Windows 10 或 11、受支持的 Steam 游戏版本，以及自己的游戏安装。便携运行包无需安装器或 Python。

1. 将 **整个 ZIP** 解压到同一文件夹，保留 `UNI2 Block List.exe` 和 `uni2-block-list.dll` 在一起。
2. 正常通过 Steam 启动游戏。
3. 以与游戏相同的用户和权限运行 `UNI2 Block List.exe`。
4. 点击 **刷新**，选中游戏进程，再点击 **连接**。保持 **启用拦截** 勾选。
5. 进入游戏的匹配搜索或待机模式，选中玩家后点击 **拉黑玩家**。在黑名单中选中玩家，点击 **解除拉黑** 即可移除。

**屏蔽 Wi-Fi 玩家** 只在勾选时生效，不会将玩家写入黑名单；网络类型未知的玩家仍可通过。右上角可以切换语言，选择会自动保存。

取消 **启用拦截** 或关闭工具即可停止筛选。**升级前请关闭工具并重启游戏**，再解压新版。注入的 DLL 会保留到游戏进程退出。

## 列表与计数的含义

玩家列表来自游戏实际观察到的匹配结果，并非完整的 Steam 匹配队列。最近出现的玩家会保留最多两分钟，方便选中；他们可能已经不在等待匹配。游戏尚未提供数据时，名称、延迟或网络类型会显示未知。延迟是估计值，并非工具独立测速的结果。

工具会阻止受支持的匹配请求到达被屏蔽玩家；对战中暂停筛选，不会取消正在进行的对局。**请求记录** 按观察时间显示实际计数的增量，不代表网络丢包率，也不是逐个数据包的完整记录。

<details>
<summary>受支持的游戏版本</summary>

工具会核验游戏文件，并拒绝连接不支持的版本。游戏更新后可能需要更新本工具。当前支持的 SHA-256 指纹如下：

| 文件 | SHA-256 |
| --- | --- |
| `uni2.exe` | `4ebed985ecbf330ab8e495573361e49df20bb555263289d1aff5425fac9b7ed9` |
| `steam_api.dll` | `67ae11d71ae6ec404090094df1e47b614d27400dc53fa450023e6fbcf347902c` |

</details>

## 设置与隐私

黑名单及偏好设置保存在本机 `%LOCALAPPDATA%\UNI2BlockList`。新目录缺少对应设置时，会导入旧版的已保存玩家和 Wi-Fi 选项；旧版语言选择不导入，公开版首次启动按 Windows 界面语言选择。

工具不会自动上传黑名单或诊断信息。点击 **诊断** 可以将报告保存为本地文件。报告可能包含 Steam ID、IP 地址和路径，分享前请检查。卸载时如需同时删除保存的数据，请删除该设置目录。

## 从源码构建

需要 Python 3.9 或更新版本，以及 **32 位 MinGW-w64** 的 GCC、G++ 和 `windres`。构建不需要游戏文件。

```sh
python build.py --package
```

输出目录和可分发 ZIP 均位于 `build/`，名称为 `UNI2-Block-List-0.1.0-rc.1-win32`。省略 `--package` 可只生成目录。如编译器未加入 `PATH`，可用 `--cc`、`--cxx` 和 `--windres` 指定其路径。

## 免责声明

本工具是非官方社区项目，与 French-Bread、Arc System Works 或 Valve 无关联。它通过注入 DLL 在内存中挂钩匹配函数，不修改磁盘上的游戏文件。工具仍属实验性质，按原样提供，不作担保；请自行判断使用风险并遵守游戏规则，未来更新可能需要相应调整。发布包不包含游戏文件。

## 许可证

[MIT](LICENSE)，© 2026 geturin。随附的 [MinHook](https://github.com/TsudaKageyu/minhook) 库及其 HDE 组件使用 BSD 2-Clause 许可证。原版权声明及许可条款保留在 [vendor/minhook/LICENSE.txt](vendor/minhook/LICENSE.txt)，并随运行包提供。
