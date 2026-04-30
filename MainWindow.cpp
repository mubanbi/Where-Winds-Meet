#include "MainWindow.h"
#include <QMessageBox>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include "Hero.h"
#include "GameMap.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    // ... 在构造函数 MainWindow::MainWindow 里 ...
    // 只有 MainWindow 本身变黑，不影响里面的按钮
    this->setStyleSheet("QMainWindow { background-color: black; }");
// 1. 核心：去掉所有边框和系统装饰（要在 show 之前调用）
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // 2. 核心：强制进入全屏模式（这会压住任务栏）
    this->showFullScreen();
    // 1. 获取当前屏幕的总宽高
    int w = this->width();
    int h = this->height();

    // 2. 使用百分比定位 (需要你运行后微调这几个 0.x 的数值)
    // 比如：背景图的字大约在宽度的 80% 处，高度的 60% 处
    int btnX = w * 0.842; // 往右挪就调大这个 0.82
    int btnY_Start = h * 0.39; // 往下挪就调大这个 0.58
    int btnY_Quit = h * 0.48;

    // 3. 强行安置按钮
    ui.btnStart->setGeometry(btnX, btnY_Start, 150, 45); // 150, 45 是按钮的大小
    ui.btnQuit->setGeometry(btnX, btnY_Quit, 150, 45);
    // 3. 核心：让背景图 Label 实时追踪屏幕宽高
    // 注意：这里的宽高必须在 show 之后获取才是最准的
    ui.bgLabel->setGeometry(0, 0, this->width(), this->height());

    // 确保背景图在所有按钮的最底层
    ui.bgLabel->lower();

    // 之前的连接逻辑记得留着
    connect(ui.btnQuit, &QPushButton::clicked, this, &MainWindow::close);
    // 这一行代码的意思是：当点击 btnQuit 时，执行窗口的关闭动作 (close)
    connect(ui.btnQuit, &QPushButton::clicked, this, &MainWindow::close);

    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &MainWindow::handleMovement);
    moveTimer->start(16); // 每16毫秒更新一次，约等于 60 FPS

}

MainWindow::~MainWindow()
{}

void MainWindow::on_btnStart_clicked()
{
    // 1. 瞬间隐藏按钮和不需要的文字Label（如果有的话）
    ui.btnStart->hide();
    ui.btnQuit->hide();

    // 如果你在 Designer 里加了其他 Label（比如 logo），也在这里 hide()
    // ui.logoLabel->hide();

    // ======================================================
    // 阶段一：让背景慢慢黑下去 (2秒)
    // ======================================================

    // 给现有的 bgLabel 加上透明度效果
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(ui.bgLabel);
    ui.bgLabel->setGraphicsEffect(opacityEffect);

    // 创建动画：控制 opacityEffect 的 "opacity" (透明度) 属性
    QPropertyAnimation* animation = new QPropertyAnimation(opacityEffect, "opacity");
    animation->setDuration(2000);   // 持续时间：2000毫秒 = 2秒
    animation->setStartValue(1.0);  // 开始状态：1.0 完全可见
    animation->setEndValue(0.0);    // 结束状态：0.0 完全透明（直到露出黑色的底色）

    // 【关键】：设置窗口本身的底色为黑色，这样图片变透明时，用户会感觉是在变黑
    this->setStyleSheet("QMainWindow { background-color: black; }");

    // 开始播放
    animation->start(QAbstractAnimation::DeleteWhenStopped); // 播放完自动清理内存

    // ======================================================
    // 阶段二：黑完了之后显示字幕 (核心逻辑)
    // ======================================================

    // 我们使用 connect，监听 animation 的 finished 信号
    // 当动画停止时，执行大括号里的内容
    connect(animation, &QPropertyAnimation::finished, this, [=]() {
        ui.bgLabel->setPixmap(QPixmap());
        ui.bgLabel->setGraphicsEffect(nullptr);

        // 1. 创建主剧情字幕
        QLabel* storyLabel = new QLabel(this);
        storyLabel->setText("你是不羡仙的少东家，今天又是美好的一天，\n早上起床的你准备前往不羡仙。");
        storyLabel->setStyleSheet("color: white; font-family: 'Microsoft YaHei'; font-size: 32px; background: transparent;");
        storyLabel->setAlignment(Qt::AlignCenter);
        storyLabel->setGeometry(0, 0, this->width(), this->height() - 100); // 稍微往上抬一点，给小字留位置
        storyLabel->show();

        // 2. 创建“点击继续”的小字提示
        QLabel* nextLabel = new QLabel(this);
        nextLabel->setText("请点击鼠标左键继续...");
        nextLabel->setStyleSheet("color: rgba(255, 255, 255, 150); font-family: 'Microsoft YaHei'; font-size: 18px; background: transparent;");
        nextLabel->setAlignment(Qt::AlignCenter);
        // 放在屏幕中下方
        nextLabel->setGeometry(0, this->height() - 150, this->width(), 50);
        nextLabel->show();

        // 3. 给小字加上“呼吸灯”效果
        QGraphicsOpacityEffect* nextOpacity = new QGraphicsOpacityEffect(nextLabel);
        nextLabel->setGraphicsEffect(nextOpacity);
        QPropertyAnimation* breathAni = new QPropertyAnimation(nextOpacity, "opacity");
        breathAni->setDuration(1500);
        breathAni->setStartValue(1.0);
        breathAni->setEndValue(0.2);
        breathAni->setLoopCount(-1); // 无限循环
        breathAni->setEasingCurve(QEasingCurve::InOutQuad);
        breathAni->start();

        // 4. 【关键】标记现在进入了“剧情等待点击”状态
        // 我们在类里定义一个变量（比如叫 isWaitingClick）
        this->isWaitingClick = true;
        });
}
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    // 只有在等待点击且按下左键时才处理剧情跳转
    if (this->isWaitingClick && event->button() == Qt::LeftButton) {

        // --- 场景 0：开局剧情点击 (进入竹林) ---
        if (this->sceneStage == 0) {
            this->isWaitingClick = false;

            // 1. 清理屏幕字幕
            QList<QLabel*> labels = this->findChildren<QLabel*>();
            for (QLabel* lbl : labels) {
                if (lbl != ui.bgLabel) lbl->deleteLater();
            }

            // 2. 初始化摄像机
            if (!gameWorld) {
                gameWorld = new QScrollArea(this);
                gameWorld->setGeometry(this->rect());
                gameWorld->setWidgetResizable(false);
                gameWorld->setFrameShape(QFrame::NoFrame);
                gameWorld->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                gameWorld->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                gameWorld->setStyleSheet("background-color: black;");
            }

            // 3. 初始化大地图
            if (!currentMap) {
                currentMap = new GameMap(":/new/prefix1/zhulinjiuju.png", this);
                QPixmap pix(":/new/prefix1/zhulinjiuju.png");
                if (pix.width() < this->width()) {
                    pix = pix.scaledToWidth(this->width(), Qt::SmoothTransformation);
                    currentMap->bgLabel->setPixmap(pix);
                    currentMap->bgLabel->resize(pix.size());
                    currentMap->resize(pix.size());
                }
                gameWorld->setWidget(currentMap);
            }

            // 4. 生成角色和红线
            if (!player) player = new Hero(currentMap);
            player->resize(70, 70);
            player->move(this->width() / 2, currentMap->height() / 2);
            player->show();

            if (!redLine) redLine = new NpcRed(currentMap);
            redLine->resize(60, 60);
            redLine->setPixmap(QPixmap(":/new/prefix1/red_line.jpg"));
            redLine->move(player->x() + 200, player->y());
            redLine->show();

            // 5. 生成漩涡
            if (!vortex) {
                vortex = new QLabel(currentMap);
                vortex->setPixmap(QPixmap(":/new/prefix1/vortex_grey.png"));
                vortex->resize(100, 100);
                vortex->setScaledContents(true);
                vortex->move(460, 350);
                vortex->show();
            }

            // 6. 提示语
            if (!interactTip) {
                interactTip = new QLabel(" 按 E 对话 ", this);
                interactTip->setStyleSheet("color: yellow; background: rgba(0,0,0,150); font-weight: bold; border: 1px solid yellow;");
                interactTip->hide();
            }

            // 7. 图层排序与对焦
            currentMap->bgLabel->lower();
            if (vortex) { vortex->lower(); vortex->raise(); }
            player->raise();
            redLine->raise();

            gameWorld->setGeometry(this->rect());
            gameWorld->show();
            gameWorld->raise();

            updateCamera();
            this->showFullScreen();
            this->setFocus();

            this->sceneStage = 2; // 进入正式游戏
            qDebug() << "✅ 竹林场景加载完毕！";
        }

        // --- 场景 1：不羡仙剧情点击 (正式传送) ---
        // 注意：这个 else if 必须在 isWaitingClick 的大括号里面！
        else if (this->sceneStage == 1) {
            this->isWaitingClick = false;
            this->sceneStage = 2;

            // 1. 安全检查：最重要的防闪退保护
            if (!currentMap || !currentMap->bgLabel || !player) {
                qDebug() << "❌ 致命错误：地图或玩家对象丢失，取消传送以防崩溃";
                return;
            }

            // 2. 清理剧情文字（只删掉刚才生成的剧情 Label，别误杀地图和主角）
            QList<QLabel*> labels = this->findChildren<QLabel*>();
            for (QLabel* lbl : labels) {
                // 只有没有父对象（或者父对象是 MainWindow）且不是核心 UI 的才删
                if (lbl != ui.bgLabel && lbl != interactTip && lbl->parent() == this) {
                    lbl->deleteLater();
                }
            }

            // 3. 换图逻辑
            QPixmap bxxPix(":/new/prefix1/buxianxian.png");
            if (!bxxPix.isNull()) {
                if (bxxPix.width() < this->width()) {
                    bxxPix = bxxPix.scaledToWidth(this->width(), Qt::SmoothTransformation);
                }
                currentMap->bgLabel->setPixmap(bxxPix);
                currentMap->bgLabel->resize(bxxPix.size());
                currentMap->resize(bxxPix.size());
            }

            // 4. 移动主角到右下角
            player->move(currentMap->width() - 200, currentMap->height() - 200);

            // 5. 显示并置顶
            if (gameWorld) {
                gameWorld->show();
                gameWorld->raise();
                ui.bgLabel->hide(); // 隐藏黑屏
            }
            player->raise();

            if (player) {
                player->showHpBar(); // 进图后，亮出血条！
                player->moveBy(0, 0); // 顺便刷一下血条坐标，防止它留在旧场景的 0,0 点
            }

            updateCamera();
            this->setFocus();
        }
        return; // 处理完剧情点击，拦截掉，不往下走
    }

    // 如果不是剧情等待状态，执行父类默认逻辑
    QMainWindow::mousePressEvent(event);
}
void MainWindow::updateCamera()
{
    if (!gameWorld || !player) return;

    // 让滚动条的位置随主角动，使主角保持在屏幕中心
    int scrollX = player->x() + (player->width() / 2) - (this->width() / 2);
    int scrollY = player->y() + (player->height() / 2) - (this->height() / 2);

    gameWorld->horizontalScrollBar()->setValue(scrollX);
    gameWorld->verticalScrollBar()->setValue(scrollY);
}
// 1. 记录按键按下
void MainWindow::keyPressEvent(QKeyEvent* event) {
    pressedKeys.insert(event->key());

    // 统一处理 E 键交互
    if (event->key() == Qt::Key_E) {

        // --- 逻辑 A：与红线对话 (你原来的内容) ---
        if (isNearNpc && !hasTalkedWithRed) {
            QMessageBox msgBox(this);
            msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            msgBox.setWindowTitle("对话");
            msgBox.setText("红线：\n“老大，你可以带我一起去不羡仙吗？”");

            QPushButton* acceptBtn = msgBox.addButton("接受", QMessageBox::AcceptRole);
            QPushButton* rejectBtn = msgBox.addButton("拒绝", QMessageBox::RejectRole);

            msgBox.exec();

            hasTalkedWithRed = true;
            if (interactTip) interactTip->hide();

            if (msgBox.clickedButton() == acceptBtn) {
                QMessageBox::information(this, "提示", "红线开心地跟在了你身后。");
                // 这里以后可以写红线跟随逻辑
            }
            else {
                QMessageBox::information(this, "提示", "红线失望地低下了头。");
            }
            this->setFocus();
        }

        // --- 逻辑 B：前往不羡仙 (新增的传送内容) ---
        // 使用 else if 可以确保如果你同时靠近 NPC 和漩涡时，优先处理对话
        else if (isNearPortal) {
            qDebug() << "准备进入剧情转场...";

            // 1. 开启黑屏遮罩（利用已有的 ui.bgLabel）
            ui.bgLabel->setParent(this); // 确保它在最顶层
            ui.bgLabel->setGeometry(this->rect());
            ui.bgLabel->setPixmap(QPixmap()); // 清空图片，露出黑色底色
            ui.bgLabel->setStyleSheet("background-color: black;");
            ui.bgLabel->show();
            ui.bgLabel->raise();

            // 2. 隐藏之前的游戏世界，营造“全黑”效果
            if (gameWorld) gameWorld->hide();
            if (interactTip) interactTip->hide();

            // 3. 创建不羡仙的转场剧情文字
            QLabel* storyLabel = new QLabel(this);
            storyLabel->setText("当你来到不羡仙，却发现这里已经是一片废墟，\n四周到处是绣金楼的人。");
            storyLabel->setStyleSheet("color: white; font-family: 'Microsoft YaHei'; font-size: 30px; background: transparent;");
            storyLabel->setAlignment(Qt::AlignCenter);
            storyLabel->setGeometry(this->rect());
            storyLabel->show();
            storyLabel->raise();

            // 4. “点击继续”的小字提示
            QLabel* nextLabel = new QLabel("—— 请点击左键进入不羡仙 ——", this);
            nextLabel->setStyleSheet("color: rgba(255, 255, 255, 150); font-size: 18px;");
            nextLabel->setGeometry(0, this->height() - 150, this->width(), 50);
            nextLabel->setAlignment(Qt::AlignCenter);
            nextLabel->show();
            nextLabel->raise();

            // 5. 【关键】进入等待点击状态，并记录我们要去“不羡仙”
            this->isWaitingClick = true;

            // 我们可以复用一个逻辑标记，让 mousePressEvent 知道点击后该干嘛
            // 假设你在类里定义了 int sceneStage = 0; 
            // 这里设为 1 表示：下次点击执行“传送到不羡仙”
            this->sceneStage = 1;

            // 清理旧场景数据（红线和漩涡）
            if (redLine) { redLine->deleteLater(); redLine = nullptr; }
            if (vortex) { vortex->deleteLater(); vortex = nullptr; }
            isNearPortal = false;
        }
    }
}

// 2. 记录按键松开
void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    pressedKeys.remove(event->key());
}

// 3. 核心：真正的移动逻辑
void MainWindow::handleMovement() {
    if (!player || isWaitingClick) return;

    int dx = 0, dy = 0;
    int speed = 2;

    if (pressedKeys.contains(Qt::Key_W)) dy -= speed;
    if (pressedKeys.contains(Qt::Key_S)) dy += speed;
    if (pressedKeys.contains(Qt::Key_A)) dx -= speed;
    if (pressedKeys.contains(Qt::Key_D)) dx += speed;

    if (dx != 0 || dy != 0) {
        // --- 核心修改点：改用 moveBy ---
        // 这样不仅主角会动，他在 Hero.cpp 里绑定的血条也会跟着 moveBy 同步移动
        player->moveBy(dx, dy);

        updateCamera(); // 镜头跟人走
    }

    // --- 后面的提示框逻辑保持不变 ---
    bool showTip = false;
    QString tipText = "";

    if (player && redLine && !hasTalkedWithRed) {
        int dist = (player->pos() - redLine->pos()).manhattanLength();
        if (dist < 150) {
            isNearNpc = true;
            tipText = " 按 E 对话 ";
            showTip = true;
        }
        else {
            isNearNpc = false;
        }
    }

    if (player && vortex) {
        int distToPortal = (player->geometry().center() - vortex->geometry().center()).manhattanLength();
        if (distToPortal < 80) {
            isNearPortal = true;
            tipText = " 按 E 前往不羡仙 ";
            showTip = true;
        }
        else {
            isNearPortal = false;
        }
    }

    if (showTip) {
        interactTip->setText(tipText);
        interactTip->move(player->x() - gameWorld->horizontalScrollBar()->value(),
            player->y() - 50 - gameWorld->verticalScrollBar()->value());
        interactTip->show();
        interactTip->raise();
    }
    else {
        interactTip->hide();
    }
}