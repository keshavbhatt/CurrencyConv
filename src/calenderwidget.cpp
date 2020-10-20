#include "calenderwidget.h"
#include "ui_calenderwidget.h"
#include <QCalendarWidget>
#include <QDebug>
#include <QToolButton>

CalenderWidget::CalenderWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalenderWidget)
{
    ui->setupUi(this);
    ui->calendarWidget->setMaximumDate(QDate::currentDate());
    ui->calendarWidget->setMinimumDate(QDate(1999,1,1));
    connect(ui->calendarWidget,&QCalendarWidget::selectionChanged,[=](){
       emit selectionChanged(ui->calendarWidget->selectedDate());
    });

    foreach (QWidget *wid, this->findChildren<QWidget*>()) {
        if(wid->objectName()=="qt_calendar_prevmonth"
                || wid->objectName()=="qt_calendar_nextmonth"){
            wid->setStyleSheet("padding:4px");
        }
        if(wid->objectName()=="qt_calendar_monthbutton"){
            QToolButton *btn = qobject_cast<QToolButton*>(wid);
            btn->setStyleSheet("QToolButton::menu-indicator{width:0px;}");
        }
        if(wid->objectName()=="qt_calendar_calendarview"){
            wid->setStyleSheet("border:none");
        }
    }
}

CalenderWidget::~CalenderWidget()
{
    delete ui;
}

void CalenderWidget::on_todayPushButon_clicked()
{
    ui->calendarWidget->setSelectedDate(QDate::currentDate());
}
