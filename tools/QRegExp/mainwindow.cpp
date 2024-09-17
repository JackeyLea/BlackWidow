#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadUserAgent();
    manager = new QNetworkAccessManager;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete reply;
    delete manager;
}

void MainWindow::on_getButton_clicked()
{
    ui->plainTextEdit->clear();
    if(ui->lineURL->text().isEmpty()){
        QMessageBox::information(this,tr("Tips"),tr("Please input URL in the lineEdit"),QMessageBox::Ok);
        return;
    }
    manager->clearConnectionCache();
    manager->clearAccessCache();
    request.setUrl(ui->lineURL->text());
    request.setRawHeader("User-Agent",UserAgent[randomNum(900)].toLatin1());
    request.setRawHeader("Keep-Alive","300");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    request.setRawHeader("Accept-Language","zh-CN,en-US,zh,en;q=0.9");
    request.setRawHeader("Accept-Encoding","deflate");
    request.setRawHeader("Referer","https://cn.bing.com/");
    reply = manager->get(request);
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<statusCode;
    if(statusCode==200){
        QByteArray ba = reply->readAll();
        QTextCodec *codec=QTextCodec::codecForName("UTF-8");
        QString html = codec->toUnicode(ba);
        int pos=0;
        QString result;
        QRegExp reg("charset=(.+)\"");
        if(html.isEmpty()|reg.isEmpty()) return;
        reg.setMinimal(true);
        while((pos=reg.indexIn(html,pos))!= -1){
                    result = reg.cap(1);
                    pos += reg.matchedLength();
        }
        qDebug()<<result;
        codec=QTextCodec::codecForName(result.toLocal8Bit());
        QString html2 = codec->toUnicode(ba).toLocal8Bit();
        ui->plainTextEdit->appendPlainText(html2);
        QPalette pa=ui->plainTextEdit->palette();
        pa.setColor(QPalette::Text,Qt::black);
        ui->plainTextEdit->setPalette(pa);
    }
}

void MainWindow::on_checkButton_clicked()
{
    QString str;
    int count=0;
    if(ui->lineExp->text().isEmpty()|ui->plainTextEdit->toPlainText().isEmpty()){
        QMessageBox::information(this,tr("tips"),tr("Please input regexp"),QMessageBox::Ok);
        return;
    }
    QRegExp testReg(ui->lineExp->text());
    if(testReg.isValid()) qDebug()<<"test regext is valid";
    int pos=0;
    testReg.setMinimal(true);
    while((pos=testReg.indexIn(ui->plainTextEdit->toPlainText(),pos))!=-1){
      str+=testReg.cap(1)+'\n';
      count+=testReg.captureCount();
      pos+=testReg.matchedLength();
    }
    ui->plainTextEdit->clear();
    ui->statusBar->showMessage(QString::number(count,10));
    ui->plainTextEdit->appendPlainText(str);
    QPalette pa=ui->plainTextEdit->palette();
    pa.setColor(QPalette::Text,Qt::blue);
    ui->plainTextEdit->setPalette(pa);
}

QString MainWindow::loadRegExp(QString website)
{
    QString regcontent;
    QString regfile=QApplication::applicationDirPath()+QDir::separator()+"reg"+QDir::separator()+website+".reg";
    QFile file(regfile);
    if(file.exists()){
        if(file.open(QIODevice::Text|QIODevice::ReadOnly)){
            QTextStream readIn(&file);
            while(!readIn.atEnd()){
                regcontent+=readIn.readLine();
            }
        }
        file.close();
    }
    return regcontent;
}

void MainWindow::loadUserAgent()
{
    int i=0;
    QFile file(":/resources/ini/user_agents.ini");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream readIn(&file);
        while(!readIn.atEnd()){
            UserAgent[i]=readIn.readLine();
            i++;
        }
    }
    file.close();
}

void MainWindow::sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

int MainWindow::randomNum(int base)
{
    static QTime t=QTime::currentTime();//time 1
    QTime T=QTime::currentTime();//time 2
    int i=T.msecsTo(t);
    qsrand(i);
    int randNum=qrand();
    t=T;
    return randNum%base;
}
