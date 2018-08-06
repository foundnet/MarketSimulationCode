#include <unistd.h>
#include "actorframe.h"
#include "squirrel-functions.h"
#include "ran2.h"

#define NUM_INFECTED_SQUIRREL 5
#define STEPS_PER_MONTH  500
#define MAX_MONTH  12

typedef enum _SimuMsgType { SM_INFECT_SET,
                            SM_INCELL,
                            SM_INFECTED } SimuMsgType;

typedef enum _ActorType { TP_SQUIRREL,
                          TP_CELL } ActorType;

typedef struct _SquirData
{
    //The total infectLevel that past 50 steps have
    int infectionLevel[50];
    int totalInfectionLevel;
    int populationInflux[50];
    int totalPopulationInflux;
    float posX;
    float posY;
    int curCellActorID;
    int curCell;
    int isInfected;
    int infectedStep;
} SquirData;

typedef struct _CellData
{
    int infectionLevel[2];
    int populationInflux[3];
    int curMonth;
    int curInfectionLevel;
    int curPopulationInflux;
    int cellNum;

} CellData;

typedef struct _CellParam
{
    int cellNum;
    int infectionLevel;
    int populationInflux;
} CellParam;

typedef struct _PubData
{
    int curMonth;
    int totalMonth;
    CellParam *cellInfo;
} PubData;

static long seed; //Random Seeds
static PubData publicData;

//The custom init function of the framework
int FrameInit()
{
    memset(&publicData, 0, sizeof(PubData));
    for (int i = 0; i < handleGlobal.actortypes; i++)
    {
        if (handleGlobal.actorinfo[i].category == TP_SQUIRREL)
        {
            for (int j = 0; j < NUM_INFECTED_SQUIRREL; j++)
            {
                float rand = ran2(&seed);
                int seq = (int)(rand * handleGlobal.actorinfo[i].actorsBuilt * 2) % handleGlobal.actorinfo[i].actorsBuilt + 1;
                int actorID = SearchMapTable(seq, TP_SQUIRREL);
                sprintf(info,"-------The number of created Squirrel is %d, infected squirrel are randomly selected, actor seq %d, ID %d" , handleGlobal.actorinfo[i].actorsBuilt, seq, actorID); 
                if (actorID > 0)
                {
                    SendActorMessage(NULL, actorID, SM_INFECT_SET, NULL, 0);
                    LogMsg("Send Random OK", DEBUG, 0);
                }
                else
                    return 0;
            }
        }
        else if (handleGlobal.actorinfo[i].category == TP_CELL)
        {
            publicData.cellInfo = (CellParam *)malloc(sizeof(CellParam) * handleGlobal.actorinfo[i].actorsBuilt);
        }
    }
    return 1;
}

typedef struct _Position
{
    float X;
    float Y;
} Position;

int SquirRunStep(Actor *actor)
{
    LogMsg("Squirrel Run!", DEBUG, actor->actorID);

    ActorMessage msg;
    SquirData *pSquirData = (SquirData *)(actor->pActorData);
    while (ReadActorMessage(actor, &msg) > 0)
    {
        switch (msg.msgCatogory)
        {
        case SM_INFECT_SET:
            LogMsg("Squirrel infected status received!", DEBUG, actor->actorID);

            pSquirData->isInfected = 1;
            pSquirData->infectedStep = actor->step;
            SendActorMessage(actor, pSquirData->curCellActorID, SM_INFECTED, NULL, 0);
        }
    }

    LogMsg("Squirrel message passed!", DEBUG, actor->actorID);

    //Maintain private data
    int addIndex = actor->step % 50;
    int minusIndex = (actor->step - 1) % 50;
    pSquirData->infectionLevel[addIndex] = publicData.cellInfo[pSquirData->curCell].infectionLevel;
    pSquirData->populationInflux[addIndex] = publicData.cellInfo[pSquirData->curCell].populationInflux;

    pSquirData->totalInfectionLevel += pSquirData->infectionLevel[addIndex];
    if (actor->step >= 50)
        pSquirData->totalInfectionLevel -= pSquirData->infectionLevel[minusIndex];
    pSquirData->totalPopulationInflux += pSquirData->populationInflux[addIndex];
    if (actor->step >= 50)
        pSquirData->totalPopulationInflux -= pSquirData->populationInflux[minusIndex];
    char info[100];
    sprintf(info, "Squirrel Run Step:%ld CurCell-%d infect:%d popula:%d Squirrel infect:%d popul:%d", actor->step, pSquirData->curCell, publicData.cellInfo[pSquirData->curCell].infectionLevel, publicData.cellInfo[pSquirData->curCell].populationInflux, pSquirData->totalInfectionLevel, pSquirData->totalPopulationInflux);
    LogMsg(info, DEBUG, actor->actorID);

    // Give birth?
    if (actor->step % 50 == 49)
    {
        float result = (float)(pSquirData->totalPopulationInflux) / 50;
        if (willGiveBirth(result, &seed))
        {
            LogMsg("--------Give birth to a new squirrel!------", INFO, actor->actorID);
            Position myPos = {pSquirData->posX, pSquirData->posY};
            SendCntlMessage(FM_ActorNew, 0, 0, &myPos, sizeof(Position));
        }
    }

    //Should be infected?
    if (!pSquirData->isInfected)
    {
        float result = (float)pSquirData->totalInfectionLevel / 50;
        if (willCatchDisease(result, &seed))
        {
            LogMsg("--------Squirrel is infected!--------", INFO, actor->actorID);
            pSquirData->isInfected = 1;
            pSquirData->infectedStep = actor->step;
            SendActorMessage(actor, pSquirData->curCellActorID, SM_INFECTED, NULL, 0);
        }
    }

    //Should die?
    if (pSquirData->isInfected && actor->step >= pSquirData->infectedStep + 50)
    {
        int random = (int)(100 * ran2(&seed)) % 6;
        if (random == 0)
        {
            LogMsg("--------Squirrel died!------", INFO, actor->actorID);

            actor->state = AS_Stopped;
        }
    }


    //Calculate next step
    if (actor->state != AS_Stopped)
    {
        SquirData *pData = pSquirData;
        squirrelStep(pData->posX, pData->posY, &pData->posX, &pData->posY, &seed);
        int cell = getCellFromPosition(pData->posX, pData->posY);

        if (cell != pSquirData->curCell)
        {
            sprintf(info, "--------Squirrel enter new cell!Before %d, Now %d---", pSquirData->curCell, cell);
            LogMsg(info, INFO, actor->actorID);

            pSquirData->curCellActorID = SearchMapTable(cell, TP_CELL);
            pSquirData->curCell = cell;
            if (pData->curCellActorID > 0)
                SendActorMessage(actor, pData->curCellActorID, SM_INCELL, &actor->actorID, sizeof(int));
        }
    }

     LogMsg("Squirrel message end!", DEBUG, actor->actorID);
    return 1;
}

int SquirInit(Actor *actor, void *data, int len)
{
    LogMsg("Begin squirrel Init", DEBUG, 0);

    SquirData *pData = (SquirData *)malloc(sizeof(SquirData));
    memset(pData, 0, sizeof(SquirData));
    squirrelStep(0, 0, &pData->posX, &pData->posY, &seed);

    char infomsg[199];
    sprintf(infomsg, "--------Squarrel init, x:%lf  y:%lf------", pData->posX, pData->posY);
    LogMsg(infomsg, INFO, 0);

    int cell = getCellFromPosition(pData->posX, pData->posY);

    pData->curCellActorID = SearchMapTable(cell, TP_CELL);
    actor->pActorData = (void *)pData;

    char info[100];
    sprintf(info, "--------Squirrel init, x:%f  y:%f, node %d, aclocated in cell %d, actorID %x,", pData->posX, pData->posY, handleGlobal.rank, cell, pData->curCellActorID);
    LogMsg(info, INFO, 0);

    //   if (pData->curCellActorID > 0)
    //       SendActorMessage(actor,pData->curCellActorID,SM_INCELL,&actor->actorID,sizeof(int));
    //   else return 0;

    return 1;
}

int SquirDestroy(Actor *actor)
{
    free(actor->pActorData);
    return 1;
}

typedef struct _BcastCellState
{
    int bcastType;
    int cellNum;
    int infectionLevel;
    int populationInflux;
    int curMonth;
} BcastCellState;

int CellRunStep(Actor *actor)
{
    ActorMessage msg;
    LogMsg("Cell: Run start up.!", DEBUG, actor->actorID);
    CellData *pCellData = (CellData *)(actor->pActorData);

    while (ReadActorMessage(actor, &msg) > 0)
    {
        switch (msg.msgCatogory)
        {
        case SM_INCELL:
            LogMsg("---------Cell: A squirrel comes into a cell!-----", INFO, actor->actorID);
            pCellData->curPopulationInflux++;
            break;
        case SM_INFECTED:
            LogMsg("---------Cell: A squirrel is infected in my cell!", INFO, actor->actorID);
            pCellData->curInfectionLevel++;
        }

        if (publicData.curMonth > pCellData->curMonth)
        {
            sprintf(info,"--------Cell: New month %d, Broadcast my new param! ",publicData.curMonth );
            LogMsg(info, INFO, actor->actorID);

            pCellData->curMonth = publicData.curMonth;
            int updateIndex = pCellData->curMonth % 2;
            pCellData->infectionLevel[updateIndex] = pCellData->curInfectionLevel;
            updateIndex = pCellData->curMonth % 3;
            pCellData->populationInflux[updateIndex] = pCellData->curPopulationInflux;

            pCellData->curInfectionLevel = 0;
            pCellData->curPopulationInflux = 0;

            int bcastInfectLevel = pCellData->infectionLevel[0] + pCellData->infectionLevel[1];
            int bcastPopulationInflux = pCellData->populationInflux[0] +
                                        pCellData->populationInflux[1] +
                                        pCellData->populationInflux[2];

            BcastCellState cellState = {1, actor->categorySeq, bcastInfectLevel, bcastPopulationInflux, publicData.curMonth};
            BroadcastCntlMessage(FM_Custom, 0, &cellState, sizeof(BcastCellState));

            if (publicData.curMonth >= publicData.totalMonth)
                actor->state = AS_Stopped;
        }
    }

    return 1;
}

int CellInit(Actor *actor, void *data, int len)
{
    LogMsg("Cell: init.!", DEBUG, actor->actorID);

    CellData *pData = (CellData *)malloc(sizeof(CellData));
    memset(pData, 0, sizeof(CellData));
    actor->pActorData = (void *)pData;
    return 1;
}

int CellDestroy(Actor *actor)
{
    free(actor->pActorData);
    return 1;
}
int MasterRun()
{
    static int curMonth = 0;
    static int steps = 0;

    steps++;

    char info[100];
    sprintf(info, "Master custom run. %d", steps);
    LogMsg(info, DEBUG, 0);
    if (steps % STEPS_PER_MONTH == 0)
    {
        curMonth++;
        if (curMonth > MAX_MONTH)
        {
            BroadcastCntlMessage(FM_Close, 0, NULL, 0);
            LogMsg("[[[[[[[[[[[[[[[[[Simulation Reach Maximum Month, Exit.]]]]]]]]]]]]]]]]",INFO,0);
            return 0;
        }
        int bcastCon[2] = {2, curMonth};
        BroadcastCntlMessage(FM_Custom, 0, &bcastCon, sizeof(int) * 2);
        sprintf(info, "--------New Month comes! New month is  %d", curMonth);
        LogMsg(info, INFO, 0);
    }

    return 1;
}

int CustomCntlProc(void *data, int len)
{
    int *cntl = data;
    char info[100];

    if (cntl[0] == 1)           //New Param from all the cell nodes.
    {
        BcastCellState *pStat = (BcastCellState *)data;
        if (pStat->curMonth >= publicData.curMonth)
        {
            sprintf(info, "--------Cell-%d, Node updated New Month from %d to %d, new infect %d, new popul %d", pStat->cellNum, publicData.curMonth, pStat->curMonth, pStat->infectionLevel, pStat->populationInflux);
            LogMsg(info, INFO, 0);

            publicData.curMonth = pStat->curMonth;
            publicData.cellInfo[pStat->cellNum].infectionLevel = pStat->infectionLevel;
            publicData.cellInfo[pStat->cellNum].populationInflux = pStat->populationInflux;
        }
    }
    else if (cntl[0] = 2)
    {
        sprintf(info, "--------Cell: New custom info :Old %d, New %d", publicData.curMonth, cntl[1]);
        LogMsg(info, INFO, 0);
        publicData.curMonth = cntl[1];
    }
    return 1;
}

int main(int argc, char *argv[])
{
    initialiseRNG(&seed);

    FrameDesc desc = {FrameInit, NULL, MasterRun, NULL, CustomCntlProc};
    int result = ConfigActorFrame(desc);

    ActorDesc squirrDesc = {TP_SQUIRREL, SquirRunStep, SquirInit, SquirDestroy, 0, 36, 20, 30, 0, AM_Even};
    ActorDesc cellDesc = {TP_CELL, CellRunStep, CellInit, CellDestroy, 0, 16, 20, 10, 0, AM_Even};

    char info[200];
    sprintf(info, "Before Cfginfo %d ,%d, %d, %d", squirrDesc.category, squirrDesc.nodelimit, squirrDesc.actorlimit, squirrDesc.actorCount);
    LogMsg(info, DEBUG, 0);

    result = AddActorProfile(squirrDesc);
    result = AddActorProfile(cellDesc);

    sprintf(info, "Cfginfo %d, %d ,%d, %d, %d", result, handleGlobal.actortypes, handleGlobal.actorinfo[0].nodelimit, handleGlobal.actorinfo[0].actorlimit, handleGlobal.actorinfo[0].actorCount);
    LogMsg(info, DEBUG, 0);

    result = InitActorFramework(argc, argv);
    result = RunFramework();
}
