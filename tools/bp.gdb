set pagination off
set confirm off
target extended-remote localhost:61234
monitor reset
tbreak main
continue
break HardFault_Handler
break lcd_draw_layout
break main.c:342
echo \n=== continue, expect one of: LCD_Init done / draw_layout / loop / HardFault ===\n
continue
echo \n=== STOP 1 ===\n
frame
info registers pc lr
printf "CFSR=0x%08x BFAR=0x%08x ticks=%u\n", *(unsigned int*)0xE000ED28, *(unsigned int*)0xE000ED38, s_ticks_ms
detach
quit
