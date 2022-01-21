//
// Created by winston on 21.01.2022.
//
#include "usespammer.h"

#include "../../settings.h"
#include "../../interfaces.h"

void UseSpammer::CreateMove(CUserCmd* cmd) {

    useSpamming = false; // for future things, don't remove

    if (!Settings::UseSpammer::enabled) {
	return;
    }

    C_BasePlayer *localplayer = (C_BasePlayer *) entityList->GetClientEntity(engine->GetLocalPlayer());

    if (!localplayer || !localplayer->GetAlive()) {
	return;
    }

    if (!inputSystem->IsButtonDown(Settings::UseSpammer::key)) {
	return;
    }

    if (cmd->tick_count % 2 != 0) { // a delay
	return;
    }

    useSpamming = true;

    cmd->buttons |= (1 << 5);

}