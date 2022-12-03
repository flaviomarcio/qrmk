#include "./qrmk_signature.h"
#include <QApplication>

namespace QRmk{


class SignaturePvt:public QObject{
public:
    QStm::MetaEnum<Signature::DocumentType> documentType=Signature::None;
    QString document;
    QString name;
    QStringList extraLines;
    explicit SignaturePvt(QObject *parent):QObject{parent}{}

};

Signature::Signature(QObject *parent)
    : QStm::ObjectWrapper{parent}
{
    this->p=new SignaturePvt{parent};
}

QStringList Signature::extraLines() const
{
    return p->extraLines;
}

Signature &Signature::extraLines(const QStringList &newExtraLines)
{
    if (p->extraLines == newExtraLines)
        return *this;
    p->extraLines = newExtraLines;
    emit extraLinesChanged();
    return *this;
}

Signature &Signature::resetExtraLines()
{
    return extraLines({});
}

QString Signature::name() const
{
    return p->name;
}

Signature &Signature::name(const QString &newName)
{
    if (p->name == newName.trimmed())
        return *this;
    p->name = newName.trimmed();
    emit nameChanged();
    return *this;
}

Signature &Signature::resetName()
{
    return name({});
}

QString Signature::nameFormatted()
{
    if(p->name.isEmpty())
        return {};

    return p->name.toUpper();
}

QString Signature::document() const
{
    return p->document;
}

Signature &Signature::document(const QVariant &newDocument)
{
    if (p->document == newDocument.toString().trimmed())
        return *this;
    p->document = newDocument.toString().trimmed();
    emit documentChanged();
    return *this;
}

Signature &Signature::resetDocument()
{
    return document({});
}

QString Signature::documentFormatted() const
{
    if(p->document.isEmpty())
        return {};
    static const auto __format=QString("%1: %2");
    return __format.arg(p->documentType.name(), p->document);
}

Signature::DocumentType Signature::documentType() const
{
    return p->documentType.type();
}

Signature &Signature::documentType(const QVariant &newDocumentType)
{
    if (p->documentType == newDocumentType)
        return *this;
    p->documentType = newDocumentType;
    emit documentTypeChanged();
    return *this;
}

Signature &Signature::resetDocumentType()
{
    return documentType({});
}

}
