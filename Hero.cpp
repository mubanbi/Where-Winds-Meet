#include "Hero.h"
#include <QPixmap>
#include <QDebug>

Hero::Hero(QWidget* parent) : QLabel(parent)
{
    // --- 1. 加载人物图片逻辑（保持不变） ---
    QPixmap pix(":/new/prefix1/shaodongjia.png");
    if (pix.isNull()) {
        qDebug() << "警告：主角图片加载失败！";
        return;
    }

    this->setPixmap(pix);
    this->setScaledContents(true);
    this->resize(120, 180); // 这里维持原来的 180 高度即可
    this->setStyleSheet("background: transparent;");

    // ======================================================
    // 【方案三核心修改点】
    // ======================================================
    // 关键：将 hpBar 的父对象设为传入的 parent（即 GameMap），而不是 this
    hpBar = new QProgressBar(parent);

    hpBar->setRange(0, 100);
    hpBar->setValue(health);

    hpBar->setStyleSheet(
        "QProgressBar {"
        "   border: none;"             // 去掉白边框
        "   background-color: rgba(0, 0, 0, 100);" // 背景设为半透明黑色
        "   border-radius: 3px;"       // 圆角好看一点
        "}"
        "QProgressBar::chunk {"
        "   background-color: #FF0000;" // 纯红血条
        "   border-radius: 3px;"
        "}"
    );
    // 把高度调细一点，比如 5 像素
    hpBar->setFixedHeight(5);

    hpBar->setTextVisible(false);

    // 初始化位置：需要根据 Hero 当前的位置来计算
    // 放在 Hero 的 x + 20, y - 15 的位置
    hpBar->setGeometry(this->x() + 20, this->y() - 15, 80, 8);

    hpBar->hide();
    hpBar->raise(); // 确保在地图之上
    // ======================================================

    this->show();
    qDebug() << "hero loaded with independent HP bar";
}

// 【方案三核心修改点】
// 既然血条父对象变了，移动时必须手动带上它
void Hero::moveBy(int dx, int dy) {
    // 1. 移动主角
    this->move(this->x() + dx, this->y() + dy);

    // 2. 重新计算血条位置 
    if (hpBar) {
        // 水平居中算法：主角X + (主角宽 - 血条宽) / 2
        // 这里：this->x() + (120 - 80) / 2 = this->x() + 20
        int targetX = this->x() -3 ;

        // 垂直高度：主角Y - 想要留出的空隙
        // 如果想紧贴头顶就 -10，想高一点就 -20
        int targetY = this->y() - 15;

        hpBar->move(targetX, targetY);
    }
}

void Hero::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
    if (hpBar) {
        hpBar->setValue(health);
    }
}