SET PATH=D:\Keil_v5\ARM\ARMCC\Bin;D:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\bin;D:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\lib;D:\MentorGraphics\9.5PADS\MGC_HOME.ixn\bin;D:\MentorGraphics\9.5PADS\MGC_HOME.ixn\lib;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\TortoiseSVN\bin;C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility;C:\Program Files (x86)\Nordic Semiconductor\nrf-command-line-tools\bin\;C:\Program Files\dotnet\;C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin;D:\Program Files\Git\cmd;C:\Users\Administrator\AppData\Local\Microsoft\WindowsApps;D:\Users\Administrator\AppData\Local\Programs\Microsoft VS Code\bin;C:\MentorGraphics\9.5PADS\SDD_HOME\CAMCAD;C:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\lib;D:\MentorGraphics\9.5PADS\SDD_HOME\CAMCAD;D:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\lib;c:\program files\esafenet\cobra docguard client;D:\MentorGraphics\PADSVX.2.7\SDD_HOME\CAMCAD
SET CPU_TYPE=N32G003F5
SET CPU_VENDOR=Nationstech
SET UV2_TARGET=N32G003
SET CPU_CLOCK=0x00B71B00
"D:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\objects\startup_n32g003._ia"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\main.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_it.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\delay.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\uartdrive.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\uartprotocol.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\iap.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\timer.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\system.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\param.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\state.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\misc.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_adc.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_rcc.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_gpio.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_tim.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_uart.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\n32g003_flash.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\objects\system_n32g003.__i"
"D:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\Objects\_5Button.lnp"
"D:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\Objects\_5Button.axf" --i32combined --output ".\Objects\_5Button.hex"
fromelf.exe --bin .\Objects\_5Button.axf -o  ./Output/_5Button.bin
