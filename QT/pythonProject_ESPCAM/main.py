import sys
import cv2
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

    def update_frame(self):
        if self.cap is None:
            return
        ret, frame = self.cap.read()
        if ret:
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            h, w, ch = rgb_frame.shape
            bytes_per_line = ch * w
            qimg = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format_RGB888)
            self.videoDisplay.setPixmap(QPixmap.fromImage(qimg))
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
