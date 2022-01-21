#include "hooks.h"

#include "../interfaces.h"

#include "../Hacks/visual/customglow.h"
#include "../Hacks/visual/chams.h"
#include "../Hacks/misc/skinchanger.h"
#include "../Hacks/visual/noflash.h"
#include "../Hacks/visual/view.h"
#include "../Hacks/aimbot/resolver.h"
#include "../Hacks/aimbot/lagcomp.h"
#include "../Hacks/visual/skybox.h"
#include "../Hacks/visual/asuswalls.h"
#include "../Hacks/visual/nosmoke.h"
#include "../Hacks/misc/thirdperson.h"

typedef void (*FrameStageNotifyFn) (void*, ClientFrameStage_t);

void Hooks::FrameStageNotify(void* thisptr, ClientFrameStage_t stage)
{
	Chams::FrameStageNotify(stage);
	CustomGlow::FrameStageNotify(stage);
	SkinChanger::FrameStageNotifyModels(stage);
	SkinChanger::FrameStageNotifySkins(stage);
	Noflash::FrameStageNotify(stage);
	View::FrameStageNotify(stage);
	Resolver::FrameStageNotify(stage);
	LagComp::FrameStageNotify(stage);
	SkyBox::FrameStageNotify(stage);
	ASUSWalls::FrameStageNotify(stage);
	NoSmoke::FrameStageNotify(stage);
	ThirdPerson::FrameStageNotify(stage);

	if (SkinChanger::forceFullUpdate)
	{
		GetLocalClient(-1)->m_nDeltaTick = -1;
		SkinChanger::forceFullUpdate = false;
	}

	clientVMT->GetOriginalMethod<FrameStageNotifyFn>(37)(thisptr, stage);

	Resolver::PostFrameStageNotify(stage);
	View::PostFrameStageNotify(stage);
}
