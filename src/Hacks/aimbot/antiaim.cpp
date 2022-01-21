#include "antiaim.h"

#include "aimbot.h"
#include "../../settings.h"
#include "../../Hooks/hooks.h"
#include "../../Utils/math.h"
#include "../../Utils/entity.h"
#include "../../interfaces.h"
#include "../misc/valvedscheck.h"
#include "../../Utils/xorstring.h"

QAngle AntiAim::realAngle;
QAngle AntiAim::fakeAngle;
static bool manualswitch = true;

float AntiAim::GetMaxDelta( CCSGOAnimState *animState ) {

    float speedFraction = std::max(0.0f, std::min(animState->feetShuffleSpeed, 1.0f));

    float speedFactor = std::max(0.0f, std::min(1.0f, animState->feetShuffleSpeed2));

    float unk1 = ((animState->runningAccelProgress * -0.30000001) - 0.19999999) * speedFraction;
    float unk2 = unk1 + 1.0f;
    float delta;

    if (animState->duckProgress > 0)
    {
        unk2 += ((animState->duckProgress * speedFactor) * (0.5f - unk2));// - 1.f
    }

    delta = *(float*)((uintptr_t)animState + 0x3A4) * unk2;

    return delta - 0.5f;
}

static float Distance(Vector a, Vector b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

// Pasted from space!hook
static bool GetBestHeadAngle(QAngle& angle)
{
	float b, r, l;

	Vector src3D, dst3D, forward, right, up, src, dst;

	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer)
		return false;

	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);

	viewAngles.x = 0;

	Math::AngleVectors(viewAngles, forward, right, up);

	auto GetTargetEntity = [ & ] ( void )
	{
		float bestFov = FLT_MAX;
		C_BasePlayer* bestTarget = NULL;

		for( int i = 0; i < engine->GetMaxClients(); ++i )
		{
			C_BasePlayer* player = (C_BasePlayer*) entityList->GetClientEntity(i);

			if (!player
				|| player == localplayer
				|| player->GetDormant()
				|| !player->GetAlive()
				|| player->GetImmune()
				|| player->GetTeam() == localplayer->GetTeam())
				continue;

			float fov = Math::GetFov(viewAngles, Math::CalcAngle(localplayer->GetEyePosition(), player->GetEyePosition()));

			if( fov < bestFov )
			{
				bestFov = fov;
				bestTarget = player;
			}
		}

		return bestTarget;
	};

	auto target = GetTargetEntity();
	filter.pSkip = localplayer;
	src3D = localplayer->GetEyePosition();
	dst3D = src3D + (forward * 384);

	if (!target)
		return false;

	ray.Init(src3D, dst3D);
	trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	b = (tr.endpos - tr.startpos).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);
	trace->TraceRay(ray2, MASK_SHOT, &filter, &tr);
	r = (tr.endpos - tr.startpos).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);
	trace->TraceRay(ray3, MASK_SHOT, &filter, &tr);
	l = (tr.endpos - tr.startpos).Length();

	if (b < r && b < l && l == r)
		return true; //if left and right are equal and better than back

	if (b > r && b > l)
		angle.y -= 180; //if back is the best angle
	else if (r > l && r > b)
		angle.y += 90; //if right is the best angle
	else if (r > l && r == b)
		angle.y += 135; //if right is equal to back
	else if (l > r && l > b)
		angle.y -= 90; //if left is the best angle
	else if (l > r && l == b)
		angle.y -= 135; //if left is equal to back
	else
		return false;

	return true;
}

static bool HasViableEnemy()
{
    C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

    for (int i = 1; i < engine->GetMaxClients(); ++i)
    {
        C_BasePlayer* entity = (C_BasePlayer*) entityList->GetClientEntity(i);

        if (!entity
            || entity == localplayer
            || entity->GetDormant()
            || !entity->GetAlive()
            || entity->GetImmune())
            continue;

        if( !Aimbot::friends.empty() ) // check for friends, if any
        {
            IEngineClient::player_info_t entityInformation;
            engine->GetPlayerInfo(i, &entityInformation);

            if (std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid) != Aimbot::friends.end())
                continue;
        }

        if (Settings::Aimbot::friendly || !Entity::IsTeamMate(entity, localplayer))
            return true;
    }

    return false;
}
static C_BasePlayer* GetClosestPlayer()
{
    C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	QAngle myAngle;
    engine->GetViewAngles(myAngle);
    float fov = 180.0f;
    C_BasePlayer* tmp = nullptr;

    for (int i = 1; i < engine->GetMaxClients(); ++i)
    {
        C_BasePlayer* entity = (C_BasePlayer*) entityList->GetClientEntity(i);

        if (!entity
            || entity == localplayer
            || entity->GetDormant()
            || !entity->GetAlive()
            || entity->GetImmune())
            continue;

        if( !Aimbot::friends.empty() ) // check for friends, if any
        {
            IEngineClient::player_info_t entityInformation;
            engine->GetPlayerInfo(i, &entityInformation);

            if (std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid) != Aimbot::friends.end())
                continue;
        }

        if (Settings::Aimbot::friendly || !Entity::IsTeamMate(entity, localplayer)){
       		float tmpFOV = Math::GetFov(myAngle, Math::CalcAngle(localplayer->GetEyePosition(), entity->GetVecOrigin()));
       		if (tmpFOV < fov)
            {
            	fov = tmpFOV;
            	tmp = entity;
            }
        }
    }

    return tmp;
}

static void DoAntiAimPreset(QAngle& angle, bool bSend, CCSGOAnimState* animState)
{
	float maxDelta = AntiAim::GetMaxDelta(animState);
	float halfDelta = maxDelta / 2;
	static bool yFlip = false;

	switch (Settings::AntiAim::Preset::type)
	{
	case AntiAimYaw_Preset::JITTER:

	    if (Settings::AntiUntrusted::enabled) { // jitters makes you get cocked by SMAC.
		break;
	    }

	    angle.x = 89.0f;
	    if (yFlip)
		angle.y += manualswitch ? halfDelta : -halfDelta;
	    else
		angle.y += manualswitch ? -halfDelta + 180.0f : halfDelta + 180.0f;

	    if (!bSend)
	    {
		if (yFlip)
		    angle.y += manualswitch ? -maxDelta : maxDelta;
		else
		    angle.y += manualswitch ? maxDelta : -maxDelta;
	    }
	    else
		yFlip = !yFlip;

	    break;
	}

}


bool leanJerking = false; //fixme
static void DoAntiAimY(QAngle& angle, bool& clamp, CCSGOAnimState* animState)
{

	if (!animState)
		return;

	AntiAimYaw_Real aa_type = Settings::AntiAim::Yaw::type;
	AntiAimYaw_Fake fake = Settings::AntiAim::Fake::type;
	float maxDelta = AntiAim::GetMaxDelta(animState);
	static bool yFlip;
	float temp;
	double factor;
	static float trigger;
	QAngle temp_qangle;
	QAngle followangle;
	int random;
	int maxJitter;
	Vector test;
	C_BasePlayer* tmp;

	switch (aa_type)
	{
		case AntiAimYaw_Real::BACKWARDS:
			angle.y -= 180.0f;
			break;
		case AntiAimYaw_Real::FORWARDS:
			angle.y -= 0.0f;
			break;
		default:
			angle.y -= 0.0f;
			break;
	}

	switch(fake) {
	    case AntiAimYaw_Fake::MEGALEAN:
	    {
		if (Settings::AntiUntrusted::enabled)
		{ // changing Z-Axis makes you get cocked by SMAC too.
		    break;
		}
		if (Settings::AntiAim::Fake::leanJerk)
		{
		    angle.z = leanJerking ? Settings::AntiAim::Fake::maxLean : -1 * Settings::AntiAim::Fake::maxLean;
		    leanJerking = !leanJerking;
		}
		else
		{
		    angle.z = Settings::AntiAim::Fake::maxLean;
		}
		break;
	    }
	}

	angle.y += Settings::AntiAim::Yaw::offset;
	if (Settings::AntiAim::Yaw::addDesyncEnabled)
		angle.y += (manualswitch ? maxDelta : -maxDelta) * (Settings::AntiAim::Yaw::addDesyncOffset / 100);
}

static void DoAntiAimX(QAngle& angle, bool bFlip, bool& clamp)
{
    static float pDance = 0.0f;
    AntiAimType_X aa_type = Settings::AntiAim::Pitch::type;

    if (Settings::AntiUntrusted::enabled)
    { // Not sure, if pitch makes untrusted, but for sAfEtY let's not use it ^_^
	return;
    }

    switch (aa_type)
    {
        case AntiAimType_X::STATIC_UP:
            angle.x = -89.0f;
            break;
        case AntiAimType_X::STATIC_DOWN:
            angle.x = 89.0f;
            break;
        case AntiAimType_X::DANCE:
            pDance += 45.0f;
            if (pDance > 100)
                pDance = 0.0f;
            else if (pDance > 75.f)
                angle.x = -89.f;
            else if (pDance < 75.f)
                angle.x = 89.f;
            break;
        case AntiAimType_X::FRONT:
            angle.x = 0.0f;
            break;
        default:
            break;
    }
}

static void DoAntiAimFake(QAngle &angle, CCSGOAnimState* animState)
{
	if (!animState)
		return;

	// float maxDelta = AntiAim::GetMaxDelta(animState);
	static bool yFlip = false;

	// i hope desyncs are not getting da untrusta cock :o
	switch (Settings::AntiAim::Fake::type)
	{
		case AntiAimYaw_Fake::STATIC_LEFT:
			angle.y += 120; // remove these annoying ACT_CSGO_IDLE_TURN_BALANCEADJUST
			break;

		case AntiAimYaw_Fake::STATIC_RIGHT:
			angle.y += 120;
			break;

		case AntiAimYaw_Fake::JITTER:
			angle.y += yFlip ? 120 : -120;
			yFlip = !yFlip;
			break;

		case AntiAimYaw_Fake::MANUAL:
			angle.y += manualswitch ? 120 : -120;
			break;
	}
}

void AntiAim::CreateMove(CUserCmd* cmd)
{
    if (!Settings::AntiAim::Yaw::enabled && !Settings::AntiAim::Pitch::enabled && !Settings::AntiAim::LBYBreaker::enabled)
        return;

    if (Settings::Aimbot::AimStep::enabled && Aimbot::aimStepInProgress)
        return;

    QAngle oldAngle = cmd->viewangles;
    float oldForward = cmd->forwardmove;
    float oldSideMove = cmd->sidemove;
    
    // AntiAim::realAngle = AntiAim::fakeAngle = CreateMove::lastTickViewAngles;

    QAngle angle = cmd->viewangles;

    C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!localplayer || !localplayer->GetAlive())
        return;

    C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    if (!activeWeapon)
        return;

    if (activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_GRENADE)
    {
        C_BaseCSGrenade* csGrenade = (C_BaseCSGrenade*) activeWeapon;

        if (csGrenade->GetThrowTime() > 0.f)
            return;
    }

    if (cmd->buttons & IN_USE || cmd->buttons & IN_ATTACK || (cmd->buttons & IN_ATTACK2 && *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER))
        return;

    if (localplayer->GetMoveType() == MOVETYPE_LADDER || localplayer->GetMoveType() == MOVETYPE_NOCLIP)
        return;

    // Knife
    if (Settings::AntiAim::AutoDisable::knifeHeld && localplayer->GetAlive() && activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_KNIFE)
        return;

    if (Settings::AntiAim::AutoDisable::noEnemy && localplayer->GetAlive() && !HasViableEnemy())
        return;

    if (inputSystem->IsButtonDown(KEY_LEFT) && !manualswitch)
		manualswitch = true;

	if (inputSystem->IsButtonDown(KEY_RIGHT) && manualswitch)
		manualswitch = false;

    QAngle edge_angle = angle;
    bool edging_head = Settings::AntiAim::HeadEdge::enabled && GetBestHeadAngle(edge_angle);

    static bool bSend = true;
    bSend = !bSend;

    bool should_clamp = Settings::AntiUntrusted::enabled;

    bool needToFlick = false;
    float tempangle = 0.f;
    static bool lbyBreak = false;
    static float lastCheck;
    static float nextUpdate = FLT_MAX;
    float vel2D = localplayer->GetVelocity().Length2D();//localplayer->GetAnimState()->verticalVelocity + localplayer->GetAnimState()->horizontalVelocity;

    CCSGOAnimState* animState = localplayer->GetAnimState();
    if( Settings::AntiAim::LBYBreaker::enabled ){
        if( vel2D >= 0.1f || !(localplayer->GetFlags() & FL_ONGROUND) || localplayer->GetFlags() & FL_FROZEN ){
            lbyBreak = false;
            lastCheck = globalVars->curtime;
            nextUpdate = globalVars->curtime + 0.22;
        } else {
            if( !lbyBreak && ( globalVars->curtime - lastCheck ) > 0.22 ){
                tempangle = Settings::AntiAim::LBYBreaker::manual ? manualswitch ? /*57.5f +*/ Settings::AntiAim::LBYBreaker::offset : /*-57.5f +*/ -Settings::AntiAim::LBYBreaker::offset : Settings::AntiAim::LBYBreaker::offset;
                lbyBreak = true;
                lastCheck = globalVars->curtime;
                nextUpdate = globalVars->curtime + 1.1;
                needToFlick = true;
            } else if( lbyBreak && ( globalVars->curtime - lastCheck ) > 1.1 ){
                tempangle = Settings::AntiAim::LBYBreaker::manual ? manualswitch ? /*57.5f +*/ Settings::AntiAim::LBYBreaker::offset : /*-57.5f +*/ -Settings::AntiAim::LBYBreaker::offset : Settings::AntiAim::LBYBreaker::offset;
                lbyBreak = true;
                lastCheck = globalVars->curtime;
                nextUpdate = globalVars->curtime + 1.1;
                needToFlick = true;
            }
        }
    }
    if(Settings::AntiAim::Preset::type != AntiAimYaw_Preset::DIY)
    {
        if ((nextUpdate - globalVars->interval_per_tick) >= globalVars->curtime && nextUpdate <= globalVars->curtime)
        	CreateMove::sendPacket = false;

        if (needToFlick){
	        CreateMove::sendPacket = false;
	        angle.y += tempangle;
    	}
    	else
	    DoAntiAimPreset(angle, bSend, animState);
    }
    else
    {
    if (Settings::AntiAim::Yaw::enabled)
    {
        DoAntiAimY(angle, should_clamp, animState);

        if ((nextUpdate - globalVars->interval_per_tick) >= globalVars->curtime && nextUpdate <= globalVars->curtime)
        	CreateMove::sendPacket = false;

        if (needToFlick){
	    CreateMove::sendPacket = false;
	    angle.y += tempangle;
    	}

        if (Settings::AntiAim::HeadEdge::enabled && edging_head && !bSend)
            angle.y = edge_angle.y;

        static bool bSavingAngles = true;
        static float SavedYawForDesync; 
        // When you make 180 while choked you will have desync >60 and you will be shot in head (teory) 
        // but for legit your mouse movement probably be a bit late
        if (Settings::AntiAim::RageDesyncFix::enabled){
	        if (Settings::FakeLag::enabled ? !CreateMove::sendPacket : !bSend && bSavingAngles && !needToFlick){ 
	        // todo: add first choked tick check instead of need to flick
	        	SavedYawForDesync = angle.y;
	        	bSavingAngles = false;
	        }else if (Settings::FakeLag::enabled ? CreateMove::sendPacket : bSend && !needToFlick){
	        	angle.y = SavedYawForDesync;
	        	bSavingAngles = true;
	        }
	    }

        Math::NormalizeAngles(angle);
    }

	if (Settings::AntiAim::Fake::enabled && Settings::FakeLag::enabled ? !CreateMove::sendPacket : !bSend && !needToFlick)
    {
	    DoAntiAimFake(angle, animState);
        Math::NormalizeAngles(angle);
    }

    if (Settings::AntiAim::Pitch::enabled)
        DoAntiAimX(angle, bSend, should_clamp);
	}
    if( should_clamp ){
        Math::NormalizeAngles(angle);
        Math::ClampAngles(angle);
    }

    if (!Settings::FakeLag::enabled)
    {
    	if (!needToFlick)
		    CreateMove::sendPacket = bSend;

        if (bSend)
            AntiAim::realAngle = CreateMove::lastTickViewAngles;
        else
            AntiAim::fakeAngle = angle;
    }
    else
    {
        if (CreateMove::sendPacket)
            AntiAim::realAngle = CreateMove::lastTickViewAngles;
        else
            AntiAim::fakeAngle = angle;
    }

    cmd->viewangles = angle;

    Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);
}
