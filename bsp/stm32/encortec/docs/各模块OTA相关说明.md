# 各模块 OTA 相关说明

## STM32

STM32 的架构为 bootloader + app，分为两个分区，然后 app 是使用的 A B 分区，OTA 只升级 app 分区的功能，
当前是 app_a 则升级到 app_b，编译时同时生成两个app.bin文件，一个A分区的，一个B分区的，同时上传到服务器，
STM32自行选择需要升级的.bin文件下载并记录

### STM32 固件版本生成

#### 1. 直接修改脚本内容

```shell
D:\Code\Github\QuecPython\rt-thread\bsp\stm32\encortec\compile.py
```

#### 2. 编译指令

```shell
python compile.py -A -p AB -v ENCORTECSTM32U575R01M02A01-V01
```

### STM32 版本查询

```c++
D:\Code\Github\QuecPython\rt-thread\bsp\stm32\encortec\application\main\main.c

void read_app_version_information(uint8_t **app_version, uint8_t **app_subedition, uint8_t **app_build_time)
```

## CAT1

CAT1 使用标准固件，无定制化内容。使用差分升级，制作差分升级包。
CAT1 目前客户的设备上使用的是云定制固件，但是 CAT1 目前没有用到云相关的功能，可以直接使用标准固件升级测试。

### CAT1版本查询

```c++
D:\Code\Github\QuecPython\rt-thread\bsp\stm32\encortec\application\board\driver\ota\ota_cat1.c

rt_err_t cat1_at_query_version(char *cat1_version, rt_size_t size)
```

## ESP32

ESP32为 A B 分区升级，自动选择分区烧录，编译一个固件即可。
ESP32 固件编译使用 VSCode ESP-IDF 插件编译

代码仓库: `https://github.com/QuecPython/encortec-esp32-project`
芯片版本号: ESP32S3
ESP-IDF: v5.3.1

编译完成后，固件为 `build/esp_encortec.bin`

**Note:**

目前ESP32固件的问题(需要强哥帮忙瞅瞅，之前叶剑开发的ESP32功能，可以联系他了解一下):

1. 目前提供测试的两个版本升级不行，查出来的版本号没变
2. `AT+QOTA` 指令的回复 `OK` 不能在所有的 `URC` 回复之后再返回，只要收到 `QOTA` 就得回 `OK`

### ESP32版本查询

```c++
D:\Code\Github\QuecPython\rt-thread\bsp\stm32\encortec\application\board\driver\ota\ota_esp32.c

rt_err_t esp32_at_query_version(char *esp32_version, rt_size_t size)
```

## GNSS版本查询

GNSS标准固件通常不用升级，非定制化的GNSS固件，GNSS升级极为固件烧录全量升级

```c++
D:\Code\Github\QuecPython\rt-thread\bsp\stm32\encortec\application\board\driver\gnss\gnss.c

rt_err_t gnss_query_version(char **gnss_version)
```
