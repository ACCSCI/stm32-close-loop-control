set pagination off
set confirm off
target extended-remote localhost:61234
monitor reset
break main
continue
break HardFault_Handler
echo \n=== stepping through init (next over each call) ===\n
next
printf "after system_init: pc=%p ticks=%u\n", $pc, s_ticks_ms
next
printf "after gpio_init: pc=%p\n", $pc
next
printf "after uart_init: pc=%p\n", $pc
next
printf "after dac_init: pc=%p\n", $pc
next
printf "after adc_init: pc=%p\n", $pc
next
printf "after LCD_Init: pc=%p\n", $pc
echo \n=== where are we now ===\n
bt
detach
quit
