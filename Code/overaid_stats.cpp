#include "overaid.h"
#include "ui_overaid.h"


void OverAid::on_tabWidget_currentChanged(int index)
{
    if(index == 1)
    {
        //Reinitialiser tout
        axisX->~QDateTimeAxis();
        axisX = new QDateTimeAxis();
        axisY->~QValueAxis();
        axisY = new QValueAxis();
        series->~QLineSeries();
        series = new QLineSeries();

        if(ui->gridLayout_stats->itemAtPosition(0,0) != nullptr)
        {
            qobject_cast<QChartView*>(ui->gridLayout_stats->itemAtPosition(0,0)->widget())->~QChartView();
            ui->gridLayout_stats->removeItem(ui->gridLayout_stats->itemAtPosition(0,0));
        }
        if(ui->gridLayout_stats->itemAtPosition(1,0) != nullptr)
        {
            qobject_cast<RangeSlider*>(ui->gridLayout_stats->itemAtPosition(1,0)->widget())->~RangeSlider();
            ui->gridLayout_stats->removeItem(ui->gridLayout_stats->itemAtPosition(1,0));
        }
        rangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, ui->tabStats);
        rangeSlider->setObjectName(QString::fromUtf8("rangeSlider"));
        rangeSlider->setMaximumSize(QSize(16777215, 50));
        ui->gridLayout_stats->addWidget(rangeSlider,1,0);

        dates.clear();
        solde.clear();
        rangeSlider->setEnabled(true);
        //----------

        stats();
        disconnect(rangeSlider, SIGNAL(lowerValueChanged(int)), this, 0);
        disconnect(rangeSlider, SIGNAL(upperValueChanged(int)), this, 0);
        connect(rangeSlider, SIGNAL(lowerValueChanged(int)), this, SLOT(stats_dates()));
        connect(rangeSlider, SIGNAL(upperValueChanged(int)), this, SLOT(stats_dates()));
    }
}

void OverAid::stats()
{
    QLineSeries *zeroSeries = new QLineSeries();

    QSqlQuery firstDate("SELECT date FROM Transactions WHERE "+where+" ORDER BY date ASC, id_trans ASC");
    if(firstDate.next())
    {
        QDateTime olderDate = QDateTime::fromString(firstDate.value("date").toString(),"yyyyMMdd").addDays(-1);
        if(where.startsWith("1+1 AND id_compte"))
        {
            QSqlQuery compteInitial("SELECT montant_initial FROM Comptes WHERE id_compte='"+QString::number(id_compte)+"'");
            if(compteInitial.next()) solde.append(compteInitial.value("montant_initial").toDouble());

            series->append(olderDate.toMSecsSinceEpoch(), compteInitial.value("montant_initial").toDouble());
            solde.append(compteInitial.value("montant_initial").toDouble());
        }
        else
        {
            series->append(olderDate.toMSecsSinceEpoch(), 0);
            solde.append(0);
        }

        dates.append(olderDate);
        zeroSeries->append(olderDate.toMSecsSinceEpoch(), 0);
    }

    for(int i = 1; i <= ui->treeWidgetSummary->topLevelItemCount(); i++) {
        QTreeWidgetItem *yearItem = ui->treeWidgetSummary->topLevelItem(ui->treeWidgetSummary->topLevelItemCount()-i);
        for(int j = 1; j <= yearItem->childCount(); j++) {
            QTreeWidgetItem *monthItem = yearItem->child(yearItem->childCount()-j);
            for(int k = 1; k <= monthItem->childCount(); k++) {
                QTreeWidgetItem *transaction = monthItem->child(monthItem->childCount()-k);

                solde.append(QString(transaction->text(10).split(" ").at(0)).replace(",",".").toDouble());
                series->append(locale.toDateTime(transaction->text(0),locale.dateFormat(QLocale::ShortFormat).contains("yyyy") ? locale.dateFormat(QLocale::ShortFormat) : locale.dateFormat(QLocale::ShortFormat).replace("yy","yyyy")).toMSecsSinceEpoch(), QString(transaction->text(10).split(" ").at(0)).replace(",",".").toDouble());
                zeroSeries->append(locale.toDateTime(transaction->text(0),locale.dateFormat(QLocale::ShortFormat).contains("yyyy") ? locale.dateFormat(QLocale::ShortFormat) : locale.dateFormat(QLocale::ShortFormat).replace("yy","yyyy")).toMSecsSinceEpoch(), 0);

                dates.append(locale.toDateTime(transaction->text(0),locale.dateFormat(QLocale::ShortFormat).contains("yyyy") ? locale.dateFormat(QLocale::ShortFormat) : locale.dateFormat(QLocale::ShortFormat).replace("yy","yyyy")));
            }
        }
    }

    rangeSlider->SetRange(0,dates.count()-1);

    QChart *chart = new QChart();
    chart->addSeries(zeroSeries);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->hide();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setTitle("");
    chart->setLocale(locale);
    if(series->count() == 0)
    {
        chart->setTitle(tr("Aucune transaction avec ces filtres"));
        rangeSlider->setEnabled(false);
    }
    else chart->setTitle("");

    chart->setTheme(QChart::ChartThemeBrownSand);
    series->setPen(QPen(QBrush(QColor(220, 134, 23)),1.75));
    zeroSeries->setPen(QPen(QBrush(Qt::black),0.5));

    axisX->setTickCount(10);
    axisX->setFormat(tr("dd MMM yyyy"));
    axisX->setTitleText(tr("Date"));
    axisX->setLabelsAngle(-25);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    zeroSeries->attachAxis(axisX);

    axisY->setLabelFormat("%i");
    axisY->setTitleText("");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    zeroSeries->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    connect(series, &QLineSeries::hovered, this, &OverAid::stats_hover);

    ui->gridLayout_stats->addWidget(chartView,0,0);
}

void OverAid::stats_dates()
{
    if(rangeSlider->GetLowerValue() < rangeSlider->GetUpperValue())
    {
        axisX->setMin(dates.at(rangeSlider->GetLowerValue()));
        axisX->setMax(dates.at(rangeSlider->GetUpperValue()));

        QList<double> soldeTemp = solde;
        for(int i = rangeSlider->GetMinimun(); i != rangeSlider->GetLowerValue(); i++) soldeTemp.takeFirst();
        for(int i = rangeSlider->GetMaximun(); i != rangeSlider->GetUpperValue(); i--) soldeTemp.takeLast();

        double min = *std::min_element(soldeTemp.begin(), soldeTemp.end());
        double max = *std::max_element(soldeTemp.begin(), soldeTemp.end());
        soldeTemp.clear();

        axisY->setRange(min,max);
    }
}

void OverAid::stats_hover(const QPointF &point, bool state)
{
    if(state)
    {
        QString symboleCompte;
        QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
        if(devise.next()) symboleCompte = devise.value("symbole").toString();

        ui->labelStats_date->setText(locale.toString(QDateTime::fromMSecsSinceEpoch(point.x()),"dddd d MMM yyyy")+" :");
        ui->labelStats_montant->setText(QString::number(point.y(),'f',2)+" "+symboleCompte);
    }
    else
    {
        ui->labelStats_date->clear();
        ui->labelStats_montant->clear();
    }
}
