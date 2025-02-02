#include "hooks.h"

#include "../interfaces.h"
#include "../settings.h"
#include "../Utils/draw.h"
#include "../fonts.h"

#include "../Hacks/visual/esp.h"
#include "../Hacks/visual/dlights.h"
#include "../Hacks/misc/grenadehelper.h"
#include "../Hacks/visual/grenadeprediction.h"
#include "../Hacks/visual/recoilcrosshair.h"
#include "../Hacks/misc/hitmarkers.h"
#include "../Hacks/misc/watermark.h"
#include "../Hacks/visual/indicators.h"
#include "../Hacks/misc/eventlog.h"
#include "../Hacks/visual/angleindicator.h"

// #include <mutex>

extern StartDrawingFn StartDrawing;
extern FinishDrawingFn FinishDrawing;

int Paint::engineWidth;
int Paint::engineHeight;
int Paint::windowWidth;
int Paint::windowHeight;

// std::mutex drawMutex;


typedef void (*PaintFn) (void*, PaintMode_t);

void Hooks::Paint(void* thisptr, PaintMode_t mode)
{
    engineVGuiVMT->GetOriginalMethod<PaintFn>(15)(thisptr, mode);

    engine->GetScreenSize(Paint::engineWidth,Paint::engineHeight );

	if (Settings::ScreenshotCleaner::enabled && engine->IsTakingScreenshot())
		return;

	if (mode & PAINT_UIPANELS)
	{
        std::unique_lock<std::mutex> lock( Draw::m_draw );
        int prevRecords = Draw::drawRequests.size(); // # of requests from last call
	lock.unlock();

        ESP::PaintToUpdateMatrix(); // Just for updating the viewMatrix
        /* These functions make drawRequests */
        Dlights::Paint();
        ESP::Paint();
        Watermark::Paint();
        GrenadeHelper::Paint();
        Recoilcrosshair::Paint();
        Hitmarkers::Paint();
        Indicators::Paint();
        Eventlog::Paint();
        AngleIndicator::Paint();
        GrenadePrediction::Paint();

        if( Settings::ESP::backend == DrawingBackend::SURFACE ){
            StartDrawing(surface);
	    lock.lock();
            for( const DrawRequest &value : Draw::drawRequests ){
                switch( value.type ){
                    case DRAW_LINE:
                        Draw::Line( value.x0, value.y0, value.x1, value.y1, Color::FromImColor( value.color ) );
                        break;
                    case DRAW_RECT:
                        Draw::Rectangle( value.x0, value.y0, value.x1, value.y1, Color::FromImColor( value.color ) );
                        break;
                    case DRAW_RECT_FILLED:
                        Draw::FilledRectangle( value.x0, value.y0, value.x1, value.y1, Color::FromImColor( value.color ) );
                        break;
                    case DRAW_CIRCLE:
                        Draw::Circle( Vector2D( value.x0, value.y0 ), value.circleSegments, value.circleRadius, Color::FromImColor( value.color ) );
                        break;
                    case DRAW_CIRCLE_FILLED:
                        Draw::FilledCircle( Vector2D( value.x0, value.y0 ), value.circleSegments, value.circleRadius, Color::FromImColor( value.color ) );
                        break;
                    case DRAW_CIRCLE_3D:
                        Draw::Circle3D( value.pos, value.circleSegments, value.circleRadius, Color::FromImColor( value.color ) );
                        break;
                    case DRAW_TEXT:
                        Draw::Text( value.x0, value.y0, value.text, esp_font, Color::FromImColor( value.color ) );
                        break;
                }
            }
	    lock.unlock();
            FinishDrawing(surface);
        }
        // std::unique_lock<std::mutex> lock( drawMutex );
        lock.lock();
        Draw::drawRequests.erase( Draw::drawRequests.begin( ), Draw::drawRequests.begin( ) + prevRecords );
    }
}

void Hooks::PaintImGui()
{
	if( Settings::ESP::backend != DrawingBackend::IMGUI )
        return;

    // std::unique_lock<std::mutex> lock( drawMutex );

    float width = (float)Paint::engineWidth;
    float height = (float)Paint::engineHeight;
    float imWidth = Paint::windowWidth;
    float imHeight = Paint::windowHeight;
    
    std::unique_lock<std::mutex> lock( Draw::m_draw );
    for( const DrawRequest &value : Draw::drawRequests ){
        /* Convert in case there are stretched resolution users - DONT write to original struct! */
        int x0 = (int)((value.x0 / width) * imWidth);
        int y0 = (int)((value.y0 / height) * imHeight);
        int x1 = (int)((value.x1 / width) * imWidth);
        int y1 = (int)((value.y1 / height) * imHeight);

        switch( value.type ){
            case DRAW_LINE:
                Draw::ImLine( ImVec2( x0, y0 ), ImVec2( x1, y1 ), value.color );
                break;
            case DRAW_RECT:
                Draw::ImRect( x0, y0, x1, y1, value.color );
                break;
            case DRAW_RECT_FILLED:
                Draw::ImRectFilled( x0, y0, x1, y1, value.color );
                break;
            case DRAW_CIRCLE:
                Draw::ImCircle( ImVec2( x0, y0 ), value.color, value.circleRadius, value.circleSegments, value.thickness );
                break;
            case DRAW_CIRCLE_FILLED:
                Draw::ImCircleFilled( ImVec2( x0, y0 ), value.color, value.circleRadius, value.circleSegments );
                break;
            case DRAW_CIRCLE_3D:
                Draw::ImCircle3D( value.pos, value.circleSegments, value.circleRadius, value.color );
                break;
            case DRAW_TEXT:
                Draw::ImText( ImVec2( x0, y0 ), value.color, value.text, nullptr, 0.0f, nullptr, value.fontflags );
                break;
            default:
                cvar->ConsoleColorPrintf(ColorRGBA(0, 225, 0), "\nERROR: UNKNOWN PaintImGui CALL (%X)\n", value.type);
                break;
        }
    }
    lock.unlock();
}