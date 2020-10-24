#ifndef GRAPHPLOT_H
#define GRAPHPLOT_H

#include <QWidget>
#include "QCustomPlot/qcustomplot.h"
#include <currencyrate.h>
#include <QSettings>

using namespace core;


class GraphPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit GraphPlot(QWidget *parent = nullptr);

    void initGraph(const QList<CurrencyRate>& currencyRates);


private slots:
    void slotMouseMove(QMouseEvent * event);

private:
    QCPItemTracer *tracer;
    bool graphHasData;
    QSettings settings;
};

#endif // GRAPHPLOT_H
