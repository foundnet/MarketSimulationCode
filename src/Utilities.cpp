#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mpi.h"
#include "pool.h"
#include "actorframe.h"

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

MPI_Request cntlRequest = NULL;
FrameMessage cntlMessage;

MPI_Request actorRequest = NULL;
ActorMessage actorMessage;

MPI_Request cntlBcastRequest = NULL;
FrameMessage cntlBcastMessage;

MPI_Request actorBcastRequest = NULL;
ActorMessage actorBcastMessage;

MPI_Status status;
MPI_Request *pInitWorkerRequests;

static MPI_Datatype AF_CNTL_TYPE;
static MPI_Datatype AF_ACTOR_TYPE;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

void CreateFrameMsgType()
{
    FrameMessage msg;
    MPI_Aint addr[3];
    MPI_Address(&msg.msgType, &addr[0]);
    MPI_Address(&msg.targetActor, &addr[1]);
    MPI_Address(&msg.data, &addr[2]);
    int blocklengths[3] = {1, 1, 64}, nitems = 3;
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_CHAR};
    MPI_Aint offsets[3] = {0, addr[1] - addr[0], addr[2] - addr[1]};
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &AF_CNTL_TYPE);
    MPI_Type_commit(&AF_CNTL_TYPE);
}

void CreateActorMsgType()
{
    ActorMessage msg;
    MPI_Aint addr[5];
    MPI_Address(&msg.sndActorID, &addr[0]);
    MPI_Address(&msg.rcvActorID, &addr[1]);
    MPI_Address(&msg.msgCatogory, &addr[2]);
    MPI_Address(&msg.bodyLength, &addr[3]);
    MPI_Address(&msg.body, &addr[4]);
    int blocklengths[5] = {1, 1, 1, 1, 256}, nitems = 5;
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR};
    MPI_Aint offsets[5] = {0, addr[1] - addr[0], addr[2] - addr[1], addr[3] - addr[2], addr[4] - addr[3]};
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &AF_ACTOR_TYPE);
    MPI_Type_commit(&AF_ACTOR_TYPE);
}

void LogMsg(char * message, int type, int actorID ) 
{
	if (type == 0)
    {
        if (actorID > 0)
            fprintf(stdout,"Node%04d:Actor0x%08x: [FRAMEERROR] %s\n", handleGlobal.rank, actorID, message);
        else fprintf(stdout,"Node%04d: [FRAMEERROR] %s\n", handleGlobal.rank, message);

    } else if (type == 1)
    {
        if (actorID > 0)
            fprintf(stdout,"Node%4d:Actor0x%08x: [ACTORFRAME] %s\n", handleGlobal.rank, actorID, message);
        else fprintf(stdout,"Node%04d: [ACTORFRAME] %s\n", handleGlobal.rank, message);
    } else if (type == 2)
    {
        if (actorID > 0)
            fprintf(stdout,"Node%4d:Actor0x%08d: [APPLICATION] %s\n", handleGlobal.rank, actorID, message);
        else fprintf(stdout,"Node%04d: [APPLICATION] %s\n", handleGlobal.rank, message); 
    }
    else 
    {
#ifdef DEBUGFRAME
        fprintf(stdout,"%4d:%8d: [DEBUGINFO] %s\n", handleGlobal.rank, actorID, message);
#endif
    }
}

int BroadcastCntlMessage(FrameMsgType msgType, int actor, void *pData, int dataLen)
{
    if (dataLen > 64 || dataLen < 0)
        return 0;

    if (handleGlobal.rank == 0)
    {
        FrameMessage msg;
        memset(msg.data, 0, 64);
        msg.msgType = msgType;
        msg.targetActor = actor;
        if (pData != NULL)  memcpy(msg.data, pData, dataLen);

        MPI_Request request;
        char info[100];
        sprintf(info, "Send bcast type %d", (int)msgType);
        LogMsg(info,DEBUG,0);

        MPI_Ibcast(&msg, 1, AF_CNTL_TYPE, 0, MPI_COMM_WORLD, &request);
        MPI_Status status;
        MPI_Wait(&request, &status);

        return 1;
    }
    else
    {
        return SendCntlMessage(FM_Custom, 0, actor, pData, dataLen);
    }

    return 0;
}

int SendActorMessage(Actor *actor, int destID, int msgType, void *pData, int dataLen)
{
    int node = destID / 0x10000;
    if (dataLen >= 128 || dataLen < 0)
        return 0;

    ActorMessage msg;
    memset(msg.body, 0, 128);
    msg.msgCatogory = msgType;
    msg.bodyLength = dataLen;
    msg.rcvActorID = destID;
    msg.sndActorID = actor == NULL ? 0 : actor->actorID;
    if (pData != NULL)
        memcpy(msg.body, pData, dataLen);

    if (handleGlobal.rank != node )
    {
        sprintf(info, "Send Actor Message type %d to node %d", (int)msgType,node);
        LogMsg(info,DEBUG,0);

  
        MPI_Request request;
        int node = destID / 0x10000;

        sprintf(info, "Send Actor Message node %d", node);
        LogMsg(info, DEBUG, 0);
        MPI_Bsend(&msg, 1, AF_ACTOR_TYPE, node, AF_ACTORMSG_TAG, MPI_COMM_WORLD);
        LogMsg("Send Actor Message OK", DEBUG, 0);
    }
    else
    {
        sprintf(info, "Send Actor Dispatch  message type %d to node %d", (int)msgType,node);
        LogMsg(info,DEBUG,0);
        DispatchActorMessage(node, &msg);
    }
    return 1;
}

int SendCntlMessage(FrameMsgType msgType, int node, int actor, void *pData, int dataLen)
{
    if (dataLen > 64 || dataLen < 0)
        return 0;

    FrameMessage msg;
    memset(msg.data, 0, 32);
    msg.msgType = msgType;
    msg.targetActor = actor;
    if (pData != NULL)
        memcpy(msg.data, pData, dataLen);

    if (handleGlobal.rank != node)
    {
        MPI_Request request;
        MPI_Bsend(&msg, 1, AF_CNTL_TYPE, node, AF_CONTROL_TAG, MPI_COMM_WORLD);
    }
    else
    {
        ProcessFrameworkMessage(node, &msg);
    }
    return 1;
}

int ReadActorMessage(Actor *actor, ActorMessage *outMsg)
{
    if (actor != NULL)
    {
        if (actor->msgQueue.tail != actor->msgQueue.head)
        {
            memcpy(outMsg, &actor->msgQueue.msg[actor->msgQueue.tail], sizeof(ActorMessage));
            int point = (actor->msgQueue.tail + 1) % ACTOR_MSGQUEUE_LEN;
            actor->msgQueue.tail = point;
            return 1;
        }
    }
    return 0;
}

int FinalizeActorFramwork()
{
    return 1;
}

/** 
 * @brief Config the Actor Framework, includes：
 *          The strategy of actor assignment   One actor per node, or even mode/Multi actors one node, given the maximum actors per node and init nodes 
 *          The initialize count of actors in total
 *          The function pointer of framework should call when initialize and finalize.
*/
int ConfigActorFrame(FrameDesc desc)
{
    if (handleGlobal.state < FS_Running)
    {
        handleGlobal.frame = desc;

        handleGlobal.state = FS_Ready;

        return 1;
    }

    return 0;
}

/** 
 * @brief Add the actor's profile, including the actor's type, personal data, strategy, function pointer, etc.
 *        The function pointer includes the init 
*/
int AddActorProfile(ActorDesc desc)
{
    if (handleGlobal.state < FS_Running &&
        handleGlobal.actortypes < 9)
    {
        handleGlobal.actorinfo[handleGlobal.actortypes] = desc;
            char info[200];
        sprintf(info,"Add Cfginfo %d, %d ,%d, %d",handleGlobal.actortypes,handleGlobal.actorinfo[handleGlobal.actortypes].nodelimit,handleGlobal.actorinfo[handleGlobal.actortypes].actorlimit,handleGlobal.actorinfo[handleGlobal.actortypes].actorCount);
        LogMsg(info,DEBUG,0);
        handleGlobal.actortypes++;

        return 1;
    }
    return 0;
}

void DeleteHandle()
{
    Actor *pActor = handleGlobal.pActorQueue;
    while (pActor != NULL)
    {
        Actor *pNext = pActor->pNext;
        if (pActor->pActorData != NULL)
            free(pActor->pActorData);
        free(pActor);
        pActor = pNext;
    }
}

int SearchMapTable(int seq, int category)
{
    for (int i = 0; i < handleGlobal.mapTableCnt; i++)
    {
        if (handleGlobal.mapTable[i].categorySeq == seq &&
            handleGlobal.mapTable[i].category == category)
            return handleGlobal.mapTable[i].actorID;
    }
    return 0;
}

// Build the mapTable and create actor queue, if faied, return 0 , or return the count of the active nodes.
int BuildMapTable()
{
    sprintf(info,"actortypes %d, rank %d, size %d",handleGlobal.actortypes, handleGlobal.rank,handleGlobal.ranksize);
    LogMsg(info,DEBUG,0);
    memset(handleGlobal.mapTable, 0, sizeof(NodeMappingTable) * MAX_ACTOR_CNT);
    handleGlobal.mapTableCnt = 0;

    memset(handleGlobal.actorinNode, 0, sizeof(int) * MAX_ACTOR_TYPES);
    handleGlobal.activeNodeCnt = 0;

    handleGlobal.pActorQueue = NULL;
    Actor **pActor = &handleGlobal.pActorQueue;

    int activeNodeCnt = 0;
    int totalActor = 0;
    int nodeCnt, actorCnt = 0;      // The actor count per node, and the node count, used temporarily
    sprintf(info,"actortypes %d, rank %d, size %d",handleGlobal.actortypes, handleGlobal.rank,handleGlobal.ranksize);
    LogMsg(info,DEBUG,0);

    for (int i = 0; i < handleGlobal.actortypes; i++)
    {
        ActorDesc *pDesc = &handleGlobal.actorinfo[i];
        pDesc->actorsBuilt = 0;

        if (pDesc->method == AM_Even)   
        {
            if (pDesc->nodelimit >= 0)
                nodeCnt = (pDesc->actorCount >= pDesc->nodelimit) ? pDesc->nodelimit : pDesc->actorCount;
            else
                nodeCnt = 0;
            nodeCnt = nodeCnt > (handleGlobal.ranksize - 1) ? (handleGlobal.ranksize - 1) : nodeCnt;
            if (pDesc->actorlimit < pDesc->actorCount / nodeCnt)
            {
                LogMsg("No enough node for actors.", ERROR, 0);
                DeleteHandle();
                return 0;
            }

            activeNodeCnt = activeNodeCnt > nodeCnt ? activeNodeCnt : nodeCnt;


            for (int n = 1; n <= nodeCnt; n++)
            {
                actorCnt = pDesc->actorCount / nodeCnt;
                actorCnt += (pDesc->actorCount % nodeCnt >= n) ? 1 : 0;
                //Build the maptable
                for (int m = 0; m < actorCnt; m++)
                {
                    pDesc->actorsBuilt++;
                    handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID = n * 0x10000 + handleGlobal.mapTableCnt + 1;
                    sprintf(info,"Create Actor node %d, actortID %x",n,handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID);
                    LogMsg(info,DEBUG,0);
                    handleGlobal.mapTable[handleGlobal.mapTableCnt].category = pDesc->category;
                    handleGlobal.mapTable[handleGlobal.mapTableCnt].categorySeq = pDesc->actorsBuilt;
                    handleGlobal.mapTable[handleGlobal.mapTableCnt].node = n;

                    // Build the actor queue.
                    if (n == handleGlobal.rank)
                    {
                        (*pActor) = (Actor *)malloc(sizeof(Actor));
                        (*pActor)->actorID = handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID;
                        (*pActor)->categorySeq = handleGlobal.mapTable[handleGlobal.mapTableCnt].categorySeq;
                        (*pActor)->category = handleGlobal.mapTable[handleGlobal.mapTableCnt].category;
                        (*pActor)->pActorData = NULL;
                        (*pActor)->msgQueue.head = 0;
                        (*pActor)->msgQueue.tail = 0;
                        (*pActor)->pNext = NULL;
                        (*pActor)->step = 0;
                        (*pActor)->state = AS_Ready;
                        (*pActor)->Run = pDesc->Run;
                        (*pActor)->Init = pDesc->Init;
                        (*pActor)->Destroy = pDesc->Destroy;
                        pActor = &((*pActor)->pNext);
                    }
                    handleGlobal.mapTableCnt++;
                    handleGlobal.actorinNode[n]++;
                }
            }
        }
        else if (pDesc->method == AM_OnePerNode)
        {
            if (pDesc->actorCount > pDesc->nodelimit)
            {
                LogMsg("The count of actors is excced the node limits.",ERROR,0);
                DeleteHandle();
                return 0;
            }
            else if (pDesc->actorCount > (handleGlobal.ranksize - 1))
            {
                LogMsg("No enough node for actors.",ERROR,0);
                DeleteHandle();
                return 0;
            }
            else
                nodeCnt = pDesc->actorCount;

            activeNodeCnt = activeNodeCnt > nodeCnt ? activeNodeCnt : nodeCnt;

            for (int n = 1; n <= nodeCnt; n++)
            {
                //Build the maptable
                pDesc->actorsBuilt++;
                handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID = n * 0x10000 + handleGlobal.mapTableCnt + 1;
                handleGlobal.mapTable[handleGlobal.mapTableCnt].category = pDesc->category;
                handleGlobal.mapTable[handleGlobal.mapTableCnt].categorySeq = pDesc->actorsBuilt;
                handleGlobal.mapTable[handleGlobal.mapTableCnt].node = n;

                // Build the actor queue.
                if (n == handleGlobal.rank)
                {
                    (*pActor) = (Actor *)malloc(sizeof(Actor));
                    (*pActor)->actorID = handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID;
                    (*pActor)->categorySeq = handleGlobal.mapTable[handleGlobal.mapTableCnt].categorySeq;
                    (*pActor)->category = handleGlobal.mapTable[handleGlobal.mapTableCnt].category;
                    (*pActor)->pActorData = NULL;
                    (*pActor)->msgQueue.head = 0;
                    (*pActor)->msgQueue.tail = 0;
                    (*pActor)->pNext = NULL;
                    (*pActor)->step = 0;
                    (*pActor)->state = AS_Ready;
                    (*pActor)->Run = pDesc->Run;
                    (*pActor)->Init = pDesc->Init;
                    (*pActor)->Destroy = pDesc->Destroy;
                    pActor = &((*pActor)->pNext);
//                    (*pActor)->Init((*pActor), NULL, 0);
                }
                handleGlobal.mapTableCnt++;
                handleGlobal.actorinNode[n]++;
            }
        }
    }

    handleGlobal.activeNodeCnt = activeNodeCnt;
    sprintf(info,"Again actortypes %d, rank %d, size %d",handleGlobal.actortypes, handleGlobal.rank,handleGlobal.ranksize);
    LogMsg(info,DEBUG,0);

    return 1;
}

int InitActor()
{
    int cnt = 0;
    Actor *pActor = handleGlobal.pActorQueue;
    while (pActor != NULL)
    {
        cnt ++;
        pActor->Init(pActor, NULL, 0);
        pActor = pActor->pNext;
    }
    sprintf(info,"Init Actors, node %d, actor cnt %d",handleGlobal.rank, cnt);
    LogMsg(info,DEBUG,0);

    return 1;
}

int InitActorFramework(int argc, char *argv[])
{
    if (handleGlobal.state != FS_Ready)
        return 0;
    //Init Framework

    // Call MPI initialize first
    MPI_Init(&argc, &argv);
    handleGlobal.commBuffer = malloc(512 * 1024);

    MPI_Buffer_attach(handleGlobal.commBuffer, 512 * 1024);

    MPI_Comm_rank(MPI_COMM_WORLD, &handleGlobal.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &handleGlobal.ranksize);

    CreateFrameMsgType();
    CreateActorMsgType();

    sprintf(info, "Myrank %d, rank size %d",handleGlobal.rank,handleGlobal.ranksize);
    LogMsg(info,DEBUG,0);

    sprintf(info, "2.Myrank %d, rank size %d",handleGlobal.rank,handleGlobal.ranksize);
    LogMsg(info,DEBUG,0);

    //Build the maptable and the actor queue
    if (BuildMapTable() == 0)
    {
        LogMsg("Can't build maptable",ERROR,0);
        MPI_Finalize();
        return 0;
    }

    sprintf(info, "3.Myrank %d, rank size %d",handleGlobal.rank,handleGlobal.ranksize);
    LogMsg(info,DEBUG,0);

    sprintf(info, "Active node %d, total actor in node %d, 1-%d,2-%d,3-%d",handleGlobal.activeNodeCnt,handleGlobal.actorinNode[handleGlobal.rank],handleGlobal.actorinNode[1],handleGlobal.actorinNode[2],handleGlobal.actorinNode[3]);
    LogMsg(info,DEBUG,0);

    LogMsg("Build maptable success!",DEBUG, 0);
    int statusCode = processPoolInit(handleGlobal.activeNodeCnt);
    
    if (statusCode == 0)
    {
        LogMsg("Can't build pool!",ERROR,0);
        MPI_Finalize();
        return 0;
    }
    LogMsg("---------Process Initialize OK!--------- ",INFO,0);

    InitActor();

    LogMsg("---------Actor Initialize OK!--------- ",INFO,0);

    if (handleGlobal.frame.Initialize() == 0)
    {
        MPI_Type_free(&AF_ACTOR_TYPE);
        MPI_Type_free(&AF_CNTL_TYPE);
        LogMsg("Frame Init Error! ",ERROR,0);

        MPI_Finalize();
        return 0;
    }
    handleGlobal.state = FS_Init;

    MPI_Barrier(MPI_COMM_WORLD);

    LogMsg("---------All the init work finished---------",INFO,0);
 
    if (handleGlobal.rank == 0)
        BroadcastCntlMessage(FM_ActorRun, 0, NULL, 0);

    LogMsg("---------Activate all the actors---------",INFO,0);

    return statusCode;
}

int GetAvailiableNode(ActorDesc *pDesc)
{
    int count, selnode = 0;
    if (pDesc->method == AM_Even)
    {
        count = pDesc->actorlimit;
        for (int i = 1; i <= handleGlobal.activeNodeCnt; i++)
        {
            if (handleGlobal.actorinNode[i] < count)
            {
                count = handleGlobal.actorinNode[i];
                selnode = i;
            }
        }
        if (selnode == 0)
        {
            if (handleGlobal.activeNodeCnt < (handleGlobal.ranksize - 1))
                selnode = -1;
        }
    }
    else if (pDesc->method == AM_OnePerNode)
    {
        for (int i = 1; i <= handleGlobal.activeNodeCnt; i++)
        {
            if (handleGlobal.actorinNode[i] == 0)
                selnode = i;
        }
        if (selnode == 0)
        {
            if (handleGlobal.activeNodeCnt < (handleGlobal.ranksize - 1))
                selnode = -1;
        }
    }

    return selnode;
}

NodeMappingTable *AddMapTable(int node, ActorDesc *pDesc)
{
    NodeMappingTable *pMap;
    pDesc->actorsBuilt++;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID = node * 0x10000 + handleGlobal.mapTableCnt + 1;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].category = pDesc->category;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].categorySeq = pDesc->actorsBuilt;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].node = node;

    handleGlobal.mapTableCnt++;
    handleGlobal.actorinNode[node]++;

    pMap = &handleGlobal.mapTable[handleGlobal.mapTableCnt];

    return pMap;
}

void AppendMappingTable(NodeMappingTable *pMap, ActorDesc *pDesc)
{
    pDesc->actorsBuilt++;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].actorID = pMap->actorID;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].category = pMap->category;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].categorySeq = pMap->categorySeq;
    handleGlobal.mapTable[handleGlobal.mapTableCnt].node = pMap->node;

    handleGlobal.mapTableCnt++;
    handleGlobal.actorinNode[pMap->node]++;
}

void RemoveMappingTable(int actorID)
{
    for (int i = 0; i < handleGlobal.mapTableCnt; i++)
    {
        if (handleGlobal.mapTable[i].actorID == actorID)
        {
            handleGlobal.mapTable[i].actorID = 0;
            handleGlobal.actorinNode[handleGlobal.mapTable[i].node]--;
        }
    }
}

int CreateActor(ActorNew *pnewActor)
{
    //Get the target node
    ActorDesc *pDesc = NULL;
    for (int i = 0; i < handleGlobal.actortypes; i++)
        if (handleGlobal.actorinfo[i].category == pnewActor->info.category)
            pDesc = &handleGlobal.actorinfo[i];
    if (pDesc != NULL)
    {
        if (handleGlobal.rank == 0)
        {
            int node = pnewActor->info.node;
            if (node == 0)
            {
                node = GetAvailiableNode(pDesc);
                if (node == -1)
                {
                    node = startWorkerProcess();
                }
            }
            if (node > 0)
            {
                NodeMappingTable *pMap = AddMapTable(node, pDesc);
                ActorNew data;
                data.info = *pMap;
                memcpy(data.data, pnewActor->data, 16);
                int result = BroadcastCntlMessage(FM_ActorNew, 0, &data, sizeof(ActorNew));
                LogMsg("------Transfer the create actor command to users!",INFO,0);
            }
        }
        else
        {
            AppendMappingTable(&pnewActor->info, pDesc);
            if (pnewActor->info.node == handleGlobal.rank)
            {
                Actor **pActor = &handleGlobal.pActorQueue;
                while ((*pActor) != NULL)
                    pActor = &(*pActor)->pNext;

                (*pActor) = (Actor *)malloc(sizeof(Actor));
                (*pActor)->actorID = pnewActor->info.actorID;
                (*pActor)->categorySeq = pnewActor->info.categorySeq;
                (*pActor)->category = pnewActor->info.category;
                (*pActor)->pActorData = NULL;
                (*pActor)->msgQueue.head = 0;
                (*pActor)->msgQueue.tail = 0;
                (*pActor)->pNext = NULL;
                (*pActor)->step = 0;
                pActor = &(*pActor)->pNext;
                (*pActor)->Init((*pActor), pnewActor->data, 16);
                (*pActor)->state = AS_Running;
            }
            return 1;
        }
    }
    return 0;
}

int ProcessFrameworkMessage(int rank, FrameMessage *pMsg)
{
    if (rank == 0)
    {
        switch (pMsg->msgType)
        {
        case FM_ActorNew:
            CreateActor((ActorNew *)&pMsg->data);
            LogMsg("Process New Actor",DEBUG,0);
            break;
        case FM_ActorDie:
            RemoveMappingTable(pMsg->targetActor);
            BroadcastCntlMessage(FM_ActorDie, pMsg->targetActor, NULL, 0);
            LogMsg("Process Actor die",DEBUG,0);
            break;
        case FM_Custom:
             LogMsg("Process Custom Message",DEBUG,0);
             BroadcastCntlMessage(FM_Custom, pMsg->targetActor, pMsg->data, 64);
        }
    }
    else
    {
        switch (pMsg->msgType)
        {
        case FM_ActorNew:
            LogMsg("New Actor",DEBUG,0);
            CreateActor((ActorNew *)&pMsg->data);
            break;
        case FM_ActorRun:
            LogMsg("Actor Run",DEBUG,0);
            if (pMsg->targetActor == 0 || pMsg->targetActor / 0x10000 == handleGlobal.rank)
            {
                Actor *pActor = handleGlobal.pActorQueue;
                while (pActor != NULL)
                {
                    pActor->state = AS_Running;
                    pActor = pActor->pNext;
                }
            }
            else
            {
                Actor *pActor = handleGlobal.pActorQueue;
                while (pActor != NULL && pMsg->targetActor != pActor->actorID)
                    pActor = pActor->pNext;
                if (pActor != NULL)
                    pActor->state = AS_Running;
            }
            break;
        case FM_ActorDie:
            if (pMsg->targetActor / 0x10000 == handleGlobal.rank)
            {
                Actor **pActor = &handleGlobal.pActorQueue;
                while ((*pActor) != NULL && pMsg->targetActor != (*pActor)->actorID)
                {
                    pActor = &(*pActor)->pNext;
                }
                if ((*pActor) != NULL)
                {
                    (*pActor)->state = AS_Stopped;
                    if ((*pActor)->pActorData != NULL)
                        free((*pActor)->pActorData);
                    Actor *pDel = (*pActor);
                    (*pActor) = (*pActor)->pNext;
                    free(pDel);
                }
            }
            RemoveMappingTable(pMsg->targetActor);
            break;
        case FM_Close:
            return 0;
 
        case FM_Custom:
            LogMsg("Custom",DEBUG,0);
            handleGlobal.frame.CustomCntlProc(&pMsg->data, 64);
        }
    }

    return 1;
}

Actor *SearchActor(int actorID)
{
    Actor *pActor = handleGlobal.pActorQueue;
    while (pActor != NULL)
    {
        if (pActor->actorID != actorID)
            pActor = pActor->pNext;
        else
            break;
    }

    return pActor;
}

int DispatchActorMessage(int rank, ActorMessage *pMsg)
{
    Actor *pActor;
    if (rank != 0) // If this is worker node
    {
        if ((pMsg->rcvActorID / 0x10000) == rank)
        {
            sprintf(info, "Dispatch a message to actor %d",pMsg->rcvActorID);
            LogMsg(info, DEBUG, 0);
            pActor = SearchActor(pMsg->rcvActorID);
            if (pActor != NULL)
            {
                sprintf(info, "Dispatch OK actor %d",pActor->actorID);
                LogMsg(info, DEBUG, 0);
 
                int point = (pActor->msgQueue.head + 1) % ACTOR_MSGQUEUE_LEN;
                if (point != pActor->msgQueue.tail)
                {
                    pActor->msgQueue.msg[point] = *pMsg;
                    pActor->msgQueue.head = point;  
                    return 1;
                }
                else return 0;
            }
        }
        else LogMsg("Dispatch msg err,not mine.",ERROR,0);
    }
    return 0;
}

int FrameworkPoll()
{
    int flag;
    MPI_Status status;

    LogMsg("Framework Poll",DEBUG,0);

    if (cntlRequest == NULL)
        MPI_Irecv(&cntlMessage, 1, AF_CNTL_TYPE, MPI_ANY_SOURCE, AF_CONTROL_TAG, MPI_COMM_WORLD, &cntlRequest);
   
    MPI_Test(&cntlRequest, &flag, &status);
    while (flag)
    {
        LogMsg("Receive a control Msg",DEBUG,0);
        if (!ProcessFrameworkMessage(handleGlobal.rank, &cntlMessage))
            return 0;
        MPI_Irecv(&cntlMessage, 1, AF_CNTL_TYPE, MPI_ANY_SOURCE, AF_CONTROL_TAG, MPI_COMM_WORLD, &cntlRequest);
        MPI_Test(&cntlRequest, &flag, &status);
    }

    if (handleGlobal.rank != 0)
    {
        if (cntlBcastRequest == NULL)
            MPI_Ibcast(&cntlBcastMessage, 1, AF_CNTL_TYPE, 0, MPI_COMM_WORLD, &cntlBcastRequest);

        MPI_Test(&cntlBcastRequest, &flag, &status);
        while (flag)
        {
            char info[100];
            
            LogMsg("Receive a bcast control Msg",DEBUG,0);

            if (!ProcessFrameworkMessage(handleGlobal.rank, &cntlBcastMessage))
                return 0;
            memset(&cntlBcastMessage, 0, sizeof(FrameMessage));

            MPI_Ibcast(&cntlBcastMessage, 1, AF_CNTL_TYPE, 0, MPI_COMM_WORLD, &cntlBcastRequest);
            MPI_Test(&cntlBcastRequest, &flag, &status);
        }
    }

    if (actorRequest == NULL)
        MPI_Irecv(&actorMessage, 1, AF_ACTOR_TYPE, MPI_ANY_SOURCE, AF_ACTORMSG_TAG, 
                    MPI_COMM_WORLD, &actorRequest);
    
    MPI_Test(&actorRequest, &flag, &status);
    while (flag)
    {
        LogMsg("Receive a actor Msg",DEBUG, 0);

        DispatchActorMessage(handleGlobal.rank, &actorMessage);
        MPI_Irecv(&actorMessage, 1, AF_ACTOR_TYPE, MPI_ANY_SOURCE, AF_CONTROL_TAG,
                    MPI_COMM_WORLD, &actorRequest);
        MPI_Test(&actorRequest, &flag, &status);
    }

    return 1;
}

int ActorRun()
{
    int cnt = 0;
    Actor **pActor = &handleGlobal.pActorQueue;
    while ((*pActor) != NULL)
    {
        char info[100];
        sprintf(info, "Run actor No. %d", cnt);
        LogMsg("Begin Actor Run", DEBUG, 0);
        if ((*pActor)->state == AS_Running)
        {
            (*pActor)->Run(*pActor);
            (*pActor)->step++;
            pActor = &((*pActor)->pNext);
            cnt++;
        }
        else if ((*pActor)->state == AS_Stopped)
        {
            LogMsg("ActorRun: One Stopped!", DEBUG, 0);
            if ((*pActor)->pActorData != NULL)
                free((*pActor)->pActorData);
            RemoveMappingTable((*pActor)->actorID);

            Actor *pDel = (*pActor);
            (*pActor) = (*pActor)->pNext;
            SendCntlMessage(FM_ActorDie, 0, pDel->actorID, NULL, 0);
            free(pDel);
        }
    }
    return 1;
}

int RunFramework()
{
    if (handleGlobal.state != FS_Init)
        return 0;

    int status = 1;

    if (handleGlobal.rank != 0)
    {
        LogMsg("-------Worker Node begin to run!-------", INFO, 0);

        while (status)
        {
            LogMsg("Worker run",INFO,0);
            status = FrameworkPoll();
            if (status == 0)
                break;
            status = ActorRun();
            if (status == 0)
                break;
            if (handleGlobal.frame.WorkerRun != NULL)
                status = handleGlobal.frame.WorkerRun();
            if (status == 0)
                break;

            usleep(MSECOND_PER_LOOP * 1000);
        }
        LogMsg("--------Worker Node Finish!-------", INFO, 0);
    }
    else if (handleGlobal.rank == 0)
    {

        LogMsg("-------Master Node begin to run!-----", INFO, 0);

        int i, activeWorkers = -1, returnCode;
        while (status)
        {
            LogMsg("Master begin loop!", DEBUG, 0);

            status = masterPoll();
            if (status == 0)    break;
            status = FrameworkPoll();
            if (status == 0)    break;
            status = ActorRun();
            if (status == 0)    break;
            if (handleGlobal.frame.MasterRun != NULL)
                status = handleGlobal.frame.MasterRun();
            if (status == 0)    break;
            LogMsg("Master run OK!", DEBUG, 0);

            activeWorkers = handleGlobal.activeNodeCnt;
            // If we have no more active workers then quit poll loop which will effectively shut the pool down when  processPoolFinalise is called
            if (activeWorkers == 0)
                break;

            usleep(MSECOND_PER_LOOP*1000);

        }
        char info[100];
        sprintf(info, "-------Master Node loop Finished! Status %d, active node %d", status, activeWorkers);
        LogMsg(info, INFO, 0);
    }
    // Finalizes the process pool, call this before closing down MPI
    processPoolFinalise();
    // Finalize MPI, ensure you have closed the process pool first

    MPI_Finalize();
}
