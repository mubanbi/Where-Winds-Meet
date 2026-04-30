#pragma once
#include <QWidget>
#include <QLabel>
#include <QDebug>

class GameMap : public QWidget {
    Q_OBJECT
public:
    // 定义背景标签指针
    QLabel* bgLabel = nullptr;

    // 构造函数声明
    GameMap(QString imagePath, QWidget* parent = nullptr);
};