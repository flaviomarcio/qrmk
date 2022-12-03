#include "./qrmk_test.h"

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
    switch (v.typeId()) {
    case QMetaType::QVariantHash:
    case QMetaType::QVariantMap:
    case QMetaType::QVariantList:
    case QMetaType::QStringList:
    case QMetaType::QVariantPair:
        bytes=QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact);
        break;
    default:
        bytes=v.toByteArray();
        break;
    }
    return QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex();
}

const QVariant SDKGoogleTest::toVar(const QVariant &v)
{
    switch (v.typeId()) {
    case QMetaType::QVariantHash:
    case QMetaType::QVariantMap:
    case QMetaType::QVariantList:
    case QMetaType::QStringList:
    case QMetaType::QVariantPair:
        return QJsonDocument::fromJson(v.toByteArray()).toVariant();
        break;
    default:
        return v;
    }
}

}
