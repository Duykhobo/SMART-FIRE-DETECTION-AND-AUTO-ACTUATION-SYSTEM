import cors from "cors";
import dotenv from "dotenv";
import express, { type Request, type Response } from "express";
import mongoose from "mongoose";

// Nạp các biến môi trường từ file .env
dotenv.config();

const app = express();
app.use(express.json());
app.use(cors());

// --- 1. KẾT NỐI MONGODB ---
// Lấy URI từ biến môi trường (Bảo mật hơn)
const MONGO_URI = process.env.MONGO_URI as string;

mongoose
  .connect(MONGO_URI)
  .then(() => console.log("✅ Da ket noi MongoDB thanh cong!"))
  .catch((err) => console.error("❌ Loi ket noi MongoDB:", err));

// --- 2. ĐỊNH NGHĨA SCHEMA & MODEL ---
const sensorSchema = new mongoose.Schema({
  temperature: { type: Number, required: true },
  humidity: { type: Number, required: true },
  gasLevel: { type: Number, required: true },
  flameStatus: { type: Number, required: true }, // 0: Có lửa, 1: An toàn
  systemStatus: { type: Number, required: true }, // 0: An toàn, 1: Gas, 2: Hỏa hoạn
  timestamp: { type: Date, default: Date.now },
});

const SensorData = mongoose.model("SensorData", sensorSchema);

// --- 3. ĐỊNH NGHĨA API ROUTE ---

// API dùng cho ESP32 đẩy dữ liệu (POST)
app.post("/api/sensors", async (req: Request, res: Response): Promise<void> => {
  try {
    const { temperature, humidity, gasLevel, flameStatus, systemStatus } = req.body;

    // Lưu vào Database
    const newData = new SensorData({
      temperature,
      humidity,
      gasLevel,
      flameStatus,
      systemStatus,
    });

    await newData.save();
    console.log("📥 Vua nhan va luu data tu ESP32:", req.body);

    res.status(200).json({ message: "Luu du lieu thanh cong!" });
  } catch (error) {
    console.error("❌ Loi luu du lieu:", error);
    res.status(500).json({ message: "Loi server" });
  }
});

// API dùng cho React Frontend kéo dữ liệu về hiển thị (GET)
app.get("/api/sensors", async (req: Request, res: Response) => {
  try {
    // Lấy 20 bản ghi mới nhất để vẽ biểu đồ
    const data = await SensorData.find().sort({ timestamp: -1 }).limit(20);
    res.status(200).json(data);
  } catch (error) {
    res.status(500).json({ message: "Loi server" });
  }
});

// --- 4. CHẠY SERVER ---
// Lấy port từ .env hoặc mặc định là 5000
const PORT = parseInt(process.env.PORT || "5000");

// Thêm "0.0.0.0" để cho phép ESP32 gọi API trong mạng LAN
app.listen(PORT, "0.0.0.0", () => {
  console.log(`🚀 Server dang chay tai cong ${PORT}, san sang nhan data tu ESP32`);
});
