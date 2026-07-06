# 题目3：模拟闭环控制系统设计
功能要求：
查阅资料，学习STM32F4与LCD的接口设计，完成LCD液晶屏驱动程序的设计，将相关信息显示在LCD上；
查阅资料，学习STM32F4内部温度传感器的配置，采集、计算片内温度并显示在LCD上；
其他功能，自由发挥扩展。

---

## 硬件平台：ALIENTEK Explorer STM32F407 (ZGT6, 144-pin)

### 引脚映射（已验证）

| silk-screen 标号 | MCU引脚 | 功能 | 备注 |
|---|---|---|---|
| **"ADC/TPAD"** | **PA5** (ADC2_IN5) | 闭环反馈输入 | silk-screen标"ADC/TPAD"的是PA5，不是PA0 |
| **"DAC"** | PA4 (DAC_OUT1) | DAC模拟输出 | |
| PA0/WKUP | PA0 (WKUP) | WKUP唤醒引脚 | 与"ADC"焊盘不同！ |
| PA9/PA10 | USART1_TX/RX | 串口 | |
| PB15 | LCD背光 | FSMC控制 | **active-high**（新版 PCB 高电平点亮，旧版低电平点亮）|
| PC0/PC1 | KEY_UP/DOWN | 电压±0.1V | 悬空读低，用非阻塞检测 |
| PF8 | Buzzer (BEEP) | 蜂鸣器 | **active-high**（PF8=1 响，PF8=0 静音）。PUPDR 务必设下拉，否则浮空会一直叫 |
| PD12-15 | LED4-6 | 状态指示 | Green/Orange/Red/Blue |

### 踩坑经验

#### 1. FLASH_ACR 等待状态位域错误（HardFault，系统完全不启动）
`FLASH_ACR = 0x705D0000` → LATENCY[3:0]=0，168MHz下0等待 → flash取指返回垃圾 → BusFault
**修复：** `FLASH_ACR = 0x00000705`（LATENCY=5 | PRFTEN | ICEN | DCEN）

#### 2. FPU未使能（NOCP usage fault）
编译 `-mfloat-abi=hard` 但未开CP10/CP11
**修复：** `system_init()` 顶部 `*(volatile uint32_t*)0xE000ED88 |= 0xFU << 20;` + dsb/isb

#### 3. 按键阻塞整个闭环（while等待松开卡死）
PC0/PC1悬空读低，`while(!(GPIOC->IDR & KEY_UP_PIN))`永久等待
**修复：** 改为非阻塞下降沿检测（记录prev状态）

#### 4. silk-screen "ADC"焊盘是PA5不是PA0（measured~0.12V vs target=1.6V）
ALIENTEK Explorer F407板上"ADC/TPAD"标的是PA5，"PA0/WKUP"是另一焊盘
**修复：** ADC读PA5 (ADC2_IN5)，不是PA0

#### 5. 单ADC通道切换竞态（measured抖动0~0.12V）
同一ADC读PA0再读温度传感器(CH16)/VREFINT(CH17)时，SQR3切换后DR残留陈旧值
**修复：** ADC2专用于PA0闭环，ADC1专用于温度+VREFINT

#### 6. 温度传感器需VREFINT校准（显示100°C而非25°C）
假设Vdda=3.3V不对，实测Vdda≈3.1V
**修复：** 读VREFINT (ADC1_IN17, 1.21V)，反算 Vdda = 1.21×4095/VREFINT_raw

#### 7. LCD 黑屏：白闪一下立刻全黑（两层叠加 bug）
**症状：** `LCD_Init()` 后立即整屏闪一下白光，约 100ms 内黑屏，且**完全黑屏**（背光也灭）。串口正常工作、LED 心跳正常。
**根因（两层叠加）：**
- **(a) 背光极性反了**：新版 ALIENTEK 探索者 STM32F407 PCB（V1.8+，约 2020+ 量产）的 LCD 背光 **PB15 高电平点亮**；代码里 `LCD_LED_SET(0)` / `GPIOB->BSRR = (1U << 31)` 把 PB15 拉低 = 灭。原厂 `lcd.c` 默认 V1.8 之前旧 PCB 是低电平点亮，新板要改。**判别方法：** 烧录后用万用表量 PB15 电压，应该是 **3.3V** 而不是 0V。
- **(b) FSMC 写时序过紧**：ILI9341 在 168MHz HCLK 下写时序 `ADDSET=3, DATAST=2`（24ns/18ns）太靠近 t_WR 下限。改成 `ADDSET=5, DATAST=15`（30ns/90ns）后稳定。注意 `LCD_LED_SET` 在 `LCD_Init` 末尾**会覆盖**之前在 LCD_Init 顶部 BSRR 写入的电平，必须把末尾那行也改成 1。
**修复：**
- `LCD_LED_SET(1)`（active-high）
- `FSMC_Bank1E->BWTR[3] = (5<<0) | (15<<8)`
**为什么"白闪一下"：** `LCD_Clear` 早期用 `lcddev.wramcmd=0` 时 FSMC 总线短暂 enable，物理噪声把屏幕刷白；之后写命令 `0x002A`（实际应是 setxcmd 但被覆盖为 0）触发了 sleep mode 黑屏。

#### 8. FSMC BCR 必须用 SRAM 类型，不是 NOR Flash
`FSMC_BCR_MTYP_SRAM` + `FSMC_BCR_MWID_16` + `FSMC_BCR_WREN` + `FSMC_BCR_EXTMOD`。**不要**用 `FSMC_BCR_MTYP_NOR`，否则异步模式时序寄存器不生效。

#### 9. 读时序 BTR4 要拉宽才能稳定读 LCD ID
ILI9341 读 ID 时响应较慢，168MHz 下 `ADDSET=0xF, DATAST=0xFF`（90ns/1.53µs）才稳定。识别出 ID 后再切回快时序写。

#### 10. 蜂鸣器一直叫：GPIO 浮空 + 极性搞反
**症状：** 上电就蜂鸣器长鸣，报警逻辑无效；key1/R 命令也清不掉。
**根因：**
- (a) GPIO 初始化漏了 `PUPDR` —— 默认浮空，被板上拉低 → 蜂鸣器三极管导通 → 一直响。ALIENTEK 例程 `beep.c` 是 **GPIO_PuPd_DOWN**。
- (b) 极性搞反：例程 `GPIO_ResetBits(PF8)` 默认拉低 = 静音 → 即 PF8=1 才响 (active-high)。网上有些老资料说"低电平触发"是错的。
**修复：**
```c
GPIOF->PUPDR |= (2U << 16);  /* pull-down */
GPIOF->ODR &= ~BUZZER_PIN;   /* 默认静音 */
```
报警时 `buzzer_on()` = `GPIOF->ODR |= BUZZER_PIN` (PF8=1)。
**集成方式：** 跟 LED 闪烁同步（250ms on / 250ms off）形成间断报警音，避免连续长鸣太吵。

### 调试方法（无串口时）
```bash
# 读RAM全局变量
STM32_Programmer_CLI -c port=SWD mode=HOTPLUG -r32 0x20000XXX 1

# 查看PC + addr2line反查
STM32_Programmer_CLI -c port=SWD mode=HOTPLUG --coreReg PC
arm-none-eabi-addr2line -e build/Debug/close-loop-control.elf -f -i 0x0800XXXX

# HardFault分析
CFSR @ 0xE000ED28  BFAR @ 0xE000ED38
stacked PC @ MSP+0x18, stacked LR @ MSP+0x14
```

