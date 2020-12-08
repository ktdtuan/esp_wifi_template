#ifndef	_ASYNC_WEB_SOCKET_H
#define _ASYNC_WEB_SOCKET_H

#include <ESPAsyncWebServer.h>
#include <TimeOutEvent.h>

#define NUM_WS_CONNECTION_MAX   (DEFAULT_MAX_WS_CLIENTS - 3)
#define WS_INTERVAL_TIMEOUT_NUM  10000 /* MS */

typedef enum {
    ASYNC_WS_DISCONNECT,
    ASYNC_WS_CONNECTED
} ws_connection_status_t;

typedef struct {
    uint32_t timestamp;
    uint8_t ws_num;
    uint8_t status;
} ws_connection_info_t;

ws_connection_info_t ws_connection[NUM_WS_CONNECTION_MAX] = {0};

/* inteval timeout check temperature */
TimeOutEvent ws_interval_broadcast(WS_INTERVAL_TIMEOUT_NUM);

AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

void web_socket_init(void (*cb)(uint8_t, char *));
void ws_disconnect(uint8_t ws_index);
void ws_connection_establish(uint8_t ws_num);
void ws_connection_remove(uint8_t ws_num);
uint8_t ws_connection_available(void);
uint8_t ws_connection_connected(void);
uint8_t ws_connection_index_has_tl_max(void);
void ws_send_txt(uint8_t ws_index, char *payload);
void ws_send_broadcast_txt(char *payload);
uint8_t ws_connection_connected(void);
void ws_interval_sync(void);

#endif