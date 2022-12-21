#include "./p_qrmk_maker.h"
#include "../qstm/src/qstm_util_formatting.h"
#include "../qstm/src/qstm_meta_enum.h"

namespace QRmk{


static const auto extPDF="pdf";
static const auto extCSV="csv";
static const auto extTXT="txt";

static const auto sepCSV=";";
static const auto sepTXT="|";

static const auto __formatWidth=QString("%1%");

static const auto __fontDefault="Sans Serif";

static const auto __spaceJoin=", ";

static const auto __rowType="__row_type__";
static const auto __rowValue="__row_value__";

static const auto __total=QObject::tr("Totalização");
static const auto __totalFinal=QObject::tr("Totalização final");

typedef QVector<QByteArray> PropertyNames;

Q_GLOBAL_STATIC_WITH_ARGS(PropertyNames, staticIgnoreMethods,({"objectName","values","measures","asJson", "measures", "baseValues", "clearOnSetFail"}))


static const QVariantHash extractHash(const QObject *object, const QStringList &ignoreProperties)
{
    QVariantHash __return;
    auto metaObject = object->metaObject();

    for(int col = 0; col < metaObject->propertyCount(); ++col) {

        auto property = metaObject->property(col);

        auto name=QByteArray{property.name()};
        if(ignoreProperties.contains(name))
            continue;

        if(!property.isReadable())
            continue;

        if(staticIgnoreMethods->contains(name))
            continue;

        auto value=property.read(object);

        auto obj=value.value<QObject*>();
        if(obj)
            value = extractHash(obj, ignoreProperties);

        switch (value.typeId()) {
        case QMetaType::QUuid:
            value=value.toUuid().toString();
            break;
        case QMetaType::QUrl:
            value=value.toUrl().toString();
            break;
        default:
            break;
        }
        __return.insert(property.name(), value);
    }
    return __return;
}


MakerPvt::MakerPvt(Maker *parent):QObject{parent}, headers{this}, summary{this}, signature{this}
{
    this->parent=parent;
}

const QVariantHash QRmk::MakerPvt::toHash() const
{
    return extractHash(this->parent, {});
}

void MakerPvt::clean()
{
    this->items.clear();
}

void MakerPvt::clear()
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
    qDeleteAll(headersSummary);
    headersSummary.clear();
}

void MakerPvt::prepare()
{
    this->headersList.clear();
    for(auto header : this->headers.list()){
        if(header->visible())
            headersList.append(header);
    }

    for(auto &header:this->summary.list()){
        if(!this->headers.contains(header->field()))
            continue;

        const auto &h=this->headers.header(header->field());
        if((header->dataType()==Header::Auto) && (header->dataType()!=h.dataType()))
            header->dataType(h.dataType());

        if(header->format().isEmpty() && !h.format().isEmpty())
            header->format(h.format());
    }
    this->maxRows=(maxRows>0)?maxRows:this->getLines();
}

int MakerPvt::getLines()
{
    if(this->lines>0)
        return this->lines;
    switch (this->orientation.type()) {
    case Maker::Orientation::Landscape:
        return 60;
    default://Maker::Portrait
        return 90;
    }
}

const QVariantList &MakerPvt::makeRecords()
{
    if(this->items.isEmpty())
        return this->outPutRecord;

    auto &outPutRecord=this->outPutRecord;

    QVariantHash itemRecord;

    RowType lastRowType;

    auto writeLine=[&outPutRecord, &lastRowType](const RowType rowType, const QVariant &rowValue){
        outPutRecord.append(QVariantHash{{__rowType, rowType}, {__rowValue,rowValue}});
        lastRowType=rowType;
    };
    auto writeColumns=[&writeLine, &lastRowType](RowType rowType=RowHeader)//draw headers
    {
        if(lastRowType==rowType)
            return;
        writeLine(rowType, {});
    };
    auto writeSingleLine=[&writeLine](const QVariant &outItem)//draw headers
    {
        writeLine(RowSingle, outItem);
    };
    auto pageStart=[&writeLine, &writeColumns](const QVariant &itemRecord){
        writeLine(RowPageInfo,itemRecord);
        writeColumns();
    };
    auto writeReportValues=[&writeLine](const QVariantHash &itemRecord)//draw headers
    {
        writeLine(RowValues, itemRecord);
    };
    auto writeSummary=[this, &itemRecord, &writeLine, &writeSingleLine, &writeColumns](const QVariantList &vSummaryList, RowType rowType, const QString &totalMessage)
    {
        Q_UNUSED(rowType)

        static const auto __P1=QString("${%1}");

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

        auto getGroup=[this](const QVariantHash &itemRecord){
            if(this->groupingFields.isEmpty())
                return QByteArray{};
            QStringList values;
            for(auto&headerName:this->groupingFields){
                if(!this->headers.contains(headerName))
                    continue;
                const auto &header=this->headers.header(headerName);
                auto value=this->vu.toByteArray(itemRecord.value(header.field()));
                values.append(value);
            }
            auto __return=this->vu.toMd5(values);
            return this->vu.toMd5(__return);
        };

        QHash<QString, QVariantHash> itemGrouping;

        for(auto &item: vSummaryList){
            auto itemRecord=item.toHash();
            QVariant value;
            for(auto &header:this->summary.list()){
                auto groupKey=getGroup(itemRecord);
                auto &itemSummary=itemGrouping[groupKey];
                auto itemSummaryValue=itemRecord.value(header->field());
                if(itemSummaryValue.isNull() || !itemSummaryValue.isValid())
                    continue;

                auto list=itemSummary.value(header->field()).toList();

                switch (header->computeMode()) {
                case Header::ComputeMode::Text:
                {
                    if(!list.contains(itemSummaryValue))
                        list.append(itemSummaryValue);
                    value=list;
                    break;
                }
                case Header::ComputeMode::Count:
                {
                    if(!list.contains(itemSummaryValue))
                        list.append(itemSummaryValue);
                    value=list;
                    break;
                }
                case Header::ComputeMode::Sum:
                case Header::ComputeMode::Max:
                case Header::ComputeMode::Min:
                case Header::ComputeMode::Avg:
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


        {//summary grouping
            auto groups=itemGrouping.keys();
            for(auto&group:groups){
                auto &itemSummary=itemGrouping[group];
                for(auto &header:this->summary.list()){

                    auto &value=itemSummary[header->field()];

                    auto vGroupedList=value.toList();

                    switch (header->computeMode()) {
                    case Header::ComputeMode::Text:
                    {
                        if(vGroupedList.isEmpty())
                            value={};
                        else if(vGroupedList.size()==1)
                            value=vGroupedList.first().toString();
                        else
                            value=value.toStringList().join(__spaceJoin);
                        break;
                    }
                    case Header::ComputeMode::Count:
                    {
                        if(header->format().isEmpty())
                            value=vGroupedList.count();
                        else{
                            auto vReplaceText=__P1.arg(header->field());
                            auto vReplaceValue=QString::number(vGroupedList.count());
                            value=header->format().replace(vReplaceText, vReplaceValue);
                        }
                        break;
                    }
                    case Header::ComputeMode::Sum:
                    case Header::ComputeMode::Max:
                    case Header::ComputeMode::Min:
                    {
                        QVariant calc;
                        for(auto&v:vGroupedList){
                            switch (header->computeMode()) {
                            case Header::ComputeMode::Sum:
                                calc=calc.toDouble()+vu.toDouble(v);
                                break;
                            case Header::ComputeMode::Max:
                                calc=checkMajor(header, calc, v);
                                break;
                            case Header::ComputeMode::Min:
                                calc=checkMinor(header, calc, v);
                            default:
                                break;
                            }
                        }
                        if(header->format().isEmpty())
                            value=calc;
                        else{
                            auto vReplaceText=__P1.arg(header->field());
                            value=header->format().replace(vReplaceText, value.toString());
                        }

                        break;
                    }
                    case Header::ComputeMode::Avg:
                    {
                        double calc=0;
                        for(auto&v:vGroupedList){
                            calc+=vu.toDouble(v);
                            break;
                        }
                        value=calc/vGroupedList.count();
                        break;
                    }
                    default:
                        value={};
                        break;
                    }
                }

            }
        }

        if(!totalMessage.trimmed().isEmpty())
            writeSingleLine(totalMessage);
        writeColumns();
        {//write grouping summary
            auto groups=itemGrouping.keys();
            groups.sort();
            for(auto&group:groups){
                QVariantHash itemRowFormatted;
                auto &itemSummary=itemGrouping[group];
                for(auto &header : this->summary.list()){
                    auto value=itemSummary.value(header->field());
                    value=header->toValue(value);
                    itemRowFormatted.insert(header->field(), value);
                }
                for(auto &header : this->headers.list()){
                    if(itemRowFormatted.contains(header->field()))
                        continue;

                    auto value=itemRecord.value(header->field());

                    itemRowFormatted.insert(header->field(), value);
                }
                writeLine(rowType, itemRowFormatted);
            }
        }
        return true;
    };

    QVariantHash vLastRow;
    QVariantList vSummaryRows;
    auto groupingCheck=[this, &vLastRow, &vSummaryRows, &writeColumns, &writeSummary, &writeSingleLine](const QVariantHash &itemRecord, bool lastSummary=false)//draw headers
    {
        Q_DECLARE_VU;

        if(this->groupingFields.isEmpty())
            return false;

        if(!lastSummary){

            if(vLastRow.isEmpty()){//first call check
                vLastRow=itemRecord;
                vSummaryRows.append(itemRecord);//rows to group summary
                return false;
            }

            {
                //new group check
                for(auto&headerName:this->groupingFields){
                    if(!this->headers.contains(headerName))
                        continue;

                    const auto &header=this->headers.header(headerName);

                    auto v0=vu.toByteArray(itemRecord.value(header.field()));
                    auto v1=vu.toByteArray(vLastRow.value(header.field()));
                    if(v0==v1)
                        continue;

                    vLastRow={};//clear for summary and grouping
                }
            }

            if(!vLastRow.isEmpty()){
                vSummaryRows.append(itemRecord);//rows to group summary
                return false;
            }
        }

        writeSummary(vSummaryRows, RowSummaryGrouping, {}/*__total*/);
        vLastRow=itemRecord;
        vSummaryRows.clear();
        vSummaryRows.append(itemRecord);//rows to group summary


        {

            QStringList vLine;
            if(!this->groupingDisplay.trimmed().isEmpty()){
                vLine.append(this->parserText(this->groupingDisplay, itemRecord));
            }
            else{
                for(auto&headerName: this->groupingFields){
                    if(!this->headers.contains(headerName))
                        continue;
                    static const auto __format=QString("%1: %2");
                    auto &header=this->headers.header(headerName);
                    auto value=header.toValue(itemRecord.value(header.field()));

                    QString valueText;
                    valueText=this->parserText(value, itemRecord);
                    if(!header.title().trimmed().isEmpty())
                        vLine.append(__format.arg(header.title(), valueText));
                    vLine.append(valueText);
                }
            }
            auto text=vLine.join(__spaceJoin).trimmed();
            if(!text.isEmpty())
                writeSingleLine(text);
        }

        writeColumns();
        return true;
    };

    auto writeSignatures=[this, &writeLine](const QVariant &itemRecord)
    {
        if(this->signature.isEmpty())
            return;
        writeLine(RowSignature, itemRecord);
    };

    {//write pages
        itemRecord=(this->items.isEmpty())?itemRecord:this->items.first().toHash();
        pageStart(itemRecord);
        for(auto &item: this->items){
            auto vHash=item.toHash();
            if(vHash.contains(__rowType) && vHash.contains(__rowValue)){
                outPutRecord.append(vHash);
                vHash=vHash.value(__rowValue).toHash();
            }
            groupingCheck(itemRecord=vHash);
            writeReportValues(vHash);
        }
    }

    groupingCheck({},true);

    writeSummary(this->items, RowSummaryTotal, __totalFinal);
    writeSignatures(itemRecord);

    return outPutRecord;
}



QByteArray MakerPvt::getColumnSeparator() const
{
    switch (this->outFormat) {
    case Maker::OutFormat::CSV:
        return sepCSV;
    case Maker::OutFormat::TXT:
        return sepTXT;
    default:
        return {};
    }
}



QByteArray MakerPvt::getExtension()
{
    switch (this->outFormat) {
    case Maker::OutFormat::PDF:
        return extPDF;
    case Maker::OutFormat::CSV:
        return extCSV;
    case Maker::OutFormat::TXT:
        return extTXT;
    default:
        return {};
    }
}



QString MakerPvt::getFileName()
{
    auto __format=QStringLiteral("%1.%2");
#ifdef QT_DEBUG
    return __format.arg("report", this->getExtension());
#else
    return __format.arg(QUuid::createUuid().toString(), this->getExtension());
#endif
}

QString QRmk::MakerPvt::parserText(const QVariant &valueParser, const QVariantHash &itemRecord)
{
    auto __return=valueParser.toString();
    static const auto __env="${";
    static const auto __format=QString{"${%1}"};
    if(!__return.contains(__env))
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



QVariant MakerPvt::parserValue(const QVariant &valueParser, const QVariantHash &itemRecord)
{
    switch (valueParser.typeId()) {
    case QMetaType::UnknownType:
    case QMetaType::QString:
    case QMetaType::QByteArray:
        break;
    default:
        return valueParser;
    }

    return parserText(valueParser, itemRecord);
}



QString MakerPvt::printPDF()
{
    auto vRecordList=this->makeRecords();

    if(vRecordList.isEmpty())
        return {};

#ifdef QT_DEBUG
    auto file=QFile(getFileName());
    if(!file.open(QFile::Truncate | QFile::WriteOnly | QFile::Unbuffered))
        return {};
#else
    auto file=QTemporaryFile(getFileName(), parent);
    file.setAutoRemove(false);
    if(!file.open())
        return {};

#endif

    QPdfWriter pdfWriter(&file);
    pdfWriter.setPageSize(QPageSize::A4);
    switch (this->orientation.type()) {
    case Maker::Orientation::Landscape:
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
    static auto fontNormal=QFont{__fontDefault, 8};
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

    QHash<Header*, QRect> columnsRow, columnsHeaders;

    {//calc columns rectangle


        int startX=0, startY=0;
        Q_DECLARE_VU;

        double sumWidth=0;
        for(auto header : this->headersList){
            sumWidth+=vu.toDouble(header->width());
        }

        if(sumWidth>0){//width ajust
            double diff=(1.00-sumWidth);
            double diffWidth=totalWidth*diff;
            for(auto header : this->headersList){
                auto per=vu.toDouble(header->width());
                auto curWidth=(totalWidth*per);
                auto incWidth=(diffWidth*per);
                auto width=(curWidth+incWidth);
                auto perNew=(width/totalWidth)*100;
                auto withNew=__formatWidth.arg(QString::number(perNew,'f',6));
                header->width(withNew);
            }
        }

        auto pointLine=QPoint{0,0};
        this->rowWidth=0;
        QRect rect={};
        static const double factor=1.2;
        for(auto header : this->headersList){
            double per=vu.toDouble(header->width());
            int w=(totalWidth*per);
            this->rowWidth+=w;
            int h=this->rowHeight;
            rect=QRect{pointLine.x(), startY, w, h};
            columnsRow.insert(header, rect);
            rect=QRect{pointLine.x(), startY, w, int(h*factor)};
            columnsHeaders.insert(header, rect);
            pointLine.setX(startX+=w);
        }

        double cumativeWidth=0;
        for(auto header : this->headersList){

            if(!this->summary.contains(header->field())){
                cumativeWidth+=vu.toDouble(header->width());
                continue;
            }

            auto newHeader=this->headersSummary.isEmpty()?nullptr:this->headersSummary.last();
            if(!newHeader){
                newHeader=new Header{this};
                headersSummary.append(newHeader);
            }

            auto perNew=cumativeWidth+vu.toDouble(header->width());
            cumativeWidth=0;
            auto withNew=__formatWidth.arg(QString::number(perNew,'f',6));
            header->width(withNew);
        }

        rectSingleRow=QRect(0,0, startX, int(this->rowHeight*factor));
    }
    rectFull=(rectFull.width()>0)?rectFull:QRect(0, 0, rowWidth, rowHeight);

    int startY=0, totalPageInfo=0;
    int rowCount=0, pageCount=0;

    auto nextY=[this, &startY, &rowCount](double factor=1)
    {
        rowCount+=(factor>=1)?factor:1;
        return (startY+=(rowHeight*factor));
    };

    FormattingUtil fu;
    const auto __time=tr("Emissão: %1 %2").arg(fu.v(QDate::currentDate()),fu.v(QTime::currentTime()));
    auto writePageInfo=[this, __time, &itemRecord, &startY, &painter, &nextY, &totalPageInfo, &pageCount]()//draw headers
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
            extra=this->parserText(extra,itemRecord);
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
        totalPageInfo=rect.height();
    };

    auto writeColumns=[this, &nextY, &startY, &painter, &columnsHeaders]()//draw headers
    {
        nextY();
        painter.setFont(fontNormal);
        for(auto header : headersList){
            auto value=header->title();
            auto rectBase = columnsHeaders.value(header);
            auto rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());

            painter.setBrush(Qt::lightGray);
            painter.setPen(Qt::black);
            painter.drawRect(rect);

            rect=QRect(rectBase.x()+textOffSetL, startY/*+textOffSetL*/, rectBase.width()-(textOffSetR), rectBase.height()/*-textOffSetB*/);

            painter.setBrush(Qt::NoBrush);
            painter.setPen(Qt::black);
            QRect boundingRect;
            painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, value, &boundingRect);
        }
        nextY(0.2);
    };

    auto writeLine=[this, &nextY, &startY, &painter, &columnsRow](const QVariantHash &itemRow)//draw headers
    {
        nextY();
        int startX=-1;
        painter.setFont(fontNormal);
        for(auto &header : headersList){
            auto value=itemRow.value(header->field()).toString();

            auto rectBase = columnsRow.value(header);
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

    auto writeReportValues=[this, &writeLine](const QVariantHash &itemRecord)//draw headers
    {
        QVariantHash itemRowFormatted;
        for(auto &header : headersList){
            auto value=itemRecord.value(header->field());

            if(value.isValid())
                value=header->toFormattedValue(value);

            if(!header->format().isEmpty())
                value=this->parserText(header->format(), itemRecord);
            else
                value=this->parserText(value, itemRecord);

            itemRowFormatted.insert(header->field(), value);

        }
        writeLine(itemRowFormatted);
    };

    auto writeSingleLine=[this, &nextY, &itemRecord, /*&startY, */&painter](const QVariant &outItem)//draw headers
    {
        if(headersList.isEmpty())
            return;
        nextY(1.5);
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
                valueText=this->parserText(valueText, itemRecord);
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
            if(!v.isEmpty()){
                v=this->parserText(v, itemRecord);
                textLine.append(v);
            }
            break;
        }

        if(textLine.isEmpty())
            return;


        Header *header=headersList.first();
        if(header==nullptr)
            return;

        QRect boundingRect;
        auto rectangle=QRect(rectSingleRow.x(), nextY(1.2), rectSingleRow.width(), rectSingleRow.height());

        painter.setBrush(Qt::lightGray);
        painter.setPen(Qt::black);
        painter.drawRect(rectangle);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::black);
        painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignVCenter, textLine.join(__spaceJoin), &boundingRect);

        nextY(0.2);

    };

    auto pageBlank=[&pdfWriter, &writePageInfo]()
    {
        pdfWriter.newPage();
        writePageInfo();
    };

    auto pageStart=[&rowCount, &vRecordList, &writeColumns, &writePageInfo]()
    {
        rowCount=0;
        writePageInfo();
        if(!vRecordList.isEmpty())
            writeColumns();
    };

    auto pageNew=[&pdfWriter, &pageStart]()
    {
        pdfWriter.newPage();
        pageStart();
    };

    auto writeSignatures=[this, &totalPageInfo, &nextY, &painter, &startY, &pageBlank](const QVariantHash &itemRecord)
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

        if((areaCurH+areaCurH)>1.00)
            pageBlank();


        const auto rectSignature=QRect(0, nextY(2), this->rowWidth, (this->totalHeight-(startY+totalPageInfo)));

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
                declaration=this->parserText(declaration, itemRecord);

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
                    auto text=this->parserText(signature->name(), itemRecord);
                    QRect boundingRect;
                    painter.drawText(rect, Qt::AlignCenter, text, &boundingRect);
                }

                if(!signature->documentFormatted().isEmpty()){//Document
                    Q_DECLARE_FU;
                    auto rect=QRect(rectSign.x(), nextY(), rectSign.width(), rectSign.height());
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(Qt::black);
                    auto text=this->parserText(signature->document(), itemRecord);
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

    RowType lastRowType=RowNONE;
    while(!vRecordList.isEmpty()){
        auto item=vRecordList.takeFirst();
        auto vHash=item.toHash();
        auto itemValue=vHash.value(__rowValue);
        auto itemRecord=itemValue.toHash();
        QStm::MetaEnum<RowType> rowType=vHash.value(__rowType);
        switch (rowType.type()) {
        case RowPageInfo:
            writePageInfo();
            break;
        case RowHeader:{
            if(!rowType.equal(lastRowType))
                writeColumns();
            break;
        }
        case RowValues:
            writeReportValues(itemRecord);
            break;
        case RowSingle:
            writeSingleLine(itemValue);
            break;
        case RowSummaryGrouping:
            writeReportValues(itemRecord);
            break;
        case RowSummaryHeader:
            writeColumns();
            break;
        case RowSummaryTotal:
            writeReportValues(itemRecord);
            break;
        case RowSignature:
            writeSignatures(itemRecord);
            break;
        default:
            break;
        }

        if(vRecordList.isEmpty())
            break;

        if((this->maxRows>0) && (this->maxRows<=++rowCount)){
            if(&item!=&this->items.last())
                pageNew();
        }
        lastRowType=rowType.type();
    }

    painter.end();

    auto __return=file.fileName();

    file.flush();
    file.close();
    return __return;
}



QString MakerPvt::printCSV_TXT()
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



QByteArray MakerPvt::fieldValueAlign(Header *header, const QString &value)
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
    case Header::Alignment::Start:
        return text.leftJustified(length,' ',true).toLatin1();
    case Header::Alignment::End:
        return text.rightJustified(length,' ', true).toLatin1();
    default://Header::Alignment::Center
        bool odd=false;
        while(text.length()<length)
            text=odd?(' '+text):(text+' ');
        return text.toLatin1();
    }
}

}
