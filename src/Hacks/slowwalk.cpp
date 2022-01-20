#include "slowwalk.hpp"

#include "../settings.h"
#include "../interfaces.h"
#include "../Hooks/hooks.h"

#define GetPercentVal(val, percent) (val * (percent/100.f))

void SlowWalk::CreateMove(CUserCmd *cmd)
{

    	if (!Settings::AntiAim::Slowwalk::enabled) {
	    return;
	}

	SlowWalking = false;
	C_BasePlayer *localplayer = (C_BasePlayer *) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer || !localplayer->GetAlive()) {
		return;
	}
	if (!inputSystem->IsButtonDown(Settings::AntiAim::Slowwalk::Key)) {
		return;
	}

	if (!(localplayer->GetFlags() & FL_ONGROUND))
	    return;

	SlowWalking = true;
	QAngle ViewAngle;
	engine->GetViewAngles(ViewAngle);

	static Vector oldOrigin = localplayer->GetAbsOrigin();
	Vector velocity = (localplayer->GetVecOrigin() - oldOrigin)
	                  * (1.f / globalVars->interval_per_tick);
	oldOrigin = localplayer->GetAbsOrigin();
	float speed = velocity.Length();

	if (speed > Settings::AntiAim::Slowwalk::Speed) {
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		CreateMove::sendPacket = false;
	} else {
		CreateMove::sendPacket = true;
	}
}
