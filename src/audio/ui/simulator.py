from PyQt5.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QLabel, QPushButton
from PyQt5.QtCore import Qt


class SimulatorWindow(QMainWindow):
    """Simple placeholder simulator window."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Simulator (Placeholder)")
        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)

        label = QLabel("Simulator is not yet implemented.")
        label.setAlignment(Qt.AlignCenter)
        layout.addWidget(label)

        close_btn = QPushButton("Close")
        close_btn.clicked.connect(self.close)
        layout.addWidget(close_btn, alignment=Qt.AlignCenter)
