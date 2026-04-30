#pragma once
#include <QLabel>
#include <QPoint>
#include <QProgressBar> // 必须包含这个头文件

class Hero : public QLabel
{
    Q_OBJECT

public:
    // 构造函数
    Hero(QWidget* parent = nullptr);

    // 动作：移动
    void moveBy(int dx, int dy);

    // 新增：受伤减血的接口
    void takeDamage(int amount);

private:
    int health = 100;
    int moveSpeed = 15;

    // --- 新增：血量条指针 ---
    QProgressBar* hpBar;

public:
    void showHpBar() { if (hpBar) hpBar->show(); } // 显示血条
    void hideHpBar() { if (hpBar) hpBar->hide(); } // 隐藏血条
};