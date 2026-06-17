# chat-id-showcase

A tiny **stand-in for the real `logos-chat-module`**, built to exercise the
[`qr-basecamp`](https://github.com/xAlisher/qr-basecamp) QR Generator's
"receive a Chat ID from another module" path without standing up the Waku chat stack.

You type a **Chat ID** (the chat module's *intro bundle*), press **Generate**, and it
hands the ID to the QR Generator over the **exact same IPC contract** the real chat
module uses — `createIntroBundle()` → the `chatCreateIntroBundleResult` event.

## Layout

```
chat-id-showcase/
├── flake.nix · metadata.json · CMakeLists.txt   # core module (mkLogosModule)
├── src/ChatIdShowcasePlugin.{h,cpp}             # core: setIntroBundle / createIntroBundle
└── plugins/chat_id_showcase_ui/                 # ui_qml: input + Generate button
```

- **`chat_id_showcase` (core, C++):** holds a Chat ID. `setIntroBundle(id)` stores it;
  `createIntroBundle()` emits `chatCreateIntroBundleResult [success, status, id, ts]` —
  the same shape as `chat_module`, so the QR Generator consumes either unchanged.
- **`chat_id_showcase_ui` (ui_qml):** the input + **Generate** button → calls
  `setIntroBundle` then `createIntroBundle` (the push).

## The one platform lesson baked in here

A core module must emit its event by **emitting the `eventResponse` signal directly**:

```cpp
emit eventResponse(eventName, data);   // ✅ ModuleProxy is connected to this; it bridges to subscribers
```

Do **NOT** route through a client to *itself*:

```cpp
logosAPI->getClient("chat_id_showcase")->onEventResponse(this, eventName, data);  // ✗ heap-corrupts → std::bad_alloc
```

A module getting a `LogosAPIClient` to its own name corrupts the heap (`std::bad_alloc` /
`corrupted size vs. prev_size`). Emitting the signal directly is the SDK's intended path.
The emit is also **deferred** (`QMetaObject::invokeMethod(..., Qt::QueuedConnection)`) so the
`Q_INVOKABLE` returns before the event fires and the host can flush the QRO reply.

## Build + install (dev)

```bash
# core
nix build .#packages.x86_64-linux.lgx-portable -L
lgpm --modules-dir ~/.local/share/Logos/LogosBasecamp/modules \
     --ui-plugins-dir ~/.local/share/Logos/LogosBasecamp/plugins \
     --allow-unsigned install --file result/logos-chat_id_showcase-module-lib.lgx
# ui
cd plugins/chat_id_showcase_ui && nix build .#packages.x86_64-linux.lgx-portable -L
lgpm ... install --file result/logos-chat_id_showcase_ui-module.lgx
rm -rf ~/.cache/Logos/LogosBasecamp/qmlcache/   # then relaunch Basecamp
```

Headless smoke test (no GUI): `logoscore -m <dir> -l chat_id_showcase -c "chat_id_showcase.setIntroBundle(@id.json)" -c "chat_id_showcase.createIntroBundle()" --quit-on-finish`.
