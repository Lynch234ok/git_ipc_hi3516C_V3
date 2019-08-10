#ifndef _SECURITY_H_
#define _SECURITY_H_

typedef struct {
	unsigned short auth; //ja
	unsigned short chip;   //31	
	unsigned short ver;  //1.0
	unsigned short checksum;  
}DEVICE_INFO_t;


extern int ENCRY_triger_authentication(void);

#endif  //_SECURITY_H_

