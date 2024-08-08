 
#include <SPI.h>

const int m_csPin = 10; // ADXL372的CS引脚连接到Arduino的数字引脚10

float x = 0.0;
float y = 0.0;
float z = 0.0;

#define DEVID_PRODUCT 0xFA // 372 in octal :)

// Data registers. Each axis data has a 12 bit value. Data is left justified, MSBFIRST.
// Register *_H contains the eight most significant bits (MSBs), and Register *_L contains the four least significant bits (LSBs) of the 12-bit value
#define XDATA_H 0x08
#define XDATA_L 0x09
#define YDATA_H 0x0A
#define YDATA_L 0x0B
#define ZDATA_H 0x0C
#define ZDATA_L 0x0D

// Peak Data registers.
#define MAXPEAK_X_H 0x15
#define MAXPEAK_X_L 0x16
#define MAXPEAK_Y_H 0x17
#define MAXPEAK_Y_L 0x18
#define MAXPEAK_Z_H 0x19
#define MAXPEAK_Z_L 0x1A

// ID registers
#define DEVID_AD 0x00
#define DEVID_MST 0x01
#define PARTID 0x02
#define REVID 0x03

// System registers
#define STATUS 0x04 // Status register

#define OFFSET_X 0x20
#define OFFSET_Y 0x21
#define OFFSET_Z 0x22

#define THRESH_ACT_X_H 0x23 // Activity threshold register
#define THRESH_ACT_X_L 0x24
#define THRESH_ACT_Y_H 0x25
#define THRESH_ACT_Y_L 0x26
#define THRESH_ACT_Z_H 0x27
#define THRESH_ACT_Z_L 0x28

#define TIME_ACT 0x29 // Activity time register

#define THRESH_INACT_X_H 0x2A // Inactivity threshold register
#define THRESH_INACT_X_L 0x2B
#define THRESH_INACT_Y_H 0x2C
#define THRESH_INACT_Y_L 0x2D
#define THRESH_INACT_Z_H 0x2E
#define THRESH_INACT_Z_L 0x2F

#define TIME_INACT_H 0x30 // Inactivity time register
#define TIME_INACT_L 0x31

#define THRESH_ACT2_X_H 0x32 // Motion Warning Threshold register
#define THRESH_ACT2_X_L 0x33
#define THRESH_ACT2_Y_H 0x34
#define THRESH_ACT2_Y_L 0x35
#define THRESH_ACT2_Z_H 0x36
#define THRESH_ACT2_Z_L 0x37

#define FIFO_SAMPLES 0x39 // FIFO samples register
#define FIFO_CTL 0x3A     // FIFO control register

#define INT1_MAP 0x3B // Interrupt 1 & 2 map register
#define INT2_MAP 0x3C

#define TIMING 0x3D    // Timing control register
#define MEASURE 0x3E   // Measurement control register
#define POWER_CTL 0x3F // Power control register

#define SELF_TEST 0x40 // Self test register
#define RESET 0x41     // Reset register
#define FIFO_DATA 0x42 // FIFO data register

// System bitmasks
#define THRESH_ACT_L_MASK 0x1F // Activity detection
#define ACT_EN_MASK 0xFE
#define ACT_REF_MASK 0xFD

#define THRESH_INACT_L_MASK 0x1F // Inactivity detection
#define INACT_EN_MASK 0xFE
#define INACT_REF_MASK 0xFD

#define THRESH_ACT2_L_MASK 0x1F // Motion Warning
#define ACT2_EN_MASK 0xFE
#define ACT2_REF_MASK 0xFD

#define FIFO_SAMPLES_8_MASK 0xFE // FIFO control
#define FIFO_MODE_MASK 0xF9
#define FIFO_FORMAT_MASK 0xC7

#define INT_MAP_MASK 0xFF // Interrupt 1 and 2

#define EXT_SYNC_MASK 0xFE // Timing
#define EXT_CLK_MASK 0xFD
#define WAKEUP_RATE_MASK 0xE3
#define ODR_MASK 0x1F

#define BANDWIDTH_MASK 0xF8 // Measure
#define LOW_NOISE_MASK 0xF7
#define LINKLOOP_MASK 0xCF
#define AUTOSLEEP_MASK 0xBF

#define MODE_MASK 0xFC // Power Control
#define HPF_DISABLE_MASK 0xFB
#define LPF_DISABLE_MASK 0xF7
#define FILTER_SETTLE_MASK 0xEF
#define INSTANT_ON_THRESH_MASK 0xDF

#define ST_MASK 0xFE // Self test
#define ST_DONE_MASK 0xFD
#define USER_ST_MASK 0xFB

// Accelerometer Constants
#define SPI_SPEED 10000000 // ADXL372 supports up to 10MHz in SCLK frequency
#define SCALE_FACTOR 100   // mg per LSB
#define MG_TO_G 0.001      // g per mg

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); // CPHA = CPOL = 0
  pinMode(m_csPin, OUTPUT);                                          // Setting chip select pin
  digitalWrite(m_csPin, HIGH);                                       // Pin ready
 
}

void loop()
{
       
    byte devidAd = readRegister(DEVID_AD);
    byte devidMst = readRegister(DEVID_MST);
    byte partId = readRegister(PARTID);
    byte revId = readRegister(REVID);
    byte status = readRegister(STATUS);

    // digitalWrite(m_csPin, LOW); // 选择 ADXL372（片选信号置低）

    // SPI.transfer(0x2D); // 写入 POWER_CTL 寄存器地址
    // SPI.transfer(0x08); // 写入数据，设置 ADXL372 为测量模式
    // digitalWrite(m_csPin, HIGH); // 释放 ADXL372（片选信号置高）

   writeRegister(0x3E,0x02);
   writeRegister(0x3F,0x03);

  status;
    do
    {
        status = readRegister(0x04);
   
    } while ((status & 0x01) == 0); // Waiting for status register

    // The register is left justified. *DATA_H has bits 11:4 of the register. *DATA_L has bits 3:0.
    short rawX = readRegister(XDATA_H) << 8 | readRegister(XDATA_L);
    short rawY = readRegister(YDATA_H) << 8 | readRegister(YDATA_L);
    short rawZ = readRegister(ZDATA_H) << 8 | readRegister(ZDATA_L);

    rawX = rawX >> 4;
    rawY = rawY >> 4;
    rawZ = rawZ >> 4;

    // Converting raw axis data to acceleration in g unit
    x = rawX * SCALE_FACTOR * MG_TO_G;
    y = rawY * SCALE_FACTOR * MG_TO_G;
    z = rawZ * SCALE_FACTOR * MG_TO_G;


 
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.println(z);

  delay(100); // Change this for faster/slower measurements
}

uint8_t readRegister(byte regAddress)
{
    digitalWrite(m_csPin, LOW);
    regAddress = regAddress << 1 | 1; // Reading from a register
    SPI.transfer(regAddress);
    uint8_t value = SPI.transfer(0x00); // Transfering dummy byte to recieve data from accelerometer
    digitalWrite(m_csPin, HIGH);
    return value;
}


void writeRegister(byte regAddress, uint8_t value)
{
    digitalWrite(m_csPin, LOW);
    regAddress = regAddress << 1; // Writing to a register
    SPI.transfer(regAddress);
    SPI.transfer(value);
    digitalWrite(m_csPin, HIGH);
}

 
