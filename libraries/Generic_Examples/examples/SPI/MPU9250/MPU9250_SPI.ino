#include "SPI.h"

// MPU9250 Register Adresleri
#define MPU9250_ADDRESS            0x68
#define MPU9250_WHO_AM_I           0x75
#define MPU9250_PWR_MGMT_1         0x6B
#define MPU9250_CONFIG             0x1A
#define MPU9250_GYRO_CONFIG        0x1B
#define MPU9250_ACCEL_CONFIG       0x1C
#define MPU9250_ACCEL_CONFIG2      0x1D
#define MPU9250_INT_PIN_CFG        0x37
#define MPU9250_ACCEL_XOUT_H       0x3B
#define MPU9250_GYRO_XOUT_H        0x43
#define MPU9250_MAG_ADDRESS        0x0C

// Chip Select (CS) Pini
const uint8_t MPU9250_CS_PIN = PA4;

// Defination SPI Class
// SPI -> SPI1
// SPI_2 -> SPI2
// SPI_3 -> SPI3

// SPI transfer  (Read/Write bit)
uint8_t spiTransfer(uint8_t reg, uint8_t data, bool read = false) {
    uint8_t result;
    
    // MPU9250 SPI formatı: 
    // Read için: 0x80 | register
    // Write için: register (MSB = 0)
    uint8_t txByte = read ? (reg | 0x80) : reg;
    
    digitalWrite(MPU9250_CS_PIN, LOW);
    SPI.transfer(txByte);
    result = SPI.transfer(data);
    digitalWrite(MPU9250_CS_PIN, HIGH);
    
    delayMicroseconds(10);
    return result;
}

// Register yazma
void writeRegister(uint8_t reg, uint8_t value) {
    spiTransfer(reg, value, false);
    delay(10);
}

// Register okuma
uint8_t readRegister(uint8_t reg) {
    return spiTransfer(reg, 0x00, true);
}

// Birden çok byte okuma
void readRegisters(uint8_t reg, uint8_t* data, uint8_t length) {
    digitalWrite(MPU9250_CS_PIN, LOW);
    SPI.transfer(reg | 0x80);  // Read bit set
    
    for (uint8_t i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00);
    }
    
    digitalWrite(MPU9250_CS_PIN, HIGH);
    delayMicroseconds(10);
}

// MPU9250 başlatma
bool initializeMPU9250() {
    Serial.println("MPU9250 SPI Başlatılıyor...");
    
    // 1. WHO_AM_I kontrolü
    uint8_t whoami = readRegister(MPU9250_WHO_AM_I);
    Serial.print("WHO_AM_I: 0x");
    Serial.println(whoami, HEX);
    
    if (whoami != 0x71 && whoami != 0x73) {
        Serial.println("HATA: MPU9250 bulunamadı!");
        return false;
    }
    
    // 2. Power Management - Reset ve Clock Source
    writeRegister(MPU9250_PWR_MGMT_1, 0x80);  // Device Reset
    delay(100);
    writeRegister(MPU9250_PWR_MGMT_1, 0x01);  // Auto select clock source
    delay(100);
    
    // 3. Konfigürasyonlar
    writeRegister(MPU9250_CONFIG, 0x03);      // DLPF_CFG = 3 (Gyro: 41Hz, Accel: 44Hz)
    
    // 4. Gyro ölçek faktörü (±500 dps)
    writeRegister(MPU9250_GYRO_CONFIG, 0x08); // FS_SEL = 1 (500 dps)
    
    // 5. Accel ölçek faktörü (±4g)
    writeRegister(MPU9250_ACCEL_CONFIG, 0x08); // AFS_SEL = 1 (4g)
    
    // 6. Accel DLPF
    writeRegister(MPU9250_ACCEL_CONFIG2, 0x03); // Accel DLPF_CFG = 3 (44Hz)
    
    // 7. Bypass Mode (Manyetometreye erişim için)
    writeRegister(MPU9250_INT_PIN_CFG, 0x02); // BYPASS_EN = 1
    delay(100);
    
    // 8. Manyetometre başlatma (isteğe bağlı)
    // initializeMagnetometer();
    
    Serial.println("MPU9250 başarıyla başlatıldı!");
    return true;
}

// 16-bit veriyi oku (signed)
int16_t read16BitRegister(uint8_t regHigh) {
    uint8_t buffer[2];
    readRegisters(regHigh, buffer, 2);
    return (int16_t)((buffer[0] << 8) | buffer[1]);
}

// Kalibrasyon için offset değerleri
float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;

void calibrateMPU9250(int samples = 500) {
    Serial.println("\nKalibrasyon başlıyor... Sensörü HAREKETSİZ tutun!");
    delay(3000);
    
    long axSum = 0, aySum = 0, azSum = 0;
    long gxSum = 0, gySum = 0, gzSum = 0;
    
    for (int i = 0; i < samples; i++) {
        int16_t ax = read16BitRegister(MPU9250_ACCEL_XOUT_H);
        int16_t ay = read16BitRegister(MPU9250_ACCEL_XOUT_H + 2);
        int16_t az = read16BitRegister(MPU9250_ACCEL_XOUT_H + 4);
        
        int16_t gx = read16BitRegister(MPU9250_GYRO_XOUT_H);
        int16_t gy = read16BitRegister(MPU9250_GYRO_XOUT_H + 2);
        int16_t gz = read16BitRegister(MPU9250_GYRO_XOUT_H + 4);
        
        axSum += ax; aySum += ay; azSum += az;
        gxSum += gx; gySum += gy; gzSum += gz;
        
        delay(5);
    }
    
    // Z ekseni offseti (1g = 8192 LSB)
    accelOffsetZ = (azSum / samples) - 8192;
    accelOffsetX = axSum / samples;
    accelOffsetY = aySum / samples;
    
    gyroOffsetX = gxSum / samples;
    gyroOffsetY = gySum / samples;
    gyroOffsetZ = gzSum / samples;
    
    Serial.println("\nKalibrasyon Tamamlandı!");
    Serial.print("Accel Offset - X:"); Serial.print(accelOffsetX);
    Serial.print(" Y:"); Serial.print(accelOffsetY);
    Serial.print(" Z:"); Serial.println(accelOffsetZ);
    
    Serial.print("Gyro Offset  - X:"); Serial.print(gyroOffsetX);
    Serial.print(" Y:"); Serial.print(gyroOffsetY);
    Serial.print(" Z:"); Serial.println(gyroOffsetZ);
    delay(7000);
}

// Sensör verilerini oku
void readSensorData() {
    // 3 eksen ivmeölçer verileri
    int16_t ax_raw = read16BitRegister(MPU9250_ACCEL_XOUT_H);
    int16_t ay_raw = read16BitRegister(MPU9250_ACCEL_XOUT_H + 2);
    int16_t az_raw = read16BitRegister(MPU9250_ACCEL_XOUT_H + 4);
    
    // 3 eksen jiroskop verileri
    int16_t gx_raw = read16BitRegister(MPU9250_GYRO_XOUT_H);
    int16_t gy_raw = read16BitRegister(MPU9250_GYRO_XOUT_H + 2);
    int16_t gz_raw = read16BitRegister(MPU9250_GYRO_XOUT_H + 4);
    
    // Sıcaklık (isteğe bağlı)
    int16_t temp = read16BitRegister(MPU9250_ACCEL_XOUT_H + 6);
    
    float ax = ax_raw - accelOffsetX;
    float ay = ay_raw - accelOffsetY;
    float az = az_raw - accelOffsetZ;

    float gx = gx_raw - gyroOffsetX;
    float gy = gy_raw - gyroOffsetY;
    float gz = gz_raw - gyroOffsetZ;

    // Ölçek faktörlerine göre dönüşüm
    // Accel: ±4g için 8192 LSB/g
    float accelX = ax / 8192.0;
    float accelY = ay / 8192.0;
    float accelZ = az / 8192.0;
    
    // Gyro: ±500 dps için 65.5 LSB/dps
    float gyroX = gx / 65.5;
    float gyroY = gy / 65.5;
    float gyroZ = gz / 65.5;
    
    // Sıcaklık dönüşümü
    // float temperature = (temp / 333.87) + 21.0;
    

    float pitchAcc = atan2(-accelX, sqrt(accelY*accelY + accelZ*accelZ)) * 180.0 / M_PI;
    float rollAcc = atan2(accelY, sqrt(accelX*accelX + accelZ*accelZ)) * 180.0 / M_PI;


    Serial.print("Pitch: "); Serial.print(pitchAcc);
    Serial.print(" Roll: "); Serial.println(rollAcc);
 
}


void setup() {
    Serial.begin(115200);

    
    Serial.println("\nMPU9250 SPI Test Programı");
    Serial.println("==========================");
    
    // CS pinini çıkış olarak ayarla
    pinMode(MPU9250_CS_PIN, OUTPUT);
    digitalWrite(MPU9250_CS_PIN, HIGH);
    
    // SPI başlat
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0)); // 8 MHz
    
    // MPU9250'i başlat
    if (!initializeMPU9250()) {
        Serial.println("Program durduruldu.");
        while (1);
    }
    
    calibrateMPU9250();
    delay(1000);
    Serial.println("\nVeri okuma başlıyor...");
}

void loop() {
    readSensorData();
    delay(20);  // 2 Hz örnekleme hızı
}
