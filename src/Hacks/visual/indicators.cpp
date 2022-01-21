//
// Created by winston on 19.01.2022.
//
#include "indicators.h"
#include "../aimbot/aimbot.h"
#include "../misc/slowwalk.hpp"

#include "../../fonts.h"
#include "../../interfaces.h"
#include "../../Hooks/hooks.h"
#include "../../Utils/draw.h"
#include "../../Utils/xorstring.h"

struct Comparator_t {
    bool operator()(Indicator_t const & a, Indicator_t const & b) const {
		return strlen(a.text) < strlen(b.text);
    }
};

void Indicators::Paint() {

    C_BasePlayer* localPlayer = ( C_BasePlayer* ) entityList->GetClientEntity( engine->GetLocalPlayer() );

    if( !Settings::Indicators::enabled || !engine->IsInGame())
		return;

    if ( !localPlayer || !localPlayer->GetAlive() )
		return;

    int s_posX = Settings::Indicators::posX; // position for le $keet indicators
    int s_posY = Settings::Indicators::posY;

    int width = Paint::engineWidth;
    int height = Paint::engineHeight;

    std::vector<Indicator_t> indicators = { };

	if (Settings::Indicators::fakeLag) { // Fake Lag
	    if (Settings::FakeLag::enabled) {
			indicators.push_back(Indicator_t(
				Color(0, 255, 0, 255),
				(("FL " + (Settings::FakeLag::adaptive ? "A" : std::to_string(Settings::FakeLag::value))))
			));
	    } else {
			indicators.push_back(Indicator_t(
				Color(255, 0, 0, 255),
				"FL"
			));
	    }
	}

	if (Settings::Indicators::trigger) {
	    if (Settings::Triggerbot::enabled) {
		if (inputSystem->IsButtonDown(Settings::Triggerbot::key)) {
		    indicators.push_back(Indicator_t(
			Color(0, 255, 0, 255),
			"TB"
		    ));
		} else {
		    indicators.push_back(Indicator_t(
			Color(255, 0, 0, 255),
			"TB"
		    ));
		}
	    }
	}

	if (Settings::Indicators::slowWalk) {
	    if (Settings::AntiAim::Slowwalk::enabled) {
		if (SlowWalk::SlowWalking) {
		    indicators.push_back(Indicator_t(
			Color(0, 255, 0, 255),
			"SW"));
		} else {
		    indicators.push_back(Indicator_t(
			Color(255, 0, 0, 255),
			"SW"));
		}
	    }
	}

    // superior sorting algorythm $$$
    std::sort(indicators.begin(), indicators.end(), Comparator_t());

    // here we draw our inidcators UwU

    int i = 0;
    switch (Settings::Indicators::indicatorsType)
    {
	case IndicatorsType::SKEET: {
	    for(Indicator_t ind : indicators) {
			Draw::Text(Vector2D(s_posX, s_posY - (i * 28)), ind.text, indicators_font_skeet, ind.color);
			i++;
	    }
	    break;
	}
	case IndicatorsType::CROSSHAIR: {
	    for(Indicator_t ind : indicators) {
			int indWidth;
			int indHeight;
			Draw::GetTextSize(ind.text, indicators_font_crosshair, indWidth, indHeight);
			Draw::Text(Vector2D((width / 2) - (indWidth / 2), (height / 2) + (i * 28)), ind.text, indicators_font_crosshair, ind.color);
			i++;
	    }
	    break;
	}
    }
}