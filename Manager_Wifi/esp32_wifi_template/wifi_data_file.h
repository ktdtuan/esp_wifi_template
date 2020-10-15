#ifndef	_WIFI_DATA_FILE_
#define _WIFI_DATA_FILE_

// typedef enum {
//     WIFI_UDP_PORT = 0,
//     WIFI_TCP_PORT,
//     WIFI_WS_PORT,
//     WIFI_USER_NAME,
//     WIFI_USER_ADDR,
//     WIFI_USER_AUTH,
//     WIFI_PASS_AUTH,
//     WIFI_STA_SSID,
//     WIFI_STA_PASS,
//     WIFI_STA_HOTNAME,
//     WIFI_STA_STATIC,
//     WIFI_STA_SSID
// } wifi_file_cmd_t;

#define WIFI_FILE_PATH "/wifi_info_file.txt"

#define Df_LengHostName       32
#define Df_LengSsid           32
#define Df_LengPass           64
#define Df_LengAuth           10
#define Df_LengAddr           100
#define Df_LengServer         32

typedef struct {
    uint16_t 		UDPPort;      /*UDP Port*/
    uint16_t 		TCPPort;      /*TCP Port*/
    uint16_t    WSPort;       /*Websocket Port*/
    struct {
        char    Username[Df_LengAddr + 1];
        char    Useraddr[Df_LengAddr + 1];
    }addr;
    struct {
        char    user[Df_LengAuth + 1];
        char    pass[Df_LengAuth + 1];
    }auth;
    struct {
        char 		ssid[Df_LengSsid + 1];
        char 		pass[Df_LengPass + 1];
        char    HostName[Df_LengHostName+1];
        uint8_t 	SaticIp;
        IPAddress 	Ip;
        IPAddress 	Gw;
        IPAddress 	Sn;
        IPAddress 	Dns;
    }sta;
    struct {
        char 		ssid[Df_LengSsid + 1];
        char 		pass[Df_LengPass + 1];
        char    DnsName[Df_LengHostName+1];
        IPAddress   Ip;
        IPAddress   Sn;
        uint8_t 	Chanel : 4;
        uint8_t 	Hidden : 1;
    }ap;
} wifi_file_json_t;

#endif