const client = mqtt.connect(MQTT_URL);

// Layout
const devices = [
    { id: "device_1", name: "Thiết bị 1" },
    // { id: "device_2", name: "Thiết bị 2" },
    // { id: "device_3", name: "Thiết bị 3" },
];

function createDeviceUI(device) {
    const container = document.getElementById("device-container");

    const deviceDiv = document.createElement("div");
    deviceDiv.classList.add("device-box");
    deviceDiv.id = device.id;
    
    deviceDiv.innerHTML = `
        <h2>${device.name}</h2>
        <p>Status: <span id="${device.id}_st" class="disconnected_ST">Disconnected</span></p>
        <div class="device-content">
            <div class="output-container">
                <h3>Alarm Bell Control</h3>
                ${[0].map(i => `
                    <div class="button-group">
                        <span id="${device.id}_bell">...</span>
                        <button class="send_button" onclick="toggleBELL()">Alarm Bell</button>
                    </div>
                `).join("")}
            </div>
            <div class="analog-container">
                <h3>Sensor Status</h3>
                    <p><span class="ain-label">Temperature:</span> <span id="${device.id}_tem">...</span></p>
                    <p><span class="ain-label">Humidity:</span> <span id="${device.id}_hum">...</span></p>
                    <p><span class="ain-label">Air:</span> <span id="${device.id}_air">...</span></p>
            </div>
        </div>
    `;
    
    container.appendChild(deviceDiv);
}

// Run
devices.forEach(createDeviceUI);

function resetDeviceUI(device) {
    const statusElement = document.getElementById(`${device.id}_st`);
    const bellElement = document.getElementById(`${device.id}_bell`);
    const temElement = document.getElementById(`${device.id}_tem`);
    const humElement = document.getElementById(`${device.id}_hum`);
    const airElement = document.getElementById(`${device.id}_air`);

    if (statusElement) {
        statusElement.textContent = "Disconnected";
        statusElement.classList.remove("connected_ST");
        statusElement.classList.add("disconnected_ST");
    }
    if (bellElement) bellElement.textContent = "...";
    if (temElement) temElement.textContent = "...";
    if (humElement) humElement.textContent = "...";
    if (airElement) airElement.textContent = "...";
}

client.on("connect", function () {
    console.log("Server connected.");
    // document.getElementById(`${devices[0].id}_st`).textContent = "Connected";
    // document.getElementById(`${devices[0].id}_st`).classList.remove("disconnected_ST");
    // document.getElementById(`${devices[0].id}_st`).classList.add("connected_ST");

    client.subscribe(TOPIC_SUB_SENSOR);
    // client.publish(TOPIC_PUB_CTRL, "0");
});

client.on("close", function () {
    console.log("Mat ket noi MQTT!");
    devices.forEach(device => {
        const statusElement = document.getElementById(`${device.id}_st`);
        const bellElement = document.getElementById(`${device.id}_bell`);
        const temElement = document.getElementById(`${device.id}_tem`);
        const humElement = document.getElementById(`${device.id}_hum`);
        const airElement = document.getElementById(`${device.id}_air`);

        if (statusElement) {
            statusElement.textContent = "Disconnected";
            statusElement.classList.remove("connected_ST");
            statusElement.classList.add("disconnected_ST");
        }
        if (bellElement) bellElement.textContent = "...";
        if (temElement) temElement.textContent = "...";
        if (humElement) humElement.textContent = "...";
        if (airElement) airElement.textContent = "...";
    });
});

client.on("error", function (error) {
    console.error("Loi ket noi MQTT:", error);
});

client.on("message", function (topic, message) {
    console.log(`Nhan du lieu tu ${topic}: ${message.toString()}`);

    const statusElement = document.getElementById(`${devices[0].id}_st`);
    if (statusElement) {
        statusElement.textContent = "Connected";
        statusElement.classList.add("connected_ST");
        statusElement.classList.remove("disconnected_ST");
    }

    let data;
    try {
        data = JSON.parse(message.toString());
    } catch (e) {
        console.error("Loi parse JSON:", e);
        return;
    }

    if (topic.startsWith(TOPIC_SUB_SENSOR) && data[_TEM] !== undefined && data[_HUM] !== undefined && data[_AIR] !== undefined) {
        const temElement = document.getElementById(`${devices[0].id}_tem`);
        const humElement = document.getElementById(`${devices[0].id}_hum`);
        const airElement = document.getElementById(`${devices[0].id}_air`);
        const bellElement = document.getElementById(`${devices[0].id}_bell`);

        temElement.textContent = `${data[_TEM]} °C`;
        humElement.textContent = `${data[_HUM]} %`;
        airElement.textContent = `${data[_AIR]} %`;
        if (bellElement) {
            bellElement.textContent = data[_BELL] ? "ON" : "OFF";
            bell_status = data[_BELL];
        }

        // airElement.textContent = `${data[`AIN${i}`].toFixed(4)} V`;
    }
    
    clearTimeout(deviceTimers[devices[0].id]);
    deviceTimers[devices[0].id] = setTimeout(() => {
        console.log(`Device 1 disconnected`);
        resetDeviceUI(devices[0]);
    }, 3000);
});

function toggleBELL() {
    let payload = JSON.stringify({
        "dev": 1,
        [_BELL]: 1
    });

    console.log("Gui du lieu:", payload);
    client.publish(TOPIC_PUB_CTRL, payload);
}