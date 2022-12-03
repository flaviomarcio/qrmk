#include "qrmk_maker.h"
#include <QTemporaryFile>
#include <QPdfWriter>
#include <QPainter>
#include <QGuiApplication>
#include <QPen>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

namespace QRmk{

static const auto extPDF="pdf";
static const auto extCSV="csv";
static const auto extTXT="txt";

static const auto sepCSV=";";
static const auto sepTXT="|";

class MakerPvt:public QObject{
public:
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
    QList<Header *> headersList;

    explicit MakerPvt(Maker *parent=nullptr):QObject{parent}, headers{this}, summary{this}, signature{this}
    {
        this->parent=parent;
    }

    void clean()
    {
        this->items.clear();
    }

    void clear()
    {
        this->outFileName.clear();
        this->items.clear();
        this->owner.clear();
        this->title.clear();
        this->filters.clear();
        this->groupingDisplay.clear();
        this->groupingFields.clear();
        this->extraPageInfo.clear();
        this->headersList.clear();
        this->headers.clear();
        this->summary.clear();
    }


    void prepare()
    {
        this->headersList.clear();
        for(auto header : this->headers.list()){
            if(header->visible())
                headersList.append(header);
        }

        for(auto &header:this->summary.list()){
            if(header->dataType()==Header::Auto){
                if(this->headers.contains(header->field())){
                    const auto &h=this->headers.header(header->field());
                    header->dataType(h.dataType());
                }
            }
        }

    }

    int getLines()
    {
        if(this->lines>0)
            return this->lines;
        switch (this->orientation.type()) {
        case Maker::Landscape:
            return 60;
        default://Maker::Portrait
            return 90;
        }
    };

    QByteArray getColumnSeparator()const
    {
        switch (this->outFormat) {
        case Maker::CSV:
            return sepCSV;
        case Maker::TXT:
            return sepTXT;
        default:
            return {};
        }
    }

    QByteArray getExtension()
    {
        switch (this->outFormat) {
        case Maker::PDF:
            return extPDF;
        case Maker::CSV:
            return extCSV;
        case Maker::TXT:
            return extTXT;
        default:
            return {};
        }
    }

    QString getFileName()
    {        
        auto __format=QStringLiteral("%1.%2");
        return __format.arg(QUuid::createUuid().toString(), this->getExtension());
    }

    QString parserString(const QString &text, const QVariantHash &itemRecord)
    {
        auto __return=text;
        static const auto __env="${";
        static const auto __format=QString{"${%1}"};
        if(!text.contains(__env))
            return __return;
        for(auto &header: this->headers.list()){
            auto env=__format.arg(header->field()).toLower();
            if(__return.contains(env)){
                auto val=itemRecord.value(header->field());
                auto replaceValue=header->toFormattedValue(val);
                __return=__return.replace(env, replaceValue);
            }
        }
        return __return.trimmed();

    }

    QString printPDF()
    {

        auto file=QTemporaryFile(getFileName(), parent);
        file.setAutoRemove(false);
        if(!file.open())
            return {};

        QPdfWriter pdfWriter(&file);
        pdfWriter.setPageSize(QPageSize::A4);
        switch (this->orientation.type()) {
        case Maker::Landscape:
            pdfWriter.setPageOrientation(QPageLayout::Orientation::Landscape);
            break;
        default:
            pdfWriter.setPageOrientation(QPageLayout::Orientation::Portrait);
            break;
        }

        QVariantHash itemRecord;

        auto painter=QPainter(&pdfWriter);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::black);
        static auto fontNormal=QFont{"Sans Serif", 8};
        static auto fontBold=fontNormal;
        fontBold.setBold(true);
        static auto fontItalic=fontNormal;
        fontItalic.setItalic(true);
        painter.setFont(fontNormal);

        this->spacing=(this->spacing>0)?this->spacing:(pdfWriter.width()*0.005);
        this->textOffSetT=(this->textOffSetL>0)?this->textOffSetT:(this->spacing);
        this->textOffSetL=(this->textOffSetL>0)?this->textOffSetL:(this->spacing);
        this->textOffSetR=(this->textOffSetR>0)?this->textOffSetR:(this->spacing*2);
        this->textOffSetB=(this->textOffSetB>0)?this->textOffSetB:(this->spacing*2);
        this->totalHeight=(this->totalHeight>0)?this->totalHeight:(painter.viewport().height());
        this->totalWidth=(this->totalWidth>0)?this->totalWidth:painter.viewport().width();
        this->rowHeight=(this->rowHeight>0)?this->rowHeight:(totalHeight*0.02);
        this->maxRows=(maxRows>0)?maxRows:this->getLines();

        QHash<Header*, QRect> columnsRec;




        {//calc columns rectangle

            int startX=0, startY=0;
            VariantUtil vu;

            double sumWidth=0;
            for(auto header : this->headersList){
                sumWidth+=vu.toDouble(header->width());
            }

            if(sumWidth>0){//width ajust
                double diff=(1.00-sumWidth);
                double diffWidth=totalWidth*diff;
                static const auto __format=QString("%1%");
                for(auto header : this->headersList){
                    auto per=vu.toDouble(header->width());
                    auto curWidth=(totalWidth*per);
                    auto incWidth=(diffWidth*per);
                    auto width=(curWidth+incWidth);
                    auto perNew=(width/totalWidth)*100;
                    auto withNew=__format.arg(QString::number(perNew,'f',6));
                    header->width(withNew);
                }
            }

            auto pointLine=QPoint{0,0};
            this->rowWidth=0;
            for(auto header : this->headersList){
                double per=vu.toDouble(header->width());
                int w=(totalWidth*per);
                this->rowWidth+=w;
                int h=this->rowHeight;
                auto rect=QRect{pointLine.x(), startY, w, h};
                columnsRec.insert(header, rect);
                pointLine.setX(startX+=w);
            }
        }
        rectFull=(rectFull.width()>0)?rectFull:QRect(0, 0, rowWidth, rowHeight);

        int startY=0;

        int rowCount=0, pageCount=0;

        auto nextY=[this, &startY, &rowCount](double factor=1)
        {
            rowCount+=(factor>=1)?factor:1;
            return (startY+=(rowHeight*factor));
        };

        FormattingUtil fu;
        const auto __time=tr("Emissão: %1 %2").arg(fu.v(QDate::currentDate()),fu.v(QTime::currentTime()));
        auto writePageInfo=[this, __time, &itemRecord, &startY, &painter, &nextY, &pageCount]()//draw headers
        {
            auto totalLinesInfo=2;//time+page
            totalLinesInfo+=this->extraPageInfo.count();

            auto infoH=(rowHeight*totalLinesInfo);

            auto rect=QRect(spacing, startY=spacing, rowWidth-textOffSetR, infoH-textOffSetB);
            painter.setFont(fontNormal);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(Qt::black);
            painter.drawText(rect, Qt::AlignRight, __time);

            for(auto extra:this->extraPageInfo){
                extra=this->parserString(extra,itemRecord);
                if(extra.isEmpty())
                    continue;
                rect.setY(nextY());
                painter.drawText(rect, Qt::AlignRight, extra);
            }
            static const auto __page=tr("Página: %1");
            rect.setY(nextY());
            painter.drawText(rect, Qt::AlignRight, __page.arg(++pageCount));

            if(!this->title.isEmpty()){
                painter.setFont(fontBold);
                painter.drawText(rect, Qt::AlignHCenter, this->title);
            }

            if(!this->owner.isEmpty()){
                painter.setFont(fontItalic);
                painter.drawText(rect, Qt::AlignLeft, this->owner);
            }

            int maxW=rowWidth;
            rect=QRect{0, 0, maxW, startY+rect.height()};
            painter.setBrush(Qt::NoBrush);
            painter.setPen(Qt::black);
            painter.drawRect(rect);
        };

        auto writeColumns=[this, &nextY, &startY, &painter, &columnsRec]()//draw headers
        {
            nextY();
            painter.setFont(fontNormal);
            for(auto header : headersList){
                auto value=header->title();
                auto rectBase = columnsRec.value(header);
                auto rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());

                painter.setBrush(Qt::lightGray);
                painter.setPen(Qt::black);
                painter.drawRect(rect);


                rect=QRect(rectBase.x()+textOffSetL, startY+textOffSetL, rectBase.width()-(textOffSetR), rectBase.height()-textOffSetB);

                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);
                QRect boundingRect;
                painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, value, &boundingRect);
            }
        };

        auto writeLine=[this, &nextY, &startY, &painter, &columnsRec](const QVariantHash &itemRow)//draw headers
        {
            nextY();
            int startX=-1;
            painter.setFont(fontNormal);
            for(auto &header : headersList){
                auto value=itemRow.value(header->field()).toString();

                auto rectBase = columnsRec.value(header);
                auto rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());

                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);
                painter.drawRect(rect);

                rect=QRect(rectBase.x()+textOffSetL, startY+textOffSetL, rectBase.width()-textOffSetR, rectBase.height()-textOffSetB);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);
                QRect boundingRect;
                painter.drawText(rect, header->alignQt(), value, &boundingRect);

                if(startX==-1)
                    startX=rect.x();
            }
        };

        auto writeReportLine=[this, &itemRecord, &writeLine](const QVariantHash &itemRow)//draw headers
        {
            QVariantHash itemRowFormatted;
            for(auto &header : headersList){
                auto value=itemRow.value(header->field());

                if(value.isValid())
                    value=header->toFormattedValue(value);

                if(!header->format().isEmpty())
                    value=this->parserString(header->format(),itemRecord);
                else
                    value=this->parserString(value.toString(),itemRecord);

                itemRowFormatted.insert(header->field(), value);

            }
            writeLine(itemRowFormatted);
        };

        auto writeSingleLine=[this, &nextY, &startY, &painter](const QVariant &outItem)//draw headers
        {
            if(headersList.isEmpty())
                return;
            nextY();
            painter.setFont(fontBold);
            QStringList textLine;
            switch (outItem.typeId()) {
            case QMetaType::QVariantPair:
            case QMetaType::QVariantHash:
            case QMetaType::QVariantMap:
            {
                auto itemRow=outItem.toHash();
                for(auto &header : headersList){
                    auto value=itemRow.value(header->field());

                    if(value.isNull() && !value.isValid())
                        continue;

                    auto valueText=header->toFormattedValue(value);
                    static const auto __format=QString("%1: %2");
                    if(!header->title().isEmpty() && !valueText.isEmpty())
                        textLine.append(__format.arg(header->title(), valueText));
                    else if(!header->title().isEmpty())
                        textLine.append(header->title());
                    else if(valueText.isEmpty())
                        textLine.append(valueText);
                }
                break;
            }
            default:
                Q_DECLARE_VU;
                auto v=vu.toStr(outItem).trimmed();
                if(!v.isEmpty())
                    textLine.append(v);
                break;
            }

            if(textLine.isEmpty())
                return;


            Header *header=headersList.first();
            if(header==nullptr)
                return;
            static const auto __spacer=", ";

            QRect boundingRect;
            auto rectangle=QRect(rectFull.x(), startY, rectFull.width()-textOffSetB, rectFull.height()-textOffSetB);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(Qt::black);
            painter.drawText(rectangle, Qt::AlignJustify | Qt::AlignVCenter, textLine.join(__spacer), &boundingRect);

        };

        auto pageStart=[&rowCount, &writeColumns, &writePageInfo]()
        {
            rowCount=0;
            writePageInfo();
            writeColumns();
        };

        auto pageBlank=[&pdfWriter, &rowCount, &writePageInfo]()
        {
            pdfWriter.newPage();
            rowCount=0;
            writePageInfo();
        };

        auto pageNew=[&pdfWriter, &pageStart]()
        {
            pdfWriter.newPage();
            pageStart();
        };

        auto writeSummary=[this, &writeColumns, &writeSingleLine, &writeLine](const QVariantList &vList, const QString &totalMessage)
        {
            if(this->summary.isEmpty())
                return false;

            Q_DECLARE_VU;

            auto checkMajor=[&vu](const Header *header, const QVariant &currentValue, const QVariant &newValue){
                if(currentValue.isNull() || !currentValue.isValid())
                    return newValue;

                switch (header->dataType()) {
                case Header::Double:
                case Header::Integer:
                case Header::Number:
                case Header::Currency:{
                    auto v0=vu.toDouble(currentValue);
                    auto v1=vu.toDouble(currentValue);
                    QVariant v=(v0<v1)?v1:v0;
                    return v;
                }
                case Header::Date:
                case Header::DateTime:
                {
                    auto v0=vu.toDateTime(currentValue);
                    auto v1=vu.toDateTime(currentValue);
                    QVariant v=(v0<v1)?v1:v0;
                    return v;
                }
                default:
                    return currentValue;
                }
            };

            auto checkMinor=[&vu](const Header *header, const QVariant &currentValue, const QVariant &newValue){
                if(currentValue.isNull() || !currentValue.isValid())
                    return newValue;

                switch (header->dataType()) {
                case Header::Double:
                case Header::Integer:
                case Header::Number:
                case Header::Currency:{
                    auto v0=vu.toDouble(currentValue);
                    auto v1=vu.toDouble(currentValue);
                    QVariant v=(v0<v1)?v1:v0;
                    return v;
                }
                case Header::Date:
                case Header::DateTime:
                {
                    auto v0=vu.toDateTime(currentValue);
                    auto v1=vu.toDateTime(currentValue);
                    QVariant v=(v0<v1)?v1:v0;
                    return v;
                }
                default:
                    return currentValue;
                }
            };

            QVariantHash itemSummary;
            for(auto &item: vList){
                auto itemRecord=item.toHash();
                QVariant value;
                for(auto &header:this->summary.list()){

                    auto itemSummaryValue=itemRecord.value(header->field());
                    if(itemSummaryValue.isNull() || !itemSummaryValue.isValid())
                        continue;

                    auto list=itemSummary.value(header->field()).toList();

                    switch (header->computeMode()) {
                    case Header::Count:
                    {
                        if(!list.contains(itemSummaryValue))
                            list.append(itemSummaryValue);
                        value=list;
                        break;
                    }
                    case Header::Sum:
                    case Header::Max:
                    case Header::Min:
                    case Header::Avg:
                    {
                        list.append(itemSummaryValue);
                        value=list;
                        break;
                    }
                    default:
                        value={};
                        break;
                    }
                    if(value.isValid() && !value.isNull())
                        itemSummary.insert(header->field(), value);
                }
            }


            for(auto &header:this->summary.list()){
                auto &value=itemSummary[header->field()];

                auto vList=value.toList();

                if(vList.isEmpty()){
                    value={};
                    continue;
                }

                switch (header->computeMode()) {
                case Header::Count:
                {
                    value=vList.count();
                    break;
                }
                case Header::Sum:
                case Header::Max:
                case Header::Min:
                {
                    QVariant calc;
                    for(auto&v:vList){
                        switch (header->computeMode()) {
                        case Header::Sum:
                            calc=calc.toDouble()+vu.toDouble(v);
                            break;
                        case Header::Max:
                            calc=checkMajor(header, calc, v);
                            break;
                        case Header::Min:
                            calc=checkMinor(header, calc, v);
                        default:
                            break;
                        }
                    }
                    value=calc;
                    break;
                }
                case Header::Avg:
                {
                    double calc=0;
                    for(auto&v:vList){
                        calc+=vu.toDouble(v);
                        break;
                    }
                    value=calc/vList.count();
                    break;
                }
                default:
                    value={};
                    break;
                }
            }

            QVariantHash itemRowFormatted;
            for(auto &header : this->summary.list()){
                auto value=itemSummary.value(header->field());
                value=header->toFormattedValue(value);
                itemRowFormatted.insert(header->field(), value);
            }

            writeSingleLine(totalMessage);
            writeColumns();
            writeLine(itemRowFormatted);
            return true;
        };


        QVariantHash vLastRow;
        QVariantList vSummaryRows;
        auto groupingCheck=[this, &nextY, &vLastRow, &vSummaryRows, &writeSingleLine, &writeSummary](const QVariantHash &itemRow, bool lastSummary=false)//draw headers
        {
            Q_UNUSED(lastSummary)


            Q_DECLARE_VU;
            QVariantHash vGroupRow;
            if(!lastSummary){

                if(vLastRow.isEmpty()){//first call check
                    vLastRow=itemRow;
                    vSummaryRows.append(itemRow);//rows to group summary
                    return false;
                }

                {
                    //new group check
                    for(auto&headerName:this->groupingFields){
                        if(!this->headers.contains(headerName))
                            continue;

                        const auto &header=this->headers.header(headerName);

                        vGroupRow.insert(header.field(), itemRow.value(header.field()));

                        auto v0=vu.toByteArray(itemRow.value(header.field()));
                        auto v1=vu.toByteArray(vLastRow.value(header.field()));
                        if(v0==v1)
                            continue;

                        vLastRow={};//clear for summary and grouping
                    }
                }

                if(!vLastRow.isEmpty()){
                    vSummaryRows.append(itemRow);//rows to group summary
                    return false;
                }
            }

            static const auto __total=tr("Totalização");
            writeSummary(vSummaryRows, __total);

            vLastRow=itemRow;
            vSummaryRows.clear();
            vSummaryRows.append(itemRow);//rows to group summary

            nextY(0.5);
            writeSingleLine(vGroupRow);
            return true;
        };


        auto writeSignatures=[this, &nextY, &painter, &itemRecord, &startY, &pageBlank]()
        {
            if(this->signature.isEmpty())
                return;

            Q_DECLARE_VU;

            auto areaMinH=vu.toDouble(this->signature.pageArea().height);
            auto areaCurH=(startY/this->totalHeight);
            {//area check
                if(areaMinH>areaCurH)
                    pageBlank();
            }

            const auto rectSignature=QRect(0, nextY(2), this->rowWidth, this->totalHeight-startY);


            QRect rectTitle=QRect(0, nextY(0), rowWidth, rowHeight);
            if(!this->signature.title().isEmpty()){//title

                auto font=fontBold;
                font.setPointSize(font.pointSize()+2);
                painter.setFont(font);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);

                rectTitle.setY(nextY(2));
                QRect boundingRect;
                painter.drawText(rectTitle, Qt::AlignCenter, this->signature.title(), &boundingRect);
            }

            {//declarations

                if(!this->signature.declaration().isEmpty()){

                    //auto startX=(this->totalWidth/2)-(areaMinW/2);

                    auto declarationRows=this->signature.declaration().count()*1.3;

                    auto wArea=totalWidth*0.66;
                    auto startX=((totalWidth/2)-(wArea/2));
                    auto rect=QRect(startX, nextY(2), wArea, (rowHeight*declarationRows));
                    auto declaration=this->signature.declaration().join(" ");
                    declaration=this->parserString(declaration, itemRecord);

                    auto font=fontNormal;
                    font.setPointSize(font.pointSize()+2);
                    painter.setFont(font);

                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(Qt::black);
                    auto pen=painter.pen();
                    pen.setWidth(pen.width()*10);
                    painter.setPen(pen);
                    painter.drawRect(rect);

                    rect=QRect(rect.x()+textOffSetL, rect.y()+textOffSetT, rect.width()-textOffSetR, rect.height()-textOffSetB);
                    QRect boundingRect;
                    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignJustify | Qt::TextWordWrap, declaration, &boundingRect);


/*
                    QTextDocument doc;
                    doc.setPageSize(rect.size());
                    doc.setHtml(declaration);
                    doc.setDefaultFont(font);

                    auto layout = doc.documentLayout();

                    QAbstractTextDocumentLayout::PaintContext context;
                    context.palette.setColor( QPalette::Text, painter.pen().color() );
                    painter.save();
                    painter.translate( rect.x(), rect.y() );
                    layout->draw(&painter, context );
                    painter.restore();

*/
                    //doc.drawContents(&painter, rect);
                    startY=rect.y()+rect.height();
                }
            }

            if(!this->signature.localFormatted().isEmpty()){//title
                auto rect=QRect(0, nextY(2),rowWidth,rowHeight);
                auto font=fontNormal;
                font.setPointSize(font.pointSize()+2);
                painter.setFont(font);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);

                QRect boundingRect;
                painter.drawText(rect, Qt::AlignCenter, this->signature.localFormatted(), &boundingRect);
            }

            if(!this->signature.signatures().isEmpty()){//title
                auto font=fontNormal;
                font.setPointSize(font.pointSize()+2);
                painter.setFont(font);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);

                const auto __startY=nextY(3);

                auto rows=this->signature.signatures().count();
                auto wArea=(totalWidth*vu.toDouble(this->signature.width()));

                auto startX=(totalWidth/2)-((wArea*rows)/2);

                for(auto &signature:this->signature.signatures()){
                    if(signature->document().isEmpty() && signature->name().isEmpty())
                        continue;

                    startY=__startY;

                    auto rectSign=QRect(startX, nextY(0), wArea, rowHeight);
                    startX+=(wArea+spacing);

                    {//line
                        auto rectLine=QRect(rectSign.x(), rectSign.y(), rectSign.width(), 1);
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(Qt::black);
                        painter.drawRect(rectLine);
                    }

                    if(!signature->nameFormatted().isEmpty()){//Name
                        auto rect=QRect(rectSign.x(), nextY(0.5), rectSign.width(), rectSign.height());
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(Qt::black);
                        auto text=this->parserString(signature->name(), itemRecord);
                        QRect boundingRect;
                        painter.drawText(rect, Qt::AlignCenter, text, &boundingRect);
                    }

                    if(!signature->documentFormatted().isEmpty()){//Document
                        auto rect=QRect(rectSign.x(), nextY(), rectSign.width(), rectSign.height());
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(Qt::black);
                        auto text=this->parserString(signature->document(), itemRecord);
                        QRect boundingRect;
                        painter.drawText(rect, Qt::AlignCenter, text, &boundingRect);
                    }
                }
            }

            {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Qt::black);
                painter.drawRect(rectSignature);
            }

        };


        {//write pages
            itemRecord=(this->items.isEmpty())?itemRecord:this->items.first().toHash();
            pageStart();
            for(auto &item: this->items){
                itemRecord=item.toHash();
                if(groupingCheck(itemRecord))
                    writeColumns();
                writeReportLine(itemRecord);
                if((this->maxRows>0) && (this->maxRows<=++rowCount)){
                    if(&item!=&this->items.last())
                        pageNew();
                }
            }
        }

        groupingCheck({},true);

        static const auto __total=tr("Totalização final");
        writeSummary(this->items, __total);

        writeSignatures();
        painter.end();

        auto __return=file.fileName();

        file.flush();
        file.close();
        return __return;
    }

    QString printCSV_TXT()
    {
        Q_DECLARE_FU;
        QByteArray separator=this->getColumnSeparator();
        QTemporaryFile file{this->getFileName(), this->parent};
        file.setAutoRemove(false);
        if(!file.open())
            return {};

        {
            QByteArray line=separator;
            for(auto header : this->headersList){
                auto value=header->title().toUtf8().trimmed().replace(' ','_');
                if(this->columnTabular)
                    value=fieldValueAlign(header, value);
                line+=value;
                line+=separator;
            }
            line+='\n';
            file.write(line);
        }

        {
            for(auto &row : this->items){
                auto itemRow=row.toHash();
                QByteArray line=separator;
                for(auto &header : this->headersList){
                    auto value=itemRow.value(header->field()).toString();
                    value=header->toFormattedValue(value);
                    if(this->columnTabular)
                        value=fieldValueAlign(header, value);
                    line+=value.toLatin1();
                    line+=separator;
                }
                line+='\n';
                file.write(line);
            }
        }
        file.flush();
        file.close();
        return file.fileName();
    }

    QByteArray fieldValueAlign(Header *header, const QString &value)
    {
        int length=header->length();
        if(length<=0){
            switch (header->dataType()) {
            case Header::String:
            case Header::Uuid:
                length=10;
                break;
            case Header::Integer:
            case Header::Number:
            case Header::Double:
            case Header::Currency:
                length=11;
                break;
            case Header::Boolean:
                length=3;
                break;
            case Header::Date:
                length=10;
                break;
            case Header::Time:
                length=8;
                break;
            case Header::DateTime:
                length=19;
                break;
            default:
                break;
            }

        }


        auto text=value.trimmed();
        switch (header->align()) {
        case Header::Start:
            return text.leftJustified(length,' ',true).toLatin1();
        case Header::End:
            return text.rightJustified(length,' ', true).toLatin1();
        default://Header::Center
            bool odd=false;
            while(text.length()<length)
                text=odd?(' '+text):(text+' ');
            return text.toLatin1();
        }
    }
};

Maker::Maker(QObject *parent)
    : QObject{parent}
{
    this->p=new MakerPvt{this};
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

