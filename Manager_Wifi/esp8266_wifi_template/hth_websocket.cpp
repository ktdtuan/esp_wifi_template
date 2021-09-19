#include "hth_console_dbg.h"
#include "hth_websocket.h"

#define WS_DBG_PORT CONSOLE_PORT
#define WS_TAG_CONSOLE(...) CONSOLE_TAG_LOGI("[WS]", __VA_ARGS__)

static wsCallbacks defaultCallbacks; //null-object-pattern

hth_websocket::hth_websocket(const String& ws, const String& event)
:_wsUrl(ws), _eventUrl(event)
{
    memset(_ws_connection, 0, sizeof(_ws_connection));
}

hth_websocket::~hth_websocket()
{
    this->end();
}

Ticker* hth_websocket::_ws_ticker = nullptr;
AsyncWebSocket* hth_websocket::_ws = nullptr;
AsyncEventSource* hth_websocket::_events = nullptr;
wsCallbacks* hth_websocket::_pCallbacks = nullptr;
dataSocketHandler hth_websocket::_dataHandler = nullptr;
ws_connection_info_t hth_websocket::_ws_connection[NUM_WS_CONNECTION_MAX] = {0};

void hth_websocket::setHandleCallbacks(wsCallbacks* pCallbacks)
{
    if (pCallbacks != nullptr)
    {
        _pCallbacks = pCallbacks;
    }
    else
    {
        _pCallbacks = &defaultCallbacks;
    }
}

void hth_websocket::end()
{
    delete _ws;
    delete _events;
    delete _ws_ticker;
}

void hth_websocket::begin()
{
    _ws_ticker = new Ticker();
    _ws = new AsyncWebSocket(_wsUrl);
    _events = new AsyncEventSource(_eventUrl);
    _ws->onEvent(onWsEvent);
    _events->onConnect([](AsyncEventSourceClient *client){
        client->send("hello!", NULL, millis(), 1000);
        WS_TAG_CONSOLE("events connect: %u", client->lastId());
    });

    _ws_ticker->attach(WS_INTERVAL_TIMEOUT_NUM, intervalCleanUpClients); 
}

/* send message to client */
void hth_websocket::sendTxt(uint8_t ws_index, char *payload)
{
    _ws->text(ws_index, payload);
}

/* send data to all connected clients */
void hth_websocket::sendBroadcastTxt(char *payload)
{
    _ws->textAll(payload);
}

void hth_websocket::intervalCleanUpClients(void)
{
    uint8_t ws_cnt;

    ws_cnt = connectedNumber();    
    if (ws_cnt)
    {        
        // _ws->textAll("{\"page\":100,\"socket_num\":" + String(ws_cnt) + "}");
        WS_TAG_CONSOLE("connectedNumber: %u", ws_cnt);       
        WS_TAG_CONSOLE("Heap: %u", ESP.getFreeHeap()); 
    }

    _ws->cleanupClients();
}

/* Init and register callback receive message */
void hth_websocket::onDataHandler(dataSocketHandler handler)
{
    _dataHandler = handler;
}

void hth_websocket::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        IPAddress ip = client->remoteIP();
        uint16_t port = client->remotePort();
        WS_TAG_CONSOLE("[%u] Connected from %d.%d.%d.%d port: %u url: %s\n", client->id(), ip[0], ip[1], ip[2], ip[3], port, server->url());
        /* NUM_WS_CONNECTION_MAX < DEFAULT_MAX_WS_CLIENTS 
         * Because at least have a socket connection free
         */
        if (0 == connectionAvailable())
        {
            /* Disconnect socket has timelive max */
            disconnect(connectionHasTimeLiveMax());
        }

        /* new connection establish */
        connectionEstablish(client->id());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        WS_TAG_CONSOLE("ws[%s][%u] disconnect\n", server->url(), client->id());
        connectionRemove(client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        WS_TAG_CONSOLE("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        WS_TAG_CONSOLE("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
            //the whole message is in a single frame and we got all of it's data
            WS_TAG_CONSOLE("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
                /* Call callback */
                if (_dataHandler)
                {
                    _dataHandler(client, (char *)msg.c_str());
                }

                _pCallbacks->onDataReceived(client, (char *)msg.c_str());
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            WS_TAG_CONSOLE("%s\n", msg.c_str());
        }
        else
        {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0)
            {
                if (info->num == 0)
                    WS_TAG_CONSOLE("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                WS_TAG_CONSOLE("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            WS_TAG_CONSOLE("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            WS_TAG_CONSOLE("%s\n", msg.c_str());

            if ((info->index + len) == info->len)
            {
                WS_TAG_CONSOLE("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final)
                {
                    WS_TAG_CONSOLE("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    if (info->message_opcode == WS_TEXT)
                    {
                        /* Call callback */
                        if (_dataHandler)
                        {
                            _dataHandler(client, (char *)msg.c_str());
                        }

                        _pCallbacks->onDataReceived(client, (char *)msg.c_str());
                    }
                }
            }
        }
    }
}

/* Disconnect */
void hth_websocket::disconnect(uint8_t ws_index)
{
    _ws->close(_ws_connection[ws_index].ws_num);
    _ws_connection[ws_index].status = ASYNC_WS_DISCONNECT;
    _ws_connection[ws_index].ws_num = NUM_WS_CONNECTION_MAX;
    _ws_connection[ws_index].timestamp = millis();
    WS_TAG_CONSOLE("Disconnect ws_num %u at %u", _ws_connection[ws_index].ws_num, ws_index);
}

/* Add connection */
void hth_websocket::connectionEstablish(uint8_t ws_num)
{
    for (uint8_t i = 0; i < NUM_WS_CONNECTION_MAX; ++i)
    {
        if (ASYNC_WS_DISCONNECT == _ws_connection[i].status)
        {
            WS_TAG_CONSOLE("Establish index %u at %u", ws_num, i);
            _ws_connection[i].status = ASYNC_WS_CONNECTED;
            _ws_connection[i].ws_num = ws_num;
            _ws_connection[i].timestamp = millis();
            break;
        }
    }
}

/* Remove connection */
void hth_websocket::connectionRemove(uint8_t ws_num)
{
    for (uint8_t i = 0; i < NUM_WS_CONNECTION_MAX; ++i)
    {
        if (ws_num == _ws_connection[i].ws_num)
        {
            WS_TAG_CONSOLE("Remove index %u at %u", ws_num, i);
            _ws_connection[i].status = ASYNC_WS_DISCONNECT;
            _ws_connection[i].ws_num = NUM_WS_CONNECTION_MAX;
            _ws_connection[i].timestamp = millis();
            break;
        }
    }
}

/* Return the number socket free 
 * 0: There are not socket index to establish
 */
uint8_t hth_websocket::connectionAvailable(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_WS_CONNECTION_MAX; ++i)
    {
        if (ASYNC_WS_DISCONNECT == _ws_connection[i].status)
        {
            ++count;
        }
    }

    WS_TAG_CONSOLE("Available %u", count);
    return count;
}

/* Return the number socket is connected
 * 0: There are not socket index connected
 */
uint8_t hth_websocket::connectedNumber(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_WS_CONNECTION_MAX; ++i)
    {
        if (ASYNC_WS_CONNECTED == _ws_connection[i].status)
        {
            ++count;
        }
    }

    // WS_TAG_CONSOLE("Connected %u", count);
    return count;
}

/* Brief: return websocket index in array_list has timelive max */
uint8_t hth_websocket::connectionHasTimeLiveMax(void)
{
    uint32_t tl_sub;
    uint32_t tl_max = 0;
    uint32_t now = millis();
    uint8_t ws_index = 0;
    for (uint8_t i = 0; i < NUM_WS_CONNECTION_MAX; ++i)
    {
        tl_sub = now - _ws_connection[i].timestamp;
        if (tl_sub >= tl_max)
        {
            tl_max = tl_sub;
            ws_index = i;
        }
    }

    WS_TAG_CONSOLE("Timelive max is ws_num %u at %u(%us)",
                 _ws_connection[ws_index].ws_num, ws_index, tl_max / 1000);

    return ws_index;
}

wsCallbacks::~wsCallbacks() {}
/**
* Handler called after once received data.
*/
void wsCallbacks::onDataReceived(AsyncWebSocketClient* client, char* data)
{
    WS_TAG_CONSOLE("[wsCallbacks] >> onDataReceived: default <<");
}