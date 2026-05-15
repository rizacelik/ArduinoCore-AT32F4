#include <SPI.h>
#include <MPU9250_WE.h>

#define MPU9250_CS_PIN    PA4
#define MPU9250_SPI       SPI

MPU9250_WE myMPU9250 = MPU9250_WE(&MPU9250_SPI, MPU9250_CS_PIN);

void setup() {
  Serial.begin(115200);
  delay(200);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  if(!myMPU9250.init()){
    Serial.println("MPU9250 bulunamadı!");
    while(1) {}
  }

  myMPU9250.autoOffsets();
  myMPU9250.enableMag();           // Manyetometre aç

  // Manyetometre kalibrasyonu (cihazı yavaşça 8 şeklinde döndür)
  Serial.println("Manyetometre kalibrasyonu - 8 şeklinde döndürün...");
  delay(5000);
  myMPU9250.autoMagOffsets();
  Serial.println("Kalibrasyon tamamlandı!");

  // AHRS filtreyi başlat (Madgwick önerilir)
  myMPU9250.setSampleRateDivider(4);
  myMPU9250.enableAcc();
  myMPU9250.enableGyro();
  myMPU9250.enableMag();
  myMPU9250.setAccRange(ACC_RANGE_2G);
  myMPU9250.setGyroRange(GYRO_RANGE_250);
  myMPU9250.enableFusion();        // Madgwick filtreyi aktif et
}

void loop() {
  xyzFloat angles = myMPU9250.getAngles();  // Roll, Pitch, Yaw

  Serial.print("Roll : ");
  Serial.print(angles.x, 1);
  Serial.print("°\tPitch: ");
  Serial.print(angles.y, 1);
  Serial.print("°\tYaw  : ");
  Serial.print(angles.z, 1);
  Serial.println("°");

  delay(50);
}