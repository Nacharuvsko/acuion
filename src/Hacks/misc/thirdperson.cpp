#include "thirdperson.h"
#include "../aimbot/antiaim.h"

#include "../../settings.h"
#include "../../interfaces.h"

QAngle fakeang= {0.0f, 0.0f, 0.0f}, realang = {0.0f, 0.0f, 0.0f};

void ThirdPerson::OverrideView(CViewSetup *pSetup)
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

	if(!localplayer)
		return;

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());

	if (activeWeapon && activeWeapon->GetCSWpnData() && activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_GRENADE)
	{
		input->m_fCameraInThirdPerson = false;
		return;
	}

	if(localplayer->GetAlive() && Settings::ThirdPerson::enabled && !engine->IsTakingScreenshot())
	{
		static bool buttonToggle = false;
		if (inputSystem->IsButtonDown(Settings::ThirdPerson::key) && !buttonToggle)
		{
			buttonToggle = true;
			Settings::ThirdPerson::toggled = !Settings::ThirdPerson::toggled;
		}
		else if (!inputSystem->IsButtonDown(Settings::ThirdPerson::key) && buttonToggle)
			buttonToggle = false;

		if (Settings::ThirdPerson::toggled)
		{
			QAngle viewAngles;
			engine->GetViewAngles(viewAngles);
			trace_t tr;
			Ray_t traceRay;
			Vector eyePos = localplayer->GetEyePosition();

			Vector camOff = Vector(cos(DEG2RAD(viewAngles.y)) * Settings::ThirdPerson::distance,
							   		sin(DEG2RAD(viewAngles.y)) * Settings::ThirdPerson::distance,
							   		sin(DEG2RAD(-viewAngles.x)) * Settings::ThirdPerson::distance);

			traceRay.Init(eyePos, (eyePos - camOff));
			CTraceFilter traceFilter;
			traceFilter.pSkip = localplayer;
			trace->TraceRay(traceRay, MASK_SOLID, &traceFilter, &tr);

       	 	input->m_fCameraInThirdPerson = true;
			input->m_vecCameraOffset = Vector(viewAngles.x, viewAngles.y, Settings::ThirdPerson::distance * ((tr.fraction < 1.0f) ? tr.fraction : 1.0f) );
		}
		else
		{
			input->m_fCameraInThirdPerson = false;
			input->m_vecCameraOffset = Vector(0.f, 0.f, 0.f);
		}
	}
	else if(input->m_fCameraInThirdPerson)
	{
		input->m_fCameraInThirdPerson = false;
		input->m_vecCameraOffset = Vector(0.f, 0.f, 0.f);
	}
}


void ThirdPerson::FrameStageNotify(ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_RENDER_START && engine->IsInGame())
	{
		C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

		if (localplayer && localplayer->GetAlive() && Settings::ThirdPerson::enabled && input->m_fCameraInThirdPerson)
		{
            switch (Settings::ThirdPerson::type)
            {
                case ShowedAngle::REAL:
                    *localplayer->GetVAngles() = realang;
                    break;
                case ShowedAngle::FAKE:
                    *localplayer->GetVAngles() = fakeang;
                    break;
            }
		}
	}
}

void ThirdPerson::CreateMove(CUserCmd* cmd) // get angles
{
    if(CreateMove::sendPacket)
        realang = cmd->viewangles;
    else
        fakeang = cmd->viewangles;
}
