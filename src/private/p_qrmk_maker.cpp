#include "./p_qrmk_maker.h"
#include "../qstm/src/qstm_util_formatting.h"
#include "../qstm/src/qstm_meta_enum.h"

namespace QRmk{


static const auto extPDF="pdf";
static const auto extCSV="csv";
static const auto extTXT="txt";

static const auto sepCSV=";";
static const auto sepTXT="|";

static const auto __spaceJoin=", ";

static const auto __rowType="__row_type__";
static const auto __rowValue="__row_value__";

static const auto __total=QObject::tr("Totalização");
static const auto __totalFinal=QObject::tr("Totalização final");

typedef QVector<QByteArray> PropertyNames;

Q_GLOBAL_STATIC_WITH_ARGS(PropertyNames, staticIgnoreMethods,({"objectName","values","measures","asJson", "measures", "baseValues", "clearOnSetFail"}))


static const double rowFactor=0.02;
static const double rowFactorSeparator=0.2;

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
    this->headersSummary.clear();
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

        this->headersSummary.append(header);
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

const QVariantList &MakerPvt::makeRecords()
{
    if(this->items.isEmpty())
        return this->outPutRecord;

    auto &outPutRecord=this->outPutRecord;

    QVariantHash itemRecord;

    auto makeObject=[](const RowType rowType, const QVariant &rowValue){
        return QVariantHash{{__rowType, rowType}, {__rowValue,rowValue}};
    };

    auto writeObject=[&outPutRecord, &makeObject](const RowType rowType, const QVariant &rowValue={}){
        outPutRecord.append(makeObject(rowType, rowValue));
    };

    auto writeLine=[&writeObject](const RowType rowType, const QVariant &rowValue){
        writeObject(rowType, rowValue);
    };
//    auto writeColumns=[&writeLine, &lastRowType](RowType rowType=RowHeader, const QVariant &values={})//draw headers
//    {
//        if(lastRowType==rowType)
//            return;
//        writeLine(rowType, values);
//    };
//    auto writeSingleLine=[&writeLine](const QVariant &outItem, RowType rowType=RowSingle)//draw headers
//    {
//        writeLine(rowType, outItem);
//    };
    auto writeLineValues=[&writeLine](const QVariantHash &itemRecord)//draw headers
    {
        writeLine(RowValues, itemRecord);
    };

    auto writeSummary=[this, &itemRecord, &writeObject, &makeObject](const QVariantList &vSummaryList, RowType rowType, const QString &singleRowText)
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

        QVariantList vFinalSummaryRows;
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
                vFinalSummaryRows.append(itemRowFormatted);
            }
        }
        if(!vFinalSummaryRows.isEmpty()){
            QVariantList vOut;
            if(!singleRowText.trimmed().isEmpty())
                vOut.append(makeObject(RowSingle, singleRowText));

            for(auto &value:vFinalSummaryRows)
                vOut.append(makeObject(RowSummaryValues, value));

            writeObject(rowType, vOut);
        }
        return true;
    };

    auto makeSingleLine=[this, &itemRecord](){
        if(this->groupingFields.isEmpty())
            return QString{};
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
        return vLine.join(__spaceJoin).trimmed();
    };

    QVariantHash vLastRow;
    QVariantList vSummaryRows;
    auto groupingCheck=[this, &vLastRow, &vSummaryRows, &writeSummary, &makeSingleLine](const QVariantHash &itemRecord, bool lastSummary=false)//draw headers
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
        writeSummary(vSummaryRows, RowSummaryGrouping, makeSingleLine());
        vLastRow=itemRecord;
        vSummaryRows.clear();
        vSummaryRows.append(itemRecord);//rows to group summary
        return true;
    };

//    auto pageStart=[&writeLine, &makeSingleLine, &writeSingleLine](const QVariantHash &itemRecord){
//        writeLine(RowPageInfo,itemRecord);
//        writeSingleLine(makeSingleLine());
//        writeColumns();
//    };

    auto writeSignatures=[this, &writeLine](const QVariant &itemRecord)
    {
        if(this->signature.isEmpty())
            return;
        writeLine(RowSignature, itemRecord);
    };

    {//write pages
        itemRecord=(this->items.isEmpty())?itemRecord:this->items.first().toHash();
        //pageStart(itemRecord);
        writeObject(RowPageInfo,itemRecord);
        writeObject(RowSingle, makeSingleLine());
        writeObject(RowHeader);
        for(auto &item: this->items){
            auto vHash=item.toHash();
            if(vHash.contains(__rowType) && vHash.contains(__rowValue)){
                outPutRecord.append(vHash);
                vHash=vHash.value(__rowValue).toHash();
            }
            groupingCheck(itemRecord=vHash);
            writeLineValues(vHash);
        }
    }

    groupingCheck({},true);

    writeSummary(this->items, RowSummaryTotal, __totalFinal);
    writeSignatures(itemRecord);

    return outPutRecord;
}

QString MakerPvt::makerPDF()
{
    this->makeRecords();

    if(outPutRecord.isEmpty())
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

    //static const double factor=1.2;

    pdfWriter=new QPdfWriter(&file);
    painter=new QPainter(pdfWriter);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    fontBold.setBold(true);
    fontItalic.setItalic(true);
    painter->setFont(fontNormal);

    pdfWriter->setPageSize(QPageSize::A4);
    switch (this->orientation.type()) {
    case Maker::Orientation::Landscape:
        pdfWriter->setPageOrientation(QPageLayout::Orientation::Landscape);
        break;
    default:
        pdfWriter->setPageOrientation(QPageLayout::Orientation::Portrait);
        break;
    }


    this->spacing=(this->spacing>0)?this->spacing:(pdfWriter->width()*0.005);
    this->textOffSetT=(this->textOffSetL>0)?this->textOffSetT:(this->spacing);
    this->textOffSetL=(this->textOffSetL>0)?this->textOffSetL:(this->spacing);
    this->textOffSetR=(this->textOffSetR>0)?this->textOffSetR:(this->spacing*2);
    this->textOffSetB=(this->textOffSetB>0)?this->textOffSetB:(this->spacing*2);
    this->totalHeight=(this->totalHeight>0)?this->totalHeight:(painter->viewport().height());
    this->totalWidth=(this->totalWidth>0)?this->totalWidth:painter->viewport().width();
    this->rowHeight=(this->rowHeight>0)?this->rowHeight:(totalHeight*rowFactor);
    this->maxRows=(maxRows>0)?maxRows:this->getLines();
    this->startY=0;
    this->totalPageInfo=0;
    this->rowCount=0;
    this->pageCount=0;
    this->rowWidth=0;


    {//calc columns rectangle


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
                static const auto __formatWidth=QString("%1%");
                auto withNew=__formatWidth.arg(QString::number(perNew,'f',6));
                header->width(withNew);
            }
        }



        int startX=0, startY=0;
        {
            //QRect rect={};
            auto pointLine=QPoint{0,0};
            columnsHeaders.clear();
            for(auto header : this->headersList){
                double per=vu.toDouble(header->width());
                double w=(totalWidth*per);
                this->rowWidth+=w;
                auto rectHeader=QRect{pointLine.x(), startY, int(w), int(this->rowHeight)};
                auto rectText=QRect{pointLine.x(), startY, int(w-textOffSetR), rectHeader.height()};
                columnsHeaders.insert(header, QPair<QRect,QRect>(rectHeader, rectText));

                pointLine.setX(startX+=w);
            }
        }
        rectSingleRow=QRect(0,0, startX, int(this->rowHeight*rowFactor));

        {//summary headers calc
            int startX=-1;
            double endWidth=0;
            for(auto header : headersList){
                auto rectBase = columnsHeaders.value(header).first;

                auto headerSummary=this->summary.at(header->field());

                if(headerSummary==nullptr){
                    if(startX<rectBase.x())
                        startX=rectBase.x();
                    if(header==headersList.last()){
                        endWidth=rectBase.width();
                    }
                    else{
                        endWidth+=rectBase.x()+rectBase.width();
                        continue;
                    }
                }
                else{
                    if(startX<0)
                        startX=rectBase.x();
                    endWidth+=rectBase.width();
                }


                startX=(startX<0)?0:startX;
                auto rectHeader=QRect(startX, startY, endWidth, rectBase.height());
                auto rectText=QRect(startX+textOffSetL, startY, endWidth-(textOffSetR), rectBase.height());
                columnsSummary.insert(header, QPair<QRect,QRect>(rectHeader, rectText));

                startX=-1;
                endWidth=0;
            }
        }
    }
    rectFull=(rectFull.width()>0)?rectFull:QRect(0, 0, rowWidth, rowHeight);



    FormattingUtil fu;
    this->__time=tr("Emissão: %1 %2").arg(fu.v(QDate::currentDate()),fu.v(QTime::currentTime()));

    parseList(outPutRecord);
    painter->end();

    auto __return=file.fileName();

    file.flush();
    file.close();
    return __return;
}

QString MakerPvt::makeCSV_TXT()
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

double QRmk::MakerPvt::pdfNextY(double factor)
{
    rowCount+=(factor>=1)?factor:1;
    return (startY+=(rowHeight*factor));
}

void QRmk::MakerPvt::pdfWritePageInfo()
{
    auto totalLinesInfo=2;//time+page
    totalLinesInfo+=this->extraPageInfo.count();

    auto infoH=(rowHeight*totalLinesInfo);

    auto rect=QRect(spacing, startY=spacing, rowWidth-textOffSetR, infoH-textOffSetB);
    painter->setFont(fontNormal);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    painter->drawText(rect, Qt::AlignRight, __time);

    for(auto extra:this->extraPageInfo){
        extra=this->parserText(extra,itemRecord);
        if(extra.isEmpty())
            continue;
        rect.setY(pdfNextY());
        painter->drawText(rect, Qt::AlignRight, extra);
    }
    static const auto __page=tr("Página: %1");
    rect.setY(pdfNextY());
    painter->drawText(rect, Qt::AlignRight, __page.arg(++pageCount));

    if(!this->title.isEmpty()){
        painter->setFont(fontBold);
        painter->drawText(rect, Qt::AlignHCenter, this->title);
    }

    if(!this->owner.isEmpty()){
        painter->setFont(fontItalic);
        painter->drawText(rect, Qt::AlignLeft, this->owner);
    }

    int maxW=rowWidth;
    rect=QRect{0, 0, maxW, startY+rect.height()};
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    painter->drawRect(rect);
    totalPageInfo=rect.height();
}

void QRmk::MakerPvt::pdfWriteLineHeaders()
{
    pdfNextY();
    painter->setFont(fontNormal);
    const auto &headers=headersList;
    const auto &columnsRect=columnsHeaders;
    for(auto header : headers){
        auto rectBase = columnsRect.value(header).first;
        auto rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());
        auto value=header->title();

        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::black);
        painter->drawRect(rect);

        rectBase = columnsRect.value(header).second;
        rect=QRect(rectBase.x()+textOffSetL, startY, rectBase.width(), rectBase.height());
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);
        QRect boundingRect;
        painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, value, &boundingRect);
    }
    //nextY(rowFactorSeparator);
}

void QRmk::MakerPvt::pdfWriteSummaryLineHeaders()
{
    //nextY();
    painter->setFont(fontNormal);

    for(auto header : this->headers.list()){
        if(!columnsSummary.contains(header))
            continue;

        auto pair=columnsSummary.value(header);
        auto rectBase = pair.first;
        auto value=header->title();

        QRect rect={};

        rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());

        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::black);
        painter->drawRect(rect);

        rectBase = pair.second;
        rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());

        if(this->summary.contains(header->field())){
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::black);
            QRect boundingRect;
            painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, value, &boundingRect);
        }
    }
    //nextY(rowFactorSeparator);
}

void QRmk::MakerPvt::pdfWriteLine(const QHash<Header *, QPair<QRect, QRect> > &columnsRow, const QVariantHash &itemRow)
{
    pdfNextY();
    int startX=-1;
    painter->setFont(fontNormal);
    for(auto &header : headersList){
        auto value=itemRow.value(header->field()).toString();

        auto rectBase = columnsRow.value(header).first;
        auto rect=QRect(rectBase.x(), startY, rectBase.width(), rectBase.height());

        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);
        painter->drawRect(rect);

        rectBase = columnsRow.value(header).second;
        rect=QRect(rectBase.x()+textOffSetL, startY+textOffSetL, rectBase.width()-textOffSetR, rectBase.height()-textOffSetB);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);
        QRect boundingRect;
        painter->drawText(rect, header->alignQt(), value, &boundingRect);

        if(startX==-1)
            startX=rect.x();
    }
}

void QRmk::MakerPvt::pdfWriteLineValues(const QVariantHash &itemRecord)
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
    pdfWriteLine(columnsHeaders, itemRowFormatted);
}

void QRmk::MakerPvt::pdfParseRow(const RowType &rowType, const QVariantHash &itemValue)
{
    QStm::MetaEnum<RowType> vType;

    if(rowType==RowNONE)
        vType=itemValue.value(__rowType);
    else
        vType=rowType;

    switch (vType.type()) {
    case RowPageInfo:
        pdfWritePageInfo();
        break;
    case RowHeader:{
        pdfWriteLineHeaders();
        break;
    }
    case RowValues:
        pdfWriteLineValues(itemValue);
        break;
    case RowSummaryValues:
        pdfWriteSummaryLineValues(itemValue);
        break;
    case RowSingle:
        pdfWriteSingleLine(itemValue);
        break;
    case RowSummaryGrouping:
    case RowSummaryTotal:
        pdfWriteSummaryLineHeaders();
        break;
    case RowSignature:
        pdfWriteSignatures(itemValue);
        break;
    default:
        break;
    }
}

void QRmk::MakerPvt::parseList(QVariantList &vRecordList)
{
    while(!vRecordList.isEmpty()){
        auto item=vRecordList.takeFirst();
        auto vHash=item.toHash();
        auto &itemValue=vHash[__rowValue];
        QStm::MetaEnum<RowType> rowType=vHash.value(__rowType);
        switch (rowType.type()) {
        case RowPageInfo:
        case RowHeader:
            pdfParseRow(rowType.type(), {});
            break;
        case RowValues:
        case RowSummaryValues:
        case RowSingle:
            pdfParseRow(rowType.type(), itemValue.toHash());
            break;
        case RowSummaryGrouping:
        case RowSummaryTotal:{
            pdfParseRow(rowType.type());
            switch (itemValue.typeId()) {
            case QMetaType::QVariantList:{
                auto vList=itemValue.toList();
                itemValue={};
                parseList(vList);
                break;
            }
            default:
                pdfWriteSummaryLineHeaders();
                break;
            }
            break;
        }
        case RowSignature:
            pdfWriteSignatures(itemValue.toHash());
            break;
        default:
            break;
        }

        if(vRecordList.isEmpty())
            break;

        if((this->maxRows>0) && (this->maxRows<=++rowCount)){
            if(&item!=&this->items.last())
                pdfPageNew();
        }
    }
}

void QRmk::MakerPvt::pdfPageBlank()
{
    pdfWriter->newPage();
    pdfWritePageInfo();
}

void QRmk::MakerPvt::pdfPageStart()
{
    rowCount=0;
    pdfWritePageInfo();
    if(!this->outPutRecord.isEmpty())
        pdfWriteLineHeaders();
}

void QRmk::MakerPvt::pdfPageNew()
{
    pdfWriter->newPage();
    pdfPageStart();
}

void QRmk::MakerPvt::pdfWriteSummaryLineValues(const QVariantHash &itemRecord)
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
    pdfWriteLine(columnsSummary,itemRowFormatted);
}

void QRmk::MakerPvt::pdfWriteSingleLine(const QVariant &outItem)
{
    if(headersList.isEmpty())
        return;
    pdfNextY(1.5);
    painter->setFont(fontBold);
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
    auto rectangle=QRect(rectSingleRow.x(), pdfNextY(1.2), rectSingleRow.width(), rectSingleRow.height());

    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::black);
    painter->drawRect(rectangle);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    painter->drawText(rectangle, Qt::AlignHCenter | Qt::AlignVCenter, textLine.join(__spaceJoin), &boundingRect);

    pdfNextY(rowFactorSeparator);
}

void QRmk::MakerPvt::pdfWriteSignatures(const QVariantHash &itemRecord)
{
    if(this->signature.isEmpty())
        return;

    Q_DECLARE_VU;

    auto areaMinH=vu.toDouble(this->signature.pageArea().height);
    auto areaCurH=(startY/this->totalHeight);
    {//area check
        if(areaMinH>areaCurH)
            pdfPageBlank();
    }

    if((areaCurH+areaCurH)>1.00)
        pdfPageBlank();


    const auto rectSignature=QRect(0, pdfNextY(2), this->rowWidth, (this->totalHeight-(startY+totalPageInfo)));

    QRect rectTitle=QRect(0, pdfNextY(0), rowWidth, rowHeight);
    if(!this->signature.title().isEmpty()){//title

        auto font=fontBold;
        font.setPointSize(font.pointSize()+2);
        painter->setFont(font);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);

        rectTitle.setY(pdfNextY(2));
        QRect boundingRect;
        painter->drawText(rectTitle, Qt::AlignCenter, this->signature.title(), &boundingRect);
    }

    {//declarations

        if(!this->signature.declaration().isEmpty()){

            //auto startX=(this->totalWidth/2)-(areaMinW/2);

            auto declarationRows=this->signature.declaration().count()*1.3;

            auto wArea=totalWidth*0.66;
            auto startX=((totalWidth/2)-(wArea/2));
            auto rect=QRect(startX, pdfNextY(2), wArea, (rowHeight*declarationRows));
            auto declaration=this->signature.declaration().join(" ");
            declaration=this->parserText(declaration, itemRecord);

            auto font=fontNormal;
            font.setPointSize(font.pointSize()+2);
            painter->setFont(font);

            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::black);
            auto pen=painter->pen();
            pen.setWidth(pen.width()*10);
            painter->setPen(pen);
            painter->drawRect(rect);

            rect=QRect(rect.x()+textOffSetL, rect.y()+textOffSetT, rect.width()-textOffSetR, rect.height()-textOffSetB);
            QRect boundingRect;
            painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignJustify | Qt::TextWordWrap, declaration, &boundingRect);
            startY=rect.y()+rect.height();
        }
    }

    if(!this->signature.localFormatted().isEmpty()){//title
        auto rect=QRect(0, pdfNextY(2),rowWidth,rowHeight);
        auto font=fontNormal;
        font.setPointSize(font.pointSize()+2);
        painter->setFont(font);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);

        QRect boundingRect;
        painter->drawText(rect, Qt::AlignCenter, this->signature.localFormatted(), &boundingRect);
    }

    if(!this->signature.signatures().isEmpty()){//title
        auto font=fontNormal;
        font.setPointSize(font.pointSize()+2);
        painter->setFont(font);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);

        const auto __startY=pdfNextY(3);

        auto rows=this->signature.signatures().count();
        auto wArea=(totalWidth*vu.toDouble(this->signature.width()));

        auto startX=(totalWidth/2)-((wArea*rows)/2);

        for(auto &signature:this->signature.signatures()){
            if(signature->document().isEmpty() && signature->name().isEmpty())
                continue;

            startY=__startY;

            auto rectSign=QRect(startX, pdfNextY(0), wArea, rowHeight);
            startX+=(wArea+spacing);

            {//line
                auto rectLine=QRect(rectSign.x(), rectSign.y(), rectSign.width(), 1);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(Qt::black);
                painter->drawRect(rectLine);
            }

            if(!signature->nameFormatted().isEmpty()){//Name
                auto rect=QRect(rectSign.x(), pdfNextY(0.5), rectSign.width(), rectSign.height());
                painter->setBrush(Qt::NoBrush);
                painter->setPen(Qt::black);
                auto text=this->parserText(signature->name(), itemRecord);
                QRect boundingRect;
                painter->drawText(rect, Qt::AlignCenter, text, &boundingRect);
            }

            if(!signature->documentFormatted().isEmpty()){//Document
                Q_DECLARE_FU;
                auto rect=QRect(rectSign.x(), pdfNextY(), rectSign.width(), rectSign.height());
                painter->setBrush(Qt::NoBrush);
                painter->setPen(Qt::black);
                auto text=this->parserText(signature->document(), itemRecord);
                QRect boundingRect;
                painter->drawText(rect, Qt::AlignCenter, text, &boundingRect);
            }
        }
    }

    {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::black);
        painter->drawRect(rectSignature);
    }
}


}
