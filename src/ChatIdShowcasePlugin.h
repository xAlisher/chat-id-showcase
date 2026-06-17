#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include "interface.h"        // PluginInterface + base-class logosAPI (builder-provided)
#include "logos_api.h"        // LogosAPI
#include "logos_api_client.h" // LogosAPIClient::onEventResponse

// Stand-in for logos-chat-module: holds a user-supplied "Chat ID" (intro bundle)
// and hands it to consumers over the EXACT same IPC contract the real chat_module
// uses — createIntroBundle() + the chatCreateIntroBundleResult event. This lets the
// QR Generator's receive path be exercised without standing up the Waku chat stack.
class ChatIdShowcasePlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.logos.ChatIdShowcaseInterface" FILE "plugin_metadata.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit ChatIdShowcasePlugin(QObject* parent = nullptr);

    QString name()    const override { return QStringLiteral("chat_id_showcase"); }
    QString version() const override { return QStringLiteral("0.1.0"); }

    // Called by the shell via reflection — do NOT mark override.
    Q_INVOKABLE void initLogos(LogosAPI* api);

    // Store the Chat ID to hand out. Returns true.
    Q_INVOKABLE bool setIntroBundle(const QString& bundle);

    // Same signature/semantics as chat_module: emits chatCreateIntroBundleResult
    // [bool success, int status, QString bundle, QString iso8601] with the stored ID.
    // Returns true if a non-empty ID was available to emit.
    Q_INVOKABLE bool createIntroBundle();

signals:
    // REQUIRED — ModuleProxy connects to this on load; without it every callModule
    // returns {"error":"Invalid response"}.
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    // Fire emitEvent on the NEXT event-loop iteration (Qt::QueuedConnection), so the
    // Q_INVOKABLE that triggered it returns first and the host can flush the QRO reply.
    // Emitting synchronously from inside an IPC-invoked slot starves/re-enters the QRO
    // event loop → heap corruption (std::bad_alloc / "corrupted size vs prev_size").
    // Mirrors logos-chat-module's deferred-emission fix (commit e3a9078).
    void deferredEmit(const QString& eventName, const QVariantList& data);
    void emitEvent(const QString& eventName, const QVariantList& data);

    QString m_introBundle;

    // Receiver for the queued metacall in deferredEmit(). Being a member, it is destroyed
    // with the plugin, so Qt drops any pending queued emit — the captured `this` is never
    // dereferenced after free.
    QObject m_emitRouter;
};
