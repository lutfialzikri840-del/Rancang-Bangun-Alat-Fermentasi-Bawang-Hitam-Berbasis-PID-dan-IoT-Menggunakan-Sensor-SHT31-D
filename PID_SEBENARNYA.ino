/**
 * ==============================================================================
 * PROYEK: Sistem Kendali Suhu Alat Fermentasi Bawang Hitam
 * FITUR : PID Control + Anti-Integral Windup Otomatis
 * ==============================================================================
 */

#include <PID_v1.h>
#include <Adafruit_SHT31.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C 0x27, ukuran 16x2

double setpoint = 60.0;
double input, output, kelembapan;

// Parameter PID Inputan dari GUI
double Kp = 7; 
double Ki = 0.2; 
double Kd = 1; 

PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

// Variabel untuk mengatur refresh rate LCD agar tidak berkedip
unsigned long previousLcdMillis = 0;
const long lcdInterval = 500; 

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  
  // Tampilan Awal (Splash Screen Identitas)
  lcd.setCursor(0, 0);
  lcd.print("Lutfi Al Zikri");
  lcd.setCursor(0, 1);
  lcd.print("NIM:2210442002");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inisialisasi...");

  // Cek Sensor
  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 tidak terdeteksi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error: SHT31");
    while (1);
  }
  
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 250); // Batas maksimal PWM adalah 250

  // Header Data untuk GUI Python
  Serial.println("Setpoint,Suhu,Kelembapan,PWM");
  lcd.clear();
}

void loop() {
  // --- BACA DATA DARI PYTHON (TUNING PID) ---
  if (Serial.available() > 0) {
    String dataIn = Serial.readStringUntil('\n');
    int firstComma = dataIn.indexOf(',');
    int secondComma = dataIn.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > 0) {
      Kp = dataIn.substring(0, firstComma).toDouble();
      Ki = dataIn.substring(firstComma + 1, secondComma).toDouble();
      Kd = dataIn.substring(secondComma + 1).toDouble();
      
      // Terapkan parameter dan reset paksa memori integral saat ada input baru
      myPID.SetTunings(Kp, Ki, Kd);
      myPID.SetMode(MANUAL);
      output = 0;
      myPID.SetMode(AUTOMATIC);

      // Feedback ke Terminal VSCode
      Serial.print("INFO: Parameter PID Diubah -> Kp: ");
      Serial.print(Kp);
      Serial.print(" | Ki: ");
      Serial.print(Ki);
      Serial.print(" | Kd: ");
      Serial.println(Kd);
    }
  }

  // --- BACA SENSOR ---
  input = sht31.readTemperature();
  kelembapan = sht31.readHumidity();
  
  if (!isnan(input)) {
    // =========================================================
    // FITUR ANTI-WINDUP OTOMATIS (SOLUSI PWM NYANGKUT)
    // =========================================================
    double errorSuhu = setpoint - input;
    
    // Jika suhu aktual terpaut jauh (lebih dari 10 derajat di bawah setpoint)
    // Paksa Ki menjadi 0 agar tabungan error tidak membengkak di awal
    if (errorSuhu > 10.0) {
      myPID.SetTunings(Kp, 0.0, Kd); 
    } 
    // Jika suhu sudah mendekati target (selisih <= 10 derajat)
    // Kembalikan ke parameter Ki asli yang kamu atur di GUI
    else {
      myPID.SetTunings(Kp, Ki, Kd);  
    }
    // =========================================================

    // Jalankan Perhitungan PID
    myPID.Compute(); 
    analogWrite(2, output); // Output sinyal PWM ke SSR

    // --- KIRIM DATA GRAFIK KE PYTHON ---
    Serial.print(setpoint);     Serial.print(",");
    Serial.print(input);        Serial.print(",");
    Serial.print(kelembapan);   Serial.print(",");
    Serial.println(output);

    // --- UPDATE TAMPILAN LCD (Non-blocking) ---
    unsigned long currentMillis = millis();
    if (currentMillis - previousLcdMillis >= lcdInterval) {
      previousLcdMillis = currentMillis;
      
      lcd.setCursor(0, 0);
      lcd.print("T:"); lcd.print(input, 1); 
      lcd.print("C S:"); lcd.print(setpoint, 1);
      lcd.print("C  "); 

      lcd.setCursor(0, 1);
      lcd.print("H:"); lcd.print(kelembapan, 1); 
      lcd.print("% P:"); lcd.print((int)output);
      lcd.print("   "); 
    }
  }
  
  delay(200); 
}