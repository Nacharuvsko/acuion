#include "EventListener.h"

#include "Hacks/aimbot/aimbot.h"
#include "Hacks/misc/eventlog.h"
#include "Hacks/misc/votecast.h"
#include "Hacks/misc/autodisconnect.h"
#include "Hacks/misc/hitmarkers.h"
#include "Hacks/misc/namestealer.h"
#include "Hacks/aimbot/resolver.h"
#include "Hacks/misc/skinchanger.h"
#include "Hacks/misc/spammer.h"
#include "Hacks/misc/valvedscheck.h"
#include "interfaces.h"
#include "SDK/IGameEvent.h"

EventListener::EventListener(std::vector<const char*> events)
{
    for (const auto& it : events)
	gameEvents->AddListener(this, it, false);
}

EventListener::~EventListener()
{
    gameEvents->RemoveListener(this);
}

void EventListener::FireGameEvent(IGameEvent* event)
{
    Aimbot::FireGameEvent(event);
    Hitmarkers::FireGameEvent(event);
    voteCast::FireGameEvent(event);
    Eventlog::FireGameEvent(event);
    NameStealer::FireGameEvent(event);
    Resolver::FireGameEvent(event);
    Spammer::FireGameEvent(event);
    ValveDSCheck::FireGameEvent(event);
    SkinChanger::FireGameEvent(event);
    AutoDisconnect::FireGameEvent(event);
}

int EventListener::GetEventDebugID()
{
    return EVENT_DEBUG_ID_INIT;
}
