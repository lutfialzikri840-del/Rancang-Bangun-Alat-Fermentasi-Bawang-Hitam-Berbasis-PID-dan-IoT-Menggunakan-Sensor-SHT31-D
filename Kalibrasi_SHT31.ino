#include <Wire.h>
#include "Adafruit_SHT31.h"

// Inisialisasi sensor SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
  Serial.begin(115200);
  
  while (!Serial) delay(10); // Menunggu Serial Monitor siap

  Serial.println("--- Uji Karakterisasi Sensor SHT31 ---");
  
  // Memulai sensor pada alamat I2C default (0x44)
  if (!sht31.begin(0x44)) {
    Serial.println("Sensor SHT31 tidak ditemukan! Periksa kabel.");
    while (1) delay(1);
  }
}

void loop() {
  // Membaca data dari sensor
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  // Cek apakah pembacaan berhasil
  if (!isnan(t) && !isnan(h)) {
    // Tampilan untuk Serial Monitor (Mudah dibaca)
    Serial.print("Suhu: "); 
    Serial.print(t); 
    Serial.print(" *C\t");
    Serial.print("Kelembapan: "); 
    Serial.print(h); 
    Serial.println(" %");

    /* 
     * Tips Karakterisasi:
     * Kamu bisa copy-paste data di bawah ini ke Excel/Google Sheets
     * Format: Suhu,Kelembapan
     */
    // Serial.print(t); Serial.print(","); Serial.println(h);
    
  } else {
    Serial.println("Gagal membaca data dari sensor!");
  }

  delay(2000); // Ambil data setiap 2 detik
}