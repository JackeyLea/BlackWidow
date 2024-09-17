#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP

#include "base.hpp"

class JsonParser{
public:
//private:
    QFile jsonFile;
    QByteArray jsonData;
public:
    JsonParser(QString filePath){
        qDebug()<<"JsonParser::JsonParser()";
        qDebug()<<"Current json file is: "<<filePath;

        assert(openJsonFile(filePath));
    }

    ~JsonParser(){
        qDebug()<<"JsonParser::~JsonParser()";
    }

    bool openJsonFile(QString filePath){
        jsonData.clear();
        jsonFile.setFileName(filePath);
        if(!jsonFile.exists()){
            qDebug()<<"Config file is not exist";
            return false;
        }
        if(!jsonFile.open(QIODevice::ReadOnly|QIODevice::Text)){
            qDebug()<<"Cannot open json file: "<<filePath;
            return false;
        }
        jsonData = jsonFile.readAll();
        jsonFile.close();

        return true;
    }

    //加载website.json配置文件，获取数据
    QStringList loadPlatform(){
        QStringList platform_list;
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData,&json_error));
        if(json_error.error !=QJsonParseError::NoError){
            qDebug()<<"json error: "<<json_error.errorString();
            return platform_list;
        }

        QJsonObject rootObj = jsonDoc.object();
        QStringList keys = rootObj.keys();
        platform_list.append(keys);
        qDebug()<<"platform list is: "<<platform_list;

        return platform_list;
    }

    //加载配置文件平台中的子数据
    conf_ getPlatfromValues(QString platformName){
        assert(!platformName.isEmpty());
        conf_ Conf;

        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData,&json_error));
        if(json_error.error !=QJsonParseError::NoError){
            qDebug()<<"json error: "<<json_error.errorString();
            Conf.status=false;//将配置结构体的状态设置为无效
            return Conf;
        }
        QJsonObject rootObj = jsonDoc.object();
        if(!rootObj.contains(platformName)){
            qDebug()<<"Configuration file does not contain current platform data";
            Conf.status=false;
            return Conf;
        }
        QJsonObject platformObj = rootObj.value(platformName).toObject();
        Conf.url = platformObj.value("url").toString();
        Conf.begin = platformObj.value("begin").toInt();
        Conf.name = platformObj.value("name").toString();
        Conf.detail = platformObj.value("detail").toString();
        Conf.level = platformObj.value("level").toString();
        Conf.cata = platformObj.value("cata").toString();
        Conf.language = platformObj.value("language").toString();
        Conf.submit = platformObj.value("submit").toString();
        Conf.passed = platformObj.value("passed").toString();
        Conf.rate = platformObj.value("rate").toString();
        Conf.end = platformObj.value("end").toString();
        //---->所有配置文件数据读取完成
        Conf.status = true;
        return Conf;
    }

    QString getValue(QString key){
        QString value = nullptr;
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData,&json_error));
        if(json_error.error !=QJsonParseError::NoError){
            qDebug()<<"json error: "<<json_error.errorString();
            return value;
        }
        QJsonObject rootObj = jsonDoc.object();
        value = rootObj.value(key).toString();

        return value;
    }

    QString getValue(QString platform,QString key){
        QString value = nullptr;
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData,&json_error));
        if(json_error.error !=QJsonParseError::NoError){
            qDebug()<<"json error: "<<json_error.errorString();
            return value;
        }
        QJsonObject rootObj = jsonDoc.object();
        if(!rootObj.contains(platform)){
            qDebug()<<"Configuration didn't contain this platform: "<<platform;
            return value;
        }
        QJsonObject platformObj = rootObj.value(platform).toObject();
        value = platformObj.value(key).toString();

        return value;
    }

    void updateValue(QString platform,QString key,QString value){
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData,&json_error));
        if(json_error.error!=QJsonParseError::NoError){
            qDebug()<<"JsonParser()::json error: "<<json_error.errorString();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();
        if(!rootObj.contains(platform)){
            qDebug()<<"Configuration didn't contain this platform: "<<platform;
            return ;
        }
        QJsonObject m_jsonObj = rootObj.value(platform).toObject();
        m_jsonObj[key]=value;

        if(jsonFile.open(QIODevice::WriteOnly|QIODevice::Text)){
            QJsonDocument tempDoc(m_jsonObj);
            jsonFile.write(tempDoc.toJson());
        }
        jsonFile.close();
    }

    void updateValue(QString key,QString value){
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData,&json_error));
        if(json_error.error !=QJsonParseError::NoError){
            qDebug()<<"json error: "<<json_error.errorString();
            return;
        }

        QJsonObject m_jsonObj = jsonDoc.object();
        m_jsonObj[key]= value;

        if(jsonFile.open(QIODevice::WriteOnly|QIODevice::Text)){
            QJsonDocument tempDoc(m_jsonObj);
            jsonFile.write(tempDoc.toJson());
        }
        jsonFile.close();
    }
};

#endif // JSONPARSER_HPP
