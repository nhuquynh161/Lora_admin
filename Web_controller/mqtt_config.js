// const MQTT_HOST = "MSI.local";
const MQTT_HOST = "lapbotlinux.local";
const MQTT_PORT = 9001;
const MQTT_URL = `mqtt://${MQTT_HOST}:${MQTT_PORT}`;

const deviceTimers = {};
let bell_status = 0;

const TOPIC_PUB_CTRL    = "/bao_chay/ctrl_a";
const TOPIC_SUB_SENSOR  = "/bao_chay/sensor_st";

const _BELL = "b";
const _TEM = "tem";
const _HUM = "hum";
const _AIR = "air";