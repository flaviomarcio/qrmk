#pragma once

#include <QObject>
#include "./qrmk_global.h"
#include "./qrmk_headers.h"
#include "./qrmk_signatures.h"

namespace QRmk{

static const auto vPDF="PDF";
static const auto vCSV="CSV";
static const auto vTXT="TXT";

class MakerPvt;
typedef std::function<void(Headers &headers)> MakerHeadersFunc;
typedef std::function<void(Signatures &signatures)> MakerSignatureFunc;
class Q_RMK_EXPORT Maker : public QObject
{
    Q_OBJECT
public:
    //!
    //! \brief The OutFormat enum
    //!
    enum OutFormat{PDF=0, CSV=1, TXT=2};
    Q_ENUM(OutFormat)

    //!
    //! \brief The Orientation enum
    //!
    enum Orientation{Portrait=0, Landscape=1};
    Q_ENUM(Orientation);

    //!
    //! \brief Maker
    //! \param parent
    //!
    explicit Maker(QObject *parent = nullptr);

    //!
    //! \brief make
    //! \param outFormat
    //! \return
    //!
    Maker &make(const OutFormat outFormat=PDF);

    //!
    //! \brief outFileName
    //! \return
    //!
    QString &outFileName()const;

    //!
    //! \brief orientation
    //! \return
    //!
    Orientation orientation()const;
    Maker &orientation(const QVariant &newOrientation);
    Maker &setOrientation(const QVariant &newOrientation);

    //!
    //! \brief items
    //! \return
    //!
    QVariantList &items()const;
    Maker &items(const QVariant &newItems);
    Maker &setItems(const QVariant &newItems);

    //!
    //! \brief owner
    //! \return
    //!
    QString owner() const;
    Maker &owner(const QString &newOwner);
    Maker &setOwner(const QString &newOwner);

    //!
    //! \brief title
    //! \return
    //!
    QString title() const;
    Maker &title(const QString &newTitle);
    Maker &setTitle(const QString &newTitle);

    //!
    //! \brief extraPageInfo
    //! \return
    //!
    QStringList extraPageInfo() const;
    Maker &extraPageInfo(const QStringList &newExtraPageInfo);
    Maker &setExtraPageInfo(const QStringList &newExtraPageInfo);
    Maker &reextraPageInfo();

    //!
    //! \brief filters
    //! \return
    //!
    QVariantHash &filters();
    Maker &filters(const QVariant &newFilters);
    Maker &setFilters(const QVariant &newFilters);

    //!
    //! \brief headers
    //! \return
    //!
    Headers &headers() const;
    Maker &headers(MakerHeadersFunc maker);

    //!
    //! \brief summary
    //! \return
    //!
    Headers &summary() const;
    Maker &summary(MakerHeadersFunc maker);

    //!
    //! \brief signature
    //! \return
    //!
    Signatures &signature() const;
    Maker &signature(MakerSignatureFunc maker);

    //!
    //! \brief groupingFields
    //! \return
    //!
    QStringList &groupingFields();
    Maker &groupingFields(const QStringList &newGroupingFields);

    //!
    //! \brief groupingDisplay
    //! \return
    //!
    QString groupingDisplay() const;
    Maker &groupingDisplay(const QString &newGroupingDisplay);

    //!
    //! \brief lines
    //! \return
    //!
    int lines() const;
    Maker &lines(int newLines);

    //!
    //! \brief columnSeparator
    //! \return
    //!
    QByteArray &columnSeparator() const;
    Maker &columnSeparator(const QByteArray &newColumnSeparator);

    //!
    //! \brief columnTabular
    //! \return
    //!
    bool columnTabular()const;
    Maker &columnTabular(bool newColumnTabular);

private:
    MakerPvt* p=nullptr;
};

}



