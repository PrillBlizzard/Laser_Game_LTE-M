#ifndef LTE_M_H
#define LTE_M_H


//define the port for HTTP and HTTPS connections
#define HTTP_PORT 8000
#define HTTPS_PORT 4443

#if defined(CONFIG_NET_CONFIG_PEER_IPV4_ADDR)
#define SERVER_ADDR4 CONFIG_NET_CONFIG_PEER_IPV4_ADDR
#else
#define SERVER_ADDR4 ""
#endif

#define MAX_RECV_BUF_LEN 512

#define TARGET_SERVER_ADDR "51.38.237.79" // replacing SERVER_ADDR4 when sending

int init_LTE(void);
int send_hit_info(unsigned long decoded_data);


#endif // LTE_M_H