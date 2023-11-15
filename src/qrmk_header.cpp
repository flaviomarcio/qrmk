#include "./qrmk_header.h"
#include <QApplication>
#include "../../qstm/src/qstm_meta_enum.h"
#include "../../qstm/src/qstm_util_formatting.h"

namespace QRmk {

static const auto __black="black";
static const auto __transparent="transparent";


class HeaderPvt:public QObject{
public:
    QString field;
    QString title;
    QString mask;
    int length=0;
    QStm::MetaEnum<Header::DataType> dataType;
    QVariant defaultValue;
    QStm::MetaEnum<Header::Alignment> align=Header::Alignment::Start;
    int order=0;
    QVariant width;
    QVariant foreGroundColor=__black;
    QVariant backGroundColor=__transparent;
#ifdef QT_GUI_LIB
    QFont font=QApplication::font();
#else
    QVariant font;
#endif
    bool visible=true;
    QStm::MetaEnum<Header::ComputeMode> computeMode=Header::ComputeMode::None;
    QString format;

    explicit HeaderPvt(QObject *parent):QObject{parent}{}

    const QString toFormatted(const QVariant &v, const int dataType)
    {
        Q_DECLARE_FU;

        switch (Header::DataType(dataType)) {
        case Header::DataType::Integer:
        case Header::DataType::Number:
        case Header::DataType::Double:
        case Header::DataType::Currency:{
            if(!v.toString().trimmed().isEmpty()){
                if(v.toDouble()==0)
                    return v.toString();
            }
            break;
        }
        case Header::DataType::Date:
        case Header::DataType::Time:
        case Header::DataType::DateTime:
        {
            if(!v.toString().trimmed().isEmpty()){
                if(!v.toDateTime().isValid())
                    return v.toString();
            }
        }
        default:
            break;
        }

        switch (Header::DataType(dataType)) {
        case Header::DataType::Integer:
            return fu.toInt(v);
        case Header::DataType::Number:
            return fu.toNumber(v);
        case Header::DataType::Double:
            return fu.toDouble(v);
        case Header::DataType::Currency:
            return fu.currencySymbol(v);
        case Header::DataType::Boolean:
            return fu.toBool(v);
        case Header::DataType::Date:
            return fu.toDate(v);
        case Header::DataType::Time:
            return fu.toTime(v);
        case Header::DataType::DateTime:
            return fu.toDateTime(v);
        default:
            return fu.v(v);
        }
    }

};

Header::Header(QObject *parent)
    : QStm::ObjectWrapper{parent}, p{new HeaderPvt{parent}}
{
}

const QString Header::toFormattedValue(const QVariant &v)const
{
    Q_DECLARE_FU;
    QString __return;
    if(p->dataType.equal(DataType::Auto))
        __return=fu.v(v);
    else
        __return=p->toFormatted(v, int(this->dataType()));

    if(!this->isFormatMask())
        return __return;

    return fu.formatMask(this->format(), __return);
}

const QVariant Header::toValue(const QVariant &v)const
{
    if(!this->isFormatMask())
        return v;

    return this->toFormattedValue(v);
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

Header &Header::align(const Alignment &newAlign)
{
    if (p->align == newAlign)
        return *this;
    p->align = newAlign;
    emit alignChanged();
    return *this;
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
    return this->align(Header::Alignment::Start);
}

Qt::Alignment Header::alignQt() const
{
    switch (p->align.type()) {
    case Alignment::Center:
        return Qt::AlignCenter;
    case Alignment::End:
        return Qt::AlignRight | Qt::AlignVCenter;
    case Alignment::Justify:
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

const QVariant Header::foreGroundColor() const
{
    return p->foreGroundColor;
}

Header &Header::foreGroundColor(const QVariant &newColor)
{
    if (p->foreGroundColor == newColor)
        return *this;
    p->foreGroundColor = newColor;
    emit foreGroundColorChanged();
    return *this;
}

Header &Header::resetForeGroundColor()
{
    return this->foreGroundColor(__black);
}

const QVariant &Header::backGroundColor() const
{
    return p->backGroundColor;
}

Header &Header::backGroundColor(const QVariant &newColor)
{
    if (p->backGroundColor == newColor)
        return *this;
    p->backGroundColor = newColor;
    emit backGroundColorChanged();
    return *this;
}

Header &Header::resetBackGroundColor()
{
    return this->backGroundColor(__transparent);
}

#ifdef QT_GUI_LIB
const QFont &Header::font() const
#else
const QVariant &Header::font() const
#endif
{
    return p->font;
}

#ifdef QT_GUI_LIB
Header &Header::font(const QFont &newFont)
#else
Header &Header::font(const QVariant &newFont)
#endif
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

Header &Header::computeMode(const ComputeMode &newComputeMode)
{
    if (p->computeMode == newComputeMode)
        return *this;
    p->computeMode = newComputeMode;
    emit computeModeChanged();
    return *this;
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
    p->format = newFormat.trimmed();
    emit formatChanged();
    return *this;
}

Header &Header::resetFormat()
{
    return format({});
}

bool Header::isFormatMask() const
{
    static const auto __char="#";
    if(p->format.contains(__char))
        return true;
    return {};
}

bool Header::isFormatParser() const
{
    static const auto __char="${";
    if(p->format.contains(__char))
        return true;
    return {};
}

}
