#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt MainWindow 影像處理與對比系統");
    resize(1000, 600);

    setupUI();
    setupActions();

    statusBar()->showMessage("就緒：請先透過 File -> Open 或工具列載入圖片");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 建立中央主容器
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // 左側：未處理原圖區
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QLabel *leftTitle = new QLabel("【未處理原圖 (Original)】", this);
    leftTitle->setAlignment(Qt::AlignCenter);
    leftTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");

    originalLabel = new QLabel("尚未載入圖片", this);
    originalLabel->setAlignment(Qt::AlignCenter);
    originalLabel->setMinimumSize(450, 450);
    originalLabel->setStyleSheet("border: 2px dashed #888; background-color: #f4f4f4; color: #555;");

    leftLayout->addWidget(leftTitle);
    leftLayout->addWidget(originalLabel, 1);

    // 右側：處理後結果區
    QVBoxLayout *rightLayout = new QVBoxLayout();
    QLabel *rightTitle = new QLabel("【處理後影像 (Processed)】", this);
    rightTitle->setAlignment(Qt::AlignCenter);
    rightTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");

    processedLabel = new QLabel("等待執行影像處理", this);
    processedLabel->setAlignment(Qt::AlignCenter);
    processedLabel->setMinimumSize(450, 450);
    processedLabel->setStyleSheet("border: 2px dashed #888; background-color: #f4f4f4; color: #555;");

    rightLayout->addWidget(rightTitle);
    rightLayout->addWidget(processedLabel, 1);

    // 將左右兩側加入水平排版
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
}

void MainWindow::setupActions()
{
    // 1. 建立共用動作 (QAction)
    openAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open Image (&O)", this);
    openAction->setShortcut(QKeySequence::Open); // Ctrl+O
    openAction->setStatusTip("從本機選擇並載入影像檔案");

    processAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), "執行灰階處理 (&R)", this);
    processAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R)); // Ctrl+R
    processAction->setStatusTip("將左側原圖轉換為灰階並顯示於右側");
    processAction->setEnabled(false); // 【防呆】未載入圖片前停用

    saveAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save (&S)", this);
    saveAction->setShortcut(QKeySequence::Save); // 綁定 Ctrl+S 快捷鍵
    saveAction->setStatusTip("將右側處理完成的影像匯出存檔 (Ctrl+S)");
    saveAction->setEnabled(false); // 【防呆】未完成處理前停用

    exitAction = new QAction(style()->standardIcon(QStyle::SP_DialogCloseButton), "Exit (&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);

    // 2. 掛載到上方選單列 (File Menu)
    QMenu *fileMenu = menuBar()->addMenu("File (&F)");
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QMenu *processMenu = menuBar()->addMenu("Process (&P)");
    processMenu->addAction(processAction);

    // 3. 掛載同一個 openAction 到工具列 (ToolBar)，達成 Menu 與 ToolBar 共用
    QToolBar *mainToolBar = addToolBar("Main ToolBar");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(processAction);
    mainToolBar->addAction(saveAction);

    // 4. 連接 Signal & Slot
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(processAction, &QAction::triggered, this, &MainWindow::processGrayscale);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "選擇測試圖片",
        "",
        "影像檔案 (*.png *.jpg *.jpeg *.bmp);;所有檔案 (*.*)"
        );

    if (fileName.isEmpty()) {
        statusBar()->showMessage("已取消開啟檔案");
        return;
    }

    QImage loadedImage(fileName);
    if (loadedImage.isNull()) {
        QMessageBox::warning(this, "錯誤", "無法載入該圖片檔案，請確認格式是否正確！");
        return;
    }

    originalImage = loadedImage;
    processedImage = QImage();
    processedLabel->clear();
    processedLabel->setText("已載入原圖，請點擊「執行灰階處理」");

    updateImageDisplay();

    // 解鎖「執行處理」，「儲存」維持鎖定直到處理完成
    processAction->setEnabled(true);
    saveAction->setEnabled(false);

    statusBar()->showMessage(QString("成功載入圖片：%1 (%2x%3)")
                                 .arg(fileName)
                                 .arg(originalImage.width())
                                 .arg(originalImage.height()));
}

void MainWindow::processGrayscale()
{
    if (originalImage.isNull()) return;

    processedImage = originalImage.convertToFormat(QImage::Format_Grayscale8);

    updateImageDisplay();

    // 處理完成，解鎖「儲存結果」(Menu、ToolBar 與 Ctrl+S 同步啟用)
    saveAction->setEnabled(true);
    statusBar()->showMessage("影像處理完成：已轉換為灰階影像 (可按 Ctrl+S 儲存)");
}

void MainWindow::saveImage()
{
    if (processedImage.isNull()) return;

    QString savePath = QFileDialog::getSaveFileName(
        this,
        "儲存處理後影像",
        "output_processed.png",
        "PNG 影像 (*.png);;JPEG 影像 (*.jpg);;BMP 影像 (*.bmp)"
        );

    if (savePath.isEmpty()) {
        statusBar()->showMessage("已取消儲存");
        return;
    }

    if (processedImage.save(savePath)) {
        QMessageBox::information(this, "成功", "影像已成功儲存至：\n" + savePath);
        statusBar()->showMessage("檔案已儲存：" + savePath);
    } else {
        QMessageBox::critical(this, "儲存失敗", "無法寫入檔案，請檢查資料夾權限！");
    }
}

void MainWindow::updateImageDisplay()
{
    if (!originalImage.isNull()) {
        QPixmap origPix = QPixmap::fromImage(originalImage);
        originalLabel->setPixmap(origPix.scaled(
            originalLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }

    if (!processedImage.isNull()) {
        QPixmap procPix = QPixmap::fromImage(processedImage);
        processedLabel->setPixmap(procPix.scaled(
            processedLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}