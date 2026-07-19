// 3 DÒNG NÀY PHẢI ĐẶT TRÊN CÙNG
#define BLYNK_TEMPLATE_ID "TMPL6hSbuqkNJ"
#define BLYNK_TEMPLATE_NAME "He Thong PCCC"
#define BLYNK_AUTH_TOKEN "H3udO5ohaQSSvB7AdbMMfh2i5lOzBgWE"

#include <WiFi.h>
#include <HTTPClient.h>       // Thư viện bắt buộc để gọi API Node.js
#include <BlynkSimpleEsp32.h>

// --- Cấu hình Wi-Fi ---
const char* ssid = "Duykhobo";
const char* password = "Duyzkskhobo@310";

// --- Cấu hình API Backend --- (ĐÃ FIX LỖI CÚ PHÁP)
String serverName = "http://10.27.163.1:5000/api/sensors"; 

// Chân giao tiếp với Arduino Uno
#define RXp2 16
#define TXp2 17

// Biến lưu trạng thái trước đó để chống spam thông báo
int lastStatus = -1;  

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  Serial.print("Dang ket noi Wi-Fi & Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  Serial.println("\nDa ket noi thanh cong!");
  Serial.print("Dia chi IP cua ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.println("=======================================");
}

void loop() {
  Blynk.run();

  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim();

    if (data.length() > 0) {
      int index1 = data.indexOf(',');
      int index2 = data.indexOf(',', index1 + 1);
      int index3 = data.indexOf(',', index2 + 1);
      int index4 = data.indexOf(',', index3 + 1);

      if (index1 > 0 && index2 > 0 && index3 > 0 && index4 > 0) {
        String tempStr = data.substring(0, index1);
        String humStr = data.substring(index1 + 1, index2);
        String gasStr = data.substring(index2 + 1, index3);
        String flameStr = data.substring(index3 + 1, index4);
        String statusStr = data.substring(index4 + 1);

        // =======================================================
        // 1. ĐẨY DỮ LIỆU & THÔNG BÁO LÊN BLYNK
        // =======================================================
        Blynk.virtualWrite(V0, tempStr.toFloat());
        Blynk.virtualWrite(V1, humStr.toFloat());
        Blynk.virtualWrite(V2, gasStr.toInt());
        Blynk.virtualWrite(V3, flameStr.toInt());

        int statusInt = statusStr.toInt();
        if (statusInt == 0) {
          Blynk.virtualWrite(V4, "AN TOAN");
        } else if (statusInt == 1) {
          Blynk.virtualWrite(V4, "RO RI GAS");
        } else if (statusInt == 2) {
          Blynk.virtualWrite(V4, "HOA HOAN !");
        }

        // Logic chống spam Push Notification
        if (statusInt != lastStatus) {
            if (statusInt == 2) {
                String fireMsg = "🔥 BÁO ĐỘNG ĐỎ: Hỏa hoạn! Nhiệt độ hiện tại: " + String(tempStr.toFloat(), 1) + "°C. Yêu cầu kiểm tra ngay lập tức!";
                Blynk.logEvent("fire_alarm", fireMsg);
                Serial.println(">>> ĐÃ GỬI THÔNG BÁO CHÁY QUA BLYNK <<<");
            } 
            else if (statusInt == 1) {
                String gasMsg = "⚠️ CẢNH BÁO VÀNG: Rò rỉ khí Gas! Nồng độ: " + gasStr + ". Hãy mở thông thoáng cửa sổ!";
                Blynk.logEvent("gas_leak", gasMsg);
                Serial.println(">>> ĐÃ GỬI THÔNG BÁO RÒ RỈ GAS QUA BLYNK <<<");
            }
            else if (statusInt == 0 && lastStatus != -1) {
                Blynk.logEvent("system_safe", "✅ AN TOÀN: Hệ thống PCCC đã hoạt động bình thường trở lại.");
                Serial.println(">>> Hệ thống đã an toàn <<<");
            }
            lastStatus = statusInt; 
        }

        // =======================================================
        // 2. GỬI HTTP POST VỀ BACKEND NODE.JS (MONGODB)
        // =======================================================
        String jsonPayload = "{";
        jsonPayload += "\"temperature\":" + tempStr + ",";
        jsonPayload += "\"humidity\":" + humStr + ",";
        jsonPayload += "\"gasLevel\":" + gasStr + ",";
        jsonPayload += "\"flameStatus\":" + flameStr + ",";
        jsonPayload += "\"systemStatus\":" + statusStr;
        jsonPayload += "}";

        if (WiFi.status() == WL_CONNECTED) {
          HTTPClient http;
          http.begin(serverName); 
          http.addHeader("Content-Type", "application/json"); 
          
          int httpResponseCode = http.POST(jsonPayload); 
          
          if (httpResponseCode > 0) {
            Serial.print("Node.js HTTP Code: ");
            Serial.println(httpResponseCode); 
          } else {
            Serial.print("Loi POST Node.js: ");
            Serial.println(httpResponseCode);
          }
          http.end(); 
        }

        Serial.println("---------------------------------------");
      }
    }
  }
}