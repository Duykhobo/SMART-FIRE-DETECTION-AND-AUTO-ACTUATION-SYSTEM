import mongoose from "mongoose";
import dotenv from "dotenv";

dotenv.config();

const connectDB = async () => {
  try {
    const MONGO_URI = process.env.MONGO_URI as string;
    await mongoose.connect(MONGO_URI);
    console.log("✅ Da ket noi MongoDB thanh cong!");
  } catch (err) {
    console.error("❌ Loi ket noi MongoDB:", err);
    process.exit(1);
  }
};

export default connectDB;
