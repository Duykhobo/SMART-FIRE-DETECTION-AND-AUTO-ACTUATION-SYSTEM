// 3 DÒNG NÀY PHẢI ĐẶT TRÊN CÙNG
#define BLYNK_TEMPLATE_ID "TMPL6hSbuqkNJ"
#define BLYNK_TEMPLATE_NAME "He Thong PCCC"
#define BLYNK_AUTH_TOKEN "H3udO5ohaQSSvB7AdbMMfh2i5lOzBgWE"

#include <WiFi.h>
#include <HTTPClient.h>       
#include <BlynkSimpleEsp32.h>

// --- Cấu hình Wi-Fi ---
const char* ssid = "Duykhobo";
const char* password = "Duyzkskhobo@890";

// --- Cấu hình API Backend ---
String serverName = "http://10.132.58.1:5000/api/sensors"; 

// Chân giao tiếp với Arduino Uno
#define RXp2 16
#define TXp2 17

int lastStatus = -1;  

// --- Biến toàn cục để chia sẻ dữ liệu giữa 2 Core ---
float g_temp = 0.0;
float g_hum = 0.0;
int g_gas = 0;
int g_flame = 0;
int g_status = 0;
bool newDataReady = false; 

// Hàm Task chạy ngầm trên Core 0 để gửi HTTP POST
void sendToWebTask(void *pvParameters) {
  for (;;) {
    // Nếu có dữ liệu mới và Wi-Fi đang kết nối
    if (newDataReady && WiFi.status() == WL_CONNECTED) {
      newDataReady = false; // Đặt lại cờ
      
      String jsonPayload = "{";
      jsonPayload += "\"temperature\":" + String(g_temp) + ",";
      jsonPayload += "\"humidity\":" + String(g_hum) + ",";
      jsonPayload += "\"gasLevel\":" + String(g_gas) + ",";
      jsonPayload += "\"flameStatus\":" + String(g_flame) + ",";
      jsonPayload += "\"systemStatus\":" + String(g_status);
      jsonPayload += "}";

      HTTPClient http;
      http.begin(serverName); 
      http.addHeader("Content-Type", "application/json"); 
      http.setTimeout(2000); // Giảm timeout xuống 2s để tránh kẹt lâu
      
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
    // Nghỉ 100ms để nhường CPU cho các tác vụ hệ thống khác
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  
  // Rút ngắn timeout của Serial xuống 20ms để không bị kẹt khi đọc dữ liệu
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  Serial2.setTimeout(20); 

  Serial.print("Dang ket noi Wi-Fi & Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  Serial.println("\nDa ket noi thanh cong!");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("=======================================");

  // Khởi tạo Task chạy trên Core 0 cho việc gửi HTTP POST
  xTaskCreatePinnedToCore(
    sendToWebTask,   // Tên hàm Task
    "TaskHTTP",      // Tên Task (để debug)
    8192,            // Kích thước Stack (tăng lên để xử lý HTTP an toàn)
    NULL,            // Tham số truyền vào
    1,               // Độ ưu tiên (1 là bình thường)
    NULL,            // Task handle
    0                // Chạy trên Core 0 (Blynk và Loop mặc định chạy trên Core 1)
  );
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

        // Lưu vào biến toàn cục cho Core 0 đọc và gửi Web
        g_temp = tempStr.toFloat();
        g_hum = humStr.toFloat();
        g_gas = gasStr.toInt();
        g_flame = flameStr.toInt();
        g_status = statusStr.toInt();
        newDataReady = true; // Báo cho Core 0 biết có data mới

        // =======================================================
        // 1. CẬP NHẬT BLYNK NGAY LẬP TỨC TRÊN CORE 1
        // =======================================================
        Blynk.virtualWrite(V0, g_temp);
        Blynk.virtualWrite(V1, g_hum);
        Blynk.virtualWrite(V2, g_gas);
        Blynk.virtualWrite(V3, g_flame);

        if (g_status == 0) {
          Blynk.virtualWrite(V4, "AN TOAN");
        } else if (g_status == 1) {
          Blynk.virtualWrite(V4, "RO RI GAS");
        } else if (g_status == 2) {
          Blynk.virtualWrite(V4, "HOA HOAN !");
        }

        // Logic chống spam Push Notification
        if (g_status != lastStatus) {
            if (g_status == 2) {
                String fireMsg = "🔥 BÁO ĐỘNG ĐỎ: Hỏa hoạn! Nhiệt độ hiện tại: " + String(g_temp, 1) + "°C. Yêu cầu kiểm tra ngay lập tức!";
                Blynk.logEvent("fire_alarm", fireMsg);
                Serial.println(">>> ĐÃ GỬI THÔNG BÁO CHÁY <<<");
            } 
            else if (g_status == 1) {
                String gasMsg = "⚠️ CẢNH BÁO VÀNG: Rò rỉ khí Gas! Nồng độ: " + String(g_gas) + ". Hãy mở thông thoáng cửa sổ!";
                Blynk.logEvent("gas_leak", gasMsg);
                Serial.println(">>> ĐÃ GỬI THÔNG BÁO RÒ RỈ GAS <<<");
            }
            else if (g_status == 0 && lastStatus != -1) {
                Blynk.logEvent("system_safe", "✅ AN TOÀN: Hệ thống PCCC đã hoạt động bình thường trở lại.");
                Serial.println(">>> Hệ thống đã an toàn <<<");
            }
            lastStatus = g_status; 
        }
      }
    }
  }
}