#include "hooks.h"

#include "../interfaces.h"

#include "../Hacks/misc/bhop.h"
#include "../Hacks/visual/chams.h"
#include "../Hacks/misc/noduckcooldown.h"
#include "../Hacks/aimbot/lagcomp.h"
#include "../Hacks/misc/autostrafe.h"
#include "../Hacks/misc/showranks.h"
#include "../Hacks/misc/autodefuse.h"
#include "../Hacks/misc/jumpthrow.h"
#include "../Hacks/misc/grenadehelper.h"
#include "../Hacks/visual/grenadeprediction.h"
#include "../Hacks/misc/edgejump.h"
#include "../Hacks/misc/autoblock.h"
#include "../Hacks/aimbot/predictionsystem.h"
#include "../Hacks/aimbot/aimbot.h"
#include "../Hacks/aimbot/triggerbot.h"
#include "../Hacks/misc/autoknife.h"
#include "../Hacks/aimbot/antiaim.h"
#include "../Hacks/misc/fakelag.h"
#include "../Hacks/visual/esp.h"
#include "../Hacks/visual/tracereffect.h"
#include "../Hacks/misc/thirdperson.h"
#include "../Hacks/misc/nofall.h"
#include "../Hacks/misc/keyfix.h"
#include "../Hacks/visual/noCSM.h"
#include "../Hacks/misc/slowwalk.hpp"

bool CreateMove::sendPacket = true;
QAngle CreateMove::lastTickViewAngles = QAngle(0, 0, 0);

typedef bool (*CreateMoveFn) (void*, float, CUserCmd*);

bool Hooks::CreateMove(void* thisptr, float flInputSampleTime, CUserCmd* cmd)
{
	clientModeVMT->GetOriginalMethod<CreateMoveFn>(25)(thisptr, flInputSampleTime, cmd);

	if (cmd && cmd->command_number)
	{
        // Special thanks to Gre-- I mean Heep ( https://www.unknowncheats.me/forum/counterstrike-global-offensive/290258-updating-bsendpacket-linux.html )
        uintptr_t rbp;
        asm volatile("mov %%rbp, %0" : "=r" (rbp));
        bool *sendPacket = ((*(bool **)rbp) - 0x18);
        CreateMove::sendPacket = true;

	/* run code that affects movement before prediction */
	BHop::CreateMove(cmd);
	NoDuckCooldown::CreateMove(cmd);
	NoCSM::CreateMove(cmd);
	AutoStrafe::CreateMove(cmd);
	ShowRanks::CreateMove(cmd);
	AutoDefuse::CreateMove(cmd);
	JumpThrow::CreateMove(cmd);
	GrenadeHelper::CreateMove(cmd);
	GrenadePrediction::CreateMove( cmd );
	EdgeJump::PrePredictionCreateMove(cmd);
	Autoblock::CreateMove(cmd);
	NoFall::PrePredictionCreateMove(cmd);

	PredictionSystem::StartPrediction(cmd);

	SlowWalk::CreateMove(cmd);

	Aimbot::CreateMove(cmd);
	Triggerbot::CreateMove(cmd);
	AutoKnife::CreateMove(cmd);
	LagComp::CreateMove(cmd);
	FakeLag::CreateMove(cmd);
	AntiAim::CreateMove(cmd);
	ESP::CreateMove(cmd);
	TracerEffect::CreateMove(cmd);
	ThirdPerson::CreateMove(cmd);
	Chams::CreateMove(cmd);
	KeyFix::CreateMove(cmd);
	PredictionSystem::EndPrediction();

	EdgeJump::PostPredictionCreateMove(cmd);
	NoFall::PostPredictionCreateMove(cmd);

        *sendPacket = CreateMove::sendPacket;

        if (CreateMove::sendPacket) {
            CreateMove::lastTickViewAngles = cmd->viewangles;
        }
	}

	return false;
}
