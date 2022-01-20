#include "fonts.h"

#include "settings.h"
#include "interfaces.h"

HFont esp_font = 0;
HFont indicators_font = 0;

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

	indicators_font = CreateFont(Settings::UI::Fonts::Indicators::family,
								Settings::UI::Fonts::Indicators::size,
								Settings::UI::Fonts::Indicators::flags); // weight is actually changing nothing :c
}
