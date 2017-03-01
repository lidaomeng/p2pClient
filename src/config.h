#ifndef CONFIG_H_
#define CONFIG_H_

#define KEYVALLEN 256

/*   删除左边的空格   */
char * l_trim(char * szOutput, const char *szInput);

/*   删除右边的空格   */
char *r_trim(char *szOutput, const char *szInput);

/*   删除两边的空格   */
char * a_trim(char * szOutput, const char * szInput);


int GetProfileString(const char *profile, const char *AppName, const char *KeyName, char *KeyVal );


#endif
