#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QAction>
#include <QImage>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImage();        // 開啟圖片 (Open Image)
    void saveImage();        // 儲存圖片 (Save)
    void processGrayscale(); // 影像處理：轉灰階 (Image Menu)
    void clearImages();      // 編輯功能：清除畫面 (Edit Menu)

private:
    void setupUI();            // 建立 CentralWidget 與雙畫面排版
    void setupActions();       // 建立 QAction (含 Icon 與 Shortcut)、Menu 與 Toolbar
    void updateImageDisplay(); // 等比例更新圖片顯示

    // CentralWidget 內的顯示元件
    QLabel *originalLabel;
    QLabel *processedLabel;

    // QAction 動作定義
    QAction *openAction;      // Open Image (同時給 File Menu 與 Toolbar 共用)
    QAction *saveAction;      // Save (綁定 Ctrl+S)
    QAction *exitAction;      // Exit
    QAction *grayscaleAction; // Grayscale (隸屬 Image Menu)
    QAction *clearAction;     // Clear (隸屬 Edit Menu)

    // 影像資料
    QImage originalImage;
    QImage processedImage;
};

#endif // MAINWINDOW_H