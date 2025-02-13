import requests
import time
import clr

clr.AddReference(r"C:\temp\LibreHardwareMonitorLib.dll")
from LibreHardwareMonitor.Hardware import Computer, SensorType

computer = Computer()
computer.IsGpuEnabled = True
computer.Open()

def get_gpu_data():
    data = {
        "gpu_temp": 0,
        "gpu_load": 0,
        "gpu_hotspot": 0,
        "gpu_memory": 0,
        "gpu_mem_used": 0,
        "gpu_mem_free": 0,
        "gpu_core_clock": 0,
        "gpu_mem_clock": 0,
        "gpu_power": 0,
        "gpu_fan": 0
    }

    for hardware in computer.Hardware:
        hardware.Update()
        
        for sensor in hardware.Sensors:
            sensor_type = str(sensor.SensorType)  # Преобразуем SensorType в строку

            print(f"📡 Датчик: {sensor.Name} | Тип: {sensor_type} | Значение: {sensor.Value}")

            if sensor_type == "Temperature" and sensor.Name == "GPU Core":
                data["gpu_temp"] = round(sensor.Value, 1)
            if sensor_type == "Temperature" and sensor.Name == "GPU Hot Spot":
                data["gpu_hotspot"] = round(sensor.Value, 1)
            if sensor_type == "Load" and sensor.Name == "GPU Core":
                data["gpu_load"] = round(sensor.Value, 1)
            if sensor_type == "Load" and sensor.Name == "GPU Memory":
                data["gpu_memory"] = round(sensor.Value, 1)
            if sensor_type == "SmallData" and sensor.Name == "GPU Memory Used":
                data["gpu_mem_used"] = round(sensor.Value, 1)
            if sensor_type == "SmallData" and sensor.Name == "GPU Memory Free":
                data["gpu_mem_free"] = round(sensor.Value, 1)
            if sensor_type == "Clock" and sensor.Name == "GPU Core":
                data["gpu_core_clock"] = round(sensor.Value, 1)
            if sensor_type == "Clock" and sensor.Name == "GPU Memory":
                data["gpu_mem_clock"] = round(sensor.Value, 1)
            if sensor_type == "Power" and sensor.Name == "GPU Package":
                data["gpu_power"] = round(sensor.Value, 1)
            if sensor_type == "Fan" and sensor.Name == "GPU Fan 1":
                data["gpu_fan"] = round(sensor.Value, 1)

    return data

ESP_IP = "192.168.54.70"
ESP_PORT = 80

while True:
    gpu_data = get_gpu_data()
    
    print(f"✅ GPU Data: {gpu_data}")

    if gpu_data["gpu_temp"] > 0:
        try:
            requests.get(f"http://{ESP_IP}:{ESP_PORT}/update", params=gpu_data)
        except Exception as e:
            print("🚨 Ошибка отправки данных:", e)
    
    time.sleep(5)
