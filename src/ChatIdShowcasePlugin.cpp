#include "ChatIdShowcasePlugin.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaObject>

ChatIdShowcasePlugin::ChatIdShowcasePlugin(QObject* parent)
    : QObject(parent)
{
    qDebug() << "ChatIdShowcasePlugin constructed";
}

void ChatIdShowcasePlugin::initLogos(LogosAPI* api)
{
    // Store in the PluginInterface base-class member (the shell checks it to
    // verify initialisation) — do NOT shadow it with a private member.
    logosAPI = api;
    qDebug() << "ChatIdShowcasePlugin: Logos API initialized";
}

bool ChatIdShowcasePlugin::setIntroBundle(const QString& bundle)
{
    m_introBundle = bundle;
    qDebug() << "ChatIdShowcasePlugin: introBundle set, length" << bundle.size();
    return true;
}

bool ChatIdShowcasePlugin::createIntroBundle()
{
    const bool success = !m_introBundle.isEmpty();
    qDebug() << "ChatIdShowcasePlugin::createIntroBundle, have id =" << success;

    QVariantList eventData;
    eventData << success;                                              // success
    eventData << 0;                                                   // status code
    eventData << m_introBundle;                                       // the Chat ID
    eventData << QDateTime::currentDateTime().toString(Qt::ISODate);  // timestamp

    // Emit the result event. Deferred (next event-loop turn) so this IPC-invoked slot
    // returns first and the host can flush the QRO reply — emitting synchronously here
    // starves the event loop (chat_module's e3a9078 deferred-emission fix).
    deferredEmit("chatCreateIntroBundleResult", eventData);

    return success;
}

void ChatIdShowcasePlugin::deferredEmit(const QString& eventName, const QVariantList& data)
{
    QMetaObject::invokeMethod(&m_emitRouter,
        [this, eventName, data]() { emitEvent(eventName, data); },
        Qt::QueuedConnection);
}

void ChatIdShowcasePlugin::emitEvent(const QString& eventName, const QVariantList& data)
{
    // Emit the eventResponse signal DIRECTLY. ModuleProxy is connected to it and bridges
    // it to subscribers. Do NOT route through logosAPI->getClient("chat_id_showcase")
    // (a client to SELF) — that path corrupts the heap (std::bad_alloc) on this SDK.
    emit eventResponse(eventName, data);
}
