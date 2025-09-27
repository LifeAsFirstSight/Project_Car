import sys
import cv2
import numpy as np
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QImage, QPixmap

from esp32_cam_ui import Ui_ESP32_CAM  # 这是 pyuic5 生成的 UI 文件


class MainWindow(QMainWindow, Ui_ESP32_CAM):
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        # 视频流相关
        self.cap = None
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_frame)

        # 按钮事件绑定
        self.startButton.clicked.connect(self.start_stream)
        self.stopButton.clicked.connect(self.stop_stream)

    def start_stream(self):
        url = self.lineEdit.text().strip()
        if not url:
            self.statusLabel.setText("❌ 流地址为空")
            return

        self.cap = cv2.VideoCapture(url)
        if not self.cap.isOpened():
            self.statusLabel.setText("❌ 无法连接到视频流")
            return

        self.startButton.setEnabled(False)
        self.stopButton.setEnabled(True)
        self.statusLabel.setText("✅ 已连接")
        self.timer.start(30)

    def process_frame(self, frame):
        """图像处理：检测路径 + 断点 + 补线"""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, mask = cv2.threshold(gray, 60, 255, cv2.THRESH_BINARY_INV)

        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if not contours:
            return frame, "⚠️ 未检测到路径"

        # 找最大轮廓
        largest = max(contours, key=cv2.contourArea)
        cv2.drawContours(frame, [largest], -1, (0, 255, 0), 2)

        # 用直线拟合路径方向
        [vx, vy, x0, y0] = cv2.fitLine(largest, cv2.DIST_L2, 0, 0.01, 0.01)

        # 路径质心
        M = cv2.moments(largest)
        if M["m00"] != 0:
            cx = int(M["m10"] / M["m00"])
            cy = int(M["m01"] / M["m00"])
        else:
            cx, cy = frame.shape[1] // 2, frame.shape[0] // 2

        # 画路径中心点
        cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)

        # 延长线（虚拟补线）
        line_len = 200
        x1 = int(cx - vx * line_len)
        y1 = int(cy - vy * line_len)
        x2 = int(cx + vx * line_len)
        y2 = int(cy + vy * line_len)
        cv2.line(frame, (x1, y1), (x2, y2), (255, 0, 0), 2)

        return frame, "✅ 路径正常"

    def update_frame(self):
        if self.cap is None:
            return
        ret, frame = self.cap.read()
        if ret:
            processed_frame, status = self.process_frame(frame)
            rgb_frame = cv2.cvtColor(processed_frame, cv2.COLOR_BGR2RGB)
            h, w, ch = rgb_frame.shape
            bytes_per_line = ch * w
            qimg = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format_RGB888)
            self.videoDisplay.setPixmap(QPixmap.fromImage(qimg))
            self.statusLabel.setText(status)
        else:
            self.statusLabel.setText("⚠️ 读取帧失败")

    def stop_stream(self):
        if self.cap:
            self.cap.release()
            self.cap = None
        self.timer.stop()
        self.videoDisplay.setText("请输入流地址并点击启动监控")
        self.startButton.setEnabled(True)
        self.stopButton.setEnabled(False)
        self.statusLabel.setText("已断开")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = MainWindow()
    w.show()
    sys.exit(app.exec_())
