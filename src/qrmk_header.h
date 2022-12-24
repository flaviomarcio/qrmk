#pragma once

#include <QColor>
#include <QFont>
#include "../../qstm/src/qstm_object_wrapper.h"
#include "./qrmk_global.h"

namespace QRmk{
class HeaderPvt;
class Q_RMK_EXPORT Header : public QStm::ObjectWrapper
{
    Q_OBJECT
    Q_STM_OBJECT_WRAPPER(Header)
    Q_PROPERTY(QString field READ field WRITE field RESET resetField NOTIFY fieldChanged)
    Q_PROPERTY(QString title READ title WRITE title RESET resetTitle NOTIFY titleChanged)
    Q_PROPERTY(QString mask READ mask WRITE mask RESET resetMask NOTIFY maskChanged)
    Q_PROPERTY(int length READ length WRITE length RESET resetLength NOTIFY lengthChanged)
    Q_PROPERTY(DataType dataType READ dataType WRITE dataType RESET resetDataType NOTIFY dataTypeChanged)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE defaultValue RESET resetDefaultValue NOTIFY defaultValueChanged)
    Q_PROPERTY(Alignment align READ align WRITE align RESET resetAlign NOTIFY alignChanged)
    Q_PROPERTY(int order READ order WRITE order RESET resetOrder NOTIFY orderChanged)
    Q_PROPERTY(QVariant width READ width WRITE width RESET resetWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor foreGroundColor READ foreGroundColor WRITE foreGroundColor RESET resetForeGroundColor NOTIFY foreGroundColorChanged)
    Q_PROPERTY(QColor backGroundColor READ backGroundColor WRITE backGroundColor RESET resetBackGroundColor NOTIFY backGroundColorChanged)
    Q_PROPERTY(QFont font READ font WRITE font RESET resetFont NOTIFY fontChanged)
    Q_PROPERTY(bool visible READ visible WRITE visible RESET resetVisible NOTIFY visibleChanged)
    Q_PROPERTY(ComputeMode computeMode READ computeMode WRITE computeMode RESET resetComputeMode NOTIFY computeModeChanged)
    Q_PROPERTY(QString format READ format WRITE format RESET resetFormat NOTIFY formatChanged)
public:

    //!
    //! \brief The Alignment enum
    //!
    enum class Alignment{
        Start=Qt::AlignLeft,
        Center=Qt::AlignCenter,
        End=Qt::AlignRight,
        Justify=Qt::AlignJustify
    };
    Q_ENUM(Alignment)

    enum DataType{
          Auto=QMetaType::UnknownType
        , String=QMetaType::QString
        , Uuid=QMetaType::QUuid
        , Boolean=QMetaType::Bool
        , Date=QMetaType::QDate
        , Time=QMetaType::QTime
        , DateTime=QMetaType::QDateTime
        , Integer=QMetaType::LongLong
        , Double=QMetaType::Double
        , QtMap=QMetaType::QVariantHash
        , QtList=QMetaType::QVariantList
        , QtObject=QMetaType::QObjectStar
        , Numeric=QMetaType::Double
        , Number
        , Currency
    };
    Q_ENUM(DataType)

    enum class ComputeMode{
          None
        , Text
        , Count
        , Sum
        , Max
        , Min
        , Avg
    };
    Q_ENUM(ComputeMode)

    //!
    //! \brief Header
    //! \param parent
    //!
    explicit Header(QObject *parent = nullptr);

    //!
    //! \brief toFormattedValue
    //! \param v
    //! \return
    //!
    const QString toFormattedValue(const QVariant &v) const;

    //!
    //! \brief toValue
    //! \param v
    //! \return
    //!
    const QVariant toValue(const QVariant &v) const;

public:
    //!
    //! \brief field
    //! \return
    //!
    const QString &field() const;
    Header &field(const QString &newField);
    Header &resetField();

    //!
    //! \brief title
    //! \return
    //!
    const QString &title() const;
    Header &title(const QString &newTitle);
    Header &resetTitle();

    //!
    //! \brief mask
    //! \return
    //!
    const QString &mask() const;
    Header &mask(const QString &newMask);
    Header &resetMask();

    //!
    //! \brief length
    //! \return
    //!
    int length() const;
    Header &length(const int newLength);
    Header &resetLength();

    //!
    //! \brief dataType
    //! \return
    //!
    Header::DataType dataType() const;
    Header &dataType(const QVariant &newDataType);
    Header &resetDataType();

    //!
    //! \brief defaultValue
    //! \return
    //!
    const QVariant &defaultValue() const;
    Header &defaultValue(const QVariant &newDefaultValue);
    Header &resetDefaultValue();

    //!
    //! \brief align
    //! \return
    //!
    Alignment align() const;
    Header &align(const Alignment &newAlign);
    Header &align(const QVariant &newAlign);
    Header &resetAlign();
    Qt::Alignment alignQt() const;

    //!
    //! \brief order
    //! \return
    //!
    int order() const;
    Header &order(int newOrder);
    Header &resetOrder();

    //!
    //! \brief width
    //! \return
    //!
    const QVariant &width() const;
    Header &width(const QVariant &newWidth);
    Header &resetWidth();

    //!
    //! \brief foreGroundColor
    //! \return
    //!
    const QColor &foreGroundColor() const;
    Header &foreGroundColor(const QColor &newColor);
    Header &resetForeGroundColor();

    //!
    //! \brief backGroundColor
    //! \return
    //!
    const QColor &backGroundColor() const;
    Header &backGroundColor(const QColor &newColor);
    Header &resetBackGroundColor();

    //!
    //! \brief font
    //! \return
    //!
    const QFont &font() const;
    Header &font(const QFont &newFont);
    Header &resetFont();

    //!
    //! \brief visible
    //! \return
    //!
    bool visible() const;
    Header &visible(bool newVisible);
    Header &resetVisible();

    //!
    //! \brief computeMode
    //! \return
    //!
    ComputeMode computeMode() const;
    Header &computeMode(const ComputeMode &newComputeMode);
    Header &computeMode(const QVariant &newComputeMode);
    Header &resetComputeMode();

    //!
    //! \brief format
    //! \return
    //!
    QString &format() const;
    Header &format(const QString &newFormat);
    Header &resetFormat();

    //!
    //! \brief isFormatMask
    //! \return
    //!
    bool isFormatMask() const;

    //!
    //! \brief isFormatParser
    //! \return
    //!
    bool isFormatParser() const;

signals:
    void fieldChanged();

    void titleChanged();

    void maskChanged();

    void lengthChanged();

    void dataTypeChanged();

    void defaultValueChanged();

    void alignChanged();

    void startChanged();

    void orderChanged();

    void widthChanged();

    void foreGroundColorChanged();

    void backGroundColorChanged();

    void fontChanged();

    void visibleChanged();

    void computeModeChanged();

    void formatChanged();

private:
    HeaderPvt *p=nullptr;
};

}

