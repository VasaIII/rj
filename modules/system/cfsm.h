

pthread_t cfsm_thread_id;
void cfsm_thread(void *arg);

/*
enum eJohnnieWalkerStatusMask {
	unknownwalkingstatus 	= 0x0,
	walkingforward		 	= 0x1,
	walkingbackward		 	= 0x2,
	boooring				= 0x4
};
struct sJohnnieWalker {
	int steps;
	int stepDuration;
	int status;
	int room;
};


enum sKeyStatusValue {
	unknownkeystatus		= 0x0,
	creating				= 0x1,
	created					= 0x2,
	good					= 0x4,
	bad						= 0x8
};
enum sDoorStatusValue {
	unknowndoorstatus		= 0x0,
	opening					= 0x1,
	open					= 0x2,
	closing					= 0x4,
	closed					= 0x8
};
enum sRoomStatusValue {
	unknownroomstatus		= 0x0
};


struct sRoom {
	struct sDoor {
		struct sKey {
			struct sTooth {
				int status;
				int time;
			} tooth[10];
			int status;
		} key[10];
		int status;
	} door[10];
	int status;
} room;

enum eRoom {
	blue 	= 0x0,
	green	= 0x1
};

MakeROOMwith2DOORS(ASP_Down, door[0], door[1])
MakeKEYforDOORoutside(key[0],door[0])
MakeTOOTHonKEY(key[0], ASP_Down_ASPDN)
MakeTOOTHonKEY(key[0], ASP_Down_Acknowledgement_ASPDNACK)
MakeKEYforDOORoutside(key[0],door[1])
MakeTOOTHonKEY(key[0], boooring)

MakeROOMwith4DOORS(ASP_Inactive, door[0], door[1], door[2], door[3])
MakeKEYforDOORoutside(key[0],door[0])
MakeTOOTHonKEY(key[0], ASP_Down_ASPDN)
MakeTOOTHonKEY(key[0], ASP_Down_Acknowledgement_ASPDNACK)



MakeTOOTHonKEY(key[0], ASP_Up_ASPUP)
MakeTOOTHonKEY(key[0], ASP_Up_Acknowledgement_ASPUPACK)
MakeKEYforDOORinside(key[1],door[0])

DEFINECLIENTROOM(ROOM_1_, Heartbeat_BEAT)
DEFINESERVERROOM(ROOM_1_, Heartbeat_Acknowledgement_BEATACK)

MAKECLIENTKEY(DOOR_2_, ASP_Active_ASPAC, ASP_Active_Acknowledgement_ASPACACK)


*/













