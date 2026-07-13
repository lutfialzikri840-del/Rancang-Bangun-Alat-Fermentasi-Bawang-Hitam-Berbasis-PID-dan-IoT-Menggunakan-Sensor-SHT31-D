/* --- KODE KONFIGURASI BLYNK (HARUS DI PALING ATAS) --- */
#define BLYNK_TEMPLATE_ID "TMPL6oiEd_SOJ"
#define BLYNK_TEMPLATE_NAME "Fermentasi Bawang Hitam"
#define BLYNK_AUTH_TOKEN "qBCXWOOwqCuEzaQu3IXcWftnfe0jA-A1"

/* --- LIBRARY --- */
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include "HX711.h"

// --- KONFIGURASI WI-FI ---
char ssid[] = "iPhone";
char pass[] = "11111111";

// --- INISIALISASI SENSOR ---
Adafruit_SHT31 sht31 = Adafruit_SHT31();

const int HX711_DOUT = 4;
const int HX711_SCK = 5;
HX711 scale;

// Ganti nilai 1.0 ini nanti dengan hasil kalibrasi Load Cell Anda
float calibration_factor = 405.0; 

// --- VARIABEL LOGIKA FERMENTASI (DITETAPKAN MANUAL) ---
float beratAwal = 2000;       // Ditetapkan awal 2000 gram
float targetSelesai = 1000;   // Ditetapkan notifikasi muncul saat <= 1000 gram
float beratSekarang = 0;      // <-- INI YANG TADI TERHAPUS (SUDAH DITAMBAHKAN)
bool sedangFermentasi = false;
bool notifikasiTerkirim = false;

BlynkTimer timer;

// --- FUNGSI TOMBOL MULAI (V4) ---
// Hanya berfungsi sebagai saklar pengaktif sistem pemantauan notifikasi
// --- FUNGSI TOMBOL MULAI (V4) ---
BLYNK_WRITE(V4) {
  int statusTombol = param.asInt();
  
  if (statusTombol == 1) { 
    Serial.println("\n--- TOMBOL DITEKAN ---");
    Serial.println("Memaksa kirim notifikasi ke server Blynk SEKARANG JUGA...");
    
    // Perintah kirim paksa
    Blynk.logEvent("fermentasi_bawang_hitam_selesai", "Ini Notifikasi Uji Coba Bypass!");
    
    sedangFermentasi = true;
    notifikasiTerkirim = true; // Langsung dikunci agar tidak dobel
  } else {
    sedangFermentasi = false;
    Serial.println("\n--- TOMBOL DIMATIKAN ---");
  }
}

// --- FUNGSI PEMBACAAN DAN PENGIRIMAN DATA ---
void sendSensorData() {
  // Baca Suhu & Kelembapan
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  
  // Baca Berat (rata-rata 5 kali agar stabil)
  beratSekarang = scale.get_units(5);
  if (beratSekarang < 0) beratSekarang = 0; 

  // Kirim data Suhu & Kelembapan ke Blynk
  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  } else {
    Serial.println("Gagal membaca sensor SHT31-D!");
  }
  
  // Kirim data Berat ke Blynk
  Blynk.virtualWrite(V3, beratSekarang);

  // Tampilkan data di Serial Monitor
  Serial.print("Suhu: "); Serial.print(t); Serial.print(" C | ");
  Serial.print("Kelembapan: "); Serial.print(h); Serial.print(" % | ");
  Serial.print("Berat: "); Serial.print(beratSekarang); Serial.println(" g");

  // Logika Notifikasi Saat Mencapai Target 1000 gram
  if (sedangFermentasi && !notifikasiTerkirim) {
    if (beratSekarang <= targetSelesai) {
      
      // Mengirimkan trigger event ke server Blynk
      Blynk.logEvent("fermentasi_selesai", "Fermentasi Selesai! Berat telah mencapai 1 Kg.");
      Serial.println("\n!!! NOTIFIKASI: Fermentasi Selesai (Target 1 Kg Tercapai) !!!\n");
      
      // Mengunci agar notifikasi tidak dikirim berulang-ulang
      notifikasiTerkirim = true;
      sedangFermentasi = false; 
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Mulai sensor SHT31-D (Alamat I2C standar 0x44)
  if (!sht31.begin(0x44)) { 
    Serial.println("SHT31-D tidak ditemukan! Cek kabel.");
  }

  // Mulai sensor HX711
  scale.begin(HX711_DOUT, HX711_SCK);
  scale.set_scale(calibration_factor); 
  scale.tare(); // Nol-kan timbangan saat pertama kali menyala

  // Mulai koneksi ke Wi-Fi dan server Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Atur interval pengiriman data ke Blynk (setiap 2 detik)
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}