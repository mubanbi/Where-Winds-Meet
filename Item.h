#pragma once

#include <QLabel>
#include <QString>

class Item : public QLabel
{
    Q_OBJECT // 必须加上这个，否则无法使用信号槽或样式表

public:
    // parent 设为 QWidget*，方便我们把它放进地图里
    Item(QWidget* parent = nullptr);

    QString itemName; // 道具的名字，比如“陈年老酒”
};