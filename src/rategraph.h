#ifndef RATEGRAPH_H
#define RATEGRAPH_H

#include <QWidget>
#include "request.h"
#include "waitingspinnerwidget.h"
#include "currencyrate.h"

namespace Ui {
class RateGraph;
}

class RateGraph : public QWidget
{
    Q_OBJECT

public:
    explicit RateGraph(QWidget *parent = nullptr);
    ~RateGraph();

private slots:
    void on_updatePushBtn_clicked();

    void init_request();
    void init_loader();
    void setStatus(QString message);
    void setRates(QString reply);
private:
    Ui::RateGraph *ui;
    Request *_request = nullptr;
    WaitingSpinnerWidget *_loader= nullptr;
    QUrl _currentUrl;
    QList<core::CurrencyRate> currencyRates;

};

#endif // RATEGRAPH_H
