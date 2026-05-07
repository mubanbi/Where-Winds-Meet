#include "MainWindow.h"
#include <QMessageBox>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include "Hero.h"
#include "GameMap.h"
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	this->currentHealth = 100;
	this->maxHealth = 100;

	// --- 重点修改：加载并显示背景图 ---
	QPixmap startPix(":/new/prefix1/Image_1777387822081_947.png"); // 这里确认为你开始界面的资源路径
	if (startPix.isNull()) {
		// 如果路径不对，这里会在输出窗口提醒你
		qDebug() << "错误：开始界面背景图加载失败，请检查资源路径！";
	}
	else {
		ui.bgLabel->setPixmap(startPix);
		ui.bgLabel->setScaledContents(true); // 开启图片自适应缩放
	}
	// --------------------------------

	this->setStyleSheet("QMainWindow { background-color: black; }");
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	this->showFullScreen();



	int w = this->width();
	int h = this->height();

	int btnX = w * 0.842;
	int btnY_Start = h * 0.39;
	int btnY_Quit = h * 0.48;

	ui.btnStart->setGeometry(btnX, btnY_Start, 150, 45);
	ui.btnQuit->setGeometry(btnX, btnY_Quit, 150, 45);

	// 再次确保 bgLabel 铺满全屏并置底
	ui.bgLabel->setGeometry(0, 0, w, h);
	ui.bgLabel->lower();

	// ... 后面的 connect 和 timer 逻辑保持不变 ...
	connect(ui.btnQuit, &QPushButton::clicked, this, &MainWindow::close);

	moveTimer = new QTimer(this);
	connect(moveTimer, &QTimer::timeout, this, &MainWindow::handleMovement);
	moveTimer->start(16);
}

MainWindow::~MainWindow()
{
}

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
	animation->setDuration(2000);// 持续时间：2000毫秒 = 2秒
	animation->setStartValue(1.0);// 开始状态：1.0 完全可见
	animation->setEndValue(0.0);// 结束状态：0.0 完全透明（直到露出黑色的底色）

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

		// --- 场景 0：开局剧情点击 (从黑屏字幕进入竹林) ---
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

			// 4. 生成角色和红线 NPC
			if (!player) player = new Hero(currentMap);
			player->resize(70, 70);
			player->move(this->width() / 2, currentMap->height() / 2);
			player->show();

			if (!redLine) redLine = new NpcRed(currentMap);
			redLine->resize(60, 60);
			redLine->setPixmap(QPixmap(":/new/prefix1/red_line.jpg"));
			redLine->move(player->x() + 200, player->y());
			redLine->show();

			// 5. 初始化血量标签
			if (!healthLabel) {
				healthLabel = new QLabel(this);
				healthLabel->setStyleSheet(
					"color: #FF3333; font-family: 'Microsoft YaHei'; font-size: 24px; "
					"font-weight: bold; background: rgba(0, 0, 0, 80); "
					"padding: 5px 15px; border-radius: 10px;"
				);
				healthLabel->show();
			}

			// 6. 生成道具
			if (!pickItem) pickItem = new Item(currentMap);
			QPixmap itemPix(":/new/prefix1/hongxian.jpg");
			if (!itemPix.isNull()) pickItem->setPixmap(itemPix);
			pickItem->move(700, 900);
			pickItem->show();
			pickItem->raise();

			// 7. 生成漩涡 (传送阵)
			if (!vortex) {
				vortex = new QLabel(currentMap);
				vortex->setPixmap(QPixmap(":/new/prefix1/vortex_grey.png"));
				vortex->resize(100, 100);
				vortex->setScaledContents(true);
				vortex->move(460, 350);
				vortex->show();
			}

			// 8. 交互提示语
			if (!interactTip) {
				interactTip = new QLabel(" 按 E 交互 ", this);
				interactTip->setStyleSheet("color: yellow; background: rgba(0,0,0,150); font-weight: bold; border: 1px solid yellow; padding: 5px;");
				interactTip->hide();
			}

			// 9. 层级与显示
			currentMap->bgLabel->lower();
			player->raise();
			gameWorld->show();
			gameWorld->raise();

			updateCamera();
			this->sceneStage = 2;
		}

		// --- 场景 1：不羡仙剧情点击 (从废墟字幕正式进入地图) ---
		else if (this->sceneStage == 1) {
			this->isWaitingClick = false;
			this->sceneStage = 2; // 标记为正式游戏阶段

			// 1. 安全检查
			if (!currentMap || !player) return;

			// 2. 清理未拾取的道具和旧剧情标签
			if (pickItem) pickItem->hide();
			isNearItem = false;
			if (interactTip) interactTip->hide();

			QList<QLabel*> labels = this->findChildren<QLabel*>();
			for (QLabel* lbl : labels) {
				if (lbl == healthLabel || lbl == interactTip || lbl == ui.bgLabel) continue;
				if (lbl->parent() == this) lbl->deleteLater();
			}

			// 3. 切换到“不羡仙”地图图片
			QPixmap bxxPix(":/new/prefix1/buxianxian.png");
			if (!bxxPix.isNull()) {
				if (bxxPix.width() < this->width()) {
					bxxPix = bxxPix.scaledToWidth(this->width(), Qt::SmoothTransformation);
				}
				currentMap->bgLabel->setPixmap(bxxPix);
				currentMap->bgLabel->resize(bxxPix.size());
				currentMap->resize(bxxPix.size());
			}

			// 4. 移动主角并显示
			player->move(currentMap->width() - 200, currentMap->height() - 200);
			if (gameWorld) {
				gameWorld->show();
				gameWorld->raise();
				ui.bgLabel->hide();
			}
			player->raise();

			// --- 新增：初始化主角血条 ---
	
			if (!playerHpBar) {
				playerHpBar = new QProgressBar(this);
				playerHpBar->setRange(0, 100);
				playerHpBar->setValue(this->currentHealth);
				playerHpBar->setGeometry(20, 60, 250, 20);
				playerHpBar->setTextVisible(false);

		
				playerHpBar->setStyleSheet(
					"QProgressBar {"
					"   border: 2px solid #555;"
					"   background-color: #222;"
					"   border-radius: 5px;"
					"}"
					"QProgressBar::chunk {"
					"   background-color: #FF3333;" // 确保是纯正的红色
					"   border-radius: 2px;"
					"}"
				);
				playerHpBar->show();
				playerHpBar->raise();
			}
			// 5. 【核心实现】：进入 5 秒后显示战斗提示
			QTimer::singleShot(5000, this, [=]() {
				QLabel* combatTip = new QLabel(this);
				combatTip->setText("—— 通过点击消灭绣金楼喽啰 ——");
				combatTip->setStyleSheet(
					"color: #FFD700; font-family: 'Microsoft YaHei'; font-size: 36px; "
					"font-weight: bold; background: rgba(0, 0, 0, 180); "
					"padding: 20px; border: 2px solid #FFD700; border-radius: 15px;"
				);
				combatTip->setAlignment(Qt::AlignCenter);
				combatTip->setGeometry(0, 0, this->width(), 120);
				combatTip->move(0, this->height() / 2 - 150);
				combatTip->show();
				combatTip->raise();

				// 3秒后文字自动消失
				QTimer::singleShot(3000, combatTip, &QLabel::deleteLater);

				combatTip->show();
				combatTip->raise();
				QTimer::singleShot(3000, combatTip, &QLabel::deleteLater);

				// 2. 【核心新增】：开启刷怪定时器
				spawnTimer = new QTimer(this);
				connect(spawnTimer, &QTimer::timeout, this, [=]() {
					if (this->sceneStage != 2) return; // 如果不在战斗阶段就停止

					// 创建喽啰
					QLabel* minion = new QLabel(this);
					minion->setPixmap(QPixmap(":/new/prefix1/louluo.jpg").scaled(60, 60)); // 确保你有喽啰图片
					minion->resize(60, 60);

					// 随机从左侧或上方刷新
					int startPos = QRandomGenerator::global()->bounded(2); // 0 或 1
					if (startPos == 0) { // 左边出来
						minion->move(-60, QRandomGenerator::global()->bounded(this->height()));
					}
					else { // 上边出来
						minion->move(QRandomGenerator::global()->bounded(this->width()), -60);
					}

					minion->installEventFilter(this);

					minion->show();
					minion->raise();
					this->minions.append(minion);
					});
				spawnTimer->start(700); // 每 0.7 秒刷一个怪
				});

			updateCamera();
			this->setFocus();
		}
		return; // 拦截剧情点击，不触发后续逻辑
	}

	// 非剧情状态，执行默认点击逻辑
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
	// --- 新增：ESC 退出确认逻辑 ---
	if (event->key() == Qt::Key_Escape) {
		QMessageBox msgBox(this);
		msgBox.setWindowTitle("离开江湖");
		msgBox.setText("确定要退出游戏，离开这段江湖旅程吗？");
		// 设置对话框风格
		msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

		QPushButton* yesBtn = msgBox.addButton("退出", QMessageBox::YesRole);
		QPushButton* noBtn = msgBox.addButton("留下", QMessageBox::NoRole);
		msgBox.setDefaultButton(noBtn);

		msgBox.exec();

		if (msgBox.clickedButton() == yesBtn) {
			this->close();
		}
		this->setFocus(); // 确保弹窗关闭后，焦点回到主窗口以便继续移动
		return;
	}

	// 记录其他按键按下状态 (W/A/S/D 等)
	pressedKeys.insert(event->key());

	// 统一处理 E 键交互
	if (event->key() == Qt::Key_E) {
		// --- 逻辑 1：拾取道具 ---
		if (isNearItem && pickItem && pickItem->isVisible()) {
			QMessageBox::information(this, "获得前尘旧物", "你捡到了：一截红线\n【气血大增：HP +50】");
			this->maxHealth = 150;
			this->currentHealth = 150;
			updatePlayerUI();
			pickItem->hide();
			isNearItem = false;
			if (interactTip) interactTip->hide();
			this->setFocus();
			return; // 拾取完直接返回
		}
		// --- 逻辑 2：与红线 NPC 对话 ---
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
				QMessageBox::information(this, "提示", "红线开心地先去往了不羡仙。");
				this->isRedJoined = true; // 结局 1 的触发条件
				if (redLine) { redLine->deleteLater(); redLine = nullptr; }
			}
			else {
				QMessageBox::information(this, "提示", "红线失望地低下了头。");
				this->isRedJoined = false; // 结局 2 的触发条件
			}
			this->setFocus();
			return;
		}

		// --- 逻辑 3：前往不羡仙 (传送逻辑) ---
		if (isNearPortal) {
			qDebug() << "准备进入剧情转场...";

			// 1. 开启黑屏遮罩
			ui.bgLabel->setParent(this);
			ui.bgLabel->setGeometry(this->rect());
			ui.bgLabel->setPixmap(QPixmap());
			ui.bgLabel->setStyleSheet("background-color: black;");
			ui.bgLabel->show();
			ui.bgLabel->raise();

			// 2. 隐藏游戏世界
			if (gameWorld) gameWorld->hide();
			if (interactTip) interactTip->hide();

			// 3. 创建转场文字
			QLabel* storyLabel = new QLabel(this);
			storyLabel->setText("当你来到不羡仙，却发现这里已经是一片废墟，\n四周到处是绣金楼的人。");
			storyLabel->setStyleSheet("color: white; font-family: 'Microsoft YaHei'; font-size: 30px; background: transparent;");
			storyLabel->setAlignment(Qt::AlignCenter);
			storyLabel->setGeometry(this->rect());
			storyLabel->show();
			storyLabel->raise();

			// 4. 点击提示
			QLabel* nextLabel = new QLabel("—— 请点击左键进入不羡仙 ——", this);
			nextLabel->setStyleSheet("color: rgba(255, 255, 255, 150); font-size: 18px;");
			nextLabel->setGeometry(0, this->height() - 150, this->width(), 50);
			nextLabel->setAlignment(Qt::AlignCenter);
			nextLabel->show();
			nextLabel->raise();

			// 5. 设置状态标记
			this->isWaitingClick = true;
			this->sceneStage = 1;

			// 清理旧场景对象
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

	// 1. 处理主角移动 (仅在原有基础上增加边界限制)
	int dx = 0, dy = 0;
	int speed = 2;
	if (pressedKeys.contains(Qt::Key_W)) dy -= speed;
	if (pressedKeys.contains(Qt::Key_S)) dy += speed;
	if (pressedKeys.contains(Qt::Key_A)) dx -= speed;
	if (pressedKeys.contains(Qt::Key_D)) dx += speed;

	if (dx != 0 || dy != 0) {
		player->moveBy(dx, dy);

		// --- 仅在这里增加四边边界强制修正 ---
		int curX = player->x();
		int curY = player->y();
		bool needFix = false;

		// 左边界
		if (curX < 0) { curX = 0; needFix = true; }
		// 右边界 (基于大地图 currentMap 的宽度)
		if (currentMap && curX > currentMap->width() - player->width()) {
			curX = currentMap->width() - player->width();
			needFix = true;
		}
		// 上边界
		if (curY < 0) { curY = 0; needFix = true; }
		// 下边界 (基于大地图 currentMap 的高度)
		if (currentMap && curY > currentMap->height() - player->height()) {
			curY = currentMap->height() - player->height();
			needFix = true;
		}

		if (needFix) {
			player->move(curX, curY); // 如果越界了，强行拉回来
		}

		updateCamera();
	}

	// 2. 【独立出来】处理喽啰移动与攻击 (优化了移速与震颤)
	for (int i = 0; i < minions.size(); ++i) {
		QLabel* m = minions[i];
		if (!m) continue;

		int speed = 3; // 你设置的移速
		QPoint targetPos = player->pos() - QPoint(gameWorld->horizontalScrollBar()->value(), gameWorld->verticalScrollBar()->value());
		QPoint currentPos = m->pos();

		// 计算 X 和 Y 方向的距离
		int diffX = targetPos.x() - currentPos.x();
		int diffY = targetPos.y() - currentPos.y();

		int moveX = 0;
		int moveY = 0;

		// --- 防震颤逻辑：如果距离小于速度，直接移动到目标点，不再往返冲刺 ---
		if (qAbs(diffX) > speed) {
			moveX = (diffX > 0) ? speed : -speed;
		}
		else {
			moveX = diffX;
		}

		if (qAbs(diffY) > speed) {
			moveY = (diffY > 0) ? speed : -speed;
		}
		else {
			moveY = diffY;
		}

		m->move(currentPos.x() + moveX, currentPos.y() + moveY);

		// --- 碰撞伤害检测 ---
		QRect playerRect = player->geometry();
		// 将玩家的世界坐标映射到当前视口坐标，以便与喽啰的 geometry(视口坐标) 比较
		playerRect.moveTopLeft(player->pos() - QPoint(gameWorld->horizontalScrollBar()->value(), gameWorld->verticalScrollBar()->value()));

		if (playerRect.intersects(m->geometry())) {
			this->currentHealth -= 10; // 扣血

			// 【关键修复】调用更新函数，这样 playerHpBar 才会刷新
			updatePlayerUI();

			m->deleteLater();
			minions.removeAt(i);
			--i;
			continue;
		}
	}

	// 3. 【独立出来】处理所有的交互提示 (对话、拾取、传送)
	// --- 这里的逻辑必须在 for 循环外面！ ---
	bool showTip = false;
	QString tipText = "";

	// 对话检测
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

	// 传送阵检测
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

	// 拾取道具检测
	if (player && pickItem && pickItem->isVisible()) {
		int dist = (player->geometry().center() - pickItem->geometry().center()).manhattanLength();
		if (dist < 80) {
			isNearItem = true;
			tipText = " 按 E 拾取 一截红线 ";
			showTip = true;
		}
		else {
			isNearItem = false;
		}
	}

	// 4. 统一更新交互提示框
	if (showTip) {
		interactTip->setText(tipText);
		interactTip->adjustSize();
		int tipX = player->x() + (player->width() / 2) - (interactTip->width() / 2) - gameWorld->horizontalScrollBar()->value();
		int tipY = player->y() - 60 - gameWorld->verticalScrollBar()->value();
		interactTip->move(tipX, tipY);
		interactTip->show();
		interactTip->raise();
	}
	else {
		if (interactTip) interactTip->hide();
	}

	// 5. 统一更新血量 UI
	if (healthLabel) {
		if (this->currentHealth <= 0) this->currentHealth = 0; // 死亡逻辑可以在这里扩展
		healthLabel->setText(QString("HP: %1 / %2").arg(this->currentHealth).arg(this->maxHealth));
		healthLabel->adjustSize();
		healthLabel->move(20, 20);
		healthLabel->raise();
	}
	if (healthLabel || playerHpBar) {
		updatePlayerUI(); // 统一交给这个函数处理，不要在这里单独写 setText
	}
}
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		// A. 检查点击小怪
		for (int i = 0; i < minions.size(); ++i) {
			if (watched == minions[i]) {
				minions[i]->deleteLater();
				minions.removeAt(i);
				this->killCount++;
				if (killCount == 30) {
					spawnBoss();
				}
				return true;
			}
		}

		// B. 检查点击 Boss
		if (boss && watched == boss) {
			currentBossHealth -= 10;
			if (bossHealthBar) {
				bossHealthBar->setValue(currentBossHealth);
			}
			qDebug() << "💥 命中千夜！剩余血量：" << currentBossHealth;

			if (currentBossHealth <= 0) {
				qDebug() << "🏆 最终胜利！";

				// --- 1. 停止所有逻辑循环 ---
				if (spawnTimer) spawnTimer->stop();
				// 如果你加了方案一的技能定时器，也在这里停止
				// if (bossSkillTimer) bossSkillTimer->stop(); 

				// --- 2. 清理战场怪物 ---
				for (QLabel* m : minions) m->deleteLater();
				minions.clear();

				// --- 3. 销毁 Boss 实体 ---
				boss->hide();
				boss->deleteLater();
				boss = nullptr;
				if (bossHealthBar) {
					bossHealthBar->hide();
					bossHealthBar->deleteLater();
					bossHealthBar = nullptr;
				}

				// --- 4. 触发最终剧情 ---
				showFinalEnding();
			}
			return true;
		}
	}
	return QMainWindow::eventFilter(watched, event);
}
void MainWindow::spawnBoss()
{
	// 1. 安全锁：防止重复创建
	if (this->boss != nullptr) return;

	// 2. 关键：先初始化数值，再让逻辑运行
	// 确保这两个变量已经在 MainWindow.h 中声明为 int
	this->bossHealth = 1000;
	this->currentBossHealth = 1000;

	// 3. 创建 Boss 实体
	boss = new QLabel(currentMap);
	QPixmap bossPix(":/new/prefix1/qianye.jpg");
	if (!bossPix.isNull()) {
		boss->setPixmap(bossPix.scaled(200, 200, Qt::KeepAspectRatio));
	}
	boss->resize(200, 200);
	boss->move(400, 400);
	boss->installEventFilter(this);
	boss->show();

	// 4. 创建血条（必须先创建对象，再设置样式）
	bossHealthBar = new QProgressBar(this);
	bossHealthBar->setRange(0, this->bossHealth);
	bossHealthBar->setValue(this->currentBossHealth);

	// 5. 设置样式（请直接复制这段，确保没有中文字符）
	bossHealthBar->setStyleSheet(
		"QProgressBar {"
		"   border: 2px solid #555;"
		"   border-radius: 5px;"
		"   background-color: #222;"
		"   text-align: center;"
		"   color: white;"
		"   font-weight: bold;"
		"}"
		"QProgressBar::chunk {"
		"   background-color: #ff0000;"
		"   border: none;"
		"}"
	);

	// 6. 布局
	int barWidth = 600;
	bossHealthBar->setGeometry((this->width() - barWidth) / 2, 50, barWidth, 35);
	bossHealthBar->setFormat("千夜 - %v/%m");
	bossHealthBar->show();
	bossHealthBar->raise();

	// 7. 文字提示
	QLabel* notice = new QLabel("—— 千夜 出现 ——", this);
	notice->setStyleSheet("color: red; font-size: 60px; font-weight: bold; background: transparent;");
	notice->setGeometry(0, this->height() / 2 - 50, this->width(), 100);
	notice->setAlignment(Qt::AlignCenter);
	notice->show();
	notice->raise();
	QTimer::singleShot(2000, notice, &QLabel::deleteLater);

	//8.技能
	bossSkillTimer = new QTimer(this);
	connect(bossSkillTimer, &QTimer::timeout, this, &MainWindow::triggerBossSkill);
	bossSkillTimer->start(3000); // 每 3 秒放一次技能

	//9.移动
	QTimer* bossMoveTimer = new QTimer(this);
	connect(bossMoveTimer, &QTimer::timeout, this, [=]() {
		if (!boss || !currentMap) return;

		// --- 核心：限制在左上角 1/4 区域 ---
		// 允许移动的最大宽度 = 屏幕总宽的一半 - Boss 自身的宽度
		int limitW = (this->width() / 2) - boss->width();
		// 允许移动的最大高度 = 屏幕总高的一半 - Boss 自身的高度
		int limitH = (this->height() / 2) - boss->height();

		// 在这个范围内生成随机坐标
		int newX = QRandomGenerator::global()->bounded(qMax(1, limitW));
		int newY = QRandomGenerator::global()->bounded(qMax(1, limitH));

		// 使用平滑动画移动到新位置
		QPropertyAnimation* ani = new QPropertyAnimation(boss, "pos");
		ani->setDuration(800); // 0.8秒移动过程
		ani->setStartValue(boss->pos());
		ani->setEndValue(QPoint(newX, newY));
		ani->setEasingCurve(QEasingCurve::InOutQuad); // 平滑起步和停止
		ani->start(QAbstractAnimation::DeleteWhenStopped);
		});

	bossMoveTimer->start(2500); // 每 2.5 秒变换一次位置
}


void MainWindow::triggerBossSkill() {
	if (!boss || !currentMap) return;

	// 每次释放 10 个落雷
	for (int i = 0; i < 10; ++i) {
		QLabel* thunder = new QLabel(currentMap);
		thunder->resize(120, 120);
		// 随机在主角周围或全图生成 (这里选全图随机)
		int rx = QRandomGenerator::global()->bounded(currentMap->width() - 120);
		int ry = QRandomGenerator::global()->bounded(currentMap->height() - 120);
		thunder->move(rx, ry);

		// 第一阶段：预警 (半透明橙色)
		thunder->setStyleSheet("background-color: rgba(255, 165, 0, 100); border-radius: 60px;");
		thunder->show();

		// 第二阶段：1秒后爆发伤害
		QTimer::singleShot(1000, this, [=]() {
			if (!thunder) return;
			// 变色
			thunder->setStyleSheet("background-color: rgba(255, 0, 0, 180); border-radius: 60px;");

			// 伤害检测
			// 注意：要对比的是在 currentMap 里的相对位置
			QRect thunderRect = thunder->geometry();
			if (thunderRect.intersects(player->geometry())) {
				this->currentHealth -= 20; // 被雷劈中扣20血
				updatePlayerUI();
				qDebug() << "⚡ 你被千夜的落雷击中了！";
			}

			// 第三阶段：消失
			QTimer::singleShot(300, thunder, &QLabel::deleteLater);
			});
	}
}



void MainWindow::updatePlayerUI() {
	// 1. 基础安全检查
	if (this->currentHealth < 0) this->currentHealth = 0;
	if (this->currentHealth > this->maxHealth) this->currentHealth = this->maxHealth;

	// 2. --- 核心修改：无伤成就判定 ---
	// 注意：这里只判断逻辑血量，不看进度条显示了多少
	// 只有在逻辑血量真的低于上限时，才取消成就
	static int lastHealth = this->maxHealth;
	if (this->currentHealth < this->maxHealth) {
		this->isPerfectClear = false;
	}

	if (this->playerHpBar) {
		this->playerHpBar->setRange(0, this->maxHealth);

		// 3. --- 视觉显示黑科技 (只针对进度条) ---
		if (this->currentHealth >= this->maxHealth) {
			// 进度条显示 149 来保住颜色，但 currentHealth 变量依然是 150
			this->playerHpBar->setValue(this->maxHealth - 1);
		}
		else {
			this->playerHpBar->setValue(this->currentHealth);
		}

		// 强制刷新样式，确保颜色在 149 时显示
		this->playerHpBar->setStyleSheet(
			"QProgressBar { border: 2px solid #555; background: #222; border-radius: 0px; }"
			"QProgressBar::chunk { background-color: #FF3333; }"
		);
	}

	// 4. 文字标签依然显示真实的 150 / 150，玩家看不出破绽
	if (this->healthLabel) {
		healthLabel->setText(QString("HP: %1 / %2").arg(this->currentHealth).arg(this->maxHealth));
		healthLabel->adjustSize();
	}

	if (this->currentHealth <= 0) handlePlayerDeath();
}
void MainWindow::handlePlayerDeath()
{
	// 1. 停止所有游戏定时器（防止角色死后还在移动或生成怪）
	if (spawnTimer) spawnTimer->stop();
	// 假设你的移动定时器叫 timer 或 movementTimer
	// if (timer) timer->stop(); 

	// 2. 创建黑屏背景
	QLabel* blackScreen = new QLabel(this);
	blackScreen->setGeometry(0, 0, this->width(), this->height());
	blackScreen->setStyleSheet("background-color: black;");
	blackScreen->show();
	blackScreen->raise(); // 确保遮盖住所有游戏元素

	// 3. 创建剧情文字
	QLabel* endingText = new QLabel(blackScreen); // 父对象设为黑屏，随黑屏一起显示
	endingText->setText("你身负重伤，仓皇逃离不羡仙，\n从此你隐姓埋名，只待有朝一日复仇雪恨");

	// 设置文字样式：金色或白色文字，居中显示
	endingText->setStyleSheet(
		"color: #D4AF37; " // 金色
		"font-size: 30px; "
		"font-family: 'Microsoft YaHei'; "
		"font-weight: bold; "
		"background: transparent;"
	);

	endingText->setAlignment(Qt::AlignCenter);
	endingText->setGeometry(0, 0, this->width(), this->height()); // 全屏居中
	endingText->show();

	// 4. (可选) 提供一个返回主菜单或退出游戏的按钮
	QPushButton* btnQuit = new QPushButton("归隐山林", blackScreen);
	btnQuit->setGeometry((this->width() - 150) / 2, this->height() * 0.8, 150, 40);
	btnQuit->setStyleSheet("QPushButton{ color: white; border: 1px solid white; padding: 5px; }");
	btnQuit->show();

	connect(btnQuit, &QPushButton::clicked, this, &MainWindow::close);
}

void MainWindow::showFinalEnding()
{
	// 1. 黑色幕布背景 (代码保持不变)
	QLabel* finalScreen = new QLabel(this);
	finalScreen->setGeometry(0, 0, this->width(), this->height());
	finalScreen->setStyleSheet("background-color: black;");
	finalScreen->show();
	finalScreen->raise();

	// 2. 结局文本判定 (双结局逻辑)
	QString endingText = this->isRedJoined ?
		"千夜带着绣金楼的人离开了，\n\n但是不羡仙已经被烧毁，寒姨与红线皆不知所踪...就此，你踏上了寻找她们的道路。" :
		"千夜带着绣金楼的人离开了，\n\n但是不羡仙已经被烧毁，寒姨也不知所踪...就此，你踏上了寻找寒姨的道路。";

	// 3. 显示主剧情文字 (代码保持不变)
	QLabel* finalLabel = new QLabel(finalScreen);
	finalLabel->setText(endingText);
	finalLabel->setStyleSheet("color: #D4AF37; font-family: 'Microsoft YaHei'; font-size: 30px; font-weight: bold;");
	finalLabel->setAlignment(Qt::AlignCenter);
	finalLabel->setGeometry(0, 0, this->width(), this->height() - 100);
	finalLabel->show();

	// 4. --- 核心修改：成就展示区 ---
	int achievementY = 50;

	// 成就 A：150血量通关 (达成条件：拿了红线且最后满血)
	if (this->currentHealth == 150) {
		QLabel* achElite = new QLabel("【 隐藏成就：气血充盈（150血通关） 】", finalScreen);
		achElite->setStyleSheet(
			"color: #FF4500; font-size: 24px; font-weight: bold; " // 橙红色，代表气血
			"background: rgba(255, 69, 0, 30); border: 1px solid #FF4500; padding: 10px;"
		);
		achElite->adjustSize();
		achElite->move(50, achievementY);
		achElite->show();

		// 滑入动画
		QPropertyAnimation* ani1 = new QPropertyAnimation(achElite, "pos");
		ani1->setDuration(1000);
		ani1->setStartValue(QPoint(-400, achievementY));
		ani1->setEndValue(QPoint(50, achievementY));
		ani1->setEasingCurve(QEasingCurve::OutBack);
		ani1->start(QAbstractAnimation::DeleteWhenStopped);

		achievementY += 80;
	}

	// 成就 B：拯救红线 (达成条件：没带走红线，让她留在竹林)
	if (!this->isRedJoined) {
		QLabel* achRed = new QLabel("【 隐藏成就：拯救红线 】", finalScreen);
		achRed->setStyleSheet(
			"color: #00FF7F; font-size: 24px; font-weight: bold; "
			"background: rgba(0, 255, 127, 30); border: 1px solid #00FF7F; padding: 10px;"
		);
		achRed->adjustSize();
		achRed->move(50, achievementY);
		achRed->show();

		QPropertyAnimation* ani2 = new QPropertyAnimation(achRed, "pos");
		ani2->setDuration(1200);
		ani2->setStartValue(QPoint(-400, achievementY));
		ani2->setEndValue(QPoint(50, achievementY));
		ani2->setEasingCurve(QEasingCurve::OutBack);
		ani2->start(QAbstractAnimation::DeleteWhenStopped);
	}
	// 6. 退出按钮
	QPushButton* btnEnd = new QPushButton("江湖再见", finalScreen);
	btnEnd->setGeometry((this->width() - 200) / 2, this->height() * 0.85, 200, 50);
	btnEnd->setStyleSheet("color: white; border: 1px solid #D4AF37; font-size: 20px;");
	btnEnd->show();
	connect(btnEnd, &QPushButton::clicked, this, &MainWindow::close);
}