#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkSession>
#include <QMainWindow>
#include <QMessageBox>
#include <QEventLoop>
#include <QPalette>
#include <QTextCodec>
#include <QDir>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest request;
    QEventLoop loop;
    QString UserAgent[900];
    QString loadRegExp(QString website);
    void loadUserAgent();
    void sleep(int msec);
    int randomNum(int base);

private slots:
    void on_getButton_clicked();

    void on_checkButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
