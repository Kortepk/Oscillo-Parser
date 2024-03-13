#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Group_Size_Slider_sliderMoved(int position);

    void on_Group_Size_Box_valueChanged(int arg1);

    void on_Counter_channel_Box_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    void ChangeGroupSize(int val);
};
#endif // MAINWINDOW_H
