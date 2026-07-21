#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// --- Cấu hình chân ---
#define MQ_SENSOR_PIN A0
#define DHTPIN A1
#define FLAME_SENSOR_PIN 8
#define BUZZER_PIN 7
#define RELAY1_PIN 6
#define RELAY2_PIN 5

// Khai báo 3 chân cho LED RGB
#define RGB_RED_PIN 11
#define RGB_GREEN_PIN 10
#define RGB_BLUE_PIN 9

#define DHTTYPE DHT22
#define GAS_THRESHOLD_ON 300
#define GAS_THRESHOLD_OFF 260 // Ngưỡng Hysteresis: Dưới mới tắt quạt
#define TEMP_THRESHOLD 40 

// Cấu hình chân UART phụ để gửi sang ESP
SoftwareSerial espSerial(2, 3);

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- CÁC BIẾN QUẢN LÝ THỜI GIAN VÀ TRẠNG THÁI ---
unsigned long previousMillis = 0;
const long updateInterval = 2000; // Cập nhật LCD và gửi ESP mỗi 2 giây
bool isGasLeaking = false;        // Biến lưu trạng thái Gas để chống nhiễu
int lastSystemStatus = -1;        // Biến kiểm tra thay đổi trạng thái

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(MQ_SENSOR_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  noTone(BUZZER_PIN); 
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  digitalWrite(RGB_RED_PIN, LOW);
  digitalWrite(RGB_GREEN_PIN, HIGH);
  digitalWrite(RGB_BLUE_PIN, LOW);

  dht.begin();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("He Thong PCCC");
  lcd.setCursor(0, 1);
  lcd.print("Ket noi ESP...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // 1. ĐỌC CẢM BIẾN TỐC ĐỘ CAO (Không bị chặn bởi delay)
  int gasValue = analogRead(MQ_SENSOR_PIN);
  int flameState = digitalRead(FLAME_SENSOR_PIN);
  
  // Logic Hysteresis cho Gas: Chống bật tắt rơ-le liên tục
  if (gasValue >= GAS_THRESHOLD_ON) {
    isGasLeaking = true;
  } else if (gasValue <= GAS_THRESHOLD_OFF) {
    isGasLeaking = false;
  }

  // Lấy thời gian hiện tại
  unsigned long currentMillis = millis();

  // 2. CHỈ CẬP NHẬT DỮ LIỆU & GỬI ESP MỖI 2 GIÂY (Tránh nháy màn hình)
  if (currentMillis - previousMillis >= updateInterval) {
    previousMillis = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int systemStatus = 0;

    Serial.print("Trang thai lua: ");
    Serial.println(flameState);

    // --- HIỂN THỊ DÒNG 1 ---
    lcd.setCursor(0, 0);
    if (isnan(t)) {
      lcd.print("Loi DHT11!      ");
    } else {
      lcd.print("T:"); lcd.print((int)t); lcd.print("C  ");
      lcd.print("Gas:"); lcd.print(gasValue); lcd.print("   ");
    }

    // --- LOGIC XỬ LÝ SỰ CỐ & HIỂN THỊ DÒNG 2 ---
    lcd.setCursor(0, 1);

    // Kịch bản 1: HỎA HOẠN (Ưu tiên cao nhất)
    if (flameState == LOW || (!isnan(t) && t >= TEMP_THRESHOLD)) {
      systemStatus = 2;
      lcd.print(">> CO HOA HOAN<<");
      tone(BUZZER_PIN, 2000);
      digitalWrite(RELAY1_PIN, LOW);   
      digitalWrite(RELAY2_PIN, HIGH);  
      digitalWrite(RGB_RED_PIN, HIGH);
      digitalWrite(RGB_GREEN_PIN, LOW);
      digitalWrite(RGB_BLUE_PIN, LOW);
    }
    // Kịch bản 2: RÒ RỈ GAS (Dùng biến isGasLeaking thay vì đọc trực tiếp)
    else if (isGasLeaking) {
      systemStatus = 1;
      lcd.print(">> RO RI GAS! <<");
      tone(BUZZER_PIN, 1000);
      digitalWrite(RELAY1_PIN, HIGH);  
      digitalWrite(RELAY2_PIN, LOW);
      digitalWrite(RGB_RED_PIN, LOW);
      digitalWrite(RGB_GREEN_PIN, LOW);
      digitalWrite(RGB_BLUE_PIN, HIGH);
    }
    // Kịch bản 3: AN TOÀN
    else {
      systemStatus = 0;
      lcd.print("TT: AN TOAN     ");
      noTone(BUZZER_PIN);
      digitalWrite(RELAY1_PIN, LOW);
      digitalWrite(RELAY2_PIN, LOW);
      digitalWrite(RGB_RED_PIN, LOW);
      digitalWrite(RGB_GREEN_PIN, HIGH);
      digitalWrite(RGB_BLUE_PIN, LOW);
    }

    // --- GỬI DỮ LIỆU SANG ESP32 ---
    String dataPacket = String(t) + "," + String(h) + "," + String(gasValue) + "," + String(flameState) + "," + String(systemStatus);
    espSerial.println(dataPacket);
  }
}