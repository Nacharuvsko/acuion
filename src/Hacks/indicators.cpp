//
// Created by winston on 19.01.2022.
//
#include "indicators.h"
#include "aimbot.h"
#include "autowall.h"

#include "../interfaces.h"
#include "../Utils/xorstring.h"
#include "../fonts.h"
#include "../Utils/draw.h"
#include "../SDK/color.h"

void Indicators::Paint() {

    C_BasePlayer* localPlayer = ( C_BasePlayer* ) entityList->GetClientEntity( engine->GetLocalPlayer() );

    if( !Settings::Indicators::enabled || !engine->IsInGame())
	return;

    if ( !localPlayer || !localPlayer->GetAlive() )
	return;

    std::vector<Indicator_t> indicators = { };
    if (Settings::Indicators::aWall)
    {
		if (Settings::Aimbot::AutoWall::enabled) {
			
		}
    }
	Draw::Text(100, 100, "TEST", indicators_font, Color(255, 255, 0, 255));
}