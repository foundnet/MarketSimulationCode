#include "CInfoPublisher.h"

Agent* clone(repast::Agent_id id) {return new InfoPublisher(id); }

int InfoPublisher::PublishInfo(Information *info, int publishMethod)
{
    broadcastInformation(info);
}