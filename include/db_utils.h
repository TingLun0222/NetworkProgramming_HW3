#ifndef DB_API_H_INCLUDED
#define DB_API_H_INCLUDED

/**
 * @brief set user password to redis server
 * @param redis redis context
 * @param username username
 * @param password password
 * @return 1 if success, -1 if failed
 **/
int set_user_password(redisContext *redis, const char *username, const char *password);

/**
 * @brief get user password from redis server
 * @param redis redis context
 * @param username username
 * @param password password
 * @return 1 if success, -1 if failed
*/
int get_user_password(redisContext *redis, const char *username, char *password);
int user_input(char *username, char *password, int socketFD);
int login_mode(redisContext *redis, char *username, char *password, int socketFD);
void register_mode(redisContext *redis, char *username, char *password, int socketFD);
int push_mailto_result(redisContext *redis, User *uhead, User *user, char *sender, char *receiver, char *message, int userfd);
char *get_listGroup_col_string(redisReply *reply, redisReply *reply_nest, int i);
void send_addTo_result(int sockfd, char (*arr)[64], int num, char *suffix);
redisReply* execute_redis_command(redisContext *redis, int expected_reply_type, const char *format, ...);
void freeMemory(int count, ...);
#endif // DB_API_H_INCLUDED
