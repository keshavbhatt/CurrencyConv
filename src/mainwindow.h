#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QStyleFactory>
#include <QAbstractSpinBox>
#include <QCloseEvent>
#include <QSettings>
#include <QCalendarWidget>

#include "waitingspinnerwidget.h"
#include "request.h"
#include "error.h"
#include "calenderwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:

    void closeEvent(QCloseEvent *event);

private slots:

    void setStyle(QString fname);

    void getRates(bool historical);

    void init_request();

    void setRates(QString reply);

    void init_loader();

    void on_s1SpinBox_valueChanged(double arg1);

    void on_s2SpinBox_valueChanged(double arg1);

    void on_s1ComboBox_currentIndexChanged(const QString &arg1);

    void on_s2ComboBox_currentIndexChanged(const QString &arg1);

    double convert(double value, QString s_cur, QString t_cur);

    void init_appMenu();

    void showHistoricalRate();

private:

    Ui::MainWindow *ui;
    WaitingSpinnerWidget *_loader = nullptr;
    Error *_error = nullptr;
    Request *_request = nullptr;

    QMap<QString,double> exchange;
    QSettings settings;
    CalenderWidget *_calWidget = nullptr;
    QString historicalDate;
};

#endif // MAINWINDOW_H
