#!/usr/bin/env python3
"""Sample ADC DR register on PA0 over time, with target/temp context."""
import subprocess, struct, re, time

PROG = r"C:\Users\ACCSCI\AppData\Local\stm32cube\bundles\programmer\2.22.0+st.1\bin\STM32_Programmer_CLI.exe"

def r32(addr):
    out = subprocess.run([PROG, "-c", "port=SWD", "mode=HOTPLUG",
                          "-r32", hex(addr), "1"],
                         capture_output=True, text=True, timeout=15).stdout
    m = re.search(r"0x[0-9A-Fa-f]+\s*:\s*([0-9A-Fa-f]{8})", out)
    return int(m.group(1), 16) if m else None

def f32(addr):
    return struct.unpack("<f", struct.pack("<I", r32(addr)))[0]

# Static: read DAC & ADC
print("=== static read ===")
print(f"  DAC_DHR12R1 (target)   = 0x{r32(0x40007408):04X}")
print(f"  DAC_DOR1    (output)   = 0x{r32(0x4000742C):04X}  -> {r32(0x4000742C)/4095*3.111:.3f} V")
print(f"  firmware target_v      = {f32(0x20000004):.3f} V")
print(f"  firmware measured_v    = {f32(0x200001c4):.3f} V")
print(f"  firmware chip_temp     = {f32(0x200001c8):.2f} C")
print()
print("=== PA0 ADC raw samples (target=1.6V) ===")
for i in range(20):
    raw = r32(0x4001204C) & 0xFFF
    sqr = r32(0x40012034) & 0x1F
    sr  = r32(0x40012000)
    v   = raw * 3.111 / 4095
    print(f"  t={i*0.2:5.2f}s  SR=0x{sr:04X}  SQR3.ch={sqr}  ADC_DR=0x{raw:03X} ({raw:4d})  -> {v:.3f} V")
    time.sleep(0.2)