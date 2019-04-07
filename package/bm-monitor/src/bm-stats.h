int  call_miner_api(char *command, char *host, short int port, json_t **result);
bool load_stats_file(json_t **json_stats);
bool create_stats_file(json_t *json_stats);
bool save_stats_file(json_t *json_stats);
void *thread_get_stats(void *argv);