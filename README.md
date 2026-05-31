# 与佛论禅 (yflc)

基于 Base64 编码的佛学词汇替换加解密工具，将任意文本转换为纯汉字密文。

## 程序结构

```
yflc/
├── yflc.h      # 头文件，声明公开接口
├── yflc.c      # 核心库，实现加解密算法
├── main.c      # 交互式命令行入口
└── README.md
```

### yflc.h
公开接口声明，包含两个函数：

| 函数 | 说明 |
|------|------|
| `yflc_encrypt(plain_text)` | 加密明文，返回 malloc 分配的密文字符串 |
| `yflc_decrypt(cipher_text)` | 解密密文，返回 malloc 分配的明文字符串 |

调用方负责 `free()` 返回值，失败时返回 `NULL`。

### yflc.c
核心实现，包含四个模块：

1. **佛词映射表** — `fo_words[64]` 将 64 个 Base64 字符一一映射为 64 个双字佛学词汇，反向查询由 `fo_index()` 完成
2. **Base64 编解码** — `b64_encode_nopad()` 编码并去除填充符 `=`；`b64_decode()` 补回填充符后解码
3. **加密函数** — 明文 -> UTF-8 字节 -> Base64 -> 逐个替换为佛词
4. **解密函数** — 密文 -> 每 6 字节切分佛词 -> 反向查 Base64 字符 -> Base64 解码 -> 明文

### main.c
交互式命令行程序，提供菜单选择加密/解密/退出。支持跨平台编译（Windows / Linux）：

- Windows 平台：自动将控制台切换为 UTF-8 编码，`strdup` 映射为 MSVC 的 `_strdup`
- Linux / POSIX 平台：定义 `_POSIX_C_SOURCE 200809L` 启用 POSIX 扩展函数

## 加解密算法

### 加密流程

```
明文 (UTF-8)
  ┃
  ┃  Base64 编码
  ▼
Base64 字符串 (去掉 = 填充)
  ┃
  ┃  逐字符查表替换
  ▼
密文 (连续佛词，纯汉字)
```

例如 `"test"` 的 UTF-8 Base64 编码为 `dGVzdA`，去掉 `=` 后逐字符替换：

| Base64 字符 | 对应佛词 |
|:---:|:---:|
| `d` | 报身 |
| `G` | 三昧 |
| `V` | 愿力 |
| `z` | 道品 |
| `d` | 报身 |
| `A` | 如来 |

最终密文：`报身三昧愿力道品报身如来`

### 解密流程

```
密文 (连续佛词，纯汉字)
  ┃
  ┃  每 6 字节切分，查表还原 Base64 字符
  ▼
Base64 字符串 (补回 = 填充)
  ┃
  ┃  Base64 解码
  ▼
明文 (UTF-8)
```

### 佛词映射表（64 对）

| Base64 范围 | 数量 | 佛词示例 |
|:---|:---:|:---|
| `A`-`Z` | 26 | 如来、菩萨、罗汉、般若... |
| `a`-`z` | 26 | 众生、佛性、法身、报身... |
| `0`-`9` | 10 | 虚空、一乘、二谛、三宝... |
| `+` `/` | 2 | 十力、百劫 |

## 使用方法

### 编译

**Linux / POSIX：**

```sh
gcc -Wall -o yflc main.c yflc.c
```

**Windows (MSVC / MinGW)：**

```sh
gcc -Wall -o yflc.exe main.c yflc.c
```

### 运行

**Linux / POSIX：**

```sh
./yflc
```

**Windows：**

```sh
yflc.exe
```

```
===== 与佛论禅 =====
1. 加密
2. 解密
0. 退出
请选择: 1
请输入明文: Hello, World!
密文: 禅那三昧愿力有生佛性三昧八风有生慈悲涅槃报身灭道法身无明八正中道慈悲持戒

请选择: 2
请输入密文: 禅那三昧愿力有生佛性三昧八风有生慈悲涅槃报身灭道法身无明八正中道慈悲持戒
解密: Hello, World!
```

### 作为库使用

```c
#include "yflc.h"

char* cipher = yflc_encrypt("Hello");
if (cipher) {
    char* plain = yflc_decrypt(cipher);
    if (plain) free(plain);
    free(cipher);
}
```
