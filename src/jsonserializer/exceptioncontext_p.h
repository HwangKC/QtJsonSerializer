#ifndef QTJSONSERIALIZER_EXCEPTIONCONTEXT_P_H
#define QTJSONSERIALIZER_EXCEPTIONCONTEXT_P_H

#include "qtjsonserializer_global.h"
#include "exception.h"

#include <QtCore/QMetaProperty>
#include <QtCore/QThreadStorage>

namespace QtJsonSerializer {

class Q_JSONSERIALIZER_EXPORT ExceptionContext
{
public:
	ExceptionContext(const QMetaProperty &property);
	ExceptionContext(int propertyType, const QByteArray &hint);
	~ExceptionContext();

	static SerializationException::PropertyTrace currentContext();

private:
	static QThreadStorage<SerializationException::PropertyTrace> contextStore;
};

}

#endif // QTJSONSERIALIZER_EXCEPTIONCONTEXT_P_H
