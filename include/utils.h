#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

/**
 * @brief Convert a int to an string
 * @param n The int to be converted
 * @return The string converted from the int
 **/
char *int_to_str(int n);

/**
 * @brief Convert a string to an int
 * @param The string to be converted
 * @return The int converted from the string
 **/
int str_to_int(char *s);

void removeFirstCharacter(char *str);
#endif // UTILS_H_INCLUDED
