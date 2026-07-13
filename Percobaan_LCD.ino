#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi alamat I2C (biasanya 0x27 atau 0x3F)
// Ukuran LCD 16 kolom dan 2 baris (16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Variabel Simulasi (Dummy) ---
// Nanti variabel ini diganti dengan hasil pembacaan sensor asli Anda
float suhu = 60.0;       // Contoh Suhu 70.5 °C
float kelembapan = 85.0; // Contoh Kelembapan 85.0 %
int berat = 1250;        // Contoh Berat 1250 gram
int hari = 12;           // Contoh Waktu Fermentasi 12 Hari

void setup() {
  // Memulai komunikasi dengan LCD
  lcd.init();       
  lcd.backlight();  

  // --- TAMPILAN AWAL (Booting) ---
  lcd.setCursor(0, 0); 
  lcd.print(" Alat Fermentasi");
  lcd.setCursor(0, 1); 
  lcd.print("  Bawang Hitam  ");
  delay(3000); // Tahan judul selama 3 detik
}

void loop() {
  // --- LAYAR 1: SUHU DAN KELEMBAPAN ---
  lcd.clear(); // Bersihkan layar sebelum menampilkan data baru
  
  // Baris 1: Suhu
  lcd.setCursor(0, 0);
  lcd.print("Suhu  : ");
  lcd.print(suhu, 1); // Angka 1 berarti menampilkan 1 angka di belakang koma
  lcd.print(" C");

  // Baris 2: Kelembapan
  lcd.setCursor(0, 1);
  lcd.print("Lembap: ");
  lcd.print(kelembapan, 1);
  lcd.print(" %");
  
  delay(3000); // Tahan layar 1 selama 3 detik

  // --- LAYAR 2: BERAT DAN WAKTU FERMENTASI ---
  lcd.clear(); 
  
  // Baris 1: Berat
  lcd.setCursor(0, 0);
  lcd.print("Berat : ");
  lcd.print(berat);
  lcd.print(" g");

  // Baris 2: Waktu
  lcd.setCursor(0, 1);
  lcd.print("Waktu : ");
  lcd.print(hari);
  lcd.print(" Hari");

  delay(3000); // Tahan layar 2 selama 3 detik
  
  // Setelah ini, program akan kembali ke atas (loop) 
  // dan menampilkan layar 1 kembali secara terus-menerus.
}