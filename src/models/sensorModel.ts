import mongoose from "mongoose";

const sensorSchema = new mongoose.Schema({
  temperature: { type: Number, required: true },
  humidity: { type: Number, required: true },
  gasLevel: { type: Number, required: true },
  flameStatus: { type: Number, required: true }, // 0: Có lửa, 1: An toàn
  systemStatus: { type: Number, required: true }, // 0: An toàn, 1: Gas, 2: Hỏa hoạn
  timestamp: { type: Date, default: Date.now },
});

const SensorData = mongoose.model("SensorData", sensorSchema);

export default SensorData;
