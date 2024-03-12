#ifndef OVERAID_H
#define OVERAID_H

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDateTime>
#include <QCompleter>
#include <QListWidgetItem>
#include <QtCharts>
#include <QWebEngineView>
#include "Classes/RangeSlider.h"
#include "transform.h"
#include "ui_overaid.h"
#include "ui_transform.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OverAid; }
QT_END_NAMESPACE

class OverAid : public QMainWindow
{
    Q_OBJECT

public:
    OverAid(QWidget *parent = nullptr);
    Ui::OverAid *ui;
    ~OverAid();

    QString language;

    void database();

public slots:
    QWidgetAction* createTextSeparator(const QString& text);

    void actu(bool remember, bool actuCat);
    void actu_compte();
    void actu_categorie();
    void actu_sousCategorie();
    void actu_devise();
    void actu_filtre();
    void actu_historique();
    void actu_desc();
    void actu_savedFilters();
    void update();

    void stats();
    void stats_dates();
    void stats_hover(const QPointF &point, bool state);

    void tout_etendre();
    void tout_retrecir();
    void copy_case();
    void copy_line();
    void transToSub(bool avecPDF);
    void modify_trans();
    void delete_trans();
    void duplicate();
    void changeAccount(QAction *actionClicked);

private slots:
    //Actions TreeWidget
    void on_treeWidgetSummary_itemClicked(QTreeWidgetItem *item, int column);
    void treeWidgetMenu(const QPoint &po);

    void on_actionCat_gories_triggered();
    void on_actionAjouter_un_compte_triggered();

    //Langues
    void on_actionAnglais_triggered();
    void on_actionFran_ais_triggered();

    void on_actionFermer_triggered();
    void on_actionExporter_au_format_CSV_triggered();
    void on_actionSauvegarder_la_base_de_donn_es_triggered();
    void on_actionExporter_les_PDF_en_ZIP_triggered();
    void on_actionAbonnements_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_pushButtonFiltre_valide_clicked();
    void on_pushButtonFiltre_cat_pressed();
    void on_pushButtonFiltre_delete_clicked();
    void on_comboBox_savedFilters_currentTextChanged();

private:
    QSqlDatabase overaidDatabase;
    int id_compte = -1;
    bool active = true;
    bool updated = false;

    TransForm *trans = new TransForm(nullptr, "Ajouter");

    QList<int> id_savedFilters;

    QList<int> id_categories;
    QList<QString> id_sousCategories;
    QList<bool> previousActionsCatState;

    QString where;
    QLocale locale;

    QList<QString> rememberOpenedYears, rememberOpenedMonth, rememberOpenedItems;
    QList<QString> rememberOpenedYearsID, rememberOpenedMonthID, rememberOpenedItemsID;
    int rememberPosition = 0;

    //Stats
    QList<QString> statsList;
    QLineSeries *series = new QLineSeries();
    RangeSlider *rangeSlider;
    QList<QDateTime> dates;
    QList<double> solde;
    QDateTimeAxis *axisX = new QDateTimeAxis;
    QValueAxis *axisY = new QValueAxis;
};
#endif // OVERAID_H
