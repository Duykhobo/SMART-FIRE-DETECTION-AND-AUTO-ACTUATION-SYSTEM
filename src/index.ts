import cors from "cors";
import dotenv from "dotenv";
import express from "express";
import connectDB from "./config/db.js";
import sensorRoutes from "./routes/sensorRoutes.js";

// Nạp các biến môi trường từ file .env
dotenv.config();

const app = express();
app.use(express.json());
app.use(cors());

// --- 1. KẾT NỐI MONGODB ---
connectDB();

// --- 2. ĐỊNH NGHĨA API ROUTE ---
app.use("/api/sensors", sensorRoutes);

// --- 3. CHẠY SERVER ---
// Lấy port từ .env hoặc mặc định là 5000
const PORT = parseInt(process.env.PORT || "5000");

// Thêm "0.0.0.0" để cho phép ESP32 gọi API trong mạng LAN
app.listen(PORT, "0.0.0.0", () => {
  console.log(`🚀 Server dang chay tai cong ${PORT}, san sang nhan data tu ESP32`);
});
