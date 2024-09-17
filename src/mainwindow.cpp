#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QStandardItemModel())
    , currentId(1)
    , isStart(false)
{
    ui->setupUi(this);
    qDebug()<<"Main::MainWIndow()";
    log("Main process started");
    //加载配置文件
    QString configPath = QDir::currentPath()+QDir::separator()+"config.json";
    jsonp = new JsonParser(configPath);
    dbPath = jsonp->getValue("db");
    qDebug()<<"db path in config is: "<<dbPath;
    QString theme = jsonp->getValue("theme");
    this->setStyleSheet(loadTheme(theme));

    dbp = new DBProcessor("main",dbPath);
    idMax = dbp->getMaxId("data");//获取数据库中最大的ID
    ui->QLineId->setText(QString::number(idMax));//界面上面的ID栏
    //添加平台数据
    jsonp->openJsonFile(QDir::currentPath()+QDir::separator()+"website.json");
    QStringList platform = jsonp->loadPlatform();
    ui->QCBPlatform->addItems(platform);//添加平台名称至下拉列表
    ui->spiderComboPlatform->addItems(platform);

    ojspider = new Spider();
    connect(ojspider,&Spider::countChanged,this,&MainWindow::countChanged);
    connect(ojspider,&Spider::displayMsg,this,&MainWindow::displayMsg);
    connect(ojspider,&Spider::updateModel,this,&MainWindow::updateModel);

    //初始化界面
    QString jsonFilePath = QDir::currentPath() + QDir::separator() + "website.json";
    jsonp = new JsonParser(jsonFilePath);

    //设置表格界面
    model->setColumnCount(3);
    model->setHorizontalHeaderItem(0,new QStandardItem("name"));
    model->setHorizontalHeaderItem(1,new QStandardItem("platform"));
    model->setHorizontalHeaderItem(2,new QStandardItem("id"));
    ui->spiderTableView->horizontalHeader()->setStretchLastSection(true);
    ui->spiderTableView->setModel(model);
}

MainWindow::~MainWindow()
{
    qDebug()<<"Main::~MainWindow()";
    delete dbp;
    delete jsonp;
    delete ui;
}

void MainWindow::displayDetailById(int qid)
{
    currentId = qid;//表示当前操作ID号
    qDebug()<<"Current display id is: "<<qid;
    if(qid>idMax || qid<0){
        qDebug()<<"The question id is wrong";
        return;
    }
     qDebug()<<"Current display question id: "<<currentId;
     sql = QString("select * from data where id=%1").arg(currentId);
     dbp->query->prepare(sql);
     if(dbp->query->exec()){
         while(dbp->query->next()){
             spiderData.level = dbp->query->value("level").toInt();
             spiderData.question = dbp->query->value("detail").toString();
             spiderData.answer = dbp->query->value("answer").toString();
             spiderData.tip = dbp->query->value("tip").toString();
             spiderData.id = currentId;
             spiderData.name = dbp->query->value("name").toString();
             spiderData.passed = dbp->query->value("passed").toString();
             spiderData.rate = dbp->query->value("rate").toString();
             spiderData.submit = dbp->query->value("submit").toString();
             spiderData.platform = dbp->query->value("platform").toInt();
             spiderData.cata = dbp->query->value("cata").toString();
         }
     }
     else{
         qDebug()<<"Cannot query question detail data: "<<dbp->query->lastError();
     }

     if(spiderData.question.isEmpty()){
         if(isNext){
             displayDetailById(qid+1);
         }
         else{
             displayDetailById(qid-1);
         }
     }

     on_actionClear_triggered();
     int level = spiderData.level-1;
     if(level<0) level = 8;
     ui->QCBLevel->setCurrentIndex(level);
     ui->QTextQuestion->setHtml(spiderData.question);
     ui->QLineId->setText(QString::number(currentId));
     ui->QLineName->setText(spiderData.name);
     ui->QLinePassed->setText(spiderData.passed);
     ui->QLineRate->setText(spiderData.rate);
     ui->QLineSubmit->setText(spiderData.submit);
     ui->QCBPlatform->setCurrentIndex(spiderData.platform);
     ui->QPlainTextCata->setPlainText(spiderData.cata);

     QString newTitle=QString("id: %1 level: %2").arg(currentId).arg(spiderData.level);
     this->setWindowTitle(newTitle);
}

void MainWindow::countChanged(int count)
{
    ui->spiderLcdNumber->display(count);
}

void MainWindow::updateModel(QList<QStandardItem *> rowList)
{
    model->appendRow(rowList);
    ui->spiderTableView->setModel(model);
}

void MainWindow::dataProcessor()
{
    int result = 0;
    qDebug()<<"result value is: "<<result;
    if(result >0){
        QString tip = QString("Current question is same to id: %1").arg(result);
        QMessageBox::information(this,tr("Warning"),tip,QMessageBox::Ok);
        on_actionClear_triggered();
        return;
    }
    else{
        idMax = dbp->getMaxId("data");
        sql = "insert into data values(:id,:platform,:name,:question,:answer,:tip,:cata,:type,:level)";
        dbp->query->prepare(sql);
        dbp->query->bindValue(":id",++idMax);
        int platform = ui->QCBPlatform->currentIndex()+1;
        dbp->query->bindValue(":platform",platform);
        dbp->query->bindValue(":name",ui->QLineName->text());
        dbp->query->bindValue(":question",ui->QTextQuestion->toPlainText());
        dbp->query->bindValue(":answer",ui->QTextAnswer->toPlainText());
        dbp->query->bindValue(":tip",ui->QTextTip->toPlainText());
        dbp->query->bindValue(":cata",ui->QPlainTextCata->toPlainText());
        int level = ui->QCBLevel->currentIndex()+1;
        dbp->query->bindValue(":level",level);
        dbp->query->bindValue(":id",idMax);
        if(dbp->query->exec()){
            qDebug()<<"Insert one question into table";
            on_actionClear_triggered();
            ui->QLineId->setText(QString::number(idMax));
        }
        else{
            qDebug()<<"Cannot insert data into table task: "<<dbp->query->lastError();
        }
    }
    //dbp->wait();
}

void MainWindow::displayMsg(QString msg)
{
    QMessageBox::information(this,tr("Tips"),msg,QMessageBox::Ok);
}

void MainWindow::on_actionReset_triggered()
{
    bool isOk = dbp->reset();
    if(isOk){
        QMessageBox::information(this,tr("Tips"),tr("Reset successfully"),QMessageBox::Ok);
    }
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit(0);
}

//开始或结束题目操作
void MainWindow::on_actionStart_End_triggered()
{
    log("Start/End action triggered");
    if(!isStart){
        qDebug()<<"Start examination";
        idMax = dbp->getMaxId("data");
        qDebug()<<"id max is: "<<idMax;
        displayDetailById(1);
        isStart = true;
        ui->stackedWidget->setCurrentIndex(1);
    }
    else {
        qDebug()<<"Examination end";
        isStart = false;
        spiderData.question.clear();
        spiderData.answer.clear();
        spiderData.tip.clear();
        on_actionClear_triggered();//清空界面
    }
}

//显示当前题号的上一题
void MainWindow::on_actionPrevious_triggered()
{
    if(!isStart){
        QMessageBox::information(this,tr("Tip"),tr("Please start first"),QMessageBox::Ok);
        return;
    }
    if(currentId<=1){
        QMessageBox::information(this,tr("Warning"),tr("Display the last question"),QMessageBox::Ok);
        displayDetailById(idMax);
    }
    else {
        displayDetailById(--currentId);
    }
    isNext = false;
}

//显示当前题号的下一题
void MainWindow::on_actionNext_triggered()
{
    if(!isStart){
        QMessageBox::information(this,tr("Tip"),tr("Please start first"),QMessageBox::Ok);
        return;
    }
    if(currentId>=idMax){
        QMessageBox::information(this,tr("Tip"),tr("We are in beginning"),QMessageBox::Ok);
        displayDetailById(1);
    }
    else {
        displayDetailById(++currentId);
    }

    isNext = true;
}

//清空界面内容
void MainWindow::on_actionClear_triggered()
{
    switch(ui->stackedWidget->currentIndex()){
    case 0:
        //设置表格界面
        model->clear();
        model->setColumnCount(3);
        model->setHorizontalHeaderItem(0,new QStandardItem("name"));
        model->setHorizontalHeaderItem(1,new QStandardItem("platform"));
        model->setHorizontalHeaderItem(2,new QStandardItem("id"));
        ui->spiderTableView->horizontalHeader()->setStretchLastSection(true);
        ui->spiderTableView->setModel(model);
        break;
    case 1:
        ui->QCBLevel->setCurrentIndex(0);
        ui->QCBPlatform->setCurrentIndex(0);
        ui->QLineId->setText(QString::number(dbp->getMaxId("data")));
        ui->QLineName->clear();
        ui->QLinePassed->clear();
        ui->QLineRate->clear();
        ui->QLineSubmit->clear();
        ui->QPlainTextCata->clear();
        ui->QTextAnswer->clear();
        ui->QTextQuestion->clear();
        ui->QTextTip->clear();
        break;
    default:
        break;
    }
}

//设置当前题目ID的状态，1表示会了
void MainWindow::on_actionDone_triggered()
{
    if(isStart){
        idMax = dbp->getMaxId("data");
        sql = QString("update data set status = '1' where id= %1").arg(currentId);
        dbp->query->prepare(sql);
        if(dbp->query->exec()){
            QMessageBox::information(this,tr("Tip"),tr("Set this question status: done"),QMessageBox::Ok);
        }
        else {
            qDebug()<<"Cannot set this question status to done: "<<dbp->query->lastError();
        }
        sql.clear();
    }
    else{
        QMessageBox::information(this,tr("Warning"),tr("Please start first"),QMessageBox::Ok);
        return;
    }
}

//跳转到爬虫界面
void MainWindow::on_actionSpider_triggered()
{
    qDebug()<<"Show spider widget";
    ui->stackedWidget->setCurrentIndex(0);
    this->setWindowTitle(tr("Spider"));
}

//跳转到问题界面
void MainWindow::on_actionQuestion_triggered()
{
    qDebug()<<"Show question widget";
    ui->stackedWidget->setCurrentIndex(1);
    this->setWindowTitle(tr("Question"));
    displayDetailById(currentId);
}

void MainWindow::on_actionAbout_triggered()
{
    QString aboutMsg =QString("<h1>BlackWidow</h1>"
                              "<h2>Author: JackeyLea</h2>"
                              "<h2>License: LGPL3</h2>"
                              "<h2>E-mail: 1768478912@qq.com</h2>"
                              "<h2>Phone: 13812991101</h2>"
                              "<h3>Please keep those message showing up</h3>");
    QMessageBox::information(this,tr("About"),aboutMsg,QMessageBox::Ok);
}

void MainWindow::on_actionQt_triggered()
{
    qApp->aboutQt();
}

void MainWindow::on_actionGithub_triggered()
{
    QDesktopServices::openUrl(QUrl(IndexUrl));
}

//开始当前平台的爬虫行为
void MainWindow::on_spiderButtonStart_clicked()
{
    qDebug()<<"Main::dbPath value is: "<<dbPath;
    QString platform = ui->spiderComboPlatform->currentText();
    qDebug()<<jsonp->getValue(platform,"status");
    qDebug()<<jsonp->getValue(platform,"valid");
    jsonp->updateValue(platform,"status","test");
//    log("Button start clicked : "+platform);
//    ojspider->Conf = jsonp->getPlatfromValues(platform);//重载运算符进行结构体赋值
//    qDebug()<<"Main::testing url is: "<<ojspider->Conf.url;

    //ojspider->setdbPath(dbPath);
    //ojspider->setPlatformName(platform);
   // ojspider->setEndMark(ojspider->Conf.end);
    //qDebug()<<"Main::spider dbPath is: "<<ojspider->dbPath;
    //ojspider->run();
    //ojspider->start();
    log("Button start running done");
}

//多线程获取数据，一次性获得所有平台数据
void MainWindow::on_spiderButtonStartAll_clicked()
{
}

//停止爬虫行为
void MainWindow::on_spiderButtonStop_clicked()
{
    ojspider->stop();
    log("Button stop clicked");
}

//如果平台下拉框被修改，那么就重置界面
void MainWindow::on_spiderComboPlatform_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString platform = ui->spiderComboPlatform->currentText();
    qDebug()<<"Change platform to: "<<platform;
    if(this->isVisible()){
        //设置表格界面
        model->clear();
        model->setColumnCount(3);
        model->setHorizontalHeaderItem(0,new QStandardItem("name"));
        model->setHorizontalHeaderItem(1,new QStandardItem("platform"));
        model->setHorizontalHeaderItem(2,new QStandardItem("id"));
        ui->spiderTableView->horizontalHeader()->setStretchLastSection(true);
        ui->spiderTableView->setModel(model);
    }
}
//显示当前题目ID的提示
void MainWindow::on_actionTip_triggered()
{
    if(!isStart) {
        QMessageBox::information(this,tr("Warning"),tr("Please start questin first"),QMessageBox::Ok);
        return;
    }
    if(spiderData.tip.isEmpty()) ui->QTextTip->setPlainText(QString("No tip"));
    else ui->QTextTip->setPlainText(spiderData.tip);
}
//显示当前题目ID的答案
void MainWindow::on_actionAnswer_triggered()
{
    if(!isStart) {
        QMessageBox::information(this,tr("Warning"),tr("Please start questin first"),QMessageBox::Ok);
        return;
    }
    if(spiderData.answer.isEmpty())
        ui->QTextAnswer->setPlainText(QString("There is no answer now,please complete this question by yourself"));
    else {
        ui->QTextAnswer->setPlainText(spiderData.answer);
    }
}
