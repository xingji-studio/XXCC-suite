# XXCC 编译器（基于Clang）

# 编译：

```
make win（对应平台名）
```
## Windows下编译
**推荐工具：`MSYS2`(更加推荐)、[`Cygwin`](#cygwin)(编译出来以后，依赖Cygwin提供的DLL文件才能够运行)**
### MSYS2
**暂待施工**

### Cygwin
1. **下载[`Cygwin`](https://cygwin.com/)**
2. **准备以下软件包: `make`,`gcc-core`,`gcc-g++`,`binutils`,`autoconf`,`automake`,`libtool`,`patch`,`perl`**
3. **打开`Cygwin Terminal`**
4. **执行以下命令：**
```bash
make win
```
> **或者直接执行`make`也可以(因为`win`的构建命令顺排第一)**
5. **完成这些以后，编译出来的程序还不能直接运行，你需要将Cygwin的bin目录下的那些`*.dll`文件移动到你编译好的程序所在目录下才行（或者将Cygwin的bin目录添加到系统的`PATH`环境变量中也可以，因为Cygwin编译出来的程序依靠其提供的DLL文件运行），在项目的[`build-tools/Windows/exe`](build-tools/Windows/exe/)文件夹下有相应的工具，你可以使用工具[`copy_dll.exe`](build-tools/Windows/exe/copy_dll.exe)(你也可以直接使用[`copy_dll.py`](build-tools/Windows/copy_dll.py))将所有DLL文件复制到编译好的文件所在文件夹下**