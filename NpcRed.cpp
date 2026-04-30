#include "NpcRed.h"

NpcRed::NpcRed(QWidget* parent) : QLabel(parent) {
    QPixmap pix(":/new/prefix1/red_line.jpg"); // 路径前缀要和你的qrc一致
    this->setPixmap(pix);
    this->setScaledContents(true);
    this->resize(120, 120); // 红线比较Q萌，可以设置成正方形
    this->setStyleSheet("background: transparent;");
}
