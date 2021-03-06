#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QStyleFactory>
#include <QAbstractSpinBox>
#include <QCloseEvent>
#include <QSettings>
#include <QCalendarWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QDesktopWidget>
#include <QSizeGrip>
#include <QScreen>
#include <QDesktopServices>

#include "waitingspinnerwidget.h"
#include "request.h"
#include "error.h"
#include "calenderwidget.h"
#include "rategraph.h"
#include "settingswidget.h"

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

    bool eventFilter(QObject *obj, QEvent *ev);
    void resizeEvent(QResizeEvent *ev);
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

    void set_style();

    void on_reload_clicked();

    void on_home_clicked();

    void showAbout();

    void load_currencies();

    int  getIndexByDataRole(QString dataValue, QComboBox *cBox);

    void s1ComboFix();

    void on_settings_clicked();

private:

    Ui::MainWindow *ui;
    WaitingSpinnerWidget *_loader = nullptr;
    Request *_request = nullptr;

    QMap<QString,QString> currencyMeta;
    QMap<QString,double> exchange;
    QSettings settings;
    CalenderWidget *_calWidget = nullptr;
    QString historicalDate;
    QUrl _currentUrl,_homeUrl;

    RateGraph * _rateGraph=nullptr;
    SettingsWidget * _settingsWidget = nullptr;
};

#endif // MAINWINDOW_H
