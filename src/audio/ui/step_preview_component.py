from PyQt5.QtWidgets import (
    QGroupBox, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QSlider
)
from PyQt5.QtCore import Qt


class StepPreviewComponent(QGroupBox):
    """Reusable widget for previewing a single step."""

    def __init__(self, parent=None):
        super().__init__("Test Step Preview", parent)

        layout = QVBoxLayout(self)

        controls_layout = QHBoxLayout()
        self.play_pause_button = QPushButton("Play")
        self.stop_button = QPushButton("Stop")
        self.reset_button = QPushButton("Reset Tester")

        controls_layout.addWidget(self.play_pause_button)
        controls_layout.addWidget(self.stop_button)
        controls_layout.addWidget(self.reset_button)
        controls_layout.addStretch()
        layout.addLayout(controls_layout)

        self.loaded_label = QLabel("No step loaded for preview.")
        self.loaded_label.setWordWrap(True)
        self.loaded_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.loaded_label)

        self.time_slider = QSlider(Qt.Horizontal)
        layout.addWidget(self.time_slider)

        self.time_label = QLabel("00:00 / 00:00")
        self.time_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.time_label)

    def reset(self):
        """Restore default UI state."""
        self.play_pause_button.setText("Play")
        self.time_slider.setRange(0, 1)
        self.time_slider.setValue(0)
        self.time_label.setText("00:00 / 00:00")
        self.loaded_label.setText("No step loaded for preview.")
