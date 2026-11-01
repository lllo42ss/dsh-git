# Vision 说明

## 角色

摄像头只负责**球位置**，不参与循迹。循迹必须使用红外光电模块（题目限制）。

## 坐标约定

- 摆杆中心 O 为 `0 cm`；
- 球向右为 `+x`；
- 输出帧：`B,<x_cm>,<confidence>,<stamp_ms>\n`；
- `confidence < 0.3` 视为无效；
- 控制端超过 150 ms 未收到新帧时保持上一有效值。

## 标定

1. 把摆杆刻度 `-6 cm`、`0 cm`、`+6 cm` 分别对准画面；
2. 记录像素坐标；
3. 用 `Calibration(centre_px, cm_per_px, sign)` 做线性映射；
4. 用直尺复核，误差应 ≤0.3 cm。

## 运行

```bash
python -m pip install opencv-python
python -m vision.ball_tracker --source 0 --centre-px 320 --cm-per-px 0.02
```

没有 OpenCV 时，`vision.protocol` 和测试仍可运行；`BallTracker.detect` 会明确报错。

## 图传

摄像头 JPEG 通过 ESP32-S3 Wi-Fi MJPEG 推流到车外：

```text
http://<car-ip>:8080/stream
```

车外浏览器/VLC 打开后录屏；图传断流不影响车载闭环。
