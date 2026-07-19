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
#define GAS_THRESHOLD 200
#define TEMP_THRESHOLD 40  // (Lưu ý: Chỗ này comment ghi 60 nhưng code đang để 40 nhé, bạn nhớ sửa lại thành 60 nếu muốn)

// Cấu hình chân UART phụ để gửi sang ESP: Chân 2 (RX), Chân 3 (TX)
SoftwareSerial espSerial(2, 3);

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int hpbdMelody[] = {
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_C5, NOTE_B4,
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D5, NOTE_C5,
  NOTE_G4, NOTE_G4, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_B4, NOTE_A4,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_C5
};

// Thời lượng nốt (1 = Tròn, 2 = Trắng, 4 = Đen, 8 = Đơn)
int hpbdDurations[] = {
  8, 8, 4, 4, 4, 2,
  8, 8, 4, 4, 4, 2,
  8, 8, 4, 4, 4, 4, 2,
  8, 8, 4, 4, 4, 2
};

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(MQ_SENSOR_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Cài đặt output cho LED RGB
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  noTone(BUZZER_PIN);  // Đảm bảo còi thụ động tắt hoàn toàn lúc khởi động
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  // Khởi động với trạng thái An Toàn (Màu Xanh Lá)
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
  int gasValue = analogRead(MQ_SENSOR_PIN);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int flameState = digitalRead(FLAME_SENSOR_PIN);

  // In ra Serial Monitor để dễ dàng debug cảm biến lửa
  Serial.print("Trang thai lua: ");
  Serial.println(flameState);

  int systemStatus = 0;

  // HIỂN THỊ DÒNG 1 (THÔNG SỐ)
  lcd.setCursor(0, 0);
  if (isnan(t)) {
    lcd.print("Loi DHT11!      ");
  } else {
    lcd.print("T:");
    lcd.print((int)t);
    lcd.print("C  ");
    lcd.print("Gas:");
    lcd.print(gasValue);
    lcd.print("   ");
  }

  // LOGIC PCCC & HIỂN THỊ DÒNG 2
  lcd.setCursor(0, 1);

  // Kịch bản 1: HỎA HOẠN (Ưu tiên cao nhất)
  if (flameState == LOW || (!isnan(t) && t >= TEMP_THRESHOLD)) {
    systemStatus = 2;
    lcd.print(">> CO HOA HOAN<<");

    // Phát âm thanh báo cháy (2000Hz - Tiếng chói, the thé)
    tone(BUZZER_PIN, 2000);

    digitalWrite(RELAY1_PIN, LOW);   // Tắt quạt
    digitalWrite(RELAY2_PIN, HIGH);  // Bật máy bơm

    // Bật LED MÀU ĐỎ
    digitalWrite(RGB_RED_PIN, HIGH);
    digitalWrite(RGB_GREEN_PIN, LOW);
    digitalWrite(RGB_BLUE_PIN, LOW);

    playHappyBirthday();

  }
  // Kịch bản 2: RÒ RỈ GAS
  else if (gasValue > GAS_THRESHOLD) {
    systemStatus = 1;
    lcd.print(">> RO RI GAS! <<");

    // Phát âm thanh báo gas (1000Hz - Tiếng bíp trầm hơn)
    tone(BUZZER_PIN, 2000);

    digitalWrite(RELAY1_PIN, HIGH);  // Bật quạt thông gió
    digitalWrite(RELAY2_PIN, LOW);

    // Bật LED MÀU XANH DƯƠNG
    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, LOW);
    digitalWrite(RGB_BLUE_PIN, HIGH);

    playHappyBirthday();
  }
  // Kịch bản 3: AN TOÀN
  else {
    systemStatus = 0;
    lcd.print("TT: AN TOAN     ");

    // Tắt còi
    noTone(BUZZER_PIN);

    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);

    // Bật LED MÀU XANH LÁ
    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, HIGH);
    digitalWrite(RGB_BLUE_PIN, LOW);
  }

  // GÓI DỮ LIỆU GỬI SANG ESP
  String dataPacket = String(t) + "," + String(h) + "," + String(gasValue) + "," + String(flameState) + "," + String(systemStatus);
  espSerial.println(dataPacket);

  delay(2000);
}

void playHappyBirthday() {
  for (int thisNote = 0; thisNote < 25; thisNote++) {
    
    // Tính thời gian phát từng nốt
    int noteDuration = 1200 / hpbdDurations[thisNote];
    tone(BUZZER_PIN, hpbdMelody[thisNote], noteDuration);
    
    // Ngắt quãng giữa các nốt một chút để giai điệu rõ chữ "Happy Birthday"
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    
    noTone(BUZZER_PIN); // Ngắt nốt
  }
}

void playHongNhan() {
  for (int thisNote = 0; thisNote < 21; thisNote++) {
    
    // Tốc độ 1300 để giai điệu luyến láy mượt mà
    int noteDuration = 1300 / hongNhanDurations[thisNote];
    tone(BUZZER_PIN, hongNhanMelody[thisNote], noteDuration);
    
    // Ngắt quãng giữa các nốt (1.25) để nghe rõ từng nốt luyến
    int pauseBetweenNotes = noteDuration * 1.25;
    delay(pauseBetweenNotes);
    
    noTone(BUZZER_PIN); // Ngắt nốt
  }
}