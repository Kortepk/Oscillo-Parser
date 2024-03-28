#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ControlPanel.h"

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
    void ChangeFlowWindowMode(bool TriggerStatus);

private slots:
    void CounterChannel_Changed(int arg1);
    void on_Connect_action_triggered();
    void on_PortSettings_action_triggered();
    void readData();

    void on_MaxPointSlider_sliderMoved(int position);

    void on_pushButton_clicked();

    void on_pushButtonTest_clicked();


private:
    Ui::MainWindow *ui;
    void ChangeGroupSize(int val);
    void initActionsConnections(void);
    void UpdateGraph(void);
    void initSettings(void);
    void CloseFlowPanel(void);

    SettingsDialog *SetDial = nullptr;
    int Channel_Size = 1;
    SettingsDialog *m_settings = nullptr;
    ControlPanel *ControlPnl = nullptr;
    QDialog *ControlPnlDialog = nullptr;
};
#endif // MAINWINDOW_H
