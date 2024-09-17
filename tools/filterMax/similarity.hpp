/*
 * @Author:幽弥狂
 * @E-mail:1768478912@qq.com
 * @Phone:13812991101
 * @Date:2019-11-15
 * @License:LGPL3
 * @Function:计算文本相似度
 * @Description:筛选文本中最长的5个句子，计算其MD5值，存储至数据库中，
 *                            添加新MD5值时先检测是否存在相同的MD5值
 */

#ifndef SIMILARITY_HPP
#define SIMILARITY_HPP

#include "base.hpp"
#include "md5.hpp"

class Similarity{
public:
    QString maxStr;
public:
    Similarity(){}

    ~Similarity(){}

    //从文本中提取最长的一句话
    QString filterMaxStr(QString ctx){
        if(ctx.isEmpty()) return nullptr;
        QString result;

        QStringList symList;
        symList<<"。"<<"？"<<"！"<<"，"<<"."<<"?"<<"!"<<",";
        foreach(QString sym,symList){
            ctx.replace(sym,"<b>");
        }
        QStringList scentenceList = ctx.split("<b>");

        int size = 0;
        foreach(QString scentence,scentenceList){
            int temp = scentence.count();
            if(temp>size){
                result=scentence;
            }
        }
        return result;
    }

    QString getMD5(QString maxStr){
        if(maxStr.isEmpty()) return nullptr;
        QString result;
        string temp = maxStr.toStdString();
        result = QString::fromStdString(MD5(temp).toStr());

        return result;
    }

    QString checkout(QString ctx){
        //获取最长子串
        maxStr = filterMaxStr(ctx);
        QString md = getMD5(maxStr);
        return md;
    }
};

#endif // SIMILARITY_HPP
