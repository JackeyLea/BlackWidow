#ifndef BASE_HPP
#define BASE_HPP

#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTableView>
#include <QTextBrowser>
#include <QTextCodec>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QWidget>

#define IndexUrl "https://github.com/JackeyLea/BlackWidow"

//数据库表列名结构体
struct data_ {
    int     id;
    int     platform;
    int     level;
    QString name;
    QString question;
    QString tip;
    QString answer;
    QString submit;
    QString passed;
    QString rate;
    QString cata;
};

//配置文件中的值名称
struct conf_ {
    bool status=false;//此结构体是否可以使用
    int     begin;
    QString url;
    QString name;
    QString detail;
    QString level;
    QString cata;
    QString language;
    QString submit;
    QString passed;
    QString rate;

    void clear(){
        begin=0;
        url.clear();
        name.clear();
        detail.clear();
        level.clear();
        cata.clear();
        language.clear();
        submit.clear();
        passed.clear();
        rate.clear();
    }
    void operator = (const conf_& b) {
        status = b.status;
        begin = b.begin;
        url = b.url;
        name = b.name;
        detail = b.detail;
        level = b.level;
        cata = b.cata;
        language = b.language;
        submit = b.submit;
        passed = b.passed;
        rate = b.rate;
    }
};

//加载皮肤
inline QString loadTheme(QString themeName) {
    QString themeCtx;
    QFile   file(QString(":/resources/themes/%1.qss").arg(themeName));
    qDebug() << "theme file path is:" << file.fileName();
    if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
        QTextStream readIn(&file);
        themeCtx = readIn.readAll();
    } else {
        qDebug() << "Cannot open file: " << file.errorString();
        qDebug() << "Application will start with no theme";
    }
    file.close();

    return themeCtx;
}

//产生伪随机数
inline int randomNum(int baseNum) {
    static QTime t = QTime::currentTime(); // time 1
    QTime        T = QTime::currentTime(); // time 2
    int          i = T.msecsTo(t);
    qsrand(static_cast<unsigned int>(i));
    int randNum = qrand();
    t           = T;
    return randNum % baseNum;
}

//非阻塞延时
inline void sleep(int msec) {
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
//日志函数
inline void log(QString logger) {
    QDateTime dt       = QDateTime::currentDateTime();
    QString   datetime = dt.toString("yyyy-MM-dd hh:mm:ss");
    QFile     file("blackwidow.log");
    if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)) {
        QTextStream writeOut(&file);
        writeOut.setCodec("UTF-8");
        writeOut << datetime + " " + logger;
    }
    file.close();
}

#endif // BASE_HPP
