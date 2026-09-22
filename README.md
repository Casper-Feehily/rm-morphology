
> 注：此README.md绝大部分由ai生成

# 二值图像形态学

使用 C++17 实现的正方形结构元素膨胀和腐蚀，并支持圆盘结构元素的圆角膨胀。在运行时可输入参数指定进行腐蚀还是扩散。

## 构建与测试

在 `morphology` 目录执行：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`ctest` 会运行正方形膨胀、腐蚀与圆角膨胀示例；程序内置断言检查对应结果。

## 终端可视化

`--visualize` 由 C++ 程序直接输出输入图像、5×5 结构元素和计算结果；`██` 表示前景，`··` 表示背景。

```bash
build/morphology_dilation --demo --rounded --visualize
```

## 输入与运行

标准输入格式为第一行 `行数 列数`，后接 `行数 × 列数` 个 `0` 或 `1`，空白字符可任意分隔。

```bash
printf '7 7\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n0 0 0 1 0 0 0\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n0 0 0 0 0 0 0\n' | build/morphology_dilation
```

默认使用 5×5 正方形结构元素膨胀；添加 `--erode` 执行腐蚀。`--rounded` 选择 5×5 离散圆盘结构元素，使膨胀边角呈圆形。`--demo` 使用内置样例，选项可组合。

```bash
build/morphology_dilation --demo
build/morphology_dilation --demo --erode
build/morphology_dilation --demo --rounded
```

## 实现要点

`StructuringElement` 以掩码参数传入，因此膨胀和腐蚀可以替换结构元素。圆角模式使用以下离散圆盘：

```text
0 0 1 0 0
0 1 1 1 0
1 1 1 1 1
0 1 1 1 0
0 0 1 0 0
```

膨胀只要任一有效掩码位置命中前景就输出 `1`；腐蚀要求所有有效掩码位置均为前景。图像外按背景 `0` 处理。
