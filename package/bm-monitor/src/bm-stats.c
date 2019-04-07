#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <jansson.h>
#include <time.h>

#include "bm-stats.h"


#define FETCH_PERIOD         (120) // unit: seconds
#define MAPI_HOST           "127.0.0.1"
#define MAPI_PORT           4028
#define MAPI_CMD_GETDEVS    "{\"command\" : \"devs\"}"
#define STATS_JSON_FILE      "/tmp/stats.json"
#define CHAIN_NUM_MAX       (8)
#define MAX_STATS           (720) // max amount of stats recorded


void *thread_get_stats(void *argv) {
    json_t *json_stats = NULL;
    json_t *p_json_rst = NULL;
    json_t *p_json_arr = NULL;
    json_t *p_json_obj = NULL;
    json_t *json_stats_arr = NULL;
    char buffer[256] = {0};
    int size;

    //Check if JSON file exits
    FILE *p_file = fopen(STATS_JSON_FILE, "r");
    if(NULL == p_file)
    {
        //Create JSON File if not Exists
        create_stats_file(json_stats);
    }

    load_stats_file(&json_stats);
    while(true) {
        json_t *hash_rates = json_object();

        if (json_array_size(json_stats)==MAX_STATS) {
            json_array_remove(json_stats,0);
        }


        if(call_miner_api(MAPI_CMD_GETDEVS, MAPI_HOST, MAPI_PORT, &p_json_rst) == 0) {
            p_json_arr = json_object_get(p_json_rst, "DEVS");
            size = json_array_size(p_json_arr);
            int i;
            for(i = 0; i < size; ++i)
            {
                p_json_obj = json_array_get(p_json_arr, i); //Get the object from specified chain id
                double hash_rate = json_real_value(json_object_get(p_json_obj, "MHS 5m")); //Get hash rate from the chain
                int asc_num = json_integer_value(json_object_get(p_json_obj, "ASC")); //Get id of the chain

                sprintf(buffer, "%d", asc_num); //Convert ID chain to string
                p_json_obj = json_object(); //Create new json object
                json_object_set_new(hash_rates, buffer, json_integer((int)hash_rate)); //assign the object the id and hash rate
            }

        }
        sprintf(buffer, "%u", (unsigned) time(NULL));
        p_json_obj = json_object();
        json_object_set_new(p_json_obj, buffer, hash_rates);
        json_array_append_new(json_stats,p_json_obj);
        save_stats_file(json_stats);

        sleep(FETCH_PERIOD);
    }
}

bool save_stats_file(json_t *json_stats)
{
   if (json_stats) {
       if( json_dump_file(json_stats, STATS_JSON_FILE, JSON_INDENT(3)) < 0){
        return false;
       }
   }
   return false;
}

bool create_stats_file(json_t *json_stats) {
    char* s = NULL;
    json_stats = json_array();
    if (save_stats_file(json_stats)==false) {
        return false;
    }
    return true;
}

bool load_stats_file(json_t **json_stats)
{
    json_error_t error;
    *json_stats = json_load_file(STATS_JSON_FILE, 0, &error);
    if (*json_stats) {
        return true;
    }
    return false;
}
int call_miner_api(char *command, char *host, short int port, json_t **result)
{

    struct hostent *ip;
    struct sockaddr_in serv;
    int sock;
    int ret = 0;
    int n = 0;
    char *buf = NULL;
    size_t len = 16383;
    size_t p = 0;
    json_error_t error;

    ip = gethostbyname(host);
    if (!ip) {
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        return -1;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr = *((struct in_addr *)ip->h_addr);
    serv.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0)
    {
        return -1;
    }

    n = send(sock, command, strlen(command), 0);
    if (n < 0)
    {
        ret = -1;
    }
    else
    {
        buf = malloc(len+1);
        if (!buf)
        {
            return -1;
        }

        while(1)
        {
            if ((len - p) < 1)
            {
                len += 16383;
                buf = realloc(buf, len+1);
                if (!buf)
                {
                    return -1;
                }
            }

            n = recv(sock, &buf[p], len - p , 0);
            if(n < 0)
            {
                ret = -1;
                break;
            }

            if(0 == n)
            {
                break;
            }

            p += n;
        }
        buf[p] = '\0';

        *result = json_loads(buf, 0, &error);
        free(buf);
        if (NULL == *result)
        {
            ret = -1;
        }
    }

    close(sock);

    return ret;
}

