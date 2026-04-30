#include "GameMap.h"

GameMap::GameMap(QString imagePath, QWidget* parent) : QWidget(parent) {
    bgLabel = new QLabel(this);
    QPixmap pix(imagePath);

    // 如果图片太小，手动把它拉伸到至少屏幕那么宽（可选，看您图片质量）
    // if (pix.width() < 1920) pix = pix.scaledToWidth(1920, Qt::SmoothTransformation);

    bgLabel->setPixmap(pix);
    bgLabel->resize(pix.size());

    // 【最重要的一行】：地图类本身必须 resize，摄像机才知道这里面有多大
    this->resize(pix.size());
}