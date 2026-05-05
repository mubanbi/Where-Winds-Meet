#include "Item.h"

Item::Item(QWidget* parent) : QLabel(parent)
{
    // 1. 设置默认属性
    this->setFixedSize(50, 50);      // 道具图标的大小
    this->setScaledContents(true);  // 图片自动填满这 50x50
    this->itemName = "一截红线";      // 默认名字

    // 2. 初始样式：背景透明
    this->setStyleSheet("background: transparent;");

    // 3. 初始隐藏，等我们在地图里设置好位置再 show
    this->hide();
}