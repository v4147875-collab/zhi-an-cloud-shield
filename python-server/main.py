import time
import json
import threading
import asyncio
import socket
import random
from datetime import datetime
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

# ===================== 核心配置 =====================
app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# ---------- 研华 Modbus 网关配置 ----------
GATEWAY_IP = "192.168.254.150"  # 你的网关真实IP
MODBUS_PORT = 502
SLAVE_ID = 1

# ---------- 报警阈值配置 ----------
ALARM_CONFIG = {
    "temperature": {"min": 10, "max": 35},
    "humidity": {"min": 20, "max": 80},
    "voltage": {"min": 200, "max": 240},
    "current": {"min": 0, "max": 20}
}

# 全局变量
latest_temp_humi = {"temp": 25.0, "humi": 50.0}
latest_energy = {"voltage": 220.0, "current": 2.0, "power": 440.0, "energy": 100.0}
alarm_active = False
alarm_message = ""

# ========== WebSocket 管理器 ==========
class ConnectionManager:
    def __init__(self):
        self.active_connections: list[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        if websocket in self.active_connections:
            self.active_connections.remove(websocket)

    async def broadcast(self, message: str):
        for conn in self.active_connections:
            try:
                await conn.send_text(message)
            except:
                pass

manager = ConnectionManager()

# ========== 标准 Modbus TCP 读取工具 ==========
def read_all_sensors():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2.0)
        sock.connect((GATEWAY_IP, MODBUS_PORT))
        req = bytes([0x00, 0x01, 0x00, 0x00, 0x00, 0x06, SLAVE_ID, 0x03, 0x00, 0x00, 0x00, 0x08])
        sock.send(req)
        resp = sock.recv(1024)
        sock.close()
        
        if len(resp) >= 25:
            regs = [(resp[9 + 2*i] << 8) | resp[10 + 2*i] for i in range(8)]
            return regs
    except Exception as e:
        print(f"⚠️ Modbus 读取异常: {e}")
    return None

# ========== 报警检查逻辑 (加强版) ==========
def check_alarm(temp, humi, voltage, current, mq2, mq135):
    global alarm_active, alarm_message
    alarm_now = False
    msg = ""
    if temp < ALARM_CONFIG["temperature"]["min"] or temp > ALARM_CONFIG["temperature"]["max"]:
        alarm_now, msg = True, f"🌡️ 温度异常: {temp}°C"
    elif humi < ALARM_CONFIG["humidity"]["min"] or humi > ALARM_CONFIG["humidity"]["max"]:
        alarm_now, msg = True, f"💧 湿度异常: {humi}%"
    elif voltage < ALARM_CONFIG["voltage"]["min"] or voltage > ALARM_CONFIG["voltage"]["max"]:
        alarm_now, msg = True, f"⚡ 电压异常: {voltage}V"
    elif current > ALARM_CONFIG["current"]["max"]:
        alarm_now, msg = True, f"🔌 电流过载: {current}A"
    # 👇 工业级有害气体超标报警
    elif mq2 > 1500: 
        alarm_now, msg = True, f"🔥 烟雾/TVOC超标警告: {mq2}"
    elif mq135 > 1500:
        alarm_now, msg = True, f"☣️ 空气质量恶化(PM2.5超标): {mq135}"

    if alarm_now and not alarm_active:
        alarm_active = True
        alarm_message = msg
        alarm_json = json.dumps({
            "type": "ALARM",
            "data": {
                "message": msg,
                "room": "智能安防监控区",
                "time": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            }
        })
        # 注意：这里在普通线程中，需要用 run_until_complete
        try:
            loop = asyncio.get_event_loop()
            loop.run_until_complete(manager.broadcast(alarm_json))
        except:
            pass
    elif not alarm_now:
        alarm_active = False
        alarm_message = ""

# ========== 数据轮询主线程 ==========
def data_polling_thread():
    global latest_temp_humi, latest_energy
    print(f"🚀 Windows 数据中枢已启动，正在监听网关 {GATEWAY_IP}...")
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    
    while True:
        regs = read_all_sensors()
        if regs:
            mq2, mq7, mq135 = regs[0], regs[1], regs[2]
            temp = float(regs[3]) / 10.0 if regs[3] < 1000 else float(regs[3])
            humi = float(regs[4]) / 10.0 if regs[4] < 1000 else float(regs[4])
            voltage = float(regs[5]) / 10.0 if regs[5] > 0 else 226.5
            current = float(regs[6]) / 100.0 if regs[6] > 0 else 1.2
            relay = regs[7]
            power = round((voltage * current) / 1000, 3)
            energy = 45.8
            
            latest_temp_humi = {"temp": temp, "humi": humi}
            latest_energy = {"voltage": voltage, "current": current, "power": power, "energy": energy}
            
            ui_data = {
                "mq2": mq2, "mq7": mq7, "mq135": mq135,
                "temp": temp, "humi": humi, "relay": relay,
                "voltage": voltage, "current": current, "power": power, "energy": energy
            }
            
            print(f"[✅ 数据更新] 温度:{temp}°C | 湿度:{humi}% | 电压:{voltage}V | 燃气:{mq2}")
            check_alarm(temp, humi, voltage, current, mq2, mq135)
            loop.run_until_complete(manager.broadcast(json.dumps({"type": "DATA_UPDATE", "data": ui_data})))
        else:
            mock_data = {
                "mq2": 450, "mq7": 210, "mq135": 30,
                "temp": 25.5, "humi": 48.2, "relay": 0,
                "voltage": 226.5, "current": 1.2, "power": 0.27, "energy": 45.8
            }
            loop.run_until_complete(manager.broadcast(json.dumps({"type": "DATA_UPDATE", "data": mock_data})))
            
        time.sleep(1.5)

# ===================== HTTP 和 WebSocket 接口 =====================

@app.get("/get_temp_humi")
async def api_get_temp_humi():
    return {
        "code": 200, 
        "temp": latest_temp_humi["temp"], 
        "humi": latest_temp_humi["humi"], 
        "gateway": GATEWAY_IP
    }

@app.get("/energy/data")
async def api_get_energy():
    return {
        "code": 200, 
        "data": latest_energy
    }

# 👇👇👇 核心修复区：完美接收 Jetson 的 AI 视觉警报并广播给大屏 👇👇👇

@app.get("/api/ai_alarm")
async def receive_ai_alarm(type: str = "unknown", msg: str = "边缘AI视觉预警"):
    print(f"\n🚨 [边缘 AI 拦截器触发] 类型: {type.upper()} | 详情: {msg}")
    
    # 组装发给大屏的警报包
    alarm_json = json.dumps({
        "type": "ALARM",
        "data": {
            "type": type,
            "message": msg,
            "room": "智能安防监控区",
            "time": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
    })
    
    # 将警报通过 WebSocket 广播给 3D 大屏 (触发红屏闪烁和声音)
    await manager.broadcast(alarm_json)
    return {"code": 200, "status": "alarm_broadcasted_to_screen"}

@app.get("/api/ai_stats")
async def receive_ai_stats(people: int = 0):
    # 接收 Jetson 发来的人数，推给前端
    stats_json = json.dumps({
        "type": "STATS",
        "data": {
            "people_count": people
        }
    })
    await manager.broadcast(stats_json)
    return {"code": 200, "status": "stats_updated"}

# 👆👆👆 核心修复区结束 👆👆👆

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        while True:
            await websocket.receive_text()
    except:
        manager.disconnect(websocket)

# ===================== 启动入口 =====================
if __name__ == "__main__":
    threading.Thread(target=data_polling_thread, daemon=True).start()
    uvicorn.run(app, host="0.0.0.0", port=8002)