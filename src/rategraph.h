#ifndef RATEGRAPH_H
#define RATEGRAPH_H

#include <QWidget>
#include <QSettings>
#include "request.h"
#include "waitingspinnerwidget.h"
#include "currencyrate.h"
#include "utils.h"
#include "lib/datamodel.h"

namespace Ui {
class RateGraph;
}

class RateGraph : public QWidget
{
    Q_OBJECT

public:
    explicit RateGraph(QWidget *parent = nullptr);
    ~RateGraph();

protected slots:
    void closeEvent(QCloseEvent *ev);
private slots:
    void on_updatePushBtn_clicked();

    void init_request();
    void init_loader();
    void setStatus(QString message);
    void setRates(QString reply);
    void load_currencies();
    void on_currencyComboBox_currentIndexChanged(int index);

private:
    Ui::RateGraph *ui;
    Request *_request = nullptr;
    WaitingSpinnerWidget *_loader= nullptr;
    QUrl _currentUrl;
    QList<core::CurrencyRate> currencyRates;
    core::datamodel::DataModel* dataModel;
    QSettings settings;
};

#endif // RATEGRAPH_H
