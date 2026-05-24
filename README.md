# 🛡️ 智安云盾 (Zhi'an Cloud Shield)

基于云边端协同的AI孪生智能家庭安防系统

**贵州电子科技职业学院 · 创新创业项目**

---

## 📋 项目架构

```
┌──────────────────────────────────────────────────┐
│  🖥️ 应用层 — Three.js 3D数字孪生大屏             │
│  Glassmorphism UI · ECharts 实时数据可视化        │
├──────────────────────────────────────────────────┤
│  🧠 云端层 — Python FastAPI + WebSocket           │
│  MQTT · HTTP · RESTful API · MySQL               │
├──────────────────────────────────────────────────┤
│  ⚡ 边缘层 — STM32F103 + Jetson Orin Nano         │
│  Modbus TCP 边缘决策 · YOLOv8 人员跌倒检测       │
├──────────────────────────────────────────────────┤
│  🛰️ 感知层 — 传感器 + 执行器 + W5500 以太网      │
│  MQ-2 · MQ-7 · MQ-135 · DHT11 · 继电器 · 蜂鸣器  │
└──────────────────────────────────────────────────┘
```

## 🛠️ 技术栈

| 层级 | 技术 |
|------|------|
| **嵌入式** | STM32F103C8T6, C语言, W5500以太网, Modbus TCP, Keil MDK |
| **边缘计算** | Jetson Orin Nano, YOLOv8, OpenCV |
| **后端** | Python, FastAPI, Uvicorn, WebSocket, MySQL |
| **前端** | Three.js 3D, ECharts, Glassmorphism UI |
| **通信** | Modbus TCP, MQTT, WebSocket, RS485, SPI, I2C |
| **PCB 设计** | 嘉立创 EDA (LCEDA) |
| **传感器** | MQ-2(烟雾), MQ-7(CO), MQ-135(空气质量), DHT11(温湿度) |

## 📁 项目结构

```
zhi-an-cloud-shield/
├── stm32-firmware/          ← STM32 嵌入式固件 (C语言)
│   ├── Core/
│   │   ├── Inc/             ← 头文件
│   │   └── Src/             ← 源文件 (main.c, adc.c, ...)
│   ├── Drivers/w5500/       ← W5500 以太网芯片驱动
│   ├── networktest1.0.ioc   ← CubeMX 配置
│   ├── networktest1.0.uvprojx  ← Keil 工程
│   └── networktest1.0.uvoptx   ← Keil 选项
├── python-server/           ← Python 后端 + 前端
│   ├── main.py              ← FastAPI 服务 (端口8002)
│   ├── requirements.txt     ← Python 依赖
│   └── static/
│       ├── index.html       ← Three.js 3D大屏 (1729行)
│       └── audio/alarm.mp3  ← 警报音效
└── README.md
```

## 🚀 快速开始

### STM32 固件
```bash
cd stm32-firmware
# 用 Keil MDK-ARM 打开 networktest1.0.uvprojx
# 编译并下载到 STM32F103C8T6
```

### Python 后端
```bash
cd python-server
pip install -r requirements.txt
python main.py
# 访问 http://localhost:8002/static/index.html
```

## 🔥 核心功能

| 模块 | 功能 |
|------|------|
| 🔥 消防报警 | MQ传感器检测燃气/烟雾 → 声光报警 + 自动关阀断电 |
| ⚡ 用电安全 | 电压/电流实时监测 → 漏电/过载保护联动 |
| 📷 AI 视觉 | YOLOv8 人员跌倒识别 → 推送物业上门处置 |
| 🌐 3D 大屏 | Three.js 实时数字孪生可视化 |
| 🔔 7×24 服务 | 值班中心联动 + APP/短信/电话多渠道通知 |
| 🛠️ 断网可用 | 边缘决策 < 1秒，本地 16~64G 存储保障 |

## 🎯 项目进展

- ✅ 原理验证样机完成
- ✅ 实验室测试通过 (AI准确率 > 95%)
- ✅ 贵安社区试点改造 10~20 户家庭
- 🏆 贵州省职业院校技能大赛参赛
- 🏆 贵阳创业大赛参赛

## 📄 许可证

MIT © 2026 陈灿 (火山)
