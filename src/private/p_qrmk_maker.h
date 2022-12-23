#include <QTemporaryFile>
#include <QPdfWriter>
#include <QPainter>
#include <QGuiApplication>
#include <QPen>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTime>
#include "../qstm/src/qstm_meta_enum.h"
#include "../qrmk_maker.h"
#include "../qstm/src/qstm_util_variant.h"

namespace QRmk{

static const auto __fontDefault="Sans Serif";

class MakerPvt:public QObject{
public:
    enum RowType{
        RowNONE, RowStart, RowSingle, RowHeader, RowValues, RowSignature, RowSummaryGrouping, RowSummaryTotal, RowSummaryValues
    };

    Q_ENUM(RowType)
public:

    Q_DECLARE_VU;
    Maker*parent=nullptr;
    QStm::MetaEnum<Maker::Orientation> orientation=Maker::Orientation::Portrait;
    Maker::OutFormat outFormat=Maker::OutFormat::PDF;
    QString outFileName;
    QVariantList items, outPutRecord;
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
    QStringList extraPageInfo;

    explicit MakerPvt(Maker *parent=nullptr);

    const QVariantHash toHash() const;

    void clean();

    void clear();

    void prepare();

    int getLines();


    QByteArray getColumnSeparator()const;

    QByteArray getExtension();

    QString getFileName();

    QString parserText(const QVariant &valueParser, const QVariantHash &itemRecord);

    QVariant parserValue(const QVariant &valueParser, const QVariantHash &itemRecord);

    QByteArray fieldValueAlign(Header *header, const QString &value);

    const QVariantList &makeRecords();


    QString textWrite();

private:

    int pdfStartY=0, pdfTotalPageInfo=0;
    int pdfRowCount=0, pdfPageCount=0;
    int pdfRowsMax=0;
    double pdfTextOffSetT=0;
    double pdfTextOffSetL=0;
    double pdfTextOffSetR=0;
    double pdfTextOffSetB=0;
    double pdfTotalHeight=0;
    double pdfTotalWidth=0;
    double pdfRowSpacing=0;
    double pdfRowWidth=0;
    double pdfRowHeight=0;
    QRect pdfRectFull={};
    QRect pdfRectSingleRow={};
    QList<Header *> pdfHeadersList;
    QList<Header *> pdfHeadersSummary;

    QFont pdfFontNormal=QFont{__fontDefault, 8};
    QFont pdfFontBold=pdfFontNormal;
    QFont pdfFontItalic=pdfFontNormal;
    QPdfWriter *pdfWriter=nullptr;
    QVariantHash pdfItemRecord;
    QPainter *pdfPainter=nullptr;
    QHash<Header*, QPair<QRect,QRect>> pdfColumnsHeaders, pdfColumnsSummary;
    QString pdfTimeText;
private://pdf
    double pdfNextY(double factor=1);
    void pdfWritePageInfo();//draw headers
    QString pdfMakeSingleLine();
    void pdfWriteLineHeaders();
    void pdfWriteSummaryLineHeaders();//draw headers
    void pdfWriteLine(const QHash<Header*, QPair<QRect,QRect>> &columnsRow, const QVariantHash &itemRow);
    void pdfWriteLineValues(const QVariantHash &itemRecord);//draw headers
    void pdfParseRow(const QRmk::MakerPvt::RowType &rowType, const QVariantHash &itemValue={});
    void pdfParseList(QVariantList &vRecordList);
    void pdfPageBlank();
    void pdfPageStart();
    void pdfPagePrepare();
    void pdfPageNew();
    bool pdfPageNewCheck(int offSet=0);
    void pdfWriteSummaryLineValues(const QVariantHash &itemRecord);//draw headers
    void pdfWriteSingleLine(const QVariant &outItem);//draw headers
    void pdfWriteSignatures(const QVariantHash &itemRecord);
    void pdfPrepare();
public:
    QString pdfWrite();

};

}

