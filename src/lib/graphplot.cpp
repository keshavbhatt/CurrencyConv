#include "graphplot.h"

GraphPlot::GraphPlot(QWidget *parent) :
    QCustomPlot (parent)
{
    this->addGraph(this->xAxis, this->yAxis);

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("d.MMMM\nyyyy");
    this->xAxis->setTicker(dateTicker);

    this->yAxis->setLabel("Rate");

    this->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(QDate(1999, 1, 1)),
                          QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime()));

    this->yAxis->setRange(0,1000);

    this->setInteraction(QCP::iRangeZoom,true);
    this->axisRect()->setRangeZoom(Qt::Horizontal);

    this->setInteraction(QCP::iRangeDrag, true);
    this->axisRect()->setRangeDrag(Qt::Horizontal);

    tracer = new QCPItemTracer(this);

    graphHasData = false;
    connect(this, &GraphPlot::mouseMove,this, &GraphPlot::slotMouseMove);

}

void GraphPlot::initGraph(const QList<CurrencyRate> &currencyRates)
{
    const int listSize = currencyRates.size();
    QVector<QCPGraphData> data(listSize);

    double minRate = INFINITY;
    double maxRate = 0;

    for(int i = 0; i < listSize; i++)
    {
        const double value = currencyRates[i].second;
        if(value > maxRate)
            maxRate = value;
        if(value < minRate)
            minRate = value;
        data[i].key = QCPAxisTickerDateTime::dateTimeToKey(currencyRates[i].first);
        data[i].value = value;
    }

    this->graph(0)->data()->set(data);

    graphHasData = true;

    this->yAxis->setRange(minRate * 0.95, maxRate + 5);
    this->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(currencyRates.last().first), QCPAxisTickerDateTime::dateTimeToKey(currencyRates.first().first));

    tracer->setGraph(this->graph(0));

    this->replot();
}

void GraphPlot::slotMouseMove(QMouseEvent *event)
{
    if(graphHasData)
    {
        double coordX = this->xAxis->pixelToCoord(event->pos().x());
        this->setToolTip(
                    "<p><b>"+ QCPAxisTickerDateTime::keyToDateTime(tracer->position->key()).toString("d.MMMM\nyyyy") +"</b></p>" +
                    "<p><b>"+ QString::number(tracer->position->value()) +"</b></p>");

        tracer->setGraphKey(coordX);

        this->replot();
    }
}
