#include "./private/p_qrmk_maker.h"

namespace QRmk{

Maker::Maker(QObject *parent)
    : QObject{parent}
{
    this->p=new MakerPvt{this};
}

const QVariantHash Maker::toHash() const
{
    return p->toHash();
}

Maker &Maker::setValues(const QVariant &v)
{
    Q_UNUSED(v)
    return *this;
}

Maker &Maker::make(const OutFormat outFormat)
{
    p->prepare();
    p->outFormat=outFormat;
    switch (p->outFormat) {
    case Maker::PDF:
        p->printPDF();
        break;
    case Maker::CSV:
    case Maker::TXT:
        p->printCSV_TXT();
        break;
    default:
        break;
    }
    return *this;
}

const QVariantList &Maker::makeRecords()
{
    p->prepare();
    return p->makeRecords();
}

Maker &Maker::clean()
{
    p->clean();
    return *this;
}

Maker &Maker::clear()
{
    p->clear();
    return *this;
}

QString &Maker::outFileName() const
{
    return p->outFileName;
}

Maker::Orientation Maker::orientation() const
{
    return p->orientation.type();
}

Maker &Maker::orientation(const QVariant &newOrientation)
{
    return this->setOrientation(newOrientation);
}

Maker &Maker::setOrientation(const QVariant &newOrientation)
{
    p->orientation=newOrientation;
    return *this;
}

QVariantList &Maker::items() const
{
    return p->items;
}

Maker &Maker::items(const QVariant &newItems)
{
    return this->setItems(newItems);
}

Maker &Maker::setItems(const QVariant &newItems)
{
    Q_DECLARE_VU;
    p->items = vu.toList(newItems);
    return *this;
}

QString Maker::owner() const
{
    return p->owner;
}

Maker &Maker::owner(const QString &newOwner)
{
    return this->setOwner(newOwner);
}

Maker &Maker::setOwner(const QString &newOwner)
{
    if (p->owner == newOwner)
        return *this;
    p->owner = newOwner;
    return *this;
}

QString Maker::title() const
{
    return p->title;
}

Maker &Maker::title(const QString &newTitle)
{
    return this->setTitle(newTitle);
}

Maker &Maker::setTitle(const QString &newTitle)
{
    if (p->title == newTitle)
        return *this;
    p->title = newTitle;
    return *this;
}

QStringList Maker::extraPageInfo() const
{
    return p->extraPageInfo;
}

Maker &Maker::extraPageInfo(const QStringList &newExtraPageInfo)
{
    return this->setExtraPageInfo(newExtraPageInfo);
}

Maker &Maker::setExtraPageInfo(const QStringList &newExtraPageInfo)
{
    if (p->extraPageInfo == newExtraPageInfo)
        return *this;
    p->extraPageInfo = newExtraPageInfo;
    return *this;
}

QVariantHash &Maker::filters()
{
    return p->filters;
}

Maker &Maker::filters(MakerFiltersFunc maker)
{
    Q_DECLARE_VU;
    if(maker)
        p->filters = maker(this->headers());
    return *this;
}


Headers &Maker::headers() const
{
    return p->headers;
}

Maker &Maker::headers(MakerHeadersFunc maker)
{
    if(maker)
        maker(p->headers);
    return *this;
}

Headers &Maker::summary() const
{
    return p->summary;
}

Maker &Maker::summary(MakerHeadersFunc maker)
{
    if(maker)
        maker(p->summary);
    return *this;
}

Signatures &Maker::signature() const
{
    return p->signature;
}

Maker &Maker::signature(MakerSignatureFunc maker)
{
    if(maker)
        maker(p->signature);
    return *this;
}

QStringList &Maker::groupingFields()
{
    return p->groupingFields;
}

Maker &Maker::groupingFields(const QStringList &newGroupingFields)
{
    p->groupingFields = newGroupingFields;
    return *this;
}

QString Maker::groupingDisplay()const
{
    if(p->groupingFields.isEmpty())
        p->groupingDisplay.clear();

    if(!p->groupingDisplay.isEmpty())
        return p->groupingDisplay;

    QStringList __return;
    for(auto &field: p->groupingFields){
        if(!p->headers.contains(field))
            continue;
        auto &header=p->headers.header(field);
        static const auto __format=QStringLiteral("%1: ${%2}");
        __return+=__format.arg(header.title(), header.field());
    }
    return __return.join(", ");
}

Maker &Maker::groupingDisplay(const QString &newGroupingDisplay)
{
    p->groupingDisplay = newGroupingDisplay;
    return *this;
}

int Maker::lines() const
{
    return p->lines;
}

Maker &Maker::lines(int newLines)
{
    if (p->lines == newLines)
        return *this;
    p->lines = newLines;
    return *this;
}

QByteArray &Maker::columnSeparator() const
{
    return p->columnSeparator;
}

Maker &Maker::columnSeparator(const QByteArray &newColumnSeparator)
{
    p->columnSeparator=newColumnSeparator;
    return *this;
}

bool Maker::columnTabular() const
{
    return p->columnTabular;
}

Maker &Maker::columnTabular(bool newColumnTabular)
{
    p->columnTabular=newColumnTabular;
    return *this;
}

}

