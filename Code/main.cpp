#include "overaid.h"
#include "ui_overaid.h"

#include <QSharedMemory>
#include <QApplication>
#include <QTranslator>
#include <QSqlQuery>

QPlainTextEdit *logTextEdit;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    QString logMessage = QString("%1 %2").arg(localMsg.constData(), file);

    if(logTextEdit) logTextEdit->appendPlainText(logMessage);

    fprintf(stderr, "%s\n", localMsg.constData());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageOutput);
    QTranslator translator, translatorQtBase;
    QSharedMemory sharedMemory("OverAid");

    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, QObject::tr("Instance déjà en cours"), QObject::tr("L'application est déjà ouverte."), QMessageBox::Close);
        return 0;
    }

    //Création du fichier de langage
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).exists() || !QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/translate.txt").exists())
    {
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/translate.txt");

        if (file.open(QIODevice::ReadWrite))
        {
            file.resize(0);
            QTextStream stream(&file);

            QString langue = "English";
            if(QLocale::languageToString(QLocale::system().language()) == "French") langue = "Français";
            else if(QLocale::languageToString(QLocale::system().language()) == "Spanish") langue = "Español";

            stream << langue;
            file.close();
        }
    }

    //Lecture du langage sélectionné
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/translate.txt");

    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        QString line = stream.readLine();

        if(line == "Français")
        {
            translatorQtBase.load(":/qrc/ressources/translations/qtbase_fr.qm");
            qDebug() << "La langue a été modifiée en Français";
        }
        else if(line == "English") {
            translator.load(":/qrc/ressources/translations/english.qm");
            qDebug() << "Language has been changed to English";
        }
        else if(line == "Español") {
            translatorQtBase.load(":/qrc/ressources/translations/qtbase_es.qm");
            translator.load(":/qrc/ressources/translations/spanish.qm");
            qDebug() << "El idioma ha sido cambiado a Español.";
        }

        a.installTranslator(&translator);
        a.installTranslator(&translatorQtBase);
        QLocale::setDefault(QLocale(translator.language()) != QLocale::C ? QLocale(translator.language()) : QLocale("fr_FR"));

        file.close();
    }

    OverAid w;
    logTextEdit = w.ui->plainTextEdit_debug;

    w.database();

    QVersionNumber versionDB, versionApp;
    versionApp = QVersionNumber::fromString(w.version);
    QSqlQuery settings("SELECT version,language FROM Settings");
    if(settings.next()) {
        //Version
        if(QVersionNumber::fromString(settings.value("version").toString()) > versionApp) {
            QMessageBox::warning(nullptr, QObject::tr("Erreur version"), QObject::tr("La version de l'application est obsolète.\nMerci d'utiliser une version plus récente."), QMessageBox::Close);
            return -1;
        }

        //Langage sélectionné
        foreach(QAction *action, w.ui->menuLangage->actions())
            if(action->isCheckable()) action->setChecked(false);
        if(settings.value("language").toString() == "Français") w.ui->actionFran_ais->setChecked(true);
        else if(settings.value("language").toString() == "English") w.ui->actionAnglais->setChecked(true);
        else if(settings.value("language").toString() == "Español") w.ui->actionEspagnol->setChecked(true);

        w.locale = QLocale(translator.language()) != QLocale::C ? QLocale(translator.language()) : QLocale("fr_FR");
    }

    w.actu_settings();
    w.show();

    a.setApplicationName("OverAid");
    //a.setOrganizationName("Tom BEBIN");
    a.setOrganizationDomain("tombebin.com");
    a.setApplicationVersion(w.version);

    return a.exec();
}
