#include "functionCollector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <QString>

//Var unbekanntes Frame
int unknownId = 0;
int unknownData = 0;

//Potentiometervariablen
int canPotiVar = 0;
/*int smoothCanPotiVar = 0;*/
int mappedCanPotiVar = 0;

//Schaltervariablen
bool canSWVarBool = false;

 //Werte glätten Variablen
float smoothfaktor1 = 0.7;
float smoothfaktor2 = 1.0-smoothfaktor1;

int smoothValue(int functionVar){
int smoothCanPotiVar = smoothfaktor1 * smoothCanPotiVar + smoothfaktor2 * functionVar;
return smoothCanPotiVar;
}

//Funktion CAN Frames empfangen
int canRxFrame(){ 
    int s;
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
     if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Socket");
		return 1;
	}

    strcpy(ifr.ifr_name, "can0" );
	ioctl(s, SIOCGIFINDEX, &ifr);

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		return 1;
	}
	nbytes = read(s, &frame, sizeof(struct can_frame));
 	if (nbytes < 0) {
		perror("Read");
		return 1;
	}
    if (frame.can_id == 0x001){
		if(frame.data[0] == 1){ 
			canSWVarBool = true; 
		} 
		else {
			canSWVarBool = false;
		}
    }
	else if (frame.can_id == 0x002){
		canPotiVar = frame.data[0] | (frame.data[1]<<8);
		/*smoothCanPotiVar = smoothfaktor1 * smoothCanPotiVar + smoothfaktor2 * canPotiVar;*/
		mappedCanPotiVar = frame.data[7];
	}
	else { 
		unknownId = frame.can_id;
		unknownData = frame.data[0] + frame.data[1] + frame.data[2] + frame.data[3] + frame.data[4] + frame.data[5] + frame.data[6] + frame.data[7];
	}
	if (close(s) < 0) {
		perror("Close");
		return 1;
	}
	return 0;
}

//Funktionen die von QML Gui aufgerufen werden
functionCollector::functionCollector(QObject *parent) : QObject(parent) {}

void functionCollector::changeValue()
{
	canRxFrame();
 	/*valChangedPotiTrue(smoothCanPotiVar);*/ 
	valChangedPotiTrue(smoothValue(canPotiVar));
	valChangedPotiGraph(mappedCanPotiVar);
	valChangedSW(canSWVarBool);
	valChangedUI(unknownId);
	valChangedUD(unknownData);
	
}
void functionCollector::changeLanguage(bool gui_changerequest) {
	if(gui_changerequest==false){
    	languageChanged("DE");
	}
	if(gui_changerequest==true){
		languageChanged("EN");
	}
} 

/* 
 	valueChangedPoti(QString::number(roundedCanPotiVar)); 
	valueChangedSW(canSWVarBool);
	valueChangedText(QString::number(smoothCanPotiVar));
	valueChangedUI(QString::number(unknownId));
	valueChangedUD(QString::number(unknownData));
	valueChangedRange(okRange);
*/