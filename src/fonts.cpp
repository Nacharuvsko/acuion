#include "fonts.h"

#include "settings.h"
#include "interfaces.h"

HFont esp_font = 0;
HFont indicators_font_skeet = 0;
HFont indicators_font_crosshair = 0;

static HFont CreateFont( const char* fontName, int size, int flag, int weight ) {
	HFont newFont = surface->CreateFont();
	surface->SetFontGlyphSet( newFont, fontName, size, weight, 0, 0, flag );
	return newFont;
}

static HFont CreateFont( const char* fontName, int size, int flag ) {
    return CreateFont( fontName, size, flag, 0 );
}

void Fonts::SetupFonts()
{
	esp_font = CreateFont(Settings::UI::Fonts::ESP::family,
								Settings::UI::Fonts::ESP::size,
								Settings::UI::Fonts::ESP::flags);

	indicators_font_skeet = CreateFont(Settings::UI::Fonts::Indicators::family_skeet,
								Settings::UI::Fonts::Indicators::size_skeet,
								Settings::UI::Fonts::Indicators::flags_skeet); // weight is actually changing nothing :c

	indicators_font_crosshair = CreateFont(Settings::UI::Fonts::Indicators::family_crosshair,
	    Settings::UI::Fonts::Indicators::size_crosshair,
	    Settings::UI::Fonts::Indicators::flags_crosshair); // weight is actually changing nothing :c
}

#pragma clang diagnostic pop