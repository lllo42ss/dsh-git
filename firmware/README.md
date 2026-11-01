# 固件说明

> 这是参考骨架，不是可直接参赛的最终固件。所有引脚、减速比、舵机方向和 PID 参数都要按实车修改。

## 编译

```bash
cd firmware
pio run
pio run -t upload
pio device monitor
```

## 任务周期

| 任务 | 周期 | 位置 |
|---|---:|---|
| 视觉串口解析 | loop 每圈 | `parseVisionLine()` |
| 速度估计 | loop 每圈 | `updateSpeedEstimate()` |
| 控制环 | 5 ms | `loop()` |
| 状态显示 | 200 ms | `updateDisplay()` |

## 视觉串口

视觉模块每帧发送：

```text
B,<x_cm>,<confidence>,<stamp_ms>\n
```

例如：

```text
B,-0.42,0.96,12345
```

`x_cm` 右正，范围 -6..+6；`confidence < 0.3` 视为无效；超过 150 ms 未更新时控制环保持上一有效值。

## 必须先标定

1. 电机方向：`setMotor()` 正负与车体前进方向一致；
2. 编码器：`CM_PER_PULSE`；
3. 舵机中位与方向：`setServoAngle(0)` 时摆杆水平；
4. 红外阈值：黑线/白底 ADC；
5. 球位置符号：球向右时 `x_cm` 增大，且 PID 输出使摆杆向右下倾斜。

## 已知未完成

- OLED 驱动未接入（当前用串口输出状态）；
- 摆角实测反馈未接入，`angle_pid` 目前是占位；
- 停车基准线检测未实现，当前用按键/超时刹车；
- 图传由 `vision/` 的 Wi-Fi 服务负责，固件只保留串口接口。
