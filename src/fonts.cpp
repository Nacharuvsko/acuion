#include "fonts.h"

#include "settings.h"
#include "interfaces.h"

HFont esp_font = 0;
HFont indicators_font = 0;

static HFont CreateFont( const char* fontName, int size, int flag ) {
	HFont newFont = surface->CreateFont();
	surface->SetFontGlyphSet( newFont, fontName, size, 0, 0, 0, flag );
	return newFont;
}

void Fonts::SetupFonts()
{
	esp_font = CreateFont(Settings::UI::Fonts::ESP::family,
								Settings::UI::Fonts::ESP::size,
								Settings::UI::Fonts::ESP::flags);

	indicators_font = CreateFont(Settings::UI::Fonts::Indicators::family,
								Settings::UI::Fonts::Indicators::size,
								Settings::UI::Fonts::Indicators::flags);
}
