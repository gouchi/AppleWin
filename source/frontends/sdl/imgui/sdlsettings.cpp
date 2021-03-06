#include "StdAfx.h"
#include "frontends/sdl/imgui/sdlsettings.h"
#include "frontends/sdl/imgui/settingshelper.h"

#include "Interface.h"
#include "CardManager.h"
#include "Core.h"
#include "CPU.h"
#include "CardManager.h"
#include "Speaker.h"
#include "Mockingboard.h"
#include "Registry.h"

namespace
{

  void HelpMarker(const char* desc)
  {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  }

}

namespace sa2
{

  void ImGuiSettings::showSettings()
  {
    if (ImGui::Begin("Settings", &myShowSettings))
    {
      ImGuiIO& io = ImGui::GetIO();

      if (ImGui::BeginTabBar("Settings"))
      {
        if (ImGui::BeginTabItem("General"))
        {
          ImGui::Checkbox("Apple Video windowed", &windowed);
          ImGui::SameLine(); HelpMarker("Show Apple Video in a separate window.");

          ImGui::Checkbox("Show Demo", &myShowDemo);
          ImGui::SameLine(); HelpMarker("Show Dear ImGui DemoWindow.");

          ImGui::Text("FPS: %d", int(io.Framerate));
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Hardware"))
        {
          if (ImGui::BeginTable("Cards", 2, ImGuiTableFlags_RowBg))
          {
            CardManager & manager = GetCardMgr();
            ImGui::TableSetupColumn("Slot");
            ImGui::TableSetupColumn("Card");
            ImGui::TableHeadersRow();

            for (size_t slot = 0; slot < 8; ++slot)
            {
              ImGui::TableNextColumn();
              ImGui::Selectable(std::to_string(slot).c_str());
              ImGui::TableNextColumn();
              const SS_CARDTYPE card = manager.QuerySlot(slot);;
              ImGui::Selectable(getCardName(card).c_str());
            }
            ImGui::TableNextColumn();
            ImGui::Selectable("AUX");
            ImGui::TableNextColumn();
            const SS_CARDTYPE card = manager.QueryAux();
            ImGui::Selectable(getCardName(card).c_str());

            ImGui::EndTable();
          }
          ImGui::Separator();

          if (ImGui::BeginTable("Type", 2, ImGuiTableFlags_RowBg))
          {
            ImGui::TableNextColumn();
            ImGui::Selectable("Apple 2");
            ImGui::TableNextColumn();
            const eApple2Type a2e = GetApple2Type();
            ImGui::Selectable(getApple2Name(a2e).c_str());

            ImGui::TableNextColumn();
            ImGui::Selectable("CPU");
            ImGui::TableNextColumn();
            const eCpuType cpu = GetMainCpu();
            ImGui::Selectable(getCPUName(cpu).c_str());

            ImGui::EndTable();
          }

          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Audio"))
        {
          const int volumeMax = GetPropertySheet().GetVolumeMax();
          if (ImGui::SliderInt("Speaker volume", &mySpeakerVolume, 0, volumeMax))
          {
            SpkrSetVolume(volumeMax - mySpeakerVolume, volumeMax);
            REGSAVE(TEXT(REGVALUE_SPKR_VOLUME), SpkrGetVolume());
          }

          if (ImGui::SliderInt("Mockingboard volume", &myMockingboardVolume, 0, volumeMax))
          {
            MB_SetVolume(volumeMax - myMockingboardVolume, volumeMax);
            REGSAVE(TEXT(REGVALUE_MB_VOLUME), MB_GetVolume());
          }

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

    }
    ImGui::End();
  }

  void ImGuiSettings::show()
  {
    if (myShowSettings)
    {
      showSettings();
    }

    if (myShowDemo)
    {
      ImGui::ShowDemoWindow(&myShowDemo);
    }
  }

  float ImGuiSettings::drawMenuBar()
  {
    float menuBarHeight;

    if (ImGui::BeginMainMenuBar())
    {
      menuBarHeight = ImGui::GetWindowHeight();
      if (ImGui::BeginMenu("System"))
      {
        ImGui::MenuItem("Settings", nullptr, &myShowSettings);
        ImGui::MenuItem("Demo", nullptr, &myShowDemo);
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
    else
    {
      menuBarHeight = 0.0;
    }

    return menuBarHeight;
  }

}
