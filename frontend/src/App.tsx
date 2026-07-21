import axios from "axios";
import { Activity, AlertTriangle, Droplets, Flame, ShieldCheck, Wind } from "lucide-react";
import { useEffect, useState } from "react";
import { CartesianGrid, Legend, Line, LineChart, ResponsiveContainer, Tooltip, XAxis, YAxis } from "recharts";

interface SensorData {
  _id: string;
  temperature: number;
  humidity: number;
  gasLevel: number;
  flameStatus: number;
  systemStatus: number;
  timestamp: string;
}

export default function App() {
  const [data, setData] = useState<SensorData[]>([]);
  const [latest, setLatest] = useState<SensorData | null>(null);

  const fetchData = async () => {
    try {
      const response = await axios.get("http://localhost:5000/api/sensors");
      const chartData = response.data.reverse().map((item: SensorData) => ({
        ...item,
        time: new Date(item.timestamp).toLocaleTimeString("vi-VN", {
          hour: "2-digit",
          minute: "2-digit",
          second: "2-digit",
        }),
      }));
      setData(chartData);
      setLatest(chartData[chartData.length - 1]);
    } catch (error) {
      console.error("Lỗi kéo dữ liệu:", error);
    }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 5000);
    return () => clearInterval(interval);
  }, []);

  // Đóng gói logic UI của trạng thái để tái sử dụng
  const getStatusUI = (status?: number) => {
    switch (status) {
      case 0:
        return { text: "AN TOÀN", color: "#10B981", icon: <ShieldCheck size={40} />, border: "#047857" };
      case 1:
        return { text: "RÒ RỈ GAS", color: "#F59E0B", icon: <AlertTriangle size={40} />, border: "#B45309" };
      case 2:
        return { text: "HỎA HOẠN", color: "#EF4444", icon: <Flame size={40} />, border: "#B91C1C", isBlinking: true };
      default:
        return { text: "MẤT KẾT NỐI", color: "#64748B", icon: <Activity size={40} />, border: "#334155" };
    }
  };

  const currentUI = getStatusUI(latest?.systemStatus);

  return (
    <div
      style={{
        backgroundColor: "#0F172A", // Nền tối slate-900
        color: "#F8FAFC",
        minHeight: "100vh",
        padding: "30px",
        fontFamily: "'Inter', system-ui, sans-serif",
      }}
    >
      {/* HEADER */}
      <div
        style={{ display: "flex", alignItems: "center", justifyContent: "center", gap: "15px", marginBottom: "40px" }}
      >
        <Activity color="#38BDF8" size={32} />
        <h1 style={{ margin: 0, fontSize: "1.8rem", fontWeight: 700, letterSpacing: "1px", color: "#E2E8F0" }}>
          HỆ THỐNG GIÁM SÁT PCCC TRUNG TÂM
        </h1>
      </div>

      {/* TẦNG 1: THẺ THÔNG SỐ (GRID LAYOUT) */}
      <div
        style={{
          display: "grid",
          gridTemplateColumns: "repeat(auto-fit, minmax(320px, 1fr))",
          gap: "24px",
          marginBottom: "40px",
        }}
      >
        {/* Card 1: Trạng Thái (Nổi bật nhất) */}
        <div
          style={{
            backgroundColor: "#1E293B",
            padding: "30px 20px",
            borderRadius: "16px",
            border: `2px solid ${currentUI.border}`,
            boxShadow: `0 0 20px ${currentUI.color}20`, // Viền phát sáng nhẹ
            display: "flex",
            flexDirection: "column",
            alignItems: "center",
            justifyContent: "center",
          }}
        >
          <span
            style={{
              color: "#94A3B8",
              fontSize: "0.9rem",
              fontWeight: 600,
              letterSpacing: "1px",
              marginBottom: "15px",
            }}
          >
            TRẠNG THÁI HỆ THỐNG
          </span>
          <div
            style={{
              display: "flex",
              alignItems: "center",
              gap: "12px",
              color: currentUI.color,
              animation: currentUI.isBlinking ? "blink 1s infinite" : "none",
            }}
          >
            {currentUI.icon}
            <h2 style={{ fontSize: "2.5rem", margin: 0, fontWeight: 800 }}>{currentUI.text}</h2>
          </div>
        </div>

        {/* Card 2: Nhiệt độ & Độ ẩm */}
        <div
          style={{
            backgroundColor: "#1E293B",
            padding: "30px 24px",
            borderRadius: "16px",
            border: "1px solid #334155",
          }}
        >
          <div style={{ display: "flex", alignItems: "center", gap: "10px", color: "#94A3B8", marginBottom: "20px" }}>
            <Wind color="#38BDF8" />
            <span style={{ fontSize: "0.9rem", fontWeight: 600, letterSpacing: "1px" }}>NHIỆT ĐỘ & ĐỘ ẨM</span>
          </div>
          <div style={{ display: "flex", alignItems: "baseline", gap: "15px" }}>
            <h2 style={{ fontSize: "3rem", margin: 0, fontWeight: 700, color: "#F8FAFC" }}>
              {latest ? `${latest.temperature.toFixed(1)}°` : "--"}
            </h2>
            <span style={{ fontSize: "1.5rem", color: "#64748B", fontWeight: 500 }}>
              / {latest ? `${latest.humidity.toFixed(1)}%` : "--"}
            </span>
          </div>
        </div>

        {/* Card 3: Khí Gas */}
        <div
          style={{
            backgroundColor: "#1E293B",
            padding: "30px 24px",
            borderRadius: "16px",
            border: "1px solid #334155",
          }}
        >
          <div style={{ display: "flex", alignItems: "center", gap: "10px", color: "#94A3B8", marginBottom: "20px" }}>
            <Droplets color="#A78BFA" />
            <span style={{ fontSize: "0.9rem", fontWeight: 600, letterSpacing: "1px" }}>NỒNG ĐỘ GAS (MQ)</span>
          </div>
          <div style={{ display: "flex", alignItems: "baseline" }}>
            <h2
              style={{
                fontSize: "3rem",
                margin: 0,
                fontWeight: 700,
                color: latest && latest.gasLevel > 700 ? "#EF4444" : "#F8FAFC",
              }}
            >
              {latest ? latest.gasLevel : "--"}
            </h2>
          </div>
        </div>
      </div>

      {/* TẦNG 2: BIỂU ĐỒ TRỰC QUAN */}
      <div
        style={{
          backgroundColor: "#1E293B",
          padding: "24px",
          borderRadius: "16px",
          border: "1px solid #334155",
        }}
      >
        <h3 style={{ color: "#E2E8F0", marginTop: 0, marginBottom: "20px", fontSize: "1.1rem", fontWeight: 600 }}>
          Biến thiên Môi trường (Real-time)
        </h3>
        <div style={{ width: "100%", height: 400 }}>
          <ResponsiveContainer>
            <LineChart data={data}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" vertical={false} />
              <XAxis dataKey="time" stroke="#64748B" tick={{ fill: "#64748B" }} tickMargin={10} />
              <YAxis yAxisId="left" stroke="#64748B" tick={{ fill: "#64748B" }} domain={[0, 100]} />
              <YAxis
                yAxisId="right"
                orientation="right"
                stroke="#64748B"
                tick={{ fill: "#64748B" }}
                domain={[0, 1024]}
              />

              {/* Custom Tooltip cho Dark Mode */}
              <Tooltip
                contentStyle={{
                  backgroundColor: "#0F172A",
                  border: "1px solid #334155",
                  borderRadius: "8px",
                  color: "#F8FAFC",
                }}
                itemStyle={{ color: "#E2E8F0" }}
              />
              <Legend wrapperStyle={{ paddingTop: "20px" }} />

              <Line
                isAnimationActive={false}
                yAxisId="left"
                type="monotone"
                dataKey="temperature"
                name="Nhiệt độ (°C)"
                stroke="#F59E0B"
                strokeWidth={3}
                dot={{ r: 4, fill: "#F59E0B", strokeWidth: 0 }}
                activeDot={{ r: 6 }}
              />
              <Line
                isAnimationActive={false}
                yAxisId="right"
                type="monotone"
                dataKey="gasLevel"
                name="Khí Gas"
                stroke="#10B981"
                strokeWidth={3}
                dot={{ r: 4, fill: "#10B981", strokeWidth: 0 }}
                activeDot={{ r: 6 }}
              />
            </LineChart>
          </ResponsiveContainer>
        </div>
      </div>

      {/* Style phụ trợ cho hiệu ứng nhấp nháy khi báo cháy */}
      <style>{`
        @keyframes blink {
          0% { opacity: 1; }
          50% { opacity: 0.3; }
          100% { opacity: 1; }
        }
      `}</style>
    </div>
  );
}
