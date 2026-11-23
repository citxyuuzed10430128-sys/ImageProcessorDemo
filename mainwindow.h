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
    void openImage();      // 開啟圖片
    void processGrayscale(); // 執行影像處理（轉灰階）
    void saveImage();      // 儲存處理後的圖片

private:
    void setupUI();        // 初始化介面排版
    void setupActions();   // 初始化 QAction、Menu 與 Toolbar
    void updateImageDisplay(); // 更新左右兩個 QLabel 的圖片顯示

    // 介面顯示元件
    QLabel *originalLabel;
    QLabel *processedLabel;

    // 共用動作 (Menu 與 Toolbar 共用)
    QAction *openAction;
    QAction *processAction;
    QAction *saveAction;
    QAction *exitAction;

    // 影像資料暫存
    QImage originalImage;
    QImage processedImage;
};

#endif // MAINWINDOW_H