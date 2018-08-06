#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mpi.h"
#include "pool.h"

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

#define MAX_ACTOR_CNT 200
#define MAX_ACTOR_TYPES 10
#define MAX_NODE_CNT 100
#define ACTOR_MSGQUEUE_LEN 100
#define MSECOND_PER_LOOP  1

#define AF_CONTROL_TAG 10000
#define AF_ACTORMSG_TAG 10001

#define ERROR 0
#define INFO 1
#define APP 2
#define DEBUG 3

#ifdef DEBUGFRAME
    static char dmsg[100];
#endif
//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------


//The actor states
typedef enum _ActorState { AS_Ready,
                           AS_Running,
                           AS_Suspended,
                           AS_Stopped,
                           AS_Cleaned } ActorState;

//The assign strategy of the actors: OnePerNode-One actor per node, Even-Assign the actors to nodes evenly
//                                   MaxInNode-Assign to one node till reach its limitation,then next node
//                                   Zero-Don't assign automatically, maybe assign manually later.
typedef enum _AssignMethod { AM_OnePerNode,
                             AM_Even,
                             AM_Zero } AssignMethod;

typedef enum _FrameState { FS_New,
                           FS_Ready,
                           FS_Init,
                           FS_Running,
                           FS_Stopped } FrameState;

typedef enum _FrameMsgType { FM_ActorNew,
                             FM_ActorDie,
                             FM_ActorRun,
                             FM_ActorSuspend,
                             FM_Close,
                             FM_Custom } FrameMsgType;

// Actor Message Structure
typedef struct _FrameMessage
{
    FrameMsgType msgType;
    int targetActor;           //And if the lower 16 bots is 0 , means this should be adopted on every actor in the node
    unsigned char data[64];
} FrameMessage;

typedef struct _ActorMessage
{
    int sndActorID; //If the lower 16 bits is 0, means it's sent by a node.
    int rcvActorID; //And if it's 0 , means a broadcast, every actor should process it.
    int msgCatogory;         //Defined by the user
    int bodyLength;
    unsigned char body[256];
} ActorMessage ;
 
// Actor Message Round Buffer
typedef struct _MsgRoundBuf
{
    int head;
    int tail;
    ActorMessage msg[ACTOR_MSGQUEUE_LEN];
} MsgRoundBuf ;

// Actor Structure
typedef struct _Actor Actor;

struct _Actor
{
    unsigned actorID;  //Generated automatically Higher 16 bit - node number , lower 16 bit - actor number
    int category; //The category of an actor, assigned by framework when create this actor.
    int categorySeq;   //The global sequece of a actor in the same category.
    long step;         //The current step number.
    ActorState state;
    MsgRoundBuf msgQueue; //The message round buffer queue that belong to the Actor.
    int (*Run)(Actor *actor);
    int (*Init)(Actor *actor, void *data, int len);
    int (*Destroy)(Actor *actor);
    void *pActorData;
//    void *pNodeShareData;
    Actor *pNext;
} ;

// Actor description, used when config the framework
typedef struct _ActorDesc
{
    int category; //The category of an actor, assigned by framework when create this actor.
    int (*Run)(Actor *actor);
    int (*Init)(Actor *actor, void *data, int len);
    int (*Destroy)(Actor *actor);
    int actorinNode;
    int actorCount; //The actor count when initialize.
    int nodelimit;  //How many nodes can be used in total, if this value < 0, means infinity.
    int actorlimit; //How many actors can be assigned in a node.
    int actorsBuilt;
    AssignMethod method;
} ActorDesc;

// Framework description, used when config the framework
typedef struct _FrameDesc
{
    int (*Initialize)();
    int (*Finalize)();
    int (*MasterRun)();
    int (*WorkerRun)();
    int (*CustomCntlProc)(void *data, int len);

} FrameDesc;

// Actor and node mapping table of the whole program.
typedef struct _NodeMappingTable
{
    int actorID;
    int node;
    int category;
    int categorySeq;
} NodeMappingTable;

//The handle of the actor
typedef struct _ActorFrameworkHandle
{
    // Actor queue, acturally is the place that save the actor's personal data and function pointer
    FrameState state;
    Actor *pActorQueue;
    NodeMappingTable mapTable[MAX_ACTOR_CNT];
    int mapTableCnt;
    int actortypes;
    FrameDesc frame;
    ActorDesc actorinfo[MAX_ACTOR_TYPES];
    unsigned char shareData[256];
    int actorinNode[MAX_NODE_CNT];
    int activeNodeCnt;
    int rank;
    int ranksize;
    void *commBuffer;
} ActorFrameworkHandle;


typedef struct _ActorNew
{
    NodeMappingTable info;
    unsigned char data[16];
} ActorNew;

char info[256];

ActorFrameworkHandle handleGlobal;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

void LogMsg(char * message, int type, int actorID );
void CreateFrameMsgType();
void CreateActorMsgType();
int BroadcastCntlMessage(FrameMsgType msgType, int actor, void *pData, int dataLen);
int SendActorMessage(Actor *actor, int destID, int msgType, void *pData, int dataLen);
int SendCntlMessage(FrameMsgType msgType, int node, int actor, void *pData, int dataLen);
int ReadActorMessage(Actor *actor, ActorMessage *outMsg);
int FinalizeActorFramwork();

int ConfigActorFrame(FrameDesc desc);
int AddActorProfile(ActorDesc desc);
    void DeleteHandle();
// Build the mapTable and create actor queue, if faied, return 0 , or return the count of the active nodes.
int BuildMapTable();
int InitActorFramework(int argc, char *argv[]);
int GetAvailiableNode(ActorDesc *pDesc);
int CreateActor(ActorNew *pnewActor);
int ProcessFrameworkMessage(int rank, FrameMessage *pMsg);
Actor *SearchActor(int actorID);
int DispatchActorMessage(int rank, ActorMessage *pMsg);
int FrameworkPoll();
int ActorRun();
int RunFramework();
int SearchMapTable(int seq, int type);
