#ifndef SPIDER_HPP
#define SPIDER_HPP

#include "base.hpp"
#include "dbprocessor.hpp"
#include "jsonparser.hpp"
#include "similarity.hpp"

class Spider: public QObject,public Similarity{
    Q_OBJECT
public:
    QString dbPath;//数据库位置
    conf_ Conf;//配置文件数据结构体
    QString cntPlatformName;//当前的平台名称
    QString endMark;//用于判断是否获取结束

private:
    DBProcessor *dbp;

    QStandardItemModel *model;
    QString sql;
    //网络四件套
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QNetworkReply *reply;
    QEventLoop loop;

    QString UserAgent[900];//用于存储模拟头文件
    bool Stopped=false;//用于判断是否已经停止
    QList<int> idList;//当前操作的题目ID列表

    int id=-1;//数据库处理时所需要的ID

signals:
    void countChanged(int count);
    void displayMsg(QString msg);
    void updateModel(QList<QStandardItem *> rowList);

public:
    Spider():
    model(new QStandardItemModel()),
    manager(new QNetworkAccessManager()){
        qDebug()<<"Spider()::Spider()";
        //各个平台的题目结束标志
//        endMark.append(QString("题目不可用!!"));///常州实验初中
//        endMark.append(QString("No Such Problem"));///福州大学 杭州电子科技大学
//        endMark.append(QString("403"));///杭州电子科技大学
//        endMark.append(QString("题目不存在"));/*NYOJ*/
//        endMark.append(QString("Can not find problem"));///POJ
//        endMark.append(QString("No such problem"));///zoj
//        endMark.append(QString("Problem is not Available!!"));///zjut

        loadUserAgent();//加载浏览器模拟头
    }

    ~Spider(){
        qDebug()<<"Spider()::~Spider()";
        delete model;
        delete manager;
    }

    void setdbPath(QString path){
        dbPath =path;
    }

    void setPlatformName(QString name){
        cntPlatformName = name;
    }

    void setEndMark(QString end){
        endMark.clear();
        endMark = end;
    }

    void run()
    {
        if(dbPath.isEmpty()) {
            qDebug()<<"Didn't set the db path in spider start()";
            return;
        }
        //初始化变量
        dbp = new DBProcessor("spider",dbPath);

        if(!Conf.status){
            qDebug()<<"No valid configuration data";
            return;
        }
        //初始化爬虫
        int maxRowId = dbp->getMaxId("data");
        qDebug()<<"Max row id is: "<<maxRowId;
        int errorNum =0;//出错计数，连续出错十次就停止爬虫
        int count=0;//成功计数
        bool status = false;//是否获取到第一个题目
        int currentNum = Conf.begin;//当前正在处理的ID，从第一题开始
        qDebug()<<"Spider::Stopped status is: "<<Stopped;

        QString url = Conf.url;

        do{
            QString platformUrl = url + QString::number(currentNum);
            qDebug()<<"Spider::current url is: "<<platformUrl;
            QString context = getPlatformCtx(platformUrl);

            if(context.isEmpty()){
                qDebug()<<"HTML context is empty";
                sleep(500);
                errorNum++;
                currentNum++;
                continue;
            }
            else{
                sql.clear();

                //题目详细数据，用于计算MD5值
                QString detail = filterHtmlText(context,Conf.detail);
                //如果获取的题目数据为空，有可能是爬取速度太快了
                if(detail.isEmpty()){
                    sleep(500);
                    currentNum++;//获取下一题
                    errorNum++;
                    continue;
                }

                //插入数据库之前先进行相似度判断
                //先把富文本转换为文本
                QTextEdit text(detail);
                QString simSource = text.toPlainText();
                QString md5 = checkout(simSource);//计算字符串MD5值
                int count =-1;
                //先确定MD5值是否已经存在
                sql = QString("select count(*) from md5 where value=:value");
                dbp->query->prepare(sql);
                dbp->query->bindValue(":value",md5);
                if(dbp->query->exec()){
                    while (dbp->query->next()) {
                        count = dbp->query->value(0).toInt();
                    }
                }
                else{
                    qDebug()<<"Similarity::Cannot query data from table md5 : "<<dbp->query->lastError();
                }
                if(count>0){
                    qDebug()<<"Spider()::This question existed";
                    currentNum++;
                    continue;
                }
                else{//如果MD5值不存在就不MD5值插入数据库。
                    int maxID = dbp->getMaxId("md5");
                    sql = QString("insert into md5 values(:id,:value)");
                    dbp->query->prepare(sql);
                    dbp->query->bindValue(":id",++maxID);
                    dbp->query->bindValue(":value",md5);
                    if(dbp->query->exec()){
                        qDebug()<<"Similarity::Save value: "<<md5<<" into db";
                    }
                    else{
                        qDebug()<<"Similarity::Cannot insert data into db: "<<dbp->query->lastError();
                    }
                }
                //-->相似度判断完毕

                //保存数据到数据库
                sql = "insert into data values(:id,:platform,:name,:question,"
                      ":answer,:tip,:cata,:level,:submit,:passed,:rate,:language,:status)";
                dbp->query->prepare(sql);
                //id部分
                dbp->query->bindValue(":id",maxRowId+1);
                //平台名称
                dbp->query->bindValue(":platform",cntPlatformName);
                //题目名字
                QString name = filterHtmlText(context,Conf.name);
                dbp->query->bindValue(":name",name.remove(QRegExp("\t")));
                dbp->query->bindValue(":question",detail);
                //题目答案
                dbp->query->bindValue(":answer","N/A");
                //提示
                dbp->query->bindValue(":tip","N/A");
                QString cata = filterHtmlText(context,Conf.cata);
                dbp->query->bindValue(":cata",cata);
                QString level = filterHtmlText(context,Conf.level);                //难度等级
                dbp->query->bindValue(":level",level);
                //提交数
                QString submit = filterHtmlText(context,Conf.submit);
                dbp->query->bindValue(":submit",submit.toInt());
                //通过数
                QString passed = filterHtmlText(context,Conf.passed);
                dbp->query->bindValue(":passed",passed.toInt());
                //通过率
                QString rate = filterHtmlText(context,Conf.rate);
                dbp->query->bindValue(":rate",rate);
                QString language = filterHtmlText(context,Conf.language);
                dbp->query->bindValue(":language",language);
                dbp->query->bindValue(":status","0");
                //MD5不存在并且插入成功，将题目数据插入到数据库中
                if(dbp->query->exec()){
                    qDebug()<<"Spider()::Insert data into db successfully";
                    qDebug()<<"name is: "<<name<<" platform is: "<<cntPlatformName<<" id is: "<<maxRowId;
                    //主界面显示部分
                    QList<QStandardItem *> rowList;
                    rowList<<new QStandardItem(name)<<new QStandardItem(cntPlatformName)<<new QStandardItem(QString::number(maxRowId));
                    emit updateModel(rowList);
                }
                else{
                    qDebug()<<"Spider()::Cannot insert data into database: "<<dbp->query->lastError();
                    currentNum++;
                    count++;
                    continue;
                }
                //至此，一题的数据保存完成；进行下一题
                status =true;
                errorNum=0;
                sleep(2000);
                currentNum++;
                count++;
                maxRowId++;
                emit countChanged(count);
            }
            if((count%49==0||count%75==0||count%126==0)&&status){
                sleep(4000);
            }
            //如果错误题目数大于100就停止爬虫
        }while(errorNum<=100&&!Stopped);
        qDebug()<<"All question retrive done";
        emit displayMsg(QString("All question retrive done"));

        if(Stopped){
            qDebug()<<"Spider has been stopped successfully";
            emit displayMsg(QString("Spider has been stopped successfully"));
        }
    }

    //停止爬虫运行
    void stop(){
        Stopped = true;
        reply->deleteLater();
        manager->deleteLater();
        qDebug()<<"Spider has been stopped";
    }

private:
    //加载爬虫浏览器的模拟头
    void loadUserAgent(){
        int i=0;
        QFile file(":/resources/user_agents.ini");
        if(file.open(QIODevice::ReadOnly)){
            QTextStream readIn(&file);
            while(!readIn.atEnd()){
                UserAgent[i++]=readIn.readLine();
            }
        }
        file.close();
    }

    //根据正则表达式过滤文本
    QString filterHtmlText(QString source,QString regText)
    {
        QString result;
        int pos=0;
        QRegExp reg(regText);
        if(source.isEmpty()|reg.isEmpty()) return "N/A";
        reg.setMinimal(true);
        while((pos=reg.indexIn(source,pos))!= -1){
            result += reg.cap(1)+" ";
            pos += reg.matchedLength();
        }
        return result;
    }

    //获取给定url的网页数据
    QString getPlatformCtx(QString url)
    {
        assert(!endMark.isEmpty());

        QString source;//网页结果
        //******模拟浏览器****************
        request.setUrl(url);
        request.setRawHeader("User-Agent",UserAgent[randomNum(900)].toLatin1());
        request.setRawHeader("Keep-Alive","300");
        request.setRawHeader("Connection","keep-alive");
        request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
        request.setRawHeader("Accept-Language","zh-CN,en-US,zh,en;q=0.9");
        request.setRawHeader("Accept-Encoding","deflate");
        request.setRawHeader("Referer","https://www.baidu.com/");
        //*******模拟结束******************
        reply = manager->get(request);
        //循环直到获取网页所有内容
        QObject::connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
        loop.exec();
        //判断网页是否存在，是否有效
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"Spider()::Status code is: "<<statusCode;
        if(statusCode==200){
            QByteArray ba = reply->readAll();
            //根据编码进行调整文字内容
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            source = codec->toUnicode(ba);
            if(source.isEmpty())    return source;//没有数据就退出

            QString charset;
            int pos=0;
            QRegExp reg("charset=[\"](.+)\"[\\s]>");
            reg.setMinimal(true);
            while((pos=reg.indexIn(source,pos))!= -1){
                charset = reg.cap(1);
                pos += reg.matchedLength();
            }
            charset.remove('\"');
            if(!charset.isEmpty()){
                QTextCodec *codec = QTextCodec::codecForName(charset.toLocal8Bit());
                source = codec->toUnicode(ba).toLocal8Bit();
            }
            //->编码结束

            //如果网页内容中包含结束标志，则清空网页源文本
            if(source.contains(endMark)) source.clear();
        }
        else{
            //如果网页获取不正常，则清空网页内容
            source.clear();
        }

        return source;
    }
};

#endif // SPIDER_HPP
