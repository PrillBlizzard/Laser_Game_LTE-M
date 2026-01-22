/*
 * Program to set up and send HTTP GET and POST requests over LTE-M using Zephyr OS (and nrf9151 DK), by IPv4
 * Greatly inspired by the HTTP_CLIENT sample from nrf connect SDK application
 *
 * This should work if the modem firmware has correctly been flashed with the nrf connect for Desktop programmer,
 * and the IP adress of the server has been properly set in the "connect socket" functions from the "run queries" function.
 * Also be careful that your SD card is active so it can access LTE Network
 *
 * -- Gaël Chauvet - January 2026
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_http_client_sample, LOG_LEVEL_DBG);

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <nrf_modem.h>
#include <modem/nrf_modem_lib.h>

#include "ca_certificate.h"
#include "LTE-M.h"

// DEF of buffer
static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];

// Function to setup a socket connection
static int setup_socket(sa_family_t family, const char *server, int port,
                        int *sock, struct sockaddr *addr, socklen_t addr_len)
{
    const char *family_str = family == AF_INET ? "IPv4" : "IPv6";
    int ret = 0;

    memset(addr, 0, addr_len);

    if (family == AF_INET)
    {
        net_sin(addr)->sin_family = AF_INET;
        net_sin(addr)->sin_port = htons(port);
        inet_pton(family, server, &net_sin(addr)->sin_addr);
    }
    else
    {
        net_sin6(addr)->sin6_family = AF_INET6;
        net_sin6(addr)->sin6_port = htons(port);
        inet_pton(family, server, &net_sin6(addr)->sin6_addr);
    }

    if (IS_ENABLED(CONFIG_NET_SOCKETS_SOCKOPT_TLS))
    {
        sec_tag_t sec_tag_list[] = {
            CA_CERTIFICATE_TAG,
        };

        *sock = socket(family, SOCK_STREAM, IPPROTO_TLS_1_2);
        if (*sock >= 0)
        {
            ret = setsockopt(*sock, SOL_TLS, TLS_SEC_TAG_LIST,
                             sec_tag_list, sizeof(sec_tag_list));
            if (ret < 0)
            {
                LOG_ERR("Failed to set %s secure option (%d)",
                        family_str, -errno);
                ret = -errno;
            }

            ret = setsockopt(*sock, SOL_TLS, TLS_HOSTNAME,
                             TLS_PEER_HOSTNAME,
                             sizeof(TLS_PEER_HOSTNAME));
            if (ret < 0)
            {
                LOG_ERR("Failed to set %s TLS_HOSTNAME "
                        "option (%d)",
                        family_str, -errno);
                ret = -errno;
            }
        }
    }
    else
    {
        *sock = socket(family, SOCK_STREAM, IPPROTO_TCP);
    }

    if (*sock < 0)
    {
        LOG_ERR("Failed to create %s HTTP socket (%d)", family_str,
                -errno);
    }

    return ret;
}

// HTTP response callback function
static int response_cb(struct http_response *rsp,
                       enum http_final_call final_data,
                       void *user_data)
{
    if (final_data == HTTP_DATA_MORE)
    {
        LOG_INF("Partial data received (%zd bytes)", rsp->data_len);
    }
    else if (final_data == HTTP_DATA_FINAL)
    {
        LOG_INF("All the data received (%zd bytes)", rsp->data_len);
    }

    LOG_INF("Response to %s", (const char *)user_data);
    LOG_INF("Response status %s", rsp->http_status);

    return 0;
}

// Function to setup and connect a socket
static int connect_socket(sa_family_t family, const char *server, int port,
                          int *sock, struct sockaddr *addr, socklen_t addr_len)
{
    int ret;

    ret = setup_socket(family, server, port, sock, addr, addr_len);
    if (ret < 0 || *sock < 0)
    {
        return -1;
    }

    ret = connect(*sock, addr, addr_len);
    if (ret < 0)
    {
        LOG_ERR("Cannot connect to %s remote (%d): %s",
                family == AF_INET ? "IPv4" : "IPv6",
                -errno, strerror(errno));
        close(*sock);
        *sock = -1;
        ret = -errno;
    }

    return ret;
}

// LTE Handler function
/* Semaphore used to block the main thread until the link controller has
 * established an LTE connection.
 */
K_SEM_DEFINE(lte_connected, 0, 1);
static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_NW_REG_STATUS:
        if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
            (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            break;
        }
        printk("Connected to: %s network\n",
               evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "home" : "roaming");
        k_sem_give(&lte_connected);
        break;
    case LTE_LC_EVT_RRC_UPDATE:
    case LTE_LC_EVT_CELL_UPDATE:
    case LTE_LC_EVT_LTE_MODE_UPDATE:
    case LTE_LC_EVT_MODEM_EVENT:
        /* Callback events carrying LTE link data */
        break;
    default:
        break;
    }
}

int init_LTE(void)
{
    int ret = 0;

    LOG_INF("Initializing Modem");
    ret = nrf_modem_lib_init();
    if (ret != 0)
    {
        LOG_ERR("Error initializing modem : %d\n", ret);
    }
    else
    {
        LOG_INF("Modem Initialized succesfully");
    }

    LOG_INF("Initializing LTE link...");
    /* Initialize LTE link */
    ret = lte_lc_connect_async(lte_handler);
    if (ret != 0)
    {
        LOG_ERR("Failed to initiate LTE connection: %d", ret);
        return ret;
    }
    else
    {
        LOG_INF("LTE link initialized");
    }
    k_sem_take(&lte_connected, K_FOREVER);

    LOG_INF("LTE ready");
}

int send_hit_info(unsigned long data_tosend)
{
    // initialize variables
    struct sockaddr_in addr4;
    int sock4 = -1;
    int32_t timeout = 3 * MSEC_PER_SEC;
    int ret = 0;
    int port = HTTP_PORT;

    // Create a socket connection to the server
    if (IS_ENABLED(CONFIG_NET_IPV4))
    {
        (void)connect_socket(AF_INET, TARGET_SERVER_ADDR, port,
                             &sock4, (struct sockaddr *)&addr4,
                             sizeof(addr4));
    }

    if (sock4 < 0)
    {
        LOG_ERR("Cannot create HTTP connection. (for POST)");
        return -ECONNABORTED;
    }

    // initialize send HTTP POST request over IPv4
    if (sock4 >= 0 && IS_ENABLED(CONFIG_NET_IPV4))
    {
        struct http_request req;
        char decoded_data_str[9]; // Buffer to hold the hex string representation of the data
        sprintf( decoded_data_str, "%08lX", data_tosend); // Convert unsigned long to hex string

        LOG_DBG("Sending HTTP POST request with data: %s", decoded_data_str);
        
        memset(&req, 0, sizeof(req));

        req.method = HTTP_POST;
        req.url = "/posty";
        req.host = SERVER_ADDR4; 
        req.protocol = "HTTP/1.1";
        req.payload = decoded_data_str;
        req.payload_len = strlen(req.payload);
        req.response = response_cb;
        req.recv_buf = recv_buf_ipv4;
        req.recv_buf_len = sizeof(recv_buf_ipv4);

        ret = http_client_req(sock4, &req, timeout, "IPv4 POST");

        close(sock4);
    }

    sock4 = -1;
    return ret;
}
