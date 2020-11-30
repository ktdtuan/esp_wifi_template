#ifndef	_APP_WS_H
#define _APP_WS_H

typedef enum {
    WS_PAGE_CARD_HISTORY = 0,
    WS_PAGE_MANAGE,
    WS_PAGE_CARD_INFO
} ws_page_num_t;

typedef enum {
    PAGE_CMD_CONNECT = 0,
    PAGE_CMD_RECEIVE_DATA,
    PAGE_CMD_REPLY_DATA,
    PAGE_CMD_ASYCN_DATA
} page_card_user_cmd_t;

typedef enum {
    WS_STATUS_ERROR = 0,
    WS_STATUS_OK
} ws_status_t;

const char *const ws_page_list[] PROGMEM = {
	"WS_PAGE_CARD_HISTORY",	
	"WS_PAGE_MANAGE",
	"WS_PAGE_CARD_INFO"
};

const char *const page_card_user_list[] PROGMEM = {
	"PAGE_CMD_CONNECT",	
	"PAGE_CMD_RECEIVE_DATA",
	"PAGE_CMD_REPLY_DATA",
    "PAGE_CMD_ASYCN_DATA"
};

void ws_receive_txt_callback(uint8_t ws_index, uint8_t * payload);

#endif