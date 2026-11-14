# lib/x86_64

XXCC 自带的 XACT CRT 运行时：

- `crt0.o`、`start.o`、`constart.o`、`libxact.a`、`libxgui.a`
  来自上游 `XJ380-SDK` 的运行时构建。
- `liballoc.a` 是 `plos-clan/liballoc` 的 release 资产（MIT，talc C binding），
  与官方 XACT 发行包及 `OpenXJ380/liballoc-x86_64.a` 字节级一致。
  `XJ380-SDK` 以 `third_party/liballoc` submodule 为源码；本目录是下游预编译分发。
- `libxnet.a`、`libxmedia.a` 来自官方 XACT 发行包的预编译归档，
  因为对应的 third_party 源码不在本仓库。
- `libxnetapi.a` 由官方 XACT 发行包的 `network/*.o` 与 `webrequest.cpp.o`
  归档而成；源码中的 `webrequest.cpp` 缺少私有 `httpclient.h`，暂不能重建。

GUI 入口链接 `start.o + libxgui.a`；终端入口链接 `constart.o`，
不链接 `libxgui.a`。完整 CRT 由 `XJ380-SDK` 组合和打包（`out/crt` 与 `xact-crt` xbps）。
