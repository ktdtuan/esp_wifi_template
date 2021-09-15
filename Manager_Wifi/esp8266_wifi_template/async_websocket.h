#ifndef	_ASYNC_WEB_SOCKET_H
#define _ASYNC_WEB_SOCKET_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>

#define NUM_WS_CONNECTION_MAX   (DEFAULT_MAX_WS_CLIENTS - 3)
#define WS_INTERVAL_TIMEOUT_NUM  60 /* S */

typedef enum {
    ASYNC_WS_DISCONNECT,
    ASYNC_WS_CONNECTED
} ws_connection_status_t;

typedef struct {
    uint32_t timestamp;
    uint8_t ws_num;
    uint8_t status;
} ws_connection_info_t;

/**
 * Params 1: client
 * Params 2: data received
*/
typedef std::function<void(AsyncWebSocketClient*, char*)> dataSocketHandler;

class wsCallbacks {
public:
	virtual ~wsCallbacks();
    /**
    * Handler called after once received data.
    */
    virtual void onDataReceived(AsyncWebSocketClient* client, char* data);
};

class async_websocket
{
private:
    static Ticker* _ws_ticker;
    static dataSocketHandler _dataHandler;
    static wsCallbacks* _pCallbacks;
    static ws_connection_info_t _ws_connection[NUM_WS_CONNECTION_MAX];
    static void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
    static void disconnect(uint8_t ws_index);
    static void connectionEstablish(uint8_t ws_num);
    static void connectionRemove(uint8_t ws_num);
    static uint8_t connectionAvailable(void);
    static uint8_t connectedNumber(void);
    static uint8_t connectionHasTimeLiveMax(void);
    static void intervalCleanUpClients(void);

    String _wsUrl;
    String _eventUrl;
public:
    async_websocket(const String& ws, const String& event);
    ~async_websocket();
    static AsyncWebSocket* _ws;
    static AsyncEventSource* _events;
    void begin();
    void end();
    void onDataHandler(dataSocketHandler handler);
    void sendTxt(uint8_t ws_index, char *payload);
    void sendBroadcastTxt(char *payload);
    void setHandleCallbacks(wsCallbacks* pCallbacks);
    void eventsSend(const char *message, const char *event)
    {
        _events->send(message, event);
    }
};

#endif
