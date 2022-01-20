#include "hvhtab.h"


#include "../../interfaces.h"
#include "../../Utils/xorstring.h"
#include "../../settings.h"
#include "../../Hacks/valvedscheck.h"
#include "../atgui.h"
#include "../../ImGUI/imgui_internal.h"

#pragma GCC diagnostic ignored "-Wformat-security"

void HvH::RenderTab()
{
	static const char* yTypes[] = {
			"BACKWARDS", "FORWARDS",
	};
	
	static const char* xTypes[] = {
			"UP", "DOWN", "FRONT", // safe
	};
	
	static const char* fTypes[] = {
			"LEFT", "RIGHT", "JITTER", "MANUAL",
	    		"MEGALEAN" // untrusted
	};
	
	static const char* pTypes[] = {
			"DIY", "JITTER"
	};

    ImGui::Columns(2, nullptr, true);
    {
        ImGui::BeginChild(XORSTR("HVH1"), ImVec2(0, 0), true);
        {
        	ImGui::Columns(2, NULL, true);
        	{
				ImGui::Text(XORSTR("AntiAim"));
        	}
        	ImGui::NextColumn();
        	{
				ImGui::PushItemWidth(-1);
				ImGui::Combo(XORSTR("##PRESETTYPE"), (int*)& Settings::AntiAim::Preset::type, pTypes, IM_ARRAYSIZE(pTypes));

				ImGui::PopItemWidth();
			}
			ImGui::Columns(1);
			ImGui::BeginChild(XORSTR("##ANTIAIM"), ImVec2(0, 0), true);
			{
				if(Settings::AntiAim::Preset::type == AntiAimYaw_Preset::DIY)
				{
					ImGui::Checkbox(XORSTR("Yaw"), &Settings::AntiAim::Yaw::enabled);
					ImGui::Separator();
					ImGui::Columns(2, NULL, true);
					{
						ImGui::ItemSize(ImVec2(0.0f, 0.0f), 0.0f);
						ImGui::Text(XORSTR("Yaw Actual"));

					}
					ImGui::NextColumn();
					{
						ImGui::PushItemWidth(-1);

						ImGui::Combo(XORSTR("##YACTUALTYPE"), (int*)& Settings::AntiAim::Yaw::type, yTypes, IM_ARRAYSIZE(yTypes));

						ImGui::PopItemWidth();
					}
					ImGui::Columns(1);
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat(XORSTR("##REALOFFSET"), &Settings::AntiAim::Yaw::offset, 0, 360, "Yaw Offset: %0.f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);
					ImGui::Checkbox(XORSTR("Yaw"), &Settings::AntiAim::Yaw::enabled);
					ImGui::Separator();
					ImGui::Columns(2, NULL, true);
					{
						ImGui::ItemSize(ImVec2(0.0f, 0.0f), 0.0f);
						ImGui::Checkbox(XORSTR("add desync to yaw"), &Settings::AntiAim::Yaw::addDesyncEnabled);
						
					}
					ImGui::NextColumn();
					{
						ImGui::PushItemWidth(-1);
						
						ImGui::SliderFloat(XORSTR("##LBYOFFSET"), &Settings::AntiAim::Yaw::addDesyncOffset, -100, 100, "%0.f%%");
						
						ImGui::PopItemWidth();
					}
					ImGui::Separator();
					ImGui::Columns(1);
					ImGui::Checkbox(XORSTR("Pitch"), &Settings::AntiAim::Pitch::enabled);
					ImGui::Separator();
					ImGui::Columns(2, NULL, true);
					{
						ImGui::ItemSize(ImVec2(0.0f, 0.0f), 0.0f);
						ImGui::Text(XORSTR("Pitch Actual"));
					}
					ImGui::NextColumn();
					{
						ImGui::PushItemWidth(-1);
						ImGui::Combo(XORSTR("##XTYPE"), (int*)& Settings::AntiAim::Pitch::type, xTypes, IM_ARRAYSIZE(xTypes));
						ImGui::PopItemWidth();
					}
					ImGui::Columns(1);
					ImGui::Separator();
					ImGui::Columns(2, NULL, true);
					{
						ImGui::Checkbox(XORSTR("Desync Yaw"), &Settings::AntiAim::Fake::enabled);
					}
					ImGui::NextColumn();
					{
						ImGui::PushItemWidth(-1);
						ImGui::Checkbox(XORSTR("Desync fail fix (Bad for legit aa)"), &Settings::AntiAim::RageDesyncFix::enabled);
						ImGui::PopItemWidth();
					}

					ImGui::Separator();
					ImGui::Columns(2, NULL, true);
					{
						ImGui::ItemSize(ImVec2(0.0f, 0.0f), 0.0f);
						ImGui::Text(XORSTR("Desync"));
					}
					ImGui::NextColumn();
					{
						ImGui::PushItemWidth(-1);
						ImGui::Combo(XORSTR("##YAWFAKETYPE"), (int*)& Settings::AntiAim::Fake::type, fTypes, IM_ARRAYSIZE(fTypes));
						if (Settings::AntiAim::Fake::type == AntiAimYaw_Fake::MEGALEAN) {
						    ImGui::SliderFloat(XORSTR("Lean Length"), &Settings::AntiAim::Fake::maxLean, -45.f, 45.f, "Lean Length: %0.f");
						    ImGui::Checkbox(XORSTR("Lean Jerk"), &Settings::AntiAim::Fake::leanJerk);
						}
						ImGui::PopItemWidth();
					}
				}
                ImGui::Columns(1);
                ImGui::Separator();
                ImGui::Text(XORSTR("Disable"));
                ImGui::Separator();
                ImGui::Checkbox(XORSTR("Knife"), &Settings::AntiAim::AutoDisable::knifeHeld);
                ImGui::Checkbox(XORSTR("No Enemy"), &Settings::AntiAim::AutoDisable::noEnemy);

                ImGui::Columns(1);
                ImGui::Separator();
                ImGui::Text(XORSTR("Edging"));
                ImGui::Separator();
                ImGui::Columns(1);
                {
                    ImGui::Checkbox(XORSTR("Enabled"), &Settings::AntiAim::HeadEdge::enabled);
                }
                ImGui::Columns(1);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(210, 85));
                if (ImGui::BeginPopupModal(XORSTR("Error###UNTRUSTED_AA")))
                {
                    ImGui::Text(XORSTR("You cannot use this antiaim type on a VALVE server."));

                    ImGui::Checkbox(XORSTR("This is not a VALVE server"), &ValveDSCheck::forceUT);

                    if (ImGui::Button(XORSTR("OK")))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();

                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
    }
    ImGui::NextColumn();
    {
        ImGui::BeginChild(XORSTR("HVH2"), ImVec2(0, 0), true);
        {
            ImGui::Text(XORSTR("Resolver"));

            ImGui::Separator();
            ImGui::Checkbox(XORSTR("Resolve All"), &Settings::Resolver::resolveAll);
            ImGui::Separator();

            ImGui::Text(XORSTR("Movement"));
            ImGui::Checkbox(XORSTR("Auto Crouch"), &Settings::Aimbot::AutoCrouch::enabled);
            ImGui::Separator();

	    ImGui::Text(XORSTR("Slowwalk"));
	    ImGui::Checkbox(XORSTR("Slowwalk"), &Settings::AntiAim::Slowwalk::enabled);
	    ImGui::SliderFloat(XORSTR("##SWSPEED"), &Settings::AntiAim::Slowwalk::Speed, 0, 100, "Slowwalk Speed: %0.f");
	    UI::KeyBindButton(&Settings::AntiAim::Slowwalk::Key);
	    ImGui::Separator();

            ImGui::Checkbox(XORSTR("Angle Indicator"), &Settings::AngleIndicator::enabled);

            ImGui::Columns(2, NULL, true);
				{
		            ImGui::Checkbox(XORSTR("LBY Breaker"), &Settings::AntiAim::LBYBreaker::enabled);
				}
				ImGui::NextColumn();
				{
					ImGui::PushItemWidth(-1);
					ImGui::Checkbox(XORSTR("Manual?"), &Settings::AntiAim::LBYBreaker::manual);
					ImGui::PopItemWidth();
				}
			ImGui::Columns(1);

            if( Settings::AntiAim::LBYBreaker::enabled ){
                ImGui::SliderFloat(XORSTR("##LBYOFFSET"), &Settings::AntiAim::LBYBreaker::offset, 0, 360, "LBY Offset(from fake): %0.f");
            }
            ImGui::EndChild();
        }
    }
}
