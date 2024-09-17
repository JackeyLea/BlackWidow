#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base.hpp"
#include "dbprocessor.hpp"
#include "jsonparser.hpp"
#include "spider.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void displayDetailById(int qid);

public slots:
    void countChanged(int count);
    void updateModel(QList<QStandardItem *> rowList);

private slots:
    void dataProcessor();
    void displayMsg(QString msg);
    void on_actionReset_triggered();

    void on_actionExit_triggered();

    void on_actionStart_End_triggered();

    void on_actionPrevious_triggered();

    void on_actionNext_triggered();

    void on_actionClear_triggered();

    void on_actionDone_triggered();

    void on_actionSpider_triggered();

    void on_actionQuestion_triggered();

    void on_actionAbout_triggered();

    void on_actionQt_triggered();

    void on_actionGithub_triggered();

    void on_spiderButtonStart_clicked();

    void on_spiderButtonStartAll_clicked();

    void on_spiderButtonStop_clicked();

    void on_spiderComboPlatform_currentIndexChanged(int index);

    void on_actionTip_triggered();

    void on_actionAnswer_triggered();

private:
    Ui::MainWindow *ui;
    DBProcessor *dbp;
    JsonParser *jsonp;
    Spider *ojspider;

    data_ spiderData;

    QStandardItemModel *model;

    QString dbPath;
    QString sql;//数据库语句

    int currentId;//现在正在处理的ID
    int idMax;//最大ID值
    bool isStart;
    bool isNext;//如果当前为空且isNext为真则获取下一题，否则获取上一题
};
#endif // MAINWINDOW_H
