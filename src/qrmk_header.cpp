#include "./qrmk_header.h"
#include <QApplication>

namespace QRmk {

class HeaderPvt:public QObject{
public:
    QString field;
    QString title;
    QString mask;
    int length=0;
    QStm::MetaEnum<Header::DataType> dataType;
    QVariant defaultValue;
    QStm::MetaEnum<Header::Alignment> align=Header::Start;
    int order=0;
    QVariant width;
    QColor foreGroundColor=Qt::black;
    QColor backGroundColor=Qt::transparent;
    QFont font=QApplication::font();
    bool visible=true;
    QStm::MetaEnum<Header::ComputeMode> computeMode=Header::ComputeMode::None;
    QString format;

    explicit HeaderPvt(QObject *parent):QObject{parent}{}

    const QString toFormatted(const QVariant &v, const int dataType)
    {
        Q_DECLARE_FU;
        switch (Header::DataType(dataType)) {
        case Header::Integer:
            return fu.toInt(v);
        case Header::Number:
            return fu.toNumber(v);
        case Header::Double:
            return fu.toDouble(v);
        case Header::Currency:
            return fu.currencySymbol(v);
        case Header::Boolean:
            return fu.toBool(v);
        case Header::Date:
            return fu.toDate(v);
        case Header::Time:
            return fu.toTime(v);
        case Header::DateTime:
            return fu.toDateTime(v);
        default:
            return fu.v(v);
        }
    }

};

Header::Header(QObject *parent)
    : QStm::ObjectWrapper{parent}
{
    this->p=new HeaderPvt{parent};
}

const QString Header::toFormattedValue(const QVariant &v)
{
    Q_DECLARE_FU;
    if(p->dataType.equal(Auto))
        return fu.v(v);
    return p->toFormatted(v, int(this->dataType()));
}

const QString &Header::field() const
{
    return p->field;
}

Header &Header::field(const QString &newField)
{
    if (p->field == newField.trimmed())
        return *this;
    p->field = newField.trimmed();
    emit fieldChanged();
    return *this;
}

Header &Header::resetField()
{
    return this->field({});
}

const QString &Header::title() const
{
    return p->title;
}

Header &Header::title(const QString &newTitle)
{
    if (p->title == newTitle.trimmed())
        return *this;
    p->title = newTitle.trimmed();
    emit titleChanged();
    return *this;
}

Header &Header::resetTitle()
{
    return this->title({});
}

const QString &Header::mask() const
{
    return p->mask;
}

Header &Header::mask(const QString &newMask)
{
    if (p->mask == newMask)
        return *this;
    p->mask = newMask;
    emit maskChanged();
    return *this;
}

Header &Header::resetMask()
{
    return this->mask({});
}

int Header::length() const
{
    return p->length;
}

Header &Header::length(const int newLength)
{
    if (p->length == newLength)
        return *this;
    p->length = newLength;
    emit lengthChanged();
    return *this;
}

Header &Header::resetLength()
{
    return this->length({});
}

Header::DataType Header::dataType() const
{
    return p->dataType.type();
}

Header &Header::dataType(const QVariant &newDataType)
{
    if (p->dataType == newDataType)
        return *this;
    p->dataType = newDataType;
    emit dataTypeChanged();
    return *this;
}

Header &Header::resetDataType()
{
    return this->dataType({});
}

const QVariant &Header::defaultValue() const
{
    return p->defaultValue;
}

Header &Header::defaultValue(const QVariant &newDefaultValue)
{
    if (p->defaultValue == newDefaultValue)
        return *this;
    p->defaultValue = newDefaultValue;
    emit defaultValueChanged();
    return *this;
}

Header &Header::resetDefaultValue()
{
    return this->defaultValue({});
}

Header::Alignment Header::align() const
{
    return p->align.type();
}

Header &Header::align(const QVariant &newAlign)
{
    if (p->align == newAlign)
        return *this;
    p->align = newAlign;
    emit alignChanged();
    return *this;
}

Header &Header::resetAlign()
{
    return this->align(Header::Start);
}

Qt::Alignment Header::alignQt() const
{
    switch (p->align.type()) {
    case Center:
        return Qt::AlignCenter;
    case End:
        return Qt::AlignRight | Qt::AlignVCenter;
    case Justify:
        return Qt::AlignJustify | Qt::AlignVCenter;
    default://Start:
        return Qt::AlignLeft | Qt::AlignVCenter;
    }
}

int Header::order() const
{
    return p->order;
}

Header &Header::order(int newOrder)
{
    if (p->order == newOrder)
        return *this;
    p->order = newOrder;
    emit orderChanged();
    return *this;
}

Header &Header::resetOrder()
{
    return this->order({});
}

const QVariant &Header::width() const
{
    return p->width;
}

Header &Header::width(const QVariant &newWidth)
{
    if (p->width == newWidth)
        return *this;
    p->width = newWidth;
    emit widthChanged();
    return *this;
}

Header &Header::resetWidth()
{
    return this->width({});
}

const QColor &Header::foreGroundColor() const
{
    return p->foreGroundColor;
}

Header &Header::foreGroundColor(const QColor &newColor)
{
    if (p->foreGroundColor == newColor)
        return *this;
    p->foreGroundColor = newColor;
    emit foreGroundColorChanged();
    return *this;
}

Header &Header::resetForeGroundColor()
{
    return this->foreGroundColor(Qt::black);
}

const QColor &Header::backGroundColor() const
{
    return p->backGroundColor;
}

Header &Header::backGroundColor(const QColor &newColor)
{
    if (p->backGroundColor == newColor)
        return *this;
    p->backGroundColor = newColor;
    emit backGroundColorChanged();
    return *this;
}

Header &Header::resetBackGroundColor()
{
    return this->backGroundColor(Qt::transparent);
}

const QFont &Header::font() const
{
    return p->font;
}

Header &Header::font(const QFont &newFont)
{
    if (p->font == newFont)
        return *this;
    p->font = newFont;
    emit fontChanged();
    return *this;
}

Header &Header::resetFont()
{
    return this->font({});
}

bool Header::visible() const
{
    return p->visible;
}

Header &Header::visible(bool newVisible)
{
    if (p->visible == newVisible)
        return *this;
    p->visible = newVisible;
    emit visibleChanged();
    return *this;
}

Header &Header::resetVisible()
{
    return this->visible(true);
}

Header::ComputeMode Header::computeMode() const
{
    return p->computeMode.type();
}

Header &Header::computeMode(const QVariant &newComputeMode)
{
    if (p->computeMode == newComputeMode)
        return *this;
    p->computeMode = newComputeMode;
    emit computeModeChanged();
    return *this;
}

Header &Header::resetComputeMode()
{
    return computeMode(Header::ComputeMode::None);
}

QString &Header::format() const
{
    return p->format;
}

Header &Header::format(const QString &newFormat)
{
    if (p->format == newFormat)
        return *this;
    p->format = newFormat;
    emit formatChanged();
    return *this;
}

Header &Header::resetFormat()
{
    return format({});
}

}
