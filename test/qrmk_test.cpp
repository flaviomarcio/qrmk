#include "./qrmk_test.h"
#include "../src/qrmk_types.h"
#include "../src/qrmk_meta_types.h"

namespace QRmk
{

SDKGoogleTest::SDKGoogleTest()
{
    QLocale::setDefault(QLocale(QLocale::Portuguese, QLocale::Brazil));
}

bool SDKGoogleTest::clear()
{
    return true;
}

QStringList SDKGoogleTest::arguments() const
{
    return qApp->arguments();
}

const QByteArray SDKGoogleTest::toMd5(const QVariant &v)
{
    QByteArray bytes;
    if(!QMetaTypeUtilObjectMetaData.contains(v.typeId()))
        bytes=v.toByteArray();
    else
        bytes=QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact);
    return QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex();
}

const QVariant SDKGoogleTest::toVar(const QVariant &v)
{
    if(QMetaTypeUtilString.contains(v.typeId()))
        return QJsonDocument::fromJson(v.toByteArray()).toVariant();
    return v;
}

}
