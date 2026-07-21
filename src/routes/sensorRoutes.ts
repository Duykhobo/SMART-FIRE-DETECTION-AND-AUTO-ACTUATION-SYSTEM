import { Router } from "express";
import { createSensorData, getSensorData } from "../controllers/sensorController.js";

const router = Router();

router.post("/", createSensorData);
router.get("/", getSensorData);

export default router;
