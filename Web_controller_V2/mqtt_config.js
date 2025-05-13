const MQTT_HOST = "MSI.local";
// const MQTT_HOST = "lapbotlinux.local";
const MQTT_PORT = 9001;
const MQTT_URL = `mqtt://${MQTT_HOST}:${MQTT_PORT}`;

// MQTT Topics
const TOPIC_PUB_CTRL    = "/bao_chay/ctrl_a";   // Topic to publish control commands
const TOPIC_SUB_SENSOR  = "/bao_chay/sensor_st";// Topic to subscribe for sensor status

// Camera
// const CAMERA_LINK = "_Gwk85pkrc0";
const CAMERA_LINK = "http://MSI.local/camera/stream.jpg";
const cameraPlayer = document.getElementById('camera-player');
let hls = null;

// JSON Payload Keys (consistent naming)
const KEY_BELL = "b";
const KEY_TEMP = "tem";
const KEY_HUM = "hum";
const KEY_AIR = "air";
const KEY_DEVICE_ID = "dev"; // Optional: If payload needs to identify device

// --- Application State ---
const deviceTimers = {}; // Keep track of device timeouts
const SENSOR_THRESHOLDS = {
    temp_warn: 40,
    temp_crit: 50,
    hum_warn: 85,
    hum_crit: 95,
    air_warn: 20, // Example: Air quality % threshold
    air_crit: 25
};