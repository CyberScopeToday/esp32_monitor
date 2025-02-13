import clr
import os
import time

# Проверяем, существует ли DLL
dll_path = r"C:\temp\LibreHardwareMonitorLib.dll"

if not os.path.exists(dll_path):
    print("🚨 Ошибка: Файл LibreHardwareMonitorLib.dll не найден!")
    sys.exit(1)

clr.AddReference(dll_path)
from LibreHardwareMonitor.Hardware import Computer

# Включаем GPU
computer = Computer()
computer.IsGpuEnabled = True
computer.Open()

print("📢 Поиск доступных сенсоров...")

# Выводим ВСЕ сенсоры + их точный SensorType
for hardware in computer.Hardware:
    hardware.Update()
    print(f"\n🔹 {hardware.Name} ({hardware.HardwareType})")
    for sensor in hardware.Sensors:
        print(f" - {sensor.Name} | Тип: {sensor.SensorType} ({type(sensor.SensorType)}) | Значение: {sensor.Value}")

print("\n⚠️ Если нужные сенсоры отсутствуют, попробуй запустить от Админа или обновить LibreHardwareMonitor!")

while True:
    time.sleep(10)