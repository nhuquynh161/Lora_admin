const client = mqtt.connect(MQTT_URL);
const connectionStatusWrapper = document.getElementById("connection-status-wrapper");
const alertBannerContainer = document.getElementById("alert-banner-container");
const cameraModal = document.getElementById("camera-modal"); // Get modal element

// Device Configuration
const devices = [
    { id: "device_1", name: "Fire Alarm Device", subtitle: "IoT Device Monitoring" },
    // Add more devices here if needed
];

// --- UI Generation ---

function createConnectionStatusUI(status = "connecting") {
    connectionStatusWrapper.innerHTML = `
        <div id="mqtt-connection-status" class="connection-status ${status}">
            <div class="connection-status-indicator"></div>
            <span class="connection-status-text">${status.charAt(0).toUpperCase() + status.slice(1)}</span>
        </div>
    `;
}

function showAlertBanner(level = "warning", message = "Sensor values approaching critical levels") {
     // Only create if not already there or if message/level changes drastically
     const existingBanner = alertBannerContainer.querySelector('.alert-banner');
     if (!existingBanner || existingBanner.dataset.level !== level || existingBanner.querySelector('.alert-message').textContent !== message) {
        alertBannerContainer.innerHTML = `
            <div class="alert-banner ${level}" role="alert" data-level="${level}">
                <div class="alert-icon-wrapper ${level}">
                   <i class="fas fa-triangle-exclamation alert-icon"></i>
                </div>
                <div class="alert-text-wrapper">
                    <h3 class="alert-title">${level === 'critical' ? 'CRITICAL' : level.charAt(0).toUpperCase() + level.slice(1)} Alert</h3>
                    <p class="alert-message">${message}</p>
                </div>
            </div>
        `;
     }
     alertBannerContainer.style.display = 'block'; // Ensure container is visible
     alertBannerContainer.querySelector('.alert-banner')?.classList.remove('hidden');
 }

function hideAlertBanner() {
    const banner = alertBannerContainer.querySelector('.alert-banner');
    if (banner) {
        banner.classList.add('hidden');
         // Optionally remove after transition:
         // setTimeout(() => {
         //    if (banner.classList.contains('hidden')) { // Check if still hidden
         //       alertBannerContainer.innerHTML = '';
         //       alertBannerContainer.style.display = 'none';
         //    }
         // }, 300); // Match CSS transition duration
    } else {
         alertBannerContainer.style.display = 'none';
    }
}

function createDeviceUI(device) {
    const container = document.getElementById("device-container");

    const deviceDiv = document.createElement("div");
    deviceDiv.classList.add("device-card");
    deviceDiv.id = device.id;

    deviceDiv.innerHTML = `
        <div class="device-card-header">
            <div class="device-card-title-group">
                <div class="device-card-icon-wrapper">
                    <!-- Using a more generic icon like target's BellRing -->
                    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="device-card-icon"><path d="M6 8a6 6 0 0 1 12 0c0 7 3 9 3 9H3s3-2 3-9"></path><path d="M10.3 21a1.94 1.94 0 0 0 3.4 0"></path><path d="M4 2C2.8 3.7 2 5.7 2 8"></path><path d="M22 8c0-2.3-.8-4.3-2-6"></path></svg>
                </div>
                <div>
                    <h2 class="device-card-title">${device.name}</h2>
                    <p class="device-card-subtitle">${device.subtitle}</p>
                </div>
            </div>
            <div class="flex items-center"> <!-- Keep flex for potential future items -->
                 <span id="${device.id}_st" class="device-status-badge offline">Offline</span>
            </div>
        </div>

        <div class="device-card-body">
            <div class="content-grid">
                <!-- Sensor Readings Section -->
                <div class="content-section">
                    <h3 class="section-title">
                        <span class="section-icon-wrapper sensors">
                            <!-- Icon matching target -->
                            <svg class="section-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path></svg>
                        </span>
                        Sensor Readings
                    </h3>
                    <div class="sensor-readings-grid">
                        <div class="sensor-reading-card">
                            <p class="sensor-label"><span class="sensor-icon">🌡️</span>Temperature</p>
                            <div class="gauge-replacement-visual">
                                <span class="gauge-value-main" id="${device.id}_tem_value">--</span>
                                <span class="gauge-value-unit">°C</span>
                            </div>
                            <p class="sensor-value-precise" id="${device.id}_tem_precise">... °C</p>
                        </div>
                        <div class="sensor-reading-card">
                            <p class="sensor-label"><span class="sensor-icon">💧</span>Humidity</p>
                            <div class="gauge-replacement-visual">
                                <span class="gauge-value-main" id="${device.id}_hum_value">--</span>
                                <span class="gauge-value-unit">%</span>
                            </div>
                             <p class="sensor-value-precise" id="${device.id}_hum_precise">... %</p>
                        </div>
                        <div class="sensor-reading-card">
                            <p class="sensor-label"><span class="sensor-icon">🌫️</span>Air Quality</p>
                             <div class="gauge-replacement-visual">
                                <span class="gauge-value-main" id="${device.id}_air_value">--</span>
                                <span class="gauge-value-unit">%</span>
                             </div>
                             <p class="sensor-value-precise" id="${device.id}_air_precise">... %</p>
                        </div>
                    </div>
                </div>

                <!-- Controls Section -->
                <div class="content-section">
                     <h3 class="section-title">
                         <span class="section-icon-wrapper controls">
                             <!-- Icon matching target -->
                            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="section-icon"><path d="M6 8a6 6 0 0 1 12 0c0 7 3 9 3 9H3s3-2 3-9"></path><path d="M10.3 21a1.94 1.94 0 0 0 3.4 0"></path><path d="M4 2C2.8 3.7 2 5.7 2 8"></path><path d="M22 8c0-2.3-.8-4.3-2-6"></path></svg>
                         </span>
                         Alarm Controls
                     </h3>
                     <div class="controls-grid"> <!-- Use grid for controls and camera -->
                        <div class="alarm-control-card">
                             <div id="${device.id}_bell_visual" class="alarm-status-visual off">
                                <!-- Icon matching target -->
                               <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="${device.id}_bell_icon" class="alarm-status-icon"><path d="M11.7 4.002a9.73 9.73 0 0 1 8.6 8.602l.035.364a8.07 8.07 0 0 1-1.6 5.2l-.07.095a13.84 13.84 0 0 1-4.33 3.63M13.19 2A8.704 8.704 0 0 0 6.9 6.93l-.13.13a6.32 6.32 0 0 0-1.89 3.79L4.8 11.7v.12a11.95 11.95 0 0 0 2.26 6.51l.15.19s.11.09.14.12a11.88 11.88 0 0 0 4.4 2.46M10.3 21a1.94 1.94 0 0 0 3.4 0M4 2C2.8 3.7 2 5.7 2 8M18 8a6 6 0 0 0-9.3-5M14 22l-3-3"></path><path d="m2 2 20 20"></path></svg>
                                <div id="${device.id}_bell_status_text" class="alarm-status-text-badge off">Standby</div>
                             </div>
                             <button id="${device.id}_bell_button" class="alarm-toggle-button off" onclick="toggleBELL('${device.id}')" disabled>
                                  <!-- Icon matching target -->
                                 <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide lucide-bell-ring h-5 w-5 mr-2"><path d="M6 8a6 6 0 0 1 12 0c0 7 3 9 3 9H3s3-2 3-9"></path><path d="M10.3 21a1.94 1.94 0 0 0 3.4 0"></path><path d="M4 2C2.8 3.7 2 5.7 2 8"></path><path d="M22 8c0-2.3-.8-4.3-2-6"></path></svg>
                                 <span>Trigger Alarm</span>
                             </button>
                         </div>
                         <!-- Camera Section -->
                         <div class="content-section"> <!-- Nested section for Camera -->
                            <h3 class="section-title">
                                <span class="section-icon-wrapper camera">
                                    <!-- Icon matching target -->
                                    <svg class="section-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 10l4.553-2.276A1 1 0 0121 8.618v6.764a1 1 0 01-1.447.894L15 14M5 18h8a2 2 0 002-2V8a2 2 0 00-2-2H5a2 2 0 00-2 2v8a2 2 0 002 2z"></path></svg>
                                </span>
                                Camera Giám Sát
                            </h3>
                            <div class="camera-placeholder-card" onclick="openCameraModal()">
                                <div class="camera-icon-bg">
                                    <!-- Icon matching target -->
                                     <svg class="camera-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 10l4.553-2.276A1 1 0 0121 8.618v6.764a1 1 0 01-1.447.894L15 14M5 18h8a2 2 0 002-2V8a2 2 0 00-2-2H5a2 2 0 00-2 2v8a2 2 0 002 2z"></path></svg>
                                </div>
                                <p class="camera-text-main">Nhấn vào đây để xem camera</p>
                                <p class="camera-text-sub">Video giám sát từ thiết bị</p>
                            </div>
                        </div>
                     </div>
                 </div>
            </div>
        </div>
    `;

    container.appendChild(deviceDiv);
}

// --- UI Update Functions ---

function updateConnectionStatusUI(status) {
    const statusElement = document.getElementById("mqtt-connection-status");
    if (!statusElement) {
        createConnectionStatusUI(status); // Create if not exists
        return;
    }
    statusElement.className = `connection-status ${status}`; // Update classes
    const textElement = statusElement.querySelector(".connection-status-text");
    if (textElement) {
        textElement.textContent = status.charAt(0).toUpperCase() + status.slice(1);
    }
}

function updateDeviceStatus(deviceId, isConnected) {
    const statusElement = document.getElementById(`${deviceId}_st`);
    const bellButton = document.getElementById(`${deviceId}_bell_button`);
    if (!statusElement) return;

    if (isConnected) {
        statusElement.textContent = "Online";
        statusElement.classList.remove("offline");
        statusElement.classList.add("online");
    } else {
        statusElement.textContent = "Offline";
        statusElement.classList.remove("online");
        statusElement.classList.add("offline");
        resetDeviceUI(devices.find(d => d.id === deviceId)); // Reset fully on disconnect
    }
}

function updateSensorUI(deviceId, sensorData) {
    const ids = { tem: KEY_TEMP, hum: KEY_HUM, air: KEY_AIR };
    const units = { tem: '°C', hum: '%', air: '%' };

    let warningTriggered = false;
    let criticalTriggered = false;
    let alertMessage = "";
    let alertLevel = "warning"; // Default to warning

    for (const key in ids) {
        const rawValue = sensorData[ids[key]]; // Get the raw value (might be string)
        const valueElement = document.getElementById(`${deviceId}_${key}_value`);
        const preciseElement = document.getElementById(`${deviceId}_${key}_precise`);
        const cardElement = valueElement?.closest('.sensor-reading-card');

        // Check if rawValue exists and is not empty before trying to convert
        if (valueElement && rawValue !== undefined && rawValue !== null && String(rawValue).trim() !== '') {
             const value = parseFloat(rawValue); // *** Convert string to floating-point number ***

             // *** Add an extra check: Is the result actually a number? ***
             if (!isNaN(value)) {
                 const roundedValue = Math.round(value); // Use the numeric value
                 valueElement.textContent = roundedValue;
                 if (preciseElement) {
                      // Use the numeric value for toFixed
                      preciseElement.textContent = `${value.toFixed(2)} ${units[key]}`;
                 }

                 const warnThreshold = SENSOR_THRESHOLDS[`${key}_warn`];
                 const critThreshold = SENSOR_THRESHOLDS[`${key}_crit`];

                 cardElement?.classList.remove('warning-value', 'critical-value'); // Reset visual indicators

                 if (critThreshold !== undefined && value >= critThreshold) {
                    criticalTriggered = true;
                    alertLevel = 'critical';
                    alertMessage = `CRITICAL: ${key.toUpperCase()} level (${value.toFixed(1)}${units[key]}) exceeded threshold (${critThreshold}${units[key]})!`;
                    cardElement?.classList.add('critical-value');
                 } else if (warnThreshold !== undefined && value >= warnThreshold) {
                    warningTriggered = true;
                    if (!criticalTriggered) {
                        alertMessage = `Warning: ${key.toUpperCase()} level (${value.toFixed(1)}${units[key]}) approaching threshold (${warnThreshold}${units[key]})`;
                    }
                    cardElement?.classList.add('warning-value');
                 }

             } else {
                 // Handle case where conversion failed (e.g., value was "N/A" or something non-numeric)
                 console.warn(`Failed to parse sensor value for ${key}:`, rawValue);
                 valueElement.textContent = '--';
                 if (preciseElement) preciseElement.textContent = `??? ${units[key]}`; // Indicate parse error
                 cardElement?.classList.remove('warning-value', 'critical-value');
             }
        } else if (valueElement) {
            // Handle case where value is missing/null/empty string
            valueElement.textContent = '--';
            if (preciseElement) preciseElement.textContent = `... ${units[key]}`;
            cardElement?.classList.remove('warning-value', 'critical-value');
        }
    }

    // Update Alert Banner based on highest severity
    if (criticalTriggered || warningTriggered) {
        showAlertBanner(alertLevel, alertMessage);
    } else {
        hideAlertBanner();
    }
}

function updateBellUI(deviceId, bellState) {
    const bellVisualElement = document.getElementById(`${deviceId}_bell_visual`);
    const bellIconElement = document.getElementById(`${deviceId}_bell_icon`); // Target the SVG or its container if needed
    const bellStatusTextElement = document.getElementById(`${deviceId}_bell_status_text`);
    const bellButton = document.getElementById(`${deviceId}_bell_button`);
    if (!bellVisualElement || !bellIconElement || !bellStatusTextElement || !bellButton) return;

    const isBellOn = (bellState === 1);

    // Update Visual Indicator (Circle)
    bellVisualElement.classList.remove("on", "off");
    bellVisualElement.classList.add(isBellOn ? "on" : "off");

    // Update Icon (Change the SVG content or use classes if applicable)
    // This is tricky with inline SVGs. Easiest is to replace the innerHTML if icons are simple.
    // Using Font Awesome for simplicity here:
    bellIconElement.outerHTML = isBellOn
        ? `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="${deviceId}_bell_icon" class="alarm-status-icon"><path d="M6 8a6 6 0 0 1 12 0c0 7 3 9 3 9H3s3-2 3-9"></path><path d="M10.3 21a1.94 1.94 0 0 0 3.4 0"></path><path d="M4 2C2.8 3.7 2 5.7 2 8"></path><path d="M22 8c0-2.3-.8-4.3-2-6"></path></svg>` // Bell ON icon
        : `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="${deviceId}_bell_icon" class="alarm-status-icon"><path d="M11.7 4.002a9.73 9.73 0 0 1 8.6 8.602l.035.364a8.07 8.07 0 0 1-1.6 5.2l-.07.095a13.84 13.84 0 0 1-4.33 3.63M13.19 2A8.704 8.704 0 0 0 6.9 6.93l-.13.13a6.32 6.32 0 0 0-1.89 3.79L4.8 11.7v.12a11.95 11.95 0 0 0 2.26 6.51l.15.19s.11.09.14.12a11.88 11.88 0 0 0 4.4 2.46M10.3 21a1.94 1.94 0 0 0 3.4 0M4 2C2.8 3.7 2 5.7 2 8M18 8a6 6 0 0 0-9.3-5M14 22l-3-3"></path><path d="m2 2 20 20"></path></svg>`; // Bell OFF icon

    // Update Status Text Badge
    bellStatusTextElement.textContent = isBellOn ? "Active" : "Standby";
    bellStatusTextElement.classList.remove("on", "off");
    bellStatusTextElement.classList.add(isBellOn ? "on" : "off");

    // Update Button
    bellButton.classList.remove("on", "off");
    bellButton.classList.add(isBellOn ? "on" : "off");
    bellButton.disabled = false;
    const buttonIconHTML = isBellOn
       ? `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide lucide-bell-off h-5 w-5 mr-2"><path d="M11.7 4.002a9.73 9.73 0 0 1 8.6 8.602l.035.364a8.07 8.07 0 0 1-1.6 5.2l-.07.095a13.84 13.84 0 0 1-4.33 3.63M13.19 2A8.704 8.704 0 0 0 6.9 6.93l-.13.13a6.32 6.32 0 0 0-1.89 3.79L4.8 11.7v.12a11.95 11.95 0 0 0 2.26 6.51l.15.19s.11.09.14.12a11.88 11.88 0 0 0 4.4 2.46M10.3 21a1.94 1.94 0 0 0 3.4 0M4 2C2.8 3.7 2 5.7 2 8M18 8a6 6 0 0 0-9.3-5M14 22l-3-3"></path><path d="m2 2 20 20"></path></svg>` // Bell Off icon
       : `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide lucide-bell-ring h-5 w-5 mr-2"><path d="M6 8a6 6 0 0 1 12 0c0 7 3 9 3 9H3s3-2 3-9"></path><path d="M10.3 21a1.94 1.94 0 0 0 3.4 0"></path><path d="M4 2C2.8 3.7 2 5.7 2 8"></path><path d="M22 8c0-2.3-.8-4.3-2-6"></path></svg>`; // Bell Ring icon
    bellButton.innerHTML = `${buttonIconHTML} <span>${isBellOn ? 'Silence Alarm' : 'Trigger Alarm'}</span>`;


    // Store current state on the button for toggle function
    bellButton.dataset.currentState = bellState;
}

function resetDeviceUI(device) {
    if (!device) return;
    const deviceId = device.id;
    console.log(`Resetting UI for ${deviceId}`);

     const statusElement = document.getElementById(`${deviceId}_st`);
     if (statusElement && !statusElement.classList.contains('online')) {
        statusElement.textContent = "Offline";
        statusElement.classList.remove("online");
        statusElement.classList.add("offline");
     }

    updateSensorUI(deviceId, { [KEY_TEMP]: undefined, [KEY_HUM]: undefined, [KEY_AIR]: undefined });
    hideAlertBanner();

    const bellVisualElement = document.getElementById(`${deviceId}_bell_visual`);
    const bellIconElement = document.getElementById(`${deviceId}_bell_icon`);
    const bellStatusTextElement = document.getElementById(`${deviceId}_bell_status_text`);
    const bellButton = document.getElementById(`${deviceId}_bell_button`);

    if (bellVisualElement) bellVisualElement.className = "alarm-status-visual off";
     if (bellIconElement) bellIconElement.outerHTML = `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="${deviceId}_bell_icon" class="alarm-status-icon"><path d="M11.7 4.002a9.73 9.73 0 0 1 8.6 8.602l.035.364a8.07 8.07 0 0 1-1.6 5.2l-.07.095a13.84 13.84 0 0 1-4.33 3.63M13.19 2A8.704 8.704 0 0 0 6.9 6.93l-.13.13a6.32 6.32 0 0 0-1.89 3.79L4.8 11.7v.12a11.95 11.95 0 0 0 2.26 6.51l.15.19s.11.09.14.12a11.88 11.88 0 0 0 4.4 2.46M10.3 21a1.94 1.94 0 0 0 3.4 0M4 2C2.8 3.7 2 5.7 2 8M18 8a6 6 0 0 0-9.3-5M14 22l-3-3"></path><path d="m2 2 20 20"></path></svg>`; // Reset icon to OFF
    if (bellStatusTextElement) {
        bellStatusTextElement.textContent = "Standby";
        bellStatusTextElement.className = "alarm-status-text-badge off";
    }
    if (bellButton) {
        // Reset button to default 'Trigger' state
        const defaultIconHTML = `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide lucide-bell-ring h-5 w-5 mr-2"><path d="M6 8a6 6 0 0 1 12 0c0 7 3 9 3 9H3s3-2 3-9"></path><path d="M10.3 21a1.94 1.94 0 0 0 3.4 0"></path><path d="M4 2C2.8 3.7 2 5.7 2 8"></path><path d="M22 8c0-2.3-.8-4.3-2-6"></path></svg>`;
        bellButton.innerHTML = `${defaultIconHTML} <span>Trigger Alarm</span>`;
        bellButton.className = "alarm-toggle-button off";
        bellButton.disabled = true;
        delete bellButton.dataset.currentState;
    }
}

// --- MQTT Event Handlers --- (Largely the same, ensure UI update functions are called)
client.on("connect", function () {
    console.log("MQTT client connected.");
    updateConnectionStatusUI("connected");
    client.subscribe(TOPIC_SUB_SENSOR, function (err) { /* ... error handling ... */ });
    devices.forEach(device => resetTimeout(device.id));
});

client.on("close", function () {
    console.warn("MQTT connection closed.");
    updateConnectionStatusUI("disconnected");
    devices.forEach(device => {
        clearTimeout(deviceTimers[device.id]);
        updateDeviceStatus(device.id, false); // This calls resetDeviceUI
    });
    hideAlertBanner();
});

client.on("error", function (error) {
    console.error("MQTT Connection Error:", error);
    updateConnectionStatusUI("error");
    hideAlertBanner();
});

client.on("message", function (topic, message) {
    const deviceId = devices[0].id; // Assuming single device
    console.log(`MQTT message on ${topic}: ${message.toString()}`);
    let data;
    try { data = JSON.parse(message.toString()); } catch (e) { return; }

    updateDeviceStatus(deviceId, true);

    const sensorData = {
        [KEY_TEMP]: data[KEY_TEMP],
        [KEY_HUM]: data[KEY_HUM],
        [KEY_AIR]: data[KEY_AIR]
    };
    updateSensorUI(deviceId, sensorData);

    if (data[KEY_BELL] !== undefined) {
        updateBellUI(deviceId, data[KEY_BELL]);
    } else {
        // If bell status isn't always sent, handle potential 'unknown' state
        const bellButton = document.getElementById(`${deviceId}_bell_button`);
        if (bellButton && bellButton.dataset.currentState === undefined) {
            // Keep button disabled, maybe show 'unknown' visual state if desired
        }
    }
    resetTimeout(deviceId);
});


// --- Device Timeout Handling ---
function resetTimeout(deviceId, timeoutDuration = 6000) {
    clearTimeout(deviceTimers[deviceId]);
    deviceTimers[deviceId] = setTimeout(() => {
        console.warn(`No message received from ${deviceId} for ${timeoutDuration}ms. Marking as disconnected.`);
        updateDeviceStatus(deviceId, false);
    }, timeoutDuration);
}

// --- Control Function ---
function toggleBELL() {
    let payload = JSON.stringify({
        "dev": 1,
        [KEY_BELL]: 1
    });

    console.log(`Publishing to ${TOPIC_PUB_CTRL}: ${payload}`);
    client.publish(TOPIC_PUB_CTRL, payload);
}


// --- Modal Functions ---
// function openCameraModal() {
//     if (cameraModal && cameraPlayer) {
//         if (Hls.isSupported()) {
//             if (hls) {
//                 hls.destroy();
//             }

//             hls = new Hls();
//             hls.loadSource(CAMERA_LINK);
//             hls.attachMedia(cameraPlayer);
//             hls.on(Hls.Events.MANIFEST_PARSED, function() {
//                 cameraPlayer.play().catch(e => console.error("Autoplay failed:", e));
//             });
//              hls.on(Hls.Events.ERROR, function (event, data) {
//                 console.error('HLS.js error:', data);
//             });
//         } else if (cameraPlayer.canPlayType('application/vnd.apple.mpegurl')) {
//             cameraPlayer.src = CAMERA_HLS_URL;
//             cameraPlayer.addEventListener('loadedmetadata', function() {
//                 cameraPlayer.play().catch(e => console.error("Autoplay failed:", e));
//             });
//         } else {
//              console.error("HLS is not supported by this browser.");
//         }
//         cameraModal.classList.add('visible');
//         console.log("Camera modal opened with HLS stream");

//         // cameraModal.classList.add('visible');

//         // // Add potential logic here later, e.g., trying to load a stream URL
//         // // cameraPlayer.src = `https://www.youtube.com/embed/${CAMERA_LINK}?autoplay=1&mute=1&rel=0`;

//         // console.log("Camera modal opened");
//     } else {
//         console.error("Camera modal or HLS video element not found!");
//     }
// }

function openCameraModal() {
    if (cameraModal && cameraPlayer) {
        // Khởi tạo một biến đếm ảnh
        let imageCounter = 0;

        // Hàm cập nhật hình ảnh liên tục
        function updateImageFeed() {
            const currentTime = new Date().getTime(); // Để tránh cache
            cameraPlayer.src = `${CAMERA_LINK}?${currentTime}`;  // Cập nhật hình ảnh mỗi giây
            imageCounter++;

            setTimeout(updateImageFeed, 100);
        }

        // Bắt đầu cập nhật hình ảnh khi mở modal
        updateImageFeed();

        // Hiển thị modal và bắt đầu tải hình ảnh
        cameraModal.classList.add('visible');
        console.log("Camera modal opened with live image feed");
    } else {
        console.error("Camera modal or image element not found!");
    }
}

function closeCameraModal() {
    // if (cameraModal) {
    //     cameraModal.classList.remove('visible');
    //     console.log("Camera modal closed");
    //     // Add cleanup logic if needed (e.g., stop video stream)
    //     cameraPlayer.src = 'about:blank';
    // }
    if (cameraModal && cameraPlayer) {
        cameraModal.classList.remove('visible');
        // *** Quan trọng: Dừng stream và hủy instance HLS.js ***
        cameraPlayer.pause();
        cameraPlayer.removeAttribute('src'); // Xóa src
        cameraPlayer.load(); // Yêu cầu trình duyệt dừng tải
        if (hls) {
            hls.stopLoad();
            hls.detachMedia();
            hls.destroy();
            hls = null;
        }
        console.log("Camera modal closed, HLS stream stopped.");
    }
}

// Close modal if clicking outside the content area
function closeCameraModalOnClickOutside(event) {
    if (event.target === cameraModal) { // Check if the click was directly on the overlay
        closeCameraModal();
    }
}


// --- Initialization ---
createConnectionStatusUI("connecting");
devices.forEach(createDeviceUI);
hideAlertBanner(); // Ensure banner is hidden initially

console.log("MQTT script loaded. Attempting to connect...");