/**
 * ==============================================================================
 * PROYEK: Rancang Bangun Alat Fermentasi Bawang Hitam
 * FITUR : PID Control + Anti-Integral Windup, Mist Maker, Load Cell, Blynk, Google Sheets (Secure)
 * ==============================================================================
 */

#define BLYNK_TEMPLATE_ID "TMPL6k7wbR5K1"
#define BLYNK_TEMPLATE_NAME "Fermentasi Bawang Hitam"
#define BLYNK_AUTH_TOKEN "OCB8BuQAF7Cs0R0VX8ckMx2nQn7py3QF"
#define GOOGLE_SCRIPT_ID "AKfycbyWShQT6FxhsYqIKAxldVpxhkibHAMC1A5sc2fkBmWZlnp1nDqb6ch8-Z3pC8o8cn4iMg" 

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h> // Library untuk koneksi HTTPS ke Google
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h> 
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>

char ssid[] = "Irawati";
char pass[] = "Pasword12345";

const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
const int RELAY_PIN = 27; 
const int SSR_PIN = 2; 

Adafruit_SHT31 sht31 = Adafruit_SHT31();
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ====================================================
// PERUBAHAN: Setpoint diubah menjadi 50.0
// ====================================================
double setpoint = 60.0;
double input, output; 
double Kp = 7.0, Ki = 0.06, Kd = 1.0; 
PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

double kelembapan;
float berat;
float calibration_factor = -400; 
float target_kelembapan = 80.0;
bool notifSelesaiSent = false;

unsigned long previousLcdMillis = 0;
const long lcdInterval = 500; 

unsigned long previousSerialMillis = 0;
const long serialInterval = 1800000; // Interval 30 menit
bool firstDataSent = false; 

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SSR_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, HIGH); 
  analogWrite(SSR_PIN, 0);       
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Lutfi Al Zikri");
  lcd.setCursor(0, 1);
  lcd.print("NIM:2210442002");
  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Konek ke WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();

  if (!sht31.begin(0x44)) {
    lcd.clear();
    lcd.print("Error: SHT31");
    while (1) delay(1); 
  }
  
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 250);
  lcd.clear();
}

void loop() {
  Blynk.run(); 
  unsigned long currentMillis = millis(); 

  input = sht31.readTemperature();
  kelembapan = sht31.readHumidity();
  
  // PERUBAHAN: Ambil rata-rata 5 sampel agar load cell lebih stabil
  berat = scale.get_units(5); 

  if (!isnan(input) && !isnan(kelembapan)) {
    
    // =========================================================
    // LOGIKA NOTIFIKASI FERMENTASI (Target 50% dari 210g = 105g)
    // =========================================================
    if (Blynk.connected()) {
      static unsigned long waktuDibawahTarget = 0;
      
      // Jika berat menyentuh 105 gram (atau di bawahnya, batas bawah 20g)
      if (berat <= 105.0 && berat > 20.0) {
        if (waktuDibawahTarget == 0) {
          waktuDibawahTarget = millis(); // Mulai timer
        } 
        // Jika stabil di bawah 105g selama 10 detik
        else if ((millis() - waktuDibawahTarget > 10000) && !notifSelesaiSent) {
          Blynk.logEvent("fermentasi_bawang_hitam_selesai", "Fermentasi Bawang Hitam Selesai! (Berat 50%)");
          notifSelesaiSent = true; 
        }
      } 
      else {
        waktuDibawahTarget = 0; // Reset timer jika berat naik lagi
      }
      
      // Reset notifikasi jika bawang baru (210g) dimasukkan (berat melewati 150g)
      if (berat > 150.0) {
        notifSelesaiSent = false;
      }
    }

    // =========================================================
    // FITUR ANTI-WINDUP OTOMATIS
    // =========================================================
    double errorSuhu = setpoint - input;
    
    if (errorSuhu > 10.0) {
      myPID.SetTunings(Kp, 0.0, Kd); 
    } 
    else {
      myPID.SetTunings(Kp, Ki, Kd);  
    }
    // =========================================================

    myPID.Compute(); 
    analogWrite(SSR_PIN, output); 
    int statusPwmBlynk = (output > 0) ? 1 : 0;

    int statusMistBlynk = 0; 
    if (kelembapan < target_kelembapan) {
      digitalWrite(RELAY_PIN, LOW); 
      statusMistBlynk = 1;
    } else {
      digitalWrite(RELAY_PIN, HIGH); 
      statusMistBlynk = 0;
    }

    if (currentMillis - previousLcdMillis >= lcdInterval) {
      previousLcdMillis = currentMillis;
      lcd.setCursor(0, 0);
      lcd.print("T:"); lcd.print(input, 1); lcd.print("C ");
      lcd.print("B:"); lcd.print(berat, 0); lcd.print("g  ");
      lcd.setCursor(0, 1);
      if (WiFi.status() != WL_CONNECTED) {
        lcd.print("WIFI TERPUTUS!  ");
      } else {
        lcd.print("H:"); lcd.print(kelembapan, 1); lcd.print("% ");
        lcd.print("P:"); lcd.print((int)output);
        lcd.print("   "); 
      }
    }

    if ((currentMillis - previousSerialMillis >= serialInterval) || !firstDataSent) {
      previousSerialMillis = currentMillis; 
      firstDataSent = true; 

      Blynk.virtualWrite(V0, input);
      Blynk.virtualWrite(V1, kelembapan);
      Blynk.virtualWrite(V2, berat);
      Blynk.virtualWrite(V3, output);
      Blynk.virtualWrite(V4, statusPwmBlynk);

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure secureClient;
        secureClient.setInsecure();
        HTTPClient http;
        String url = "https://script.google.com/macros/s/" + String(GOOGLE_SCRIPT_ID) + 
                     "/exec?suhu=" + String(input, 1) + 
                     "&kelembapan=" + String(kelembapan, 1) + 
                     "&berat=" + String(berat, 0) + 
                     "&pwm=" + String((int)output) + 
                     "&mist=" + String(statusMistBlynk);
        
        http.begin(secureClient, url);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        http.end();
        Serial.print("Data dikirim ke server. Kode HTTP: ");
        Serial.println(httpCode);
      }
    }
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    analogWrite(SSR_PIN, 0);
  }
}