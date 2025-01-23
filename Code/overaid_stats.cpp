#include "Classes/custommenu.h"
#include "overaid.h"
#include "ui_overaid.h"


void OverAid::on_tabWidget_currentChanged(int index)
{
    if(index == 1) {
        for (int i = ui->gridLayout_stats->count(); i >= 0; i--) {
            QLayoutItem *item = ui->gridLayout_stats->itemAt(i);
            if (item && item->widget()) {
                ui->gridLayout_stats->removeItem(item);
                item->widget()->deleteLater();
            }
        }

        stats_solde();
        stats_categories(true,"-1");
        stats_debitCredit();

        for (int i = 0; i < ui->gridLayout_stats->rowCount(); i++)
            ui->gridLayout_stats->setRowStretch(i, 100);
        for (int i = 0; i < ui->gridLayout_stats->columnCount(); i++)
            ui->gridLayout_stats->setColumnStretch(i, 100);
    }
}

void OverAid::stats_solde()
{
    QLineSeries *zeroSeries = new QLineSeries();
    QList<QString> statsList;
    QLineSeries *series = new QLineSeries();
    QList<QDateTime> dates;
    QList<double> solde;

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

    QChart *chart = new QChart();
    chart->addSeries(zeroSeries);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setLocale(locale);
    if(series->count() == 0) chart->setTitle(tr("Aucune transaction avec ces filtres"));
    else chart->setTitle(tr("Solde"));

    chart->setTheme(QChart::ChartThemeBrownSand);
    series->setPen(QPen(QBrush(QColor(220, 134, 23)),1.75));
    zeroSeries->setPen(QPen(QBrush(Qt::black),0.5));

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat(tr("dd MMM yyyy"));
    axisX->setLabelsAngle(-25);
    axisX->setTruncateLabels(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    zeroSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    zeroSeries->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setRubberBand(QChartView::RectangleRubberBand);

    //Recadrer sur les 60 dernières transactions
    axisX->setMin(dates[dates.count() - (dates.count() > 60 ? 60 : dates.count())]);

    QList<double> soldeTemp = solde.mid(dates.count() - (dates.count() > 60 ? 60 : dates.count()-1), dates.count() > 60 ? 60 : dates.count());
    double min = *std::min_element(soldeTemp.begin(), soldeTemp.end());
    double max = *std::max_element(soldeTemp.begin(), soldeTemp.end());
    soldeTemp.clear();
    axisY->setRange(min,max);

    ui->gridLayout_stats->addWidget(chartView,0,0,2,1);

    //Boutons d'actions
    QPushButton *fullScreenButton = new QPushButton("⛶");
    fullScreenButton->setFixedWidth(40);
    fullScreenButton->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
    QGraphicsProxyWidget *fullScreenProxy = chartView->scene()->addWidget(fullScreenButton);

    QPushButton *minimizeButton = new QPushButton("▬");
    minimizeButton->setFixedWidth(40);
    minimizeButton->hide();
    minimizeButton->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
    QGraphicsProxyWidget *minimizeProxy = chartView->scene()->addWidget(minimizeButton);

    QPushButton *resetButton = new QPushButton("⤺");
    resetButton->setFixedWidth(40);
    resetButton->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
    QGraphicsProxyWidget *resetProxy = chartView->scene()->addWidget(resetButton);

    connect(series, &QLineSeries::hovered, this, &OverAid::stats_solde_hover);

    connect(resetButton, &QPushButton::clicked, [=]() {
        axisX->setMin(dates[0]);
        double min = *std::min_element(solde.begin(), solde.end());
        double max = *std::max_element(solde.begin(), solde.end());
        axisY->setRange(min,max);
        chart->zoomReset();
        resetButton->hide();
    });
    connect(axisX, &QDateTimeAxis::rangeChanged, [=]() {resetButton->show();});
    connect(axisY, &QValueAxis::rangeChanged, [=]() {resetButton->show();});

    connect(fullScreenButton, &QPushButton::clicked, [=]() {
        setFullScreen(chartView);
        fullScreenButton->hide();
        minimizeButton->show();
    });

    connect(minimizeButton, &QPushButton::clicked, [=]() {
        restoreFullScreen();
        fullScreenButton->show();
        minimizeButton->hide();
    });

    connect(chart, &QChart::geometryChanged, [=]() {
        fullScreenProxy->setPos(chart->geometry().topRight() - QPointF(fullScreenButton->width(), 0));
        minimizeProxy->setPos(chart->geometry().topRight() - QPointF(minimizeButton->width(), 0));
        resetProxy->setPos(chart->geometry().topRight() - QPointF(resetButton->width(), 0) - (QPointF(fullScreenButton->width(), 0)));
    });
}

void OverAid::stats_solde_hover(const QPointF &point, bool state)
{
    if(state)
    {
        QString symboleCompte;
        QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
        if(devise.next()) symboleCompte = devise.value("symbole").toString();

        qobject_cast<QChartView*>(ui->gridLayout_stats->itemAtPosition(0,0)->widget())->chart()->setTitle(locale.toString(QDateTime::fromMSecsSinceEpoch(point.x()),"dddd d MMM yyyy")+" : "+QString::number(point.y(),'f',2).replace(".",",")+" "+symboleCompte);
    }
    else qobject_cast<QChartView*>(ui->gridLayout_stats->itemAtPosition(0,0)->widget())->chart()->setTitle(tr("Solde"));
}

void OverAid::stats_categories(bool isMainCategory, QString cat0)
{
    bool isDebitCurrentWidget = false, isCreditCurrentWidget = false;
    if(ui->gridLayout_stats->itemAtPosition(0,1))
    {
        if(!ui->gridLayout_stats->itemAtPosition(0,1)->widget()->isHidden()) isDebitCurrentWidget = true;
        ui->gridLayout_stats->itemAtPosition(0,1)->widget()->deleteLater();
    }
    if(ui->gridLayout_stats->itemAtPosition(0,2))
    {
        if(!ui->gridLayout_stats->itemAtPosition(0,2)->widget()->isHidden()) isCreditCurrentWidget = true;
        ui->gridLayout_stats->itemAtPosition(0,2)->widget()->deleteLater();
    }

    QPieSeries *seriesDebit = new QPieSeries();
    QStringList catDebit;
    QList<double> catDebit_montant;
    QPieSeries *seriesCredit = new QPieSeries();
    QStringList catCredit;
    QList<double> catCredit_montant;

    QString symboleCompte;
    QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
    if(devise.next()) symboleCompte = devise.value("symbole").toString();

    bool isDebit;

    for(int i = 1; i <= ui->treeWidgetSummary->topLevelItemCount(); i++) {
        QTreeWidgetItem *yearItem = ui->treeWidgetSummary->topLevelItem(ui->treeWidgetSummary->topLevelItemCount()-i);
        for(int j = 1; j <= yearItem->childCount(); j++) {
            QTreeWidgetItem *monthItem = yearItem->child(yearItem->childCount()-j);
            for(int k = 1; k <= monthItem->childCount(); k++) {
                QTreeWidgetItem *transaction = monthItem->child(monthItem->childCount()-k);

                if(transaction->childCount() > 0)
                {
                    for(int l = 1; l <= transaction->childCount(); l++)
                    {
                        QTreeWidgetItem *multiTrans = transaction->child(transaction->childCount()-l);
                        while(!multiTrans->text(6).contains(symboleCompte)) on_treeWidgetSummary_itemClicked(multiTrans,6);

                        QString cat = multiTrans->text(3);
                        QString cat2 = multiTrans->text(4);
                        if(cat2 == "") cat2 = tr("Vide");

                        QAction *catAction = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat->menu())->findAction(cat);
                        QAction *cat2Action = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat2->menu())->findAction(cat2);
                        if(catAction && catAction->isChecked() && cat2Action && cat2Action->isChecked() && (cat0 == "-1" || cat0 == multiTrans->text(3)))
                        {
                            isDebit = (transaction->text(2) == tr("Débit") && QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').toDouble() > 0) || (transaction->text(2) == tr("Crédit") && QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').toDouble() < 0);
                            if(!(isDebit ?catDebit:catCredit).contains(isMainCategory?cat:cat2))
                            {
                                (isDebit?catDebit:catCredit).append(isMainCategory?cat:cat2);
                                (isDebit?catDebit_montant:catCredit_montant).append(QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').remove("-").toDouble());
                            }
                            else (isDebit?catDebit_montant:catCredit_montant)[(isDebit?catDebit:catCredit).indexOf(isMainCategory?cat:cat2)] += QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').remove("-").toDouble();
                        }
                    }
                }
                else
                {
                    while(!transaction->text(6).contains(symboleCompte)) on_treeWidgetSummary_itemClicked(transaction,6);

                    QString cat = transaction->text(3);
                    QString cat2 = transaction->text(4);
                    if(cat2 == "") cat2 = tr("Vide");

                    QAction *catAction = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat->menu())->findAction(cat);
                    QAction *cat2Action = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat2->menu())->findAction(cat2);
                    if(catAction && catAction->isChecked() && cat2Action && cat2Action->isChecked() && (cat0 == "-1" || cat0 == transaction->text(3)))
                    {
                        isDebit = (transaction->text(2) == tr("Débit") && QString(transaction->text(6).split(" ").at(0)).replace(',','.').toDouble() > 0) || (transaction->text(2) == tr("Crédit") && QString(transaction->text(6).split(" ").at(0)).replace(',','.').toDouble() < 0);
                        if(!(isDebit?catDebit:catCredit).contains(isMainCategory?cat:cat2))
                        {
                            (isDebit?catDebit:catCredit).append(isMainCategory?cat:cat2);
                            (isDebit?catDebit_montant:catCredit_montant).append(QString(transaction->text(6).split(" ").at(0)).replace(',','.').remove("-").toDouble());
                        }
                        else (isDebit?catDebit_montant:catCredit_montant)[(isDebit?catDebit:catCredit).indexOf(isMainCategory?cat:cat2)] += QString(transaction->text(6).split(" ").at(0)).replace(',','.').remove("-").toDouble();
                    }
                }
            }
        }
    }

    QColor color1(220, 134, 23);
    QColor color2(237, 199, 133);
    for (int i = 0; i < catDebit.count(); i++) {
        float ratio = static_cast<float>(i) / (catDebit.count() - 1);

        int red   = color1.red()   + ratio * (color2.red()   - color1.red());
        int green = color1.green() + ratio * (color2.green() - color1.green());
        int blue  = color1.blue()  + ratio * (color2.blue()  - color1.blue());

        seriesDebit->append(catDebit[i], catDebit_montant[catDebit.indexOf(catDebit[i])])->setColor(QColor(red, green, blue));
    }

    for (int i = 0; i < catCredit.count(); i++) {
        float ratio = static_cast<float>(i) / (catCredit.count() - 1);

        int red   = color1.red()   + ratio * (color2.red()   - color1.red());
        int green = color1.green() + ratio * (color2.green() - color1.green());
        int blue  = color1.blue()  + ratio * (color2.blue()  - color1.blue());

        seriesCredit->append(catCredit[i], catCredit_montant[catCredit.indexOf(catCredit[i])])->setColor(QColor(red, green, blue));
    }

    seriesDebit->setLabelsVisible(true);

    QChart *chartDebit = new QChart();
    chartDebit->setObjectName("Debit");
    chartDebit->addSeries(seriesDebit);
    chartDebit->setAnimationOptions(QChart::SeriesAnimations);
    chartDebit->legend()->hide();
    chartDebit->layout()->setContentsMargins(0, 0, 0, 0);
    chartDebit->setLocale(locale);
    if(seriesDebit->count() == 0) chartDebit->setTitle(tr("Aucune transaction avec ces filtres"));
    else chartDebit->setTitle(tr("Débit"));

    chartDebit->setTheme(QChart::ChartThemeBrownSand);

    QChartView *chartViewDebit = new QChartView(chartDebit);
    chartViewDebit->setRenderHint(QPainter::Antialiasing);
    chartViewDebit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->gridLayout_stats->addWidget(chartViewDebit,0,1,1,1);

    seriesCredit->setLabelsVisible(true);

    QChart *chartCredit = new QChart();
    chartCredit->setObjectName("Credit");
    chartCredit->addSeries(seriesCredit);
    chartCredit->setAnimationOptions(QChart::SeriesAnimations);
    chartCredit->legend()->hide();
    chartCredit->layout()->setContentsMargins(0, 0, 0, 0);
    chartCredit->setLocale(locale);
    if(seriesCredit->count() == 0) chartCredit->setTitle(tr("Aucune transaction avec ces filtres"));
    else chartCredit->setTitle(tr("Crédit"));

    chartCredit->setTheme(QChart::ChartThemeBrownSand);

    QChartView *chartViewCredit = new QChartView(chartCredit);
    chartViewCredit->setRenderHint(QPainter::Antialiasing);
    chartViewCredit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->gridLayout_stats->addWidget(chartViewCredit,0,2,1,1);

    //Boutons d'actions
    auto createButton = [&](const QString &text, bool hideInitially, QChartView *chartView) -> QPair<QPushButton*, QGraphicsProxyWidget*> {
        QPushButton *button = new QPushButton(text);
        button->setFixedWidth(40);
        button->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
        if(hideInitially) button->hide();
        QGraphicsProxyWidget *proxy = chartView->scene()->addWidget(button);
        return {button, proxy};
    };

    // Création des boutons pour chartViewDebit
    auto [fullScreenButtonDebit, fullScreenProxyDebit] = createButton("⛶", false, chartViewDebit);
    auto [minimizeButtonDebit, minimizeProxyDebit] = createButton("▬", true, chartViewDebit);
    auto [resetButtonDebit, resetProxyDebit] = createButton("⤺", isMainCategory, chartViewDebit);

    // Création des boutons pour chartViewCredit
    auto [fullScreenButtonCredit, fullScreenProxyCredit] = createButton("⛶", false, chartViewCredit);
    auto [minimizeButtonCredit, minimizeProxyCredit] = createButton("▬", true, chartViewCredit);
    auto [resetButtonCredit, resetProxyCredit] = createButton("⤺", isMainCategory, chartViewCredit);

    connect(seriesDebit, &QPieSeries::hovered, this, &OverAid::stats_categories_hover);
    connect(seriesCredit, &QPieSeries::hovered, this, &OverAid::stats_categories_hover);

    auto seriesClickConnect = [=](QPieSlice *slice) {
        resetButtonDebit->show();
        resetButtonCredit->show();
        if(isMainCategory) stats_categories(false, slice->label());
    };
    connect(seriesDebit, &QPieSeries::clicked, seriesClickConnect);
    connect(seriesCredit, &QPieSeries::clicked, seriesClickConnect);

    auto resetButtonConnect = [=]() {
        resetButtonDebit->hide();
        resetButtonCredit->hide();
        stats_categories(true, "-1");
    };
    connect(resetButtonDebit, &QPushButton::clicked, resetButtonConnect);
    connect(resetButtonCredit, &QPushButton::clicked, resetButtonConnect);

    connect(fullScreenButtonDebit, &QPushButton::clicked, [=]() {
        setFullScreen(chartViewDebit);
        fullScreenButtonDebit->hide();
        minimizeButtonDebit->show();
    });
    connect(fullScreenButtonCredit, &QPushButton::clicked, [=]() {
        setFullScreen(chartViewCredit);
        fullScreenButtonCredit->hide();
        minimizeButtonCredit->show();
    });

    connect(minimizeButtonDebit, &QPushButton::clicked, [=]() {
        restoreFullScreen();
        fullScreenButtonDebit->show();
        minimizeButtonDebit->hide();
    });
    connect(minimizeButtonCredit, &QPushButton::clicked, [=]() {
        restoreFullScreen();
        fullScreenButtonCredit->show();
        minimizeButtonCredit->hide();
    });

    auto chartButtonPos = [=](QChart *chart,
                              QGraphicsProxyWidget *fullScreenProxy, QPushButton *fullScreenButton,
                              QGraphicsProxyWidget *minimizeProxy, QPushButton *minimizeButton,
                              QGraphicsProxyWidget *resetProxy, QPushButton *resetButton) {
        return [=]() {
            fullScreenProxy->setPos(chart->geometry().topRight() - QPointF(fullScreenButton->width(), 0));
            minimizeProxy->setPos(chart->geometry().topRight() - QPointF(minimizeButton->width(), 0));
            resetProxy->setPos(chart->geometry().topRight() - QPointF(resetButton->width(), 0) - QPointF(fullScreenButton->width(), 0));
        };
    };
    connect(chartDebit, &QChart::geometryChanged, chartButtonPos(chartDebit, fullScreenProxyDebit, fullScreenButtonDebit, minimizeProxyDebit, minimizeButtonDebit, resetProxyDebit, resetButtonDebit));
    connect(chartCredit, &QChart::geometryChanged, chartButtonPos(chartCredit, fullScreenProxyCredit, fullScreenButtonCredit, minimizeProxyCredit, minimizeButtonCredit, resetProxyCredit, resetButtonCredit));

    if(ui->gridLayout_stats->itemAtPosition(0,0)->widget()->isHidden())
    {
        auto load = [=]()
        {
            return [=]()
            {
                if(isDebitCurrentWidget && !isCreditCurrentWidget) fullScreenButtonDebit->click();
                if(!isDebitCurrentWidget && isCreditCurrentWidget) fullScreenButtonCredit->click();
            };
        };
        QTimer::singleShot(0,load());
    }
}

void OverAid::stats_categories_hover(QPieSlice *slice, bool state)
{
    if(state)
    {
        QString symboleCompte;
        QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
        if(devise.next()) symboleCompte = devise.value("symbole").toString();

        slice->setExploded(true);
        slice->setBorderColor(Qt::black);
        slice->setBorderWidth(2);
        qobject_cast<QChart *>(slice->parent()->parent()->parent())->setTitle(slice->label() + " : " + QString::number(slice->value(),'f',2).replace(".",",") + " " + symboleCompte);
    }
    else
    {
        slice->setExploded(false);
        slice->setBorderColor(Qt::white);
        slice->setBorderWidth(0);
        qobject_cast<QChart *>(slice->parent()->parent()->parent())->setTitle(tr(qobject_cast<QChart *>(slice->parent()->parent()->parent())->objectName() == "Debit" ? "Débit" : "Crédit"));
    }
}

void OverAid::stats_debitCredit()
{
    QStringList mois;
    QBarSeries *series = new QBarSeries();
    QBarSet *debitBar = new QBarSet(tr("Débit"));
    QBarSet *creditBar = new QBarSet(tr("Crédit"));

    QString symboleCompte;
    QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
    if(devise.next()) symboleCompte = devise.value("symbole").toString();

    bool isDebit;

    for(int i = 1; i <= ui->treeWidgetSummary->topLevelItemCount(); i++) {
        QTreeWidgetItem *yearItem = ui->treeWidgetSummary->topLevelItem(ui->treeWidgetSummary->topLevelItemCount()-i);
        for(int j = 1; j <= yearItem->childCount(); j++) {
            QTreeWidgetItem *monthItem = yearItem->child(yearItem->childCount()-j);
            mois.append(monthItem->text(0)+" "+yearItem->text(0));
            debitBar->append(0);
            creditBar->append(0);

            for(int k = 1; k <= monthItem->childCount(); k++) {
                QTreeWidgetItem *transaction = monthItem->child(monthItem->childCount()-k);

                if(transaction->childCount() > 0)
                {
                    for(int l = 1; l <= transaction->childCount(); l++)
                    {
                        QTreeWidgetItem *multiTrans = transaction->child(transaction->childCount()-l);
                        while(!multiTrans->text(6).contains(symboleCompte)) on_treeWidgetSummary_itemClicked(multiTrans,6);

                        QString cat2 = multiTrans->text(4);
                        if(cat2 == "") cat2 = tr("Vide");

                        QAction *catAction = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat->menu())->findAction(multiTrans->text(3));
                        QAction *cat2Action = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat2->menu())->findAction(cat2);
                        if(catAction && catAction->isChecked() && cat2Action && cat2Action->isChecked())
                        {
                            isDebit = (transaction->text(2) == tr("Débit") && QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').toDouble() > 0) || (transaction->text(2) == tr("Crédit") && QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').toDouble() < 0);
                            (isDebit?debitBar:creditBar)->replace((isDebit?debitBar:creditBar)->count()-1, (isDebit?debitBar:creditBar)->at((isDebit?debitBar:creditBar)->count()-1) + QString(multiTrans->text(6).split(" ").at(0)).replace(',','.').remove("-").toDouble());
                        }
                    }
                }
                else
                {
                    while(!transaction->text(6).contains(symboleCompte)) on_treeWidgetSummary_itemClicked(transaction,6);

                    QString cat2 = transaction->text(4);
                    if(cat2 == "") cat2 = tr("Vide");

                    QAction *catAction = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat->menu())->findAction(transaction->text(3));
                    QAction *cat2Action = qobject_cast<CustomMenu*>(ui->pushButtonFiltre_cat2->menu())->findAction(cat2);
                    if(catAction && catAction->isChecked() && cat2Action && cat2Action->isChecked())
                    {
                        isDebit = (transaction->text(2) == tr("Débit") && QString(transaction->text(6).split(" ").at(0)).replace(',','.').toDouble() > 0) || (transaction->text(2) == tr("Crédit") && QString(transaction->text(6).split(" ").at(0)).replace(',','.').toDouble() < 0);
                        (isDebit?debitBar:creditBar)->replace((isDebit?debitBar:creditBar)->count()-1, (isDebit?debitBar:creditBar)->at((isDebit?debitBar:creditBar)->count()-1) + QString(transaction->text(6).split(" ").at(0)).replace(',','.').remove("-").toDouble());
                    }
                }
            }
        }
    }

    debitBar->setColor(QColor(255, 0, 0));
    creditBar->setColor(QColor(0, 255, 0));
    series->append(debitBar);
    series->append(creditBar);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setLocale(locale);
    if(series->count() == 0) chart->setTitle(tr("Aucune transaction avec ces filtres"));
    else chart->setTitle(tr("Budget"));

    chart->setTheme(QChart::ChartThemeBrownSand);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%i");
    axisY->setTitleText("");
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(mois);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    axisX->setLabelsAngle(-25);
    axisX->setTruncateLabels(false);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setRubberBand(QChartView::RectangleRubberBand);

    //Recadrer sur les 6 derniers mois
    axisX->setMin(mois[mois.count() - (mois.count() > 6 ? 6 : mois.count())]);

    ui->gridLayout_stats->addWidget(chartView,1,1,1,2);

    //Boutons d'actions
    QPushButton *fullScreenButton = new QPushButton("⛶");
    fullScreenButton->setFixedWidth(40);
    fullScreenButton->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
    QGraphicsProxyWidget *fullScreenProxy = chartView->scene()->addWidget(fullScreenButton);

    QPushButton *minimizeButton = new QPushButton("▬");
    minimizeButton->setFixedWidth(40);
    minimizeButton->hide();
    minimizeButton->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
    QGraphicsProxyWidget *minimizeProxy = chartView->scene()->addWidget(minimizeButton);

    QPushButton *resetButton = new QPushButton("⤺");
    resetButton->setFixedWidth(40);
    resetButton->setStyleSheet("color:black; font-size:26px; background-color: rgba(0, 0, 0, 0);");
    QGraphicsProxyWidget *resetProxy = chartView->scene()->addWidget(resetButton);

    connect(series, &QBarSeries::hovered, this, &OverAid::stats_debitCredit_hover);

    connect(resetButton, &QPushButton::clicked, [=]() {
        axisX->setMin(mois[0]);
        chart->zoomReset();
        resetButton->hide();
    });
    connect(axisX, &QBarCategoryAxis::rangeChanged, [=]() {resetButton->show();});
    connect(axisY, &QValueAxis::rangeChanged, [=]() {resetButton->show();});

    connect(fullScreenButton, &QPushButton::clicked, [=]() {
        setFullScreen(chartView);
        fullScreenButton->hide();
        minimizeButton->show();
    });

    connect(minimizeButton, &QPushButton::clicked, [=]() {
        restoreFullScreen();
        fullScreenButton->show();
        minimizeButton->hide();
    });

    connect(chart, &QChart::geometryChanged, [=]() {
        fullScreenProxy->setPos(chart->geometry().topRight() - QPointF(fullScreenButton->width(), 0));
        minimizeProxy->setPos(chart->geometry().topRight() - QPointF(minimizeButton->width(), 0));
        resetProxy->setPos(chart->geometry().topRight() - QPointF(resetButton->width(), 0) - (QPointF(fullScreenButton->width(), 0)));
    });
}

void OverAid::stats_debitCredit_hover(bool status, int index, QBarSet *barSet)
{
    if(status)
    {
        QString symboleCompte;
        QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
        if(devise.next()) symboleCompte = devise.value("symbole").toString();

        qobject_cast<QChartView*>(ui->gridLayout_stats->itemAtPosition(1,1)->widget())->chart()->setTitle(barSet->label()+" : "+QString::number(barSet->at(index),'f',2)+" "+symboleCompte);
    }
    else qobject_cast<QChartView*>(ui->gridLayout_stats->itemAtPosition(1,1)->widget())->chart()->setTitle(tr("Budget"));
}

void OverAid::setFullScreen(QWidget *widget)
{
    if (!widget) return;

    int widgetRow = -1, widgetCol = -1;

    // Trouver les indices de ligne et colonne du widget
    for(int row = 0; row < ui->gridLayout_stats->rowCount(); row++)
    {
        for(int col = 0; col < ui->gridLayout_stats->columnCount(); ++col)
        {
            QLayoutItem *item = ui->gridLayout_stats->itemAtPosition(row, col);
            if(item && item->widget() == widget)
            {
                widgetRow = row;
                widgetCol = col;
                break;
            }
        }
        if(widgetRow != -1) break;
    }
    if(widgetRow == -1 || widgetCol == -1) return;

    for(int row = 0; row < ui->gridLayout_stats->rowCount(); row++) ui->gridLayout_stats->setRowStretch(row, (row == widgetRow) ? 100 : 0);
    for(int col = 0; col < ui->gridLayout_stats->columnCount(); col++) ui->gridLayout_stats->setColumnStretch(col, (col == widgetCol) ? 100 : 0);

    for(int i = 0; i < ui->gridLayout_stats->count(); ++i)
        if(ui->gridLayout_stats->itemAt(i)->widget() && ui->gridLayout_stats->itemAt(i)->widget() != widget) ui->gridLayout_stats->itemAt(i)->widget()->hide();
}

void OverAid::restoreFullScreen()
{
    for(int row = 0; row < ui->gridLayout_stats->rowCount(); row++) ui->gridLayout_stats->setRowStretch(row, 100);
    for(int col = 0; col < ui->gridLayout_stats->columnCount(); col++) ui->gridLayout_stats->setColumnStretch(col, 100);

    for(int i = 0; i < ui->gridLayout_stats->count(); ++i)
        if(ui->gridLayout_stats->itemAt(i)->widget()) ui->gridLayout_stats->itemAt(i)->widget()->show();
}
