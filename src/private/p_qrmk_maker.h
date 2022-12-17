#include <QTemporaryFile>
#include <QPdfWriter>
#include <QPainter>
#include <QGuiApplication>
#include <QPen>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include "../qstm/src/qstm_meta_enum.h"
#include "../qrmk_maker.h"
#include "../qstm/src/qstm_util_variant.h"

namespace QRmk{


class MakerPvt:public QObject{
    enum RowType{
        RowNONE, RowPageInfo, RowHeader, RowValues, RowSingle, RowSummaryGrouping, RowSummaryHeader, RowSummaryTotal, RowSignature
    };

    Q_ENUM(RowType)
public:

    Q_DECLARE_VU;
    Maker*parent=nullptr;
    QStm::MetaEnum<Maker::Orientation> orientation=Maker::Portrait;
    Maker::OutFormat outFormat=Maker::PDF;
    QString outFileName;
    QVariantList items;
    QString owner;
    QString title;
    QVariantHash filters;
    Headers headers;
    Headers summary;
    Signatures signature;
    QStringList groupingFields;
    QString groupingDisplay;
    int lines=0;
    QByteArray columnSeparator;
    bool columnTabular=true;

    double textOffSetT=0;
    double textOffSetL=0;
    double textOffSetR=0;
    double textOffSetB=0;
    double totalHeight=0;
    double totalWidth=0;
    int maxRows=0;
    double spacing=0;
    double rowWidth=0;
    double rowHeight=0;
    QStringList extraPageInfo;
    QRect rectFull={};
    QRect rectSingleRow={};
    QList<Header *> headersList;

    explicit MakerPvt(Maker *parent=nullptr);

    const QVariantHash toHash() const;

    void clean();

    void clear();

    void prepare();

    int getLines();;

    QVariantList makeRecords();


    QByteArray getColumnSeparator()const;

    QByteArray getExtension();

    QString getFileName();

    QString parserString(const QString &text, const QVariantHash &itemRecord);

    QString printPDF();

    QString printCSV_TXT();

    QByteArray fieldValueAlign(Header *header, const QString &value);
};

}

