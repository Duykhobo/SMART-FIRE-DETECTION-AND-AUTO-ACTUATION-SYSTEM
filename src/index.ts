import cors from "cors";
import dotenv from "dotenv";
import express from "express";
import http from "http";
import { Server } from "socket.io";
import connectDB from "./config/db.js";
import sensorRoutes from "./routes/sensorRoutes.js";

// Nạp các biến môi trường từ file .env
dotenv.config();

const app = express();
const httpServer = http.createServer(app);
const io = new Server(httpServer, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// Set io instance in app so it can be accessed in controllers
app.set("io", io);

app.use(express.json());
app.use(cors());

// --- 1. KẾT NỐI MONGODB ---
connectDB();

// --- 2. ĐỊNH NGHĨA API ROUTE ---
app.use("/api/sensors", sensorRoutes);

// --- 3. XỬ LÝ SOCKET.IO ---
io.on("connection", (socket) => {
  console.log("🟢 Client connected to Socket.IO:", socket.id);
  socket.on("disconnect", () => {
    console.log("🔴 Client disconnected:", socket.id);
  });
});

// --- 4. CHẠY SERVER ---
// Lấy port từ .env hoặc mặc định là 5000
const PORT = parseInt(process.env.PORT || "5000");

// Thêm "0.0.0.0" để cho phép ESP32 gọi API trong mạng LAN
httpServer.listen(PORT, "0.0.0.0", () => {
  console.log(`🚀 Server dang chay tai cong ${PORT}, san sang nhan data tu ESP32`);
});
