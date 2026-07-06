set pagination off
set confirm off
target extended-remote localhost:61234
monitor reset
break HardFault_Handler
break main
echo \n===== continue from reset =====\n
continue
echo \n===== STOPPED =====\n
info registers pc lr sp xpsr
bt
printf "CFSR = 0x%08x\n", *(unsigned int*)0xE000ED28
printf "HFSR = 0x%08x\n", *(unsigned int*)0xE000ED2C
printf "BFAR = 0x%08x\n", *(unsigned int*)0xE000ED38
printf "ticks = %u\n", s_ticks_ms
detach
quit
