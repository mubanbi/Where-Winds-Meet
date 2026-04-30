#pragma once

#include <QMainWindow>
#include <QTimer>      // 必须有：处理平滑移动
#include <QSet>       // 必须有：记录按键
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include "ui_MainWindow.h"
#include "Hero.h"
#include "NpcRed.h"
#include "GameMap.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    // 必须在这里声明这三个事件函数，否则 .cpp 里的代码不会被执行
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void on_btnStart_clicked();

private:
    Ui::MainWindowClass ui;

    // 户口登记：确保这些变量都在这里
    bool isWaitingClick = false;
    QScrollArea* gameWorld = nullptr;
    QLabel* mapLabel = nullptr;
    Hero* player = nullptr;

    QTimer* moveTimer = nullptr;    // 移动定时器
    QSet<int> pressedKeys;         // 按键容器
    void updateCamera();           // 摄像机函数
    void handleMovement();         // 移动处理函数

    GameMap* currentMap = nullptr; // 新增这一行

    NpcRed* redLine = nullptr;
    QLabel* interactTip = nullptr; // “按E对话”的提示
    bool isNearNpc = false;        // 是否靠近了

    bool hasTalkedWithRed = false; // 初始值为 false，表示还没聊过

    QLabel* vortex = nullptr;       // 传送光圈
    bool isNearPortal = false;      // 是否靠近传送阵

    int sceneStage = 0;

};