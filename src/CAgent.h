typedef struct _Position {
    int x;
    int y;
} Position; 

class BaseMktAgent {
public:
//Properties
    int agentID;        //ID of Agent, a unique identifier
    Position pos;       //Position of Agent
    int agentType;      //Agent type
//Actions
    int run();
};