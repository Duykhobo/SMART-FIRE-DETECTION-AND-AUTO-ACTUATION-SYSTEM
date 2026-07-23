#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define MQ_SENSOR_PIN A0
#define DHTPIN A1
#define FLAME_SENSOR_PIN 8
#define BUZZER_PIN 7
#define RELAY1_PIN 6
#define RELAY2_PIN 5
#define RGB_RED_PIN 11
#define RGB_GREEN_PIN 10
#define RGB_BLUE_PIN 9
#define DHTTYPE DHT22

#define GAS_THRESHOLD_ON 300
#define GAS_THRESHOLD_OFF 260 
#define TEMP_THRESHOLD 40 

SoftwareSerial espSerial(2, 3);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillisESP = 0;
const long intervalESP = 500; // Gửi lên mạng mỗi 500ms

unsigned long previousMillisDHT = 0;
const long intervalDHT = 2000; // DHT22 cần 2s để đọc 1 lần

bool isGasLeaking = false;
float t = 0.0;
float h = 0.0;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(MQ_SENSOR_PIN, INPUT); pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); pinMode(RELAY1_PIN, OUTPUT); pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT); pinMode(RGB_GREEN_PIN, OUTPUT); pinMode(RGB_BLUE_PIN, OUTPUT);

  noTone(BUZZER_PIN); 
  digitalWrite(RELAY1_PIN, LOW); digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RGB_RED_PIN, LOW); digitalWrite(RGB_GREEN_PIN, HIGH); digitalWrite(RGB_BLUE_PIN, LOW);

  dht.begin();
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("He Thong PCCC");
  delay(2000); lcd.clear();
  
  t = dht.readTemperature();
  h = dht.readHumidity();
}

void loop() {
  unsigned long currentMillis = millis();

  // Bắt đầu bấm giờ đo tốc độ chip
  unsigned long startProcess = millis();

  // 1. ĐỌC CẢM BIẾN TỐC ĐỘ CAO
  int gasValue = analogRead(MQ_SENSOR_PIN);
  int flameState = digitalRead(FLAME_SENSOR_PIN);

  if (currentMillis - previousMillisDHT >= intervalDHT) {
    previousMillisDHT = currentMillis;
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    if (!isnan(newT)) { t = newT; h = newH; }
  }

  // Logic Hysteresis
  if (gasValue >= GAS_THRESHOLD_ON) isGasLeaking = true;
  else if (gasValue <= GAS_THRESHOLD_OFF) isGasLeaking = false;

  int systemStatus = 0;

  // 2. KÍCH HOẠT RƠ-LE NGAY LẬP TỨC (Không bị chặn thời gian)
  if (flameState == LOW || t >= TEMP_THRESHOLD) {
    systemStatus = 2;
    tone(BUZZER_PIN, 2000);
    digitalWrite(RELAY1_PIN, LOW); digitalWrite(RELAY2_PIN, HIGH);  
    digitalWrite(RGB_RED_PIN, HIGH); digitalWrite(RGB_GREEN_PIN, LOW); digitalWrite(RGB_BLUE_PIN, LOW);
  }
  else if (isGasLeaking) {
    systemStatus = 1;
    tone(BUZZER_PIN, 2000);
    digitalWrite(RELAY1_PIN, HIGH); digitalWrite(RELAY2_PIN, LOW);
    digitalWrite(RGB_RED_PIN, LOW); digitalWrite(RGB_GREEN_PIN, LOW); digitalWrite(RGB_BLUE_PIN, HIGH);
  }
  else {
    systemStatus = 0;
    noTone(BUZZER_PIN);
    digitalWrite(RELAY1_PIN, LOW); digitalWrite(RELAY2_PIN, LOW);
    digitalWrite(RGB_RED_PIN, LOW); digitalWrite(RGB_GREEN_PIN, HIGH); digitalWrite(RGB_BLUE_PIN, LOW);
  }

  unsigned long endProcess = millis();
  unsigned long timeProcess = endProcess - startProcess;

  // 3. GỬI DỮ LIỆU ĐI ĐỊNH KỲ 500ms
  if (currentMillis - previousMillisESP >= intervalESP) {
    previousMillisESP = currentMillis;

    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print((int)t); lcd.print("C  ");
    lcd.print("Gas:"); lcd.print(gasValue); lcd.print("   ");

    lcd.setCursor(0, 1);
    if (systemStatus == 2) lcd.print(">> CO HOA HOAN<<");
    else if (systemStatus == 1) lcd.print(">> RO RI GAS! <<");
    else lcd.print("TT: AN TOAN     ");

    // Đoạn này in ra log cho tui copy nè
    if (systemStatus != 0) {
        Serial.print("Thoi gian xu ly (T_processing): "); 
        Serial.print(timeProcess); 
        Serial.println(" ms");
    }

    String dataPacket = String(t) + "," + String(h) + "," + String(gasValue) + "," + String(flameState) + "," + String(systemStatus);
    espSerial.println(dataPacket);
  }
}