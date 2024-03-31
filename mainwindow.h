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

private:
    Ui::MainWindow *ui;
    void ChangeGroupSize(int val);
    void initActionsConnections(void);
    void initSine(int channel);
    void UpdateGraph(void);
    void initSettings(void);
    void CloseFlowPanel(void);
    void closeEvent(QCloseEvent *event);
    void ConcreteChangeGraph(int *channel, float *min_x, float *min_y, float *max_x, float *max_y);
    void ChangeGraph(int channel);
    void StartPauseReadData();
    void TestFunction();

    SettingsDialog *SetDial = nullptr;
    SettingsDialog *m_settings = nullptr;
    ControlPanel *ControlPnl = nullptr;
    QDialog *ControlPnlDialog = nullptr;

    int Channel_Size = 1;
    int fillingIndex = 0;// Заполняемый индекс
    bool PortReadFlag = true;


};
#endif // MAINWINDOW_H
