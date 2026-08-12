const REFRESH_MS = 2000;

function formatValue(value, suffix) {
  if (value === null || value === undefined) {
    return "N/A";
  }
  return `${value}${suffix}`;
}

function setText(id, text) {
  const element = document.getElementById(id);
  if (element) {
    element.textContent = text;
  }
}

async function refreshData() {
  try {
    const [dataResponse, statusResponse] = await Promise.all([
      fetch("/api/data"),
      fetch("/api/status")
    ]);

    if (!dataResponse.ok || !statusResponse.ok) {
      throw new Error("API error");
    }

    const data = await dataResponse.json();
    const status = await statusResponse.json();

    setText("temp-main", formatValue(data.temperature_main, " °C"));
    setText("temp-bmp", formatValue(data.temperature_bmp, " °C"));
    setText("temp-dht", formatValue(data.temperature_dht, " °C"));
    setText("humidity", formatValue(data.humidity, " %"));
    setText("pressure", formatValue(data.pressure, " hPa"));
    setText("altitude", formatValue(data.altitude, " m"));

    setText("sensor-bmp", `BMP180: ${data.bmp180_ok ? "OK" : "ERROR"}`);
    setText("sensor-dht", `DHT11: ${data.dht11_ok ? "OK" : "ERROR"}`);

    setText("wifi", `Wi-Fi: ${status.wifi_connected ? "Conectado" : "Desconectado"}`);
    setText("rssi", `RSSI: ${status.rssi} dBm`);
    setText("ip", `IP: ${status.ip || "N/A"}`);
    setText("hostname", `Host: ${status.hostname}`);
    setText("uptime", `Uptime: ${status.uptime} s`);
    setText("firmware", `Firmware: ${status.firmware}`);
    setText("heap", `Memoria libre: ${status.free_heap} bytes`);
    setText("status-line", "Datos actualizados");
  } catch (error) {
    setText("status-line", "Error al obtener datos");
  }
}

refreshData();
setInterval(refreshData, REFRESH_MS);
