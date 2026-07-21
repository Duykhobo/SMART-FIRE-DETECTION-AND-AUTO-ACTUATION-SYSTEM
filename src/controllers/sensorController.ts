import type { Request, Response } from "express";
import SensorData from "../models/sensorModel.js";

// API dùng cho ESP32 đẩy dữ liệu (POST)
export const createSensorData = async (req: Request, res: Response): Promise<void> => {
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

    // Phát (emit) dữ liệu mới qua Socket.IO tới tất cả Client đang kết nối
    const io = req.app.get("io");
    if (io) {
      io.emit("new_sensor_data", newData);
    }

    res.status(200).json({ message: "Luu du lieu thanh cong!" });
  } catch (error) {
    console.error("❌ Loi luu du lieu:", error);
    res.status(500).json({ message: "Loi server" });
  }
};

// API dùng cho React Frontend kéo dữ liệu về hiển thị (GET)
export const getSensorData = async (req: Request, res: Response) => {
  try {
    // Lấy 20 bản ghi mới nhất để vẽ biểu đồ
    const data = await SensorData.find().sort({ timestamp: -1 }).limit(20);
    res.status(200).json(data);
  } catch (error) {
    res.status(500).json({ message: "Loi server" });
  }
};
