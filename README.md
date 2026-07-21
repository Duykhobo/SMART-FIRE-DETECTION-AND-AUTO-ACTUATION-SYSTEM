# IOT Project Backend

Đây là backend server cho hệ thống theo dõi môi trường IoT (Internet of Things), được xây dựng bằng Node.js, Express và TypeScript, kết hợp với cơ sở dữ liệu MongoDB. Server này đóng vai trò trung gian, nhận dữ liệu cảm biến từ vi điều khiển ESP32 và cung cấp API cho ứng dụng frontend (ví dụ: React) để hiển thị.

## Tính năng

- **Nhận dữ liệu cảm biến:** API `POST /api/sensors` cho phép thiết bị ESP32 đẩy dữ liệu nhiệt độ, độ ẩm, mức gas, trạng thái lửa và trạng thái hệ thống lên server.
- **Lưu trữ dữ liệu:** Dữ liệu nhận được được lưu trữ an toàn vào cơ sở dữ liệu MongoDB.
- **Cung cấp dữ liệu:** API `GET /api/sensors` trả về 20 bản ghi mới nhất, giúp frontend có thể hiển thị thông tin trạng thái môi trường (ví dụ dạng biểu đồ, hoặc bảng).
- **Cấu hình dễ dàng:** Hỗ trợ sử dụng biến môi trường (`.env`) để cấu hình cổng (PORT) và chuỗi kết nối MongoDB, giúp mã nguồn an toàn và dễ triển khai.

## Cấu trúc dữ liệu cảm biến (MongoDB Schema)

Dữ liệu được lưu với các trường thông tin sau:

- `temperature` (Number): Nhiệt độ môi trường.
- `humidity` (Number): Độ ẩm môi trường.
- `gasLevel` (Number): Nồng độ khí gas.
- `flameStatus` (Number): Trạng thái cảm biến lửa (`0`: Có lửa, `1`: An toàn).
- `systemStatus` (Number): Trạng thái tổng quát của hệ thống (`0`: An toàn, `1`: Phát hiện Gas, `2`: Phát hiện Hỏa hoạn).
- `timestamp` (Date): Thời gian ghi nhận dữ liệu (tự động điền).

## Yêu cầu hệ thống

- Node.js (phiên bản >= 14)
- Cơ sở dữ liệu MongoDB (cục bộ hoặc MongoDB Atlas)

## Hướng dẫn cài đặt và chạy

1. **Cài đặt các gói thư viện (dependencies):**
   Mở terminal tại thư mục dự án và chạy:

   ```bash
   npm install
   ```
2. **Cấu hình biến môi trường:**
   Tạo một file `.env` ở thư mục gốc của dự án nếu chưa có, và thêm các dòng sau:

   ```env
   # Chọn cổng chạy server (mặc định là 5000 nếu không thiết lập)
   PORT=5000

   # Chuỗi kết nối đến cơ sở dữ liệu MongoDB của bạn
   MONGO_URI=mongodb://localhost:27017/iot_database
   ```
3. **Chạy Server (Chế độ phát triển):**
   Chạy lệnh sau để khởi động server với `nodemon` (tự động khởi động lại khi có thay đổi code):

   ```bash
   npm run dev
   ```

   *Lưu ý:* Server được cấu hình chạy trên host `0.0.0.0`, cho phép ESP32 có thể gửi dữ liệu trực tiếp đến địa chỉ IP của máy tính (trong cùng mạng LAN).

## Các API Endpoints

### 1. Đẩy dữ liệu lên Server (Dành cho ESP32)

- **URL:** `/api/sensors`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Ví dụ Body:**
  ```json
  {
    "temperature": 27.5,
    "humidity": 65,
    "gasLevel": 120,
    "flameStatus": 1,
    "systemStatus": 0
  }
  ```

### 2. Lấy dữ liệu cảm biến (Dành cho Frontend Web)

- **URL:** `/api/sensors`
- **Method:** `GET`
- **Mô tả:** Trả về danh sách 20 bản ghi cảm biến được cập nhật gần nhất (sắp xếp giảm dần theo thời gian).

## Công nghệ sử dụng

- **Express.js:** Framework xây dựng web server.
- **Mongoose:** Thư viện giao tiếp và làm việc với MongoDB.
- **TypeScript:** Cung cấp kiểu dữ liệu chặt chẽ cho JavaScript.
- **Cors:** Cho phép chia sẻ tài nguyên chéo nguồn (cần thiết khi frontend gọi API).
- **Dotenv:** Đọc các biến môi trường từ file `.env`.
