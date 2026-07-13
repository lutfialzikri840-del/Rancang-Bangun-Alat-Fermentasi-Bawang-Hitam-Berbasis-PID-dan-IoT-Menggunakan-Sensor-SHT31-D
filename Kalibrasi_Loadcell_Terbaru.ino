#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

HX711 scale;

// Inisialisasi LCD I2C dengan alamat 0x27, ukuran 16 kolom dan 2 baris
// Jika LCD tidak tampil, coba ganti alamat 0x27 menjadi 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Ubah angka ini sampai berat yang terbaca di Serial Monitor/LCD sesuai
float calibration_factor = -404.0; 

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi LCD
  lcd.init();      // Beberapa library menggunakan lcd.begin();
  lcd.backlight(); // Menyalakan lampu latar LCD
  
  lcd.setCursor(0, 0);
  lcd.print("Timbangan Digital");
  lcd.setCursor(0, 1);
  lcd.print("Memulai...");
  delay(2000);
  lcd.clear();
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); // Reset saat awal dinyalakan

  Serial.println("Ketik 'a' (+10) atau 'z' (-10) untuk kalibrasi");
  Serial.println("Ketik 't' untuk TARE (Nol-kan)");
}

void loop() {
  scale.set_scale(calibration_factor);

  // Baca input dari Serial Monitor
  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == 't' || temp == 'T') scale.tare();
    else if (temp == 'a') calibration_factor += 10.0;
    else if (temp == 'z') calibration_factor -= 10.0;
  }

  // Simpan nilai berat ke dalam variabel
  float berat = scale.get_units(5);

  // === TAMPILAN KE SERIAL MONITOR ===
  Serial.print("Berat: ");
  Serial.print(berat, 1);
  Serial.print(" g | Faktor Kalibrasi: ");
  Serial.println(calibration_factor);

  // === TAMPILAN KE LCD ===
  // Baris 1: Menampilkan Berat
  lcd.setCursor(0, 0);
  lcd.print("Berat: ");
  lcd.print(berat, 1);
  lcd.print(" g    "); // Tambahan spasi untuk menghapus sisa karakter lama

  // Baris 2: Menampilkan Faktor Kalibrasi
  lcd.setCursor(0, 1);
  lcd.print("Faktor:");
  lcd.print(calibration_factor, 0); 
  lcd.print("    "); // Tambahan spasi
  
  delay(200);
}