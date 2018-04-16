SET PATH=E:\Keil_v5\ARM\ARMCC\Bin;C:\Program Files\VanDyke Software\Clients\;E:\Program Files\Python36\Scripts\;E:\Program Files\Python36\;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\Program Files\Microsoft SQL Server\130\Tools\Binn\;C:\Program Files (x86)\Nordic Semiconductor\nrf5x\bin\;C:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\Library\;C:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\sam-ba 2.9;E:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\Library\;E:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\sam-ba 2.9;C:\Users\Administrator\AppData\Local\Microsoft\WindowsApps;
SET CPU_TYPE=ARMCM3
SET CPU_VENDOR=ARM
SET UV2_TARGET=Target 1
SET CPU_CLOCK=0x00B71B00
"E:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\main.__i"
"E:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\switch.__i"
"E:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\bitmap.__i"
"E:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\list.__i"
"E:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\objects\startup_armcm3._ia"
"E:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\system_armcm3.__i"
"E:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\Objects\tinyOS.lnp"
"E:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\Objects\tinyOS.axf" --i32combined --output ".\Objects\tinyOS.hex"
