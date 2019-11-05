#ifndef QJSONSTDVARIANTCONVERTER_P_H
#define QJSONSTDVARIANTCONVERTER_P_H

#include "qtjsonserializer_global.h"
#include "qjsontypeconverter.h"

namespace QtJsonSerializer::TypeConverters {

class Q_JSONSERIALIZER_EXPORT QJsonStdVariantConverter : public QJsonTypeConverter
{
public:
	bool canConvert(int metaTypeId) const override;
	QList<QCborValue::Type> allowedCborTypes(int metaTypeId, QCborTag tag) const override;
	QCborValue serialize(int propertyType, const QVariant &value) const override;
	QVariant deserializeCbor(int propertyType, const QCborValue &value, QObject *parent) const override;
};

}

#endif // QJSONSTDVARIANTCONVERTER_P_H
