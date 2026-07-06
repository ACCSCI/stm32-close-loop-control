#!/usr/bin/env python3
"""Read live firmware globals over SWD via STM32_Programmer_CLI and decode them.

Usage: python tools/read_state.py [interval_seconds] [count]
"""
import subprocess, struct, sys, re, time

PROG = r"C:\Users\ACCSCI\AppData\Local\stm32cube\bundles\programmer\2.22.0+st.1\bin\STM32_Programmer_CLI.exe"

VARS = [
    ("target",      0x20000004, "f"),
    ("measured",    0x200001c8, "f"),
    ("temp",        0x200001cc, "f"),
    ("vrefint_raw", 0x200001d0, "f"),
    ("alarm",       0x200001d4, "B"),
    ("ticks_ms",    0x2000021c, "u"),
]

def rdw(addr):
    out = subprocess.run([PROG, "-c", "port=SWD", "mode=HOTPLUG",
                          "-r32", hex(addr), "1"],
                         capture_output=True, text=True, timeout=15).stdout
    m = re.search(r"0x[0-9A-Fa-f]+\s*:\s*([0-9A-Fa-f]{8})", out)
    if not m:
        raise RuntimeError("no data for %#x\n%s" % (addr, out))
    return int(m.group(1), 16)

def snapshot():
    s = {}
    for name, addr, typ in VARS:
        w = rdw(addr)
        if typ == "f":
            s[name] = struct.unpack("<f", struct.pack("<I", w))[0]
        elif typ == "B":
            s[name] = w & 0xFF
        else:
            s[name] = w
    return s

if __name__ == "__main__":
    interval = float(sys.argv[1]) if len(sys.argv) > 1 else 0.5
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    for i in range(n):
        s = snapshot()
        err = s["target"] - s["measured"]
        print(f"t={s['ticks_ms']/1000:5.2f}s  tgt={s['target']:.3f}V  meas={s['measured']:.3f}V  "
              f"err={err:+.3f}V  temp={s['temp']:5.1f}C  alarm={s['alarm']}  vref={s['vrefint_raw']:.0f}")
        if i < n - 1:
            time.sleep(interval)