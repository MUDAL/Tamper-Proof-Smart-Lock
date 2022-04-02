#ifndef GSM_H
#define GSM_H

/**
* GSM: SIM800L
*/
extern void GSM_Init(void);
extern void GSM_SendText(char* phoneNo,char* msg);

#endif //GSM_H
