# 二值图像形态学

使用 C++17 实现的 5×5 正方形结构元素膨胀和腐蚀，无第三方依赖。

## 构建与测试

在 `morphology` 目录执行：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`ctest` 会运行膨胀与腐蚀示例；程序内置断言检查单点经 5×5 膨胀后形成 5×5 方块，随后腐蚀恢复为单点。

## 输入与运行

标准输入格式为第一行 `行数 列数`，后接 `行数 × 列数` 个 `0` 或 `1`，空白字符可任意分隔。

```bash
printf '7 7\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n0 0 0 1 0 0 0\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n' | build/morphology_dilation
```

默认执行膨胀；添加 `--erode` 执行腐蚀。`--demo` 使用内置样例，可与 `--erode` 组合。

```bash
build/morphology_dilation --demo
build/morphology_dilation --demo --erode
```

## 实现要点

`StructuringElement` 以掩码参数传入，因此膨胀和腐蚀可以替换结构元素。膨胀只要任一有效掩码位置命中前景就输出 `1`；腐蚀要求所有有效掩码位置均为前景。图像外按背景 `0` 处理。
