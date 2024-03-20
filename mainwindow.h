#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SettingsDialog;

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
    void on_Connect_action_triggered();
    void on_PortSettings_action_triggered();
    void readData();

private:
    Ui::MainWindow *ui;
    void ChangeGroupSize(int val);
    void initActionsConnections(void);
    void initSettings(void);

    SettingsDialog *SetDial = nullptr;
    int Last_Num_Channel = 1;
    SettingsDialog *m_settings = nullptr;
};
#endif // MAINWINDOW_H
