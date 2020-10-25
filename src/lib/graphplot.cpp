#include "graphplot.h"

GraphPlot::GraphPlot(QWidget *parent) :
    QCustomPlot (parent)
{
    this->addGraph(this->xAxis, this->yAxis);

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("dd\nMMM\nyyyy");
    this->xAxis->setTicker(dateTicker);

    this->yAxis->setLabel(tr("Rate"));

    this->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(QDate(1999, 1, 1)),
                          QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime()));

    this->yAxis->setRange(-5.00,5.00); //initial values,we change em in init

    this->axisRect()->setupFullAxesBox();
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    tracer = new QCPItemTracer(this);
    tracer->setStyle(QCPItemTracer::tsCircle);
    graphHasData = false;
    connect(this, &GraphPlot::mouseMove,this, &GraphPlot::slotMouseMove);

    //style
    QColor backgroundColor = settings.value("theme").toString() == "light" ? QColor("#eff0f1") : QColor("#31363b") ;
    setBackground(QBrush(backgroundColor));

    QColor labelColor = settings.value("theme").toString() == "light" ? QColor("#232627") : QColor("#eff0f1") ;

    this->xAxis->setLabelColor(labelColor);
    this->yAxis->setLabelColor(labelColor);

    QList<QCPAxis *> axises;
    axises<<xAxis<<xAxis2<<yAxis<<yAxis2;

    foreach (QCPAxis *axis, axises) {
        axis->setTickPen(QPen(labelColor));
        axis->setTickLabelColor(labelColor);
        axis->setSubTickPen(QPen(labelColor));
        axis->setBasePen(QPen(labelColor));
    }

}

void GraphPlot::initGraph(const QList<CurrencyRate> &currencyRates)
{
    if(!currencyRates.isEmpty()){
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
        this->graph()->setLineStyle((QCPGraph::LineStyle)(1));
        if (rand()%100 > 50)
          this->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(5)));

        QPen graphPen;
        //QColor randColor = QColor(rand()%245+10, rand()%245+10, rand()%245+10);
        graphPen.setColor(QColor("#3DAEE9"));
        graphPen.setWidthF(2.0);
        this->graph()->setPen(graphPen);

        graphHasData = true;

        this->yAxis->setRange(minRate * 0.95, maxRate + 5);
        this->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(currencyRates.last().first), QCPAxisTickerDateTime::dateTimeToKey(currencyRates.first().first));

        tracer->setGraph(this->graph(0));

        this->replot();
    }else{
        QMessageBox::information(this,tr("Error"),tr("No data returned."));
    }
}

void GraphPlot::slotMouseMove(QMouseEvent *event)
{
    if(graphHasData)
    {
        double coordX = this->xAxis->pixelToCoord(event->pos().x());
        this->setToolTip(
                    "<p><b>"+ QCPAxisTickerDateTime::keyToDateTime(tracer->position->key()).toString("dd/MMM/yyyy") +"</b></p>" +
                    "<p><b>"+ QString::number(tracer->position->value()) +"</b></p>");

        tracer->setGraphKey(coordX);

        this->replot();
    }
}
