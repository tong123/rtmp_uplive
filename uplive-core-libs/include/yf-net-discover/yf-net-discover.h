#ifndef YFNETDISCOVER_H
#define YFNETDISCOVER_H

#include <string>

#ifdef WIN32

#ifdef	YFNETDISCOVER_EXPORTS
#define YFDLL_YFNETDISCOVER_API __declspec(dllexport) bool __stdcall
#else
#define YFDLL_YFNETDISCOVER_API __declspec(dllimport) bool __stdcall
#endif

#else

#define YFDLL_YFNETDISCOVER_API bool
#endif

#ifndef DEFINE_YHANDLE
    typedef void* YHANDLE;
    #define DEFINE_YHANDLE
#endif

//网络设备发现库服务端收到客户端回应时的回调函数
//s_name:网络设备发送到服务端的自己的设备名
//s_ip:网络设备发送服务端的自己的ip
//p_user_data:用户自定义数据
typedef bool ( *NET_DISCOVER_CB_FUNC )( const std::string& s_name, const std::string& s_ip, void *p_user_data );
//网络设备发现库服务端初始化
//h:网络设备发现库句柄
//p_func:网络设备发现库服务端收到客户端回应时的回调函数
//p_user_data:用户自定义数据
YFDLL_YFNETDISCOVER_API yf_net_discover_server_init( YHANDLE &h, NET_DISCOVER_CB_FUNC p_func, void *p_user_data );
//服务端广播设备发现命令
//h:网络设备发现库句柄
YFDLL_YFNETDISCOVER_API yf_net_discover_server_find_equipments( YHANDLE h );
//网络设备发现服务端结束
YFDLL_YFNETDISCOVER_API yf_net_discover_server_uninit( YHANDLE &h );
//网络设备发现客户端初始化
//h:网络设备发现库句柄
//str_client_name:设备发现客户端的设备名
YFDLL_YFNETDISCOVER_API yf_net_discover_client_init( YHANDLE &h, const std::string &str_client_name );
//网络设备发现库客户端结束
//h:网络设备发现库句柄
YFDLL_YFNETDISCOVER_API yf_net_discover_client_uninit( YHANDLE &h );

#endif // YFNETDISCOVER_H

