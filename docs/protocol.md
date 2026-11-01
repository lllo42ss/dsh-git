# 图传与调试协议

## 1. 设计目标

- 车外只看画面，不参与实时闭环；
- 图传断流不影响小车继续运行；
- 调试数据可记录、可回放、可用于赛后分析。

## 2. 视频链路

```text
OV2640 ──JPEG──► ESP32-S3 ──HTTP MJPEG──► 车外浏览器/VLC
```

- 分辨率：VGA 640×480（优先帧率）或 SVGA；
- 帧率：15–30 fps；
- 端口：`8080`，路径 `/stream`；
- 车外接收端用浏览器打开 `http://<car-ip>:8080/stream`，同时用录屏软件录制；
- 比赛时关闭无关 Wi-Fi，固定信道，避免与调试网络冲突。

## 3. 串口调试帧

ASCII CSV，便于直接看：

```text
$IMU,<time_ms>,<ball_cm>,<target_cm>,<angle_deg>,<line_error>,<pwm_l>,<pwm_r>,<state>*<checksum>\n
```

示例：

```text
$IMU,12345,0.42,0.00,-1.8,0.12,320,318,RUN*5A
```

字段：

| 字段 | 单位 | 说明 |
|---|---|---|
| time_ms | ms | 上电计时 |
| ball_cm | cm | 视觉球位置，右正 |
| target_cm | cm | 目标位置 |
| angle_deg | ° | 摆杆角 |
| line_error | - | 归一化循迹误差 |
| pwm_l/r | -1000..1000 | 左右轮 PWM |
| state | 枚举 | BOOT/CAL/READY/RUN/BRAKE/FAULT |

## 4. 视觉内部协议

视觉线程 → 控制线程：

```c
typedef struct {
  float x_cm;       // 球位置，右正
  float confidence; // 0..1
  uint32_t stamp_ms;
  uint8_t valid;
} BallObservation;
```

控制线程只接受 `stamp_ms` 在 150 ms 内的观测；超时使用上一有效值，1 s 后回中。

## 5. 版本与兼容

- 协议版本放在 `docs/protocol.md` 顶部，破坏性变更升 major；
- 新增字段只能追加到行尾；
- 车外工具必须容忍未知字段；
- 日志文件按 `run-YYYYMMDD-HHMMSS.csv` 命名。
