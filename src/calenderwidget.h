#ifndef CALENDERWIDGET_H
#define CALENDERWIDGET_H

#include <QDialog>
#include <QDate>
#include <QSettings>

namespace Ui {
class CalenderWidget;
}

class CalenderWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CalenderWidget(QWidget *parent = nullptr);
    ~CalenderWidget();
signals:
    void selectionChanged(const QDate date);

private slots:
    void on_todayPushButon_clicked();

    void on_stayVisible_toggled(bool checked);

private:
    Ui::CalenderWidget *ui;
    QSettings settings;
};

#endif // CALENDERWIDGET_H
