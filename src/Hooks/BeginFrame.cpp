#include "hooks.h"

#include "../Utils/skins.h"
#include "../interfaces.h"

#include "../Hacks/misc/clantagchanger.h"
#include "../Hacks/misc/namechanger.h"
#include "../Hacks/misc/namestealer.h"
#include "../Hacks/misc/spammer.h"
#include "../Hacks/visual/radar.h"
#include "../Hacks/visual/disablepostprocessing.h"

typedef void (*BeginFrameFn) (void*, float);

void Hooks::BeginFrame(void* thisptr, float frameTime)
{
	ClanTagChanger::BeginFrame(frameTime);
	NameChanger::BeginFrame(frameTime);
	NameStealer::BeginFrame(frameTime);
	Spammer::BeginFrame(frameTime);
	Radar::BeginFrame();
	DisablePostProcessing::BeginFrame();

	if (!engine->IsInGame())
		CreateMove::sendPacket = true;

	Skins::Localize();

	return materialVMT->GetOriginalMethod<BeginFrameFn>(42)(thisptr, frameTime);
}
