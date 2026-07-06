# STM32 模拟闭环控制系统

> 题目3 — ALIENTEK Explorer STM32F407 (ZGT6, 144-pin) 上的 DAC→ADC 闭环反馈控制

## 功能

- **DAC 输出**模拟电压 (PA4)，通过按键 / 串口设置目标值 (0.0–3.3V, 步进 0.1V)
- **ADC 采样**DAC 输出 (PA5, ADC2_IN5) 作为闭环反馈
- **窗口看门狗**：电压超界触发 LED 闪烁 + 蜂鸣器间断报警 (哔-哔-哔)
- **串口主机**：115200 baud，可设目标 / 阈值 / 读状态 / 清除报警
- **STM32F407 内部温度传感器** (ADC1_IN16) + VREFINT 校准 → 显示片内温度
- **TFT LCD (ILI9341)**：实时显示目标/实测电压、窗口阈值、片内温度、报警状态、24 秒历史曲线

## 硬件

- 板子：ALIENTEK Explorer STM32F407 V1.8+（2020+ 量产版）
- 编译：arm-none-eabi-gcc + CMake + Ninja（参考 `CMakePresets.json`）
- 烧录：ST-Link V2 (SWD)

## 关键引脚

| 引脚 | 功能 | 备注 |
|------|------|------|
| PA4 | DAC_OUT1 | 模拟输出 |
| PA5 | ADC2_IN5 | 闭环反馈（注意：丝印"ADC/TPAD"焊盘） |
| PB15 | LCD 背光 | **active-high**（新版 PCB） |
| PF8 | Buzzer (BEEP) | **active-high** + PUPDR 必须下拉 |
| PD12-15 | LED4-6 (Green/Orange/Red/Blue) | active-low |
| PC0/PC1 | KEY_UP/DOWN | 上拉，按下拉低 |
| PE2/PE3/PE4 | KEY2/KEY1/KEY0 | 上拉，按下拉低 |

LCD (FSMC Bank1 NE4) 使用 PD/PE/PF12(A6)/PG12(NE4) 数据/地址/片选。

## 串口命令

| 命令 | 作用 | 示例 |
|------|------|------|
| `V:x.xx` | 设置目标电压 (0.0–3.3V) | `V:2.50` |
| `H:nnnn` | 设置窗口上限 (12-bit ADC raw) | `H:3000` |
| `L:nnnn` | 设置窗口下限 | `L:1000` |
| `?` | 打印当前状态 | `?` |
| `R` | 清除报警 / 静音 | `R` |

## 按键

| 按键 | 功能 |
|------|------|
| PE4 (KEY0) | 目标电压 -0.1V |
| PE2 (KEY2) | 目标电压 +0.1V |
| PE3 (KEY1) | 清除报警 |

## 构建与烧录

```bash
# 1. 配置
cmake --preset Debug

# 2. 构建
cmake --build build/Debug

# 3. 烧录 (ST-Link, SWD)
STM32_Programmer_CLI -c port=SWD mode=HOTPLUG -w build/Debug/close-loop-control.elf -v -rst

# 4. (可选) 调试 - GDB + OpenOCD/ST-Link
arm-none-eabi-gdb -x tools/bp.gdb build/Debug/close-loop-control.elf
```

> 注：本项目**未**附带完整工具链。`build/` 目录被 `.gitignore` 排除，源码用 VS Code + CMake Tools 扩展构建。

## 踩坑经验 (完整列表见 [CLAUDE.md](CLAUDE.md))

### LCD 黑屏：白闪一下立刻全黑（两层叠加 bug）
- 根因 (a)：背光 PB15 极性反了 → 新版 PCB 高电平点亮
- 根因 (b)：FSMC 写时序 `ADDSET=3, DATAST=2` (24ns/18ns) 在 168MHz 下太紧
- 修复：`LCD_LED_SET(1)` + `BWTR[3] = (5<<0)|(15<<8)`

### 蜂鸣器一直叫：GPIO 浮空 + 极性搞反
- 根因 (a)：PUPDR 未设 → 浮空被板上拉
- 根因 (b)：极性反了 → 例程 `ResetBits` 是静音 (active-high)
- 修复：`PUPDR |= (2<<16)` + `ODR &= ~BUZZER_PIN`

### FSMC BCR 必须用 SRAM 类型
`FSMC_BCR_MTYP_SRAM` + `MWID_16` + `WREN` + `EXTMOD`，不能用 NOR Flash 类型，否则异步模式时序寄存器不生效。

### ADC 读 PA0 还是 PA5？
ALIENTEK 探索者 F407 板上，**丝印标 "ADC/TPAD" 的焊盘是 PA5** (ADC2_IN5)，不是 PA0；丝印标 "PA0/WKUP" 的才是真 PA0。

## 仓库

- GitHub: https://github.com/ACCSCI/stm32-close-loop-control