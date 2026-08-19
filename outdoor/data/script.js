const REFRESH_MS = 2000;
const HISTORY_REFRESH_MS = 5000;

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

function formatDelta(localValue, externalValue, suffix) {
  if (
    localValue === null ||
    localValue === undefined ||
    externalValue === null ||
    externalValue === undefined
  ) {
    return "N/A";
  }
  const delta = localValue - externalValue;
  const sign = delta > 0 ? "+" : "";
  return `${sign}${delta.toFixed(1)}${suffix}`;
}

function labelFromEpoch(epoch, index, total, intervalSec) {
  if (epoch && epoch > 100000) {
    // NTPClient aplica el offset local al epoch: usar getters UTC.
    const date = new Date(epoch * 1000);
    const hh = String(date.getUTCHours()).padStart(2, "0");
    const mm = String(date.getUTCMinutes()).padStart(2, "0");
    return `${hh}:${mm}`;
  }
  const minutesAgo = Math.round((total - 1 - index) * intervalSec / 60);
  return `-${Math.max(0, minutesAgo)}m`;
}

function drawLineChart(canvasId, points, valueKey, color) {
  const canvas = document.getElementById(canvasId);
  if (!canvas) {
    return;
  }

  const ctx = canvas.getContext("2d");
  const width = canvas.width;
  const height = canvas.height;
  const padding = { top: 10, right: 8, bottom: 22, left: 36 };

  ctx.clearRect(0, 0, width, height);
  ctx.fillStyle = "#0b1220";
  ctx.fillRect(0, 0, width, height);

  const values = points
    .map((p) => p[valueKey])
    .filter((v) => v !== null && v !== undefined && !Number.isNaN(v));

  if (values.length < 2) {
    ctx.fillStyle = "#94a3b8";
    ctx.font = "14px Arial";
    ctx.fillText("Esperando más muestras...", padding.left, height / 2);
    return;
  }

  let min = Math.min(...values);
  let max = Math.max(...values);
  if (min === max) {
    min -= 1;
    max += 1;
  }
  const span = max - min;
  min -= span * 0.08;
  max += span * 0.08;

  const plotW = width - padding.left - padding.right;
  const plotH = height - padding.top - padding.bottom;

  ctx.strokeStyle = "#334155";
  ctx.lineWidth = 1;
  for (let i = 0; i < 4; i++) {
    const y = padding.top + (plotH * i) / 3;
    ctx.beginPath();
    ctx.moveTo(padding.left, y);
    ctx.lineTo(width - padding.right, y);
    ctx.stroke();

    const value = max - ((max - min) * i) / 3;
    ctx.fillStyle = "#94a3b8";
    ctx.font = "11px Arial";
    ctx.fillText(value.toFixed(1), 4, y + 4);
  }

  const usable = points.filter((p) => p[valueKey] !== null && p[valueKey] !== undefined);
  ctx.beginPath();
  usable.forEach((point, index) => {
    const x = padding.left + (plotW * index) / Math.max(usable.length - 1, 1);
    const y = padding.top + plotH * (1 - (point[valueKey] - min) / (max - min));
    if (index === 0) {
      ctx.moveTo(x, y);
    } else {
      ctx.lineTo(x, y);
    }
  });
  ctx.strokeStyle = color;
  ctx.lineWidth = 2;
  ctx.stroke();

  const labelEvery = Math.max(1, Math.floor(usable.length / 5));
  ctx.fillStyle = "#94a3b8";
  ctx.font = "11px Arial";
  usable.forEach((point, index) => {
    if (index % labelEvery !== 0 && index !== usable.length - 1) {
      return;
    }
    const x = padding.left + (plotW * index) / Math.max(usable.length - 1, 1);
    ctx.fillText(point._label || "", x - 12, height - 8);
  });
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
    setText("temp-aht", formatValue(data.temperature_aht, " °C"));
    setText("humidity", formatValue(data.humidity, " %"));
    setText("pressure", formatValue(data.pressure, " hPa"));
    setText("altitude", formatValue(data.altitude, " m"));

    setText("sensor-bmp", `BMP180: ${data.bmp180_ok ? "OK" : "ERROR"}`);
    setText("sensor-aht", `AHT10: ${data.aht10_ok ? "OK" : "ERROR"}`);

    setText("cmp-temp-local", formatValue(data.temperature_main, " °C"));
    setText("cmp-temp-ext", formatValue(data.external_temperature, " °C"));
    setText(
      "cmp-temp-delta",
      formatDelta(data.temperature_main, data.external_temperature, " °C")
    );
    setText("cmp-hum-local", formatValue(data.humidity, " %"));
    setText("cmp-hum-ext", formatValue(data.external_humidity, " %"));
    setText(
      "cmp-hum-delta",
      formatDelta(data.humidity, data.external_humidity, " %")
    );

    const desc = data.external_description || "";
    setText(
      "external-desc",
      data.external_ok
        ? (desc ? desc : "OpenWeatherMap")
        : (desc ? desc : "Clima externo no disponible")
    );
    if (data.external_ok && data.external_age_s !== null && data.external_age_s !== undefined) {
      setText("external-meta", `Actualizado hace ${data.external_age_s}s · Δ = local − API`);
    } else {
      setText("external-meta", "N/A si no hay Wi-Fi, API key o falla la consulta · sensores locales siguen activos");
    }

    setText("wifi", `Wi-Fi: ${status.wifi_connected ? "Conectado" : "Desconectado"}`);
    setText("rssi", `RSSI: ${status.rssi} dBm`);
    setText("ip", `IP: ${status.ip || "N/A"}`);
    setText("hostname", `Host: ${status.hostname}`);
    setText("uptime", `Uptime: ${status.uptime} s`);
    setText("firmware", `Firmware: ${status.firmware}`);
    setText("heap", `Memoria libre: ${status.free_heap} bytes`);

    setText("clock", status.time || "--:--:--");
    setText(
      "clock-sync",
      status.ntp_synced ? "Sincronizado (NTP)" : "Esperando NTP..."
    );

    setText("status-line", "Datos actualizados");
  } catch (error) {
    setText("status-line", "Error al obtener datos");
  }
}

async function refreshHistory() {
  try {
    const response = await fetch("/api/history");
    if (!response.ok) {
      throw new Error("history error");
    }

    const history = await response.json();
    const intervalSec = history.interval_s || 30;
    const rawPoints = history.points || [];
    const points = rawPoints.map((point, index) => ({
      temp: point.temp,
      hum: point.hum,
      pres: point.pres,
      _label: labelFromEpoch(point.t, index, rawPoints.length, intervalSec)
    }));

    drawLineChart("chart-temp", points, "temp", "#38bdf8");
    drawLineChart("chart-hum", points, "hum", "#34d399");
    drawLineChart("chart-pres", points, "pres", "#fbbf24");

    const windowMin = Math.round((history.capacity || 0) * intervalSec / 60);
    setText(
      "history-meta",
      `Historial: ${points.length}/${history.capacity || "?"} muestras · cada ${intervalSec}s · ventana ~${windowMin} min`
    );
  } catch (error) {
    setText("history-meta", "Historial no disponible");
  }
}

refreshData();
refreshHistory();
setInterval(refreshData, REFRESH_MS);
setInterval(refreshHistory, HISTORY_REFRESH_MS);
