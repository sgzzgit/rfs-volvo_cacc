//DVI outputs
typedef struct {
	float sec_past_midnight;
	int msg_count;
	unsigned char DriverSelectedControlMode;	//1-2	"1=ACC 2=CACC"
	unsigned char DriverSelectedTimeGap;	//1-5	1 shortest timegap and then increasing
} __attribute__((__packed__)) DVI_to_controller_t;

//DVI inputs
typedef struct {
	int msg_count;
	float sec_past_midnight;
	unsigned char Vehicle1State;		// 0-4:	0=Not available 1=Off 2=CC 3=ACC 4=CACC 
	unsigned char Vehicle1Braking;		// 0-3: 0=Not available	1=Not braking 2=Braking 3=Hard braking
	unsigned char Vehicle1CutIn;		// 0-3: 0=Not available	1=No cut-in 3=Cut-in from left 3=Cut-in from right
	unsigned char Vehicle1LaneChange;	// 0-3:	0=Not available 1=No lane change 2=Lane change to left 3=Lane change to right 
	unsigned char Vehicle1Communication;	// 0-2:	0=Not available 1=Not communicating 2=Communicating 
	unsigned char Vehicle1Malfunction;	// 0-2:	0=Not available 1=Functioning 2=Malfunctioning 
	unsigned char Vehicle2State;		// 0-4:	0=Not available 1=Off 2=CC 3=ACC 4=CACC 
	unsigned char Vehicle2Braking;		// 0-3:	0=Not available 1=Not braking 2=Braking 3=Hard braking 
	unsigned char Vehicle2CutIn;		// 0-3:	0=Not available 1=No cut-in 2=Cut-in from left 3=Cut-in from right 
	unsigned char Vehicle2LaneChange;	// 0-3:	0=Not available 1=No lane change 2=Lane change to left 3=Lane change to right 
	unsigned char Vehicle2Communication;	// 0-2:	0=Not available 1=Not communicating 2=Communicating 
	unsigned char Vehicle2Malfunction;	// 0-2:	0=Not available 1=Functioning 2=Malfunctioning 
	unsigned char Vehicle3State;		// 0-4:	0=Not available 1=Off 2=CC 3=ACC 4=CACC 
	unsigned char Vehicle3Braking;		// 0-3:	0=Not available 1=Not braking 2=Braking 3=Hard braking 
	unsigned char Vehicle3CutIn;		// 0-3:	0=Not available 1=No cut-in 2=Cut-in from left 3=Cut-in from right 
	unsigned char Vehicle3LaneChange;	// 0-3:	0=Not available 1=No lane change 2=Lane change to left 3=Lane change to right 
	unsigned char Vehicle3Communication;	// 0-2:	0=Not available 1=Not communicating 2=Communicating 
	unsigned char Vehicle3Malfunction;	// 0-2:	0=Not available 1=Functioning 2=Malfunctioning 
	unsigned char EgoVehiclePosition;	// 1-3:	0=Not available 1=First position 2=Second position 3=Third position 
} __attribute__((__packed__)) controller_to_DVI_t;
