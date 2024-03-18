#ifndef BULKIMPORT_H
#define BULKIMPORT_H

#include <QWidget>
#include <QScrollBar>

namespace Ui {
class BulkImport;
}

class BulkImport : public QWidget
{
    Q_OBJECT

public:
    explicit BulkImport(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event);
    ~BulkImport();

private slots:
    void on_pushButton_download_clicked();
    void on_pushButton_close_clicked();
    void on_pushButton_check_clicked();
    void on_pushButton_import_clicked();
    QString checkFields(QString line);
    bool addLine(QString line);

    void on_plainTextEdit_import_textChanged();

private:
    Ui::BulkImport *ui;

signals :
    void actualiser();
};

#endif // BULKIMPORT_H
