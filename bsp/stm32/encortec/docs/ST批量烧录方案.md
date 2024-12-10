# ST批量烧录

## Jlink 批量烧录

### 烧录指令和说明

```bash
JFlash.exe -openprj[xxx/xxx.jflash] -open[xxx/xxx.bin],[0x08000000] -usb[JLINKSN] -auto -exit
```

\-openprj 后面为jflash 配置文件完整路径

\-open 后面为烧录固件完整路径

0x08000000 固件烧录的起始地址

\-usb 后面为JLINK 的S/N 号，批量烧录可以直接复制上述指令，修改-usb后面的SN号即可

### 批处理示例

```bash
@echo off
JFlash.exe -openprjD:\stm32.jflash -openD:\stm23.bin,0x08000000 -usb10000000 -auto -exit

JFlash.exe -openprjD:\stm32.jflash -openD:\stm23.bin,0x08000000 -usb10000001 -auto -exit

JFlash.exe -openprjD:\stm32.jflash -openD:\stm23.bin,0x08000000 -usb10000002 -auto -exit

IF ERRORLEVEL 1 goto ERROR
goto END
:ERROR
ECHO JFlash ARM:Error
pause
:END
```

### &#x20;参考说明文档

C:\Program Files\SEGGER\JLink\_xxx\Doc\Manuals\UM08003\_JFlash.pdf

5.2 Command line options

5.3 Batch processing

## STLink 批量烧录

使用最新的 STM32CubeProgramamer 功能，该工具支持命令行，可以进行批量烧录开发

[新一代烧写工具 - STM32CubeProgrammer - STM32团队 ST意法半导体中文论坛](https://shequ.stmicroelectronics.cn/thread-627621-1-1.html)

[STM32CubeProgrammer软件工具介绍](https://www.st.com/resource/en/user_manual/dm00403500-stm32cubeprogrammer-software-description-stmicroelectronics.pdf)
