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
#include "Item.h"

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

protected:
    // 声明事件过滤器，用来处理点击喽啰
    bool eventFilter(QObject* watched, QEvent* event) override;

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

    Item* pickItem = nullptr;      // 道具指针，记得初始化为 nullptr
    bool isNearItem = false;       // 是否靠近道具的标记

    QLabel* healthLabel = nullptr; // 血量数字显示
    int currentHealth = 100;      // 假设初始血量是100
    int maxHealth = 100;

    QList<QLabel*> minions;

private:
    int killCount = 0;        // 杀敌计数器
    QLabel* boss = nullptr;   // BOSS 指针
    void spawnBoss();

    // --- 新增 Boss 相关变量 ---
    int bossHealth = 1000;          // 总血量
    int currentBossHealth = 1000;   // 当前血量
    QProgressBar* bossHealthBar = nullptr; // 血条控件
};