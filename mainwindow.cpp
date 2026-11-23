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
    setWindowTitle("Qt MainWindow 影像處理系統");
    resize(1000, 600);

    setupUI();
    setupActions();

    statusBar()->showMessage("就緒：請點擊 Open Image 載入圖片");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 【需求：CentralWidget】建立中央主容器並設為 CentralWidget
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

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
}

void MainWindow::setupActions()
{
    // 【需求：QAction 套用 icon 和 shortcut】
    // 1. Open Image (追加項目：將同時掛載至 File Menu 與 Toolbar)
    openAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open Image", this);
    openAction->setShortcut(QKeySequence::Open); // Ctrl+O
    openAction->setStatusTip("開啟本機影像檔案 (Ctrl+O)");

    // 2. Save (追加項目：加入 Ctrl+S 快捷鍵)
    saveAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save", this);
    saveAction->setShortcut(QKeySequence::Save); // Ctrl+S
    saveAction->setStatusTip("儲存處理後的影像 (Ctrl+S)");
    saveAction->setEnabled(false); // 尚未處理圖片前先停用

    // 3. Exit
    exitAction = new QAction(style()->standardIcon(QStyle::SP_DialogCloseButton), "Exit", this);
    exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q)); // Ctrl+Q
    exitAction->setStatusTip("關閉應用程式 (Ctrl+Q)");

    // 4. Image Menu 功能：轉灰階
    grayscaleAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), "Grayscale (灰階處理)", this);
    grayscaleAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G)); // Ctrl+G
    grayscaleAction->setStatusTip("將原圖轉換為灰階影像 (Ctrl+G)");
    grayscaleAction->setEnabled(false); // 尚未開圖前先停用

    // 5. Edit Menu 功能：清除畫面
    clearAction = new QAction(style()->standardIcon(QStyle::SP_DialogResetButton), "Clear (清除畫面)", this);
    clearAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete)); // Ctrl+Del
    clearAction->setStatusTip("清除目前載入與處理的所有影像");
    clearAction->setEnabled(false);

    // 【需求：File / Image / Edit 的 Menu】
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(openAction); // Open Image 出現在 File Menu
    fileMenu->addAction(saveAction); // Save 出現在 File Menu (顯示 Ctrl+S)
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction); // Exit 出現在 File Menu

    QMenu *imageMenu = menuBar()->addMenu("Image");
    imageMenu->addAction(grayscaleAction);

    QMenu *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(clearAction);

    // 【需求：Toolbar 中包含常見 QAction】
    QToolBar *mainToolBar = addToolBar("Main Toolbar");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->addAction(openAction);      // 追加項目：同一個 Open Image 出現在 Toolbar，執行相同功能
    mainToolBar->addAction(saveAction);      // 常見動作：Save
    mainToolBar->addSeparator();
    mainToolBar->addAction(grayscaleAction); // 常見動作：Grayscale
    mainToolBar->addAction(clearAction);     // 常見動作：Clear
    mainToolBar->addSeparator();
    mainToolBar->addAction(exitAction);      // 常見動作：Exit

    // 連接 Signal & Slot
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(grayscaleAction, &QAction::triggered, this, &MainWindow::processGrayscale);
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearImages);
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Image",
        "",
        "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*.*)"
        );

    if (fileName.isEmpty()) {
        statusBar()->showMessage("已取消開啟圖片");
        return;
    }

    QImage loadedImage(fileName);
    if (loadedImage.isNull()) {
        QMessageBox::warning(this, "錯誤", "無法載入該影像檔案！");
        return;
    }

    originalImage = loadedImage;
    processedImage = QImage();
    processedLabel->clear();
    processedLabel->setText("已載入原圖，請執行 Image -> Grayscale");

    updateImageDisplay();

    // 啟用影像處理與清除按鈕，儲存按鈕待處理後再啟用
    grayscaleAction->setEnabled(true);
    clearAction->setEnabled(true);
    saveAction->setEnabled(false);

    statusBar()->showMessage(QString("已載入：%1 (%2x%3)")
                                 .arg(fileName)
                                 .arg(originalImage.width())
                                 .arg(originalImage.height()));
}

void MainWindow::processGrayscale()
{
    if (originalImage.isNull()) return;

    processedImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
    updateImageDisplay();

    // 處理完成，啟用 Save (同時解鎖 Menu、Toolbar 與 Ctrl+S)
    saveAction->setEnabled(true);
    statusBar()->showMessage("灰階處理完成！可按 Ctrl+S 儲存結果");
}

void MainWindow::saveImage()
{
    if (processedImage.isNull()) return;

    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Save Image",
        "processed_output.png",
        "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)"
        );

    if (savePath.isEmpty()) {
        statusBar()->showMessage("已取消儲存");
        return;
    }

    if (processedImage.save(savePath)) {
        QMessageBox::information(this, "儲存成功", "影像已儲存至：\n" + savePath);
        statusBar()->showMessage("已儲存：" + savePath);
    } else {
        QMessageBox::critical(this, "儲存失敗", "檔案寫入失敗！");
    }
}

void MainWindow::clearImages()
{
    originalImage = QImage();
    processedImage = QImage();

    originalLabel->clear();
    originalLabel->setText("尚未載入圖片");
    processedLabel->clear();
    processedLabel->setText("等待執行影像處理");

    grayscaleAction->setEnabled(false);
    saveAction->setEnabled(false);
    clearAction->setEnabled(false);

    statusBar()->showMessage("已清除所有畫面");
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