# lib/x86_64

XXCC 自带的 XACT CRT 运行时：

- `crt0.o`、`start.o`、`constart.o`、`libxact.a`、`libxgui.a`
  来自上游 `XJ380-SDK` 的运行时构建。
- `liballoc.a` 是 `OpenXJ380` 跟踪的预编译第三方归档（与官方 XACT 发行包一致）；
  OpenXJ380 没有它的源码，真正上游是 rustc 的 `alloc`/`compiler_builtins` 与 `talc`。
- `libxnet.a`、`libxmedia.a` 来自官方 XACT 发行包的预编译归档，
  因为对应的 third_party 源码不在本仓库。

GUI 入口链接 `start.o + libxgui.a`；终端入口链接 `constart.o`，
不链接 `libxgui.a`。完整 CRT 由下游 `XACT-CRT` 组合和打包。
