#ifndef OVERAID_H
#define OVERAID_H

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QDateTime>
#include <QCompleter>
#include <QListWidgetItem>
#include <QtCharts>
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

    QString version = "2.2.5";
    QLocale locale;

    void database();

protected:
    void closeEvent(QCloseEvent *event) override;

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
    void actu_projet();
    void actu_savedFilters();
    void actu_settings();
    void actu_langage(QString langage);
    void update();
    void changeAccount(QAction *actionClicked);

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
    void showAllColumns();
    void hideThisColumn();

private slots:
    //Actions TreeWidget
    void on_treeWidgetSummary_itemClicked(QTreeWidgetItem *item, int column);
    void treeWidgetMenu(const QPoint &po);

    void on_actionCat_gories_triggered();
    void on_actionGerer_les_comptes_triggered();

    //Langues
    void on_actionAnglais_triggered();
    void on_actionFran_ais_triggered();
    void on_actionEspagnol_triggered();

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

    void on_groupBox_filtre_toggled(bool arg1);

    void on_actionAfficher_le_d_bugger_triggered();
    void on_action_about_triggered();

    void on_actionImporter_une_base_de_donn_es_triggered();
    bool importDatabase();
    void on_actionImport_en_masse_triggered();
    void on_actionTest_de_vitesse_triggered();

private:
    QSqlDatabase overaidDatabase;
    int id_compte = -1;
    bool active = true;

    TransForm *trans = new TransForm(nullptr, "Ajouter");

    QList<int> id_savedFilters;

    QList<int> id_categories;
    QList<QString> id_sousCategories;
    QList<bool> previousActionsCatState;

    QString where;

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

    //Devises
    //Liste en Janvier 2024
    QStringList devisesList = (QStringList() << "AED-United Arab Emirates Dirham-AED" << "AFN-افغانۍ-؋" << "ALL-Leku shqiptar-Lekë" << "AMD-հայկական դրամ-֏" << "ANG-Netherlands Antillean Guilder-NAf." << "AOA-Kwanza ya Angóla-Kz" << "ARS-peso argentino-$" << "AUD-Australian Dollar-$" << "AWG-Arubaanse gulden-Afl." << "AZN-Azərbaycan Manatı-₼" << "BAM-Bosanskohercegovačka konvertibilna marka-KM" << "BBD-Barbadian Dollar-$" << "BDT-বাংলাদেশী টাকা-৳" << "BGN-Български лев-лв." << "BHD-دينار بحريني-د.ب.\u200F" << "BIF-Burundian Franc-FBu" << "BMD-Bermudian Dollar-$" << "BND-دولر بروني-$" << "BOB-Boliviano-Bs" << "BRL-Real brasileiro-R$" << "BSD-Bahamian Dollar-$" << "BTN-དངུལ་ཀྲམ-Nu." << "BWP-Botswanan Pula-P" << "BYN-беларускі рубель-Br" << "BZD-Belize Dollar-$" << "CAD-Canadian Dollar-$" << "CDF-franc congolais-FC" << "CHF-franc suisse-CHF" << "CLP-Peso chileno-$" << "CNY-人民币-￥" << "COP-peso colombiano-$" << "CRC-colón costarricense-₡" << "CUP-peso cubano-$" << "CVE-Skudu Kabuverdianu-\u200B" << "CZK-česká koruna-Kč" << "DJF-فرنك جيبوتي-Fdj" << "DKK-Danish Krone-kr." << "DOP-peso dominicano-RD$" << "DZD-دينار جزائري-د.ج.\u200F" << "EGP-جنيه مصري-ج.م.\u200F" << "ERN-Eritrean Nakfa-Nfk" << "ETB-የኢትዮጵያ ብር-ብር" << "EUR-Euro-€" << "FJD-Fijian Dollar-$" << "FKP-Falkland Islands Pound-£" << "GBP-UK Pound-£" << "GEL-ქართული ლარი-₾" << "GHS-Ghanaian Cedi-GH₵" << "GIP-Gibraltar Pound-£" << "GMD-Gambian Dalasi-D" << "GNF-franc guinéen-FG" << "GTQ-quetzal-Q" << "GYD-Guyanaese Dollar-$" << "HKD-Hong Kong Dollar-HK$" << "HNL-lempira hondureño-L" << "HTG-gourde haïtienne-G" << "HUF-magyar forint-Ft" << "IDR-Rupiah Indonesia-Rp" << "ILS-Israeli New Shekel-₪" << "INR-Indian Rupee-₹" << "IQD-دينار عراقي-د.ع.\u200F" << "IRR-ایران ریال-IRR" << "ISK-íslensk króna-kr." << "JMD-Jamaican Dollar-$" << "JOD-دينار أردني-د.أ.\u200F" << "JPY-日本円-￥" << "KES-Kenyan Shilling-Ksh" << "KGS-Кыргызстан сому-сом" << "KHR-រៀល\u200Bកម្ពុជា-៛" << "KMF-فرنك جزر القمر-CF" << "KPW-조선 민주주의 인민 공화국 원-KPW" << "KRW-대한민국 원-₩" << "KWD-دينار كويتي-د.ك.\u200F" << "KYD-Cayman Islands Dollar-$" << "KZT-Қазақстан теңгесі-₸" << "LAK-ລາວ ກີບ-₭" << "LBP-جنيه لبناني-ل.ل.\u200F" << "LKR-ශ්\u200Dරී ලංකා රුපියල-රු." << "LRD-Liberian Dollar-$" << "LYD-دينار ليبي-د.ل.\u200F" << "MAD-درهم مغربي-د.م.\u200F" << "MDL-leu moldovenesc-L" << "MGA-Malagasy Ariary-Ar" << "MKD-Denari maqedonas-den" << "MMK-မြန်မာ ကျပ်-K" << "MNT-Монгол төгрөг-₮" << "MOP-Macanese Pataca-MOP$" << "MRU-أوقية موريتانية-أ.م." << "MUR-Mauritian Rupee-Rs" << "MVR-Maldivian Rufiyaa-Rf" << "MWK-Malawian Kwacha-MK" << "MXN-peso mexicano-$" << "MYR-Malaysian Ringgit-RM" << "MZN-metical moçambicano-MTn" << "NAD-Namibian Dollar-$" << "NGN-Nigerian Naira-₦" << "NIO-córdoba nicaragüense-C$" << "NOK-norgga kruvdno-kr" << "NPR-नेपाली रूपैयाँ-नेरू" << "NZD-New Zealand Dollar-$" << "OMR-ريال عماني-ر.ع.\u200F" << "PAB-balboa panameño-B/." << "PEN-Sol Peruano-S/" << "PGK-Papua New Guinean Kina-K" << "PHP-Philippine Peso-₱" << "PKR-Pakistani Rupee-Rs" << "PLN-złoty polski-zł" << "PYG-guaraní paraguayo-Gs." << "QAR-ريال قطري-ر.ق.\u200F" << "RON-leu românesc-RON" << "RSD-српски динар-RSD" << "RUB-Российн сом-₽" << "RWF-Rwandan Franc-RF" << "SAR-ريال سعودي-ر.س.\u200F" << "SBD-Solomon Islands Dollar-$" << "SCR-Seychellois Rupee-SR" << "SDG-جنيه سوداني-ج.س." << "SEK-Swedish Krona-kr" << "SGD-Singapore Dollar-$" << "SHP-St Helena Pound-£" << "SLE-Sierra Leonean Leone-Le" << "SOS-شلن صومالي-S" << "SRD-Surinaamse dollar-$" << "SSP-South Sudanese Pound-£" << "STN-dobra de São Tomé e Príncipe-Db" << "SYP-ليرة سورية-ل.س.\u200F" << "SZL-Swazi Lilangeni-E" << "THB-บาท-฿" << "TJS-Сомонӣ-сом." << "TMT-Türkmen manady-TMT" << "TND-دينار تونسي-د.ت.\u200F" << "TOP-Tongan Paʻanga-T$" << "TRY-Türk Lirası-₺" << "TTD-Trinidad & Tobago Dollar-$" << "TWD-新台幣-$" << "TZS-Tanzanian Shilling-TSh" << "UAH-украинская гривна-₴" << "UGX-Ugandan Shilling-USh" << "USD-US Dollar-$" << "UYU-peso uruguayo-$" << "UZS-O‘zbekiston so‘mi-soʻm" << "VES-bolívar soberano-Bs.S" << "VND-Đồng Việt Nam-₫" << "VUV-Vanuatu Vatu-VT" << "WST-Samoan Tala-WS$" << "XAF-Central African CFA Franc-FCFA" << "XCD-East Caribbean Dollar-$" << "XOF-sefa Fraŋ (BCEAO)-F CFA" << "XPF-franc CFP-FCFP" << "YER-ريال يمني-ر.ي.\u200F" << "ZAR-South African Rand-R" << "ZMW-Zambian Kwacha-K");
};
#endif // OVERAID_H
