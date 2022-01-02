#include "autostrafe.h"

#include "../settings.h"
#include "../interfaces.h"
#include "../Utils/math.h"
#include <math.h>

#define PI 3.14159265

static void LegitStrafe(C_BasePlayer* localplayer, CUserCmd* cmd)
{
	if (localplayer->GetFlags() & FL_ONGROUND)
		return;

	if (cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT)
		return;

	if (cmd->mousedx <= 1 && cmd->mousedx >= -1)
		return;

	switch (Settings::AutoStrafe::type)
	{
		case AutostrafeType::AS_FORWARDS:
			cmd->sidemove = cmd->mousedx < 0.f ? -250.f : 250.f;
			break;
		case AutostrafeType::AS_BACKWARDS:
			cmd->sidemove = cmd->mousedx < 0.f ? 250.f : -250.f;
			break;
		case AutostrafeType::AS_LEFTSIDEWAYS:
			cmd->forwardmove = cmd->mousedx < 0.f ? -250.f : 250.f;
			break;
		case AutostrafeType::AS_RIGHTSIDEWAYS:
			cmd->forwardmove = cmd->mousedx < 0.f ? 250.f : -250.f;
			break;
		default:
			break;
	}
}

static void RageStrafe(C_BasePlayer* localplayer, CUserCmd* cmd)
{
	static bool leftRight;
	static float prevYaw;
	bool inMove = cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT;

	float yaw_change = 0.0f;

	if (localplayer->GetVelocity().Length() > 50.f)
		yaw_change = 30.0f * fabsf(30.0f / localplayer->GetVelocity().Length());

	if(yaw_change > 90.0f)
		yaw_change = 90.0f;

	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);

	float forwardmove = cmd->forwardmove;
	float sidemove = cmd->sidemove;
	
	static float newDir = 0.0f;
	float goal = atan2(forwardmove, sidemove);

	float max = PI * 2;
	float temp = fmod(goal - newDir, max);
	
	float delta = fmod(2 * temp, max) - temp;

	float yawChangeInRadians = yaw_change * PI / 180.0f;
		
	if(delta > yawChangeInRadians)
		delta = yawChangeInRadians;

	if(delta < -yawChangeInRadians)
		delta = -yawChangeInRadians;

	if(inMove) {
		newDir += delta;
	}
	
	float goalYaw = viewAngles.y + newDir / PI * 180.0f - 90.0f;
	
	float deltaYaw = prevYaw - goalYaw;
	
	if (!(localplayer->GetFlags() & FL_ONGROUND) || cmd->buttons & IN_JUMP) {
		forwardmove = 0.0f;

		if(deltaYaw == 0.0f) {
			if (leftRight)
			{
				goalYaw += yaw_change;
				sidemove = 450.0f;
			}
			else if (!leftRight)
			{
				goalYaw -= yaw_change;
				sidemove = -450.0f;
			}

			leftRight = !leftRight;
		} else if(deltaYaw > 0.0f) {
			sidemove = 450.0f;
		} else if(deltaYaw < 0.0f) {
			sidemove = -450.0f;
		}
	}
	
	if (!(localplayer->GetFlags() & FL_ONGROUND) || cmd->buttons & IN_JUMP) {
		viewAngles.y = goalYaw;
	} else {
		prevYaw = goalYaw;
		newDir = goal;
	}
	
	prevYaw = viewAngles.y;

	Math::NormalizeAngles(viewAngles);
	Math::ClampAngles(viewAngles);

	if (!Settings::AutoStrafe::silent) {
		cmd->viewangles = viewAngles;
		cmd->forwardmove = forwardmove;
		cmd->sidemove = sidemove;
	}
	else
		Math::CorrectMovement(viewAngles, cmd, forwardmove, sidemove);
}

void AutoStrafe::CreateMove(CUserCmd* cmd)
{
	if (!Settings::AutoStrafe::enabled)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer)
		return;

	if (!localplayer->GetAlive())
		return;

	if (localplayer->GetMoveType() == MOVETYPE_LADDER || localplayer->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	switch (Settings::AutoStrafe::type)
	{
		case AutostrafeType::AS_FORWARDS:
		case AutostrafeType::AS_BACKWARDS:
		case AutostrafeType::AS_LEFTSIDEWAYS:
		case AutostrafeType::AS_RIGHTSIDEWAYS:
			LegitStrafe(localplayer, cmd);
			break;
		case AutostrafeType::AS_RAGE:
			RageStrafe(localplayer, cmd);
			break;
	}
}
