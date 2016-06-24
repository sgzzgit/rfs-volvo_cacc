#define DB_DVI_RCV_TYPE		6543
#define DB_DVI_RCV_VAR		DB_DVI_RCV_TYPE

#define quint8 unsigned char
#define quint32 unsigned int
#define qint8 char


//Vehicle -> DVI - port 10007 (string data and popups)
struct VehicleStruct {
  quint8 type; // 0=nothing 1=truck 2=truck with communication error
  quint8 hasIntruder; //0:false, 1:truck, 2:car, 3:MC (PATH: The graphical indication is the same for all intruders)
  quint8 isBraking; //0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
};



struct SeretUdpStruct{
    quint8 platooningState; //0=standby, 1=joining, 2=platooning, 3=leaving, 4=dissolve (PATH: I guess only 0 and 2 is used?)
    qint8 position; //-1:nothing (follower with no platoon), 0:leader, >0 Follower (Ego position of vehicle)
    quint8 TBD; //Not used for the moment
    quint8 popup;//0:no popup, 1:Platoon found - join? (PATH: Not currently used)
    quint32 exitDistance; //value/10.0 km (PATH: Not currently used)
    struct VehicleStruct vehicles[3];
};

//Vehicle -> DVI - port 10005 (ACC/CACC information)
struct ExtraDataCACCStruct{
    quint8 CACCState; //0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active
    quint8 CACCTargetActive; //0:false, 1:true (also used for target in ACC)
    quint8 CACCDegraded;//0: false, 1:Overheated brakes (I guess you don't need this one)
    quint8 CACCActiveConnectionToTarget;//0:no connection 1:connection (if this or ...fromFollower equals 1 the WIFI icon will appear)
    quint8 CACCActiveConnectionFromFollower;//0:no connection, 1:connection
    quint8 CACCTimeGap;//0-4
    quint8 ACCTimeGap;//0-4
    quint8 CACCEvents;//0:"No popup", 1:"FCW",2:"Brake Capacity",3:"LC Left",4:"LC Right",5:"Obstacle ahead",6:"Connection lost"
    quint8 platooningState;//0:"Platooning",1:"Joining",2:"Leaving",3:"Left",4:"Dissolving",5:"Dissolved" (NOT CURRENTLY USED!)
    quint8 counter;//Counter for dissolving, not implemented for the moment
};


//DVI -> Vehicle - port 8003 (button pressed). Is being sent with an interval of 50ms. Triggers when a button is released and is kept high for 200ms.
struct PathButtonStruct{
    quint8 buttonPressed;
    //0: no button pressed
    //1: joinpopup_JOIN
    //2: joinpopup_IGONORE
    //3: LEAVE
    //4: DISSOLVE
    //5: ACC_BUTTON
    //6: CACC_BUTTON
    //7: TIMEGAP_MINUS
    //8: TIMEGAP_PLUS
};

#define no_button_pressed       0
#define joinpopup_JOIN          1
#define joinpopup_IGONORE       2
#define LEAVE                   3
#define DISSOLVE                4
#define ACC_BUTTON              5
#define CACC_BUTTON             6
#define TIMEGAP_MINUS           7
#define TIMEGAP_PLUS            8

