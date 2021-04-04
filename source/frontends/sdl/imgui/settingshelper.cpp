#include "StdAfx.h"
#include "CardManager.h"
#include "Registry.h"
#include "Harddisk.h"
#include "Core.h"

#include "frontends/sdl/imgui/settingshelper.h"

namespace
{
  const std::map<SS_CARDTYPE, std::string> cards =
    {
     {CT_Empty, "CT_Empty"},
     {CT_Disk2, "CT_Disk2"},
     {CT_SSC, "CT_SSC"},
     {CT_MockingboardC, "CT_MockingboardC"},
     {CT_GenericPrinter, "CT_GenericPrinter"},
     {CT_GenericHDD, "CT_GenericHDD"},
     {CT_GenericClock, "CT_GenericClock"},
     {CT_MouseInterface, "CT_MouseInterface"},
     {CT_Z80, "CT_Z80"},
     {CT_Phasor, "CT_Phasor"},
     {CT_Echo, "CT_Echo"},
     {CT_SAM, "CT_SAM"},
     {CT_80Col, "CT_80Col"},
     {CT_Extended80Col, "CT_Extended80Col"},
     {CT_RamWorksIII, "CT_RamWorksIII"},
     {CT_Uthernet, "CT_Uthernet"},
     {CT_LanguageCard, "CT_LanguageCard"},
     {CT_LanguageCardIIe, "CT_LanguageCardIIe"},
     {CT_Saturn128K, "CT_Saturn128K"},
    };

  const std::map<eApple2Type, std::string> apple2Types =
    {
     {A2TYPE_APPLE2, "A2TYPE_APPLE2"},
     {A2TYPE_APPLE2PLUS, "A2TYPE_APPLE2PLUS"},
     {A2TYPE_APPLE2JPLUS, "A2TYPE_APPLE2JPLUS"},
     {A2TYPE_APPLE2E, "A2TYPE_APPLE2E"},
     {A2TYPE_APPLE2EENHANCED, "A2TYPE_APPLE2EENHANCED"},
     {A2TYPE_PRAVETS8M, "A2TYPE_PRAVETS8M"},
     {A2TYPE_PRAVETS82, "A2TYPE_PRAVETS82"},
     {A2TYPE_BASE64A, "A2TYPE_BASE64A"},
     {A2TYPE_PRAVETS8A, "A2TYPE_PRAVETS8A"},
     {A2TYPE_TK30002E, "A2TYPE_TK30002E"},
    };

  const std::map<eCpuType, std::string> cpuTypes =
    {
     {CPU_6502, "CPU_6502"},
     {CPU_65C02, "CPU_65C02"},
     {CPU_Z80, "CPU_Z80"},
    };

  const std::map<AppMode_e, std::string> appModes =
    {
     {MODE_LOGO, "MODE_LOGO"},
     {MODE_PAUSED, "MODE_PAUSED"},
     {MODE_RUNNING, "MODE_RUNNING"},
     {MODE_DEBUG, "MODE_DEBUG"},
     {MODE_STEPPING, "MODE_STEPPING"},
     {MODE_BENCHMARK, "MODE_BENCHMARCK"},
    };

  const std::map<Disk_Status_e, std::string> statuses =
    {
      {DISK_STATUS_OFF, "OFF"},
      {DISK_STATUS_READ, "READ"},
      {DISK_STATUS_WRITE, "WRITE"},
      {DISK_STATUS_PROT, "PROT"},
    };

  const std::map<VideoType_e, std::string> videoTypes =
  {
    {VT_MONO_CUSTOM, "Monochrome (Custom)"},
    {VT_COLOR_IDEALIZED, "Color (Composite Idealized)"},
    {VT_COLOR_VIDEOCARD_RGB, "Color (RGB Card/Monitor)"},
    {VT_COLOR_MONITOR_NTSC, "Color (Composite Monitor)"},
    {VT_COLOR_TV, "Color TV"},
    {VT_MONO_TV, "B&W TV"},
    {VT_MONO_AMBER, "Monochrome (Amber)"},
    {VT_MONO_GREEN, "Monochrome (Green)"},
    {VT_MONO_WHITE, "Monochrome (White)"},
  };

  const std::map<size_t, std::vector<SS_CARDTYPE>> cardsForSlots =
    {
      {0, {CT_Empty, CT_LanguageCard, CT_Saturn128K}},
      {1, {CT_Empty, CT_GenericPrinter}},
      {2, {CT_Empty, CT_SSC}},
      {3, {CT_Empty, CT_Uthernet}},
      {4, {CT_Empty, CT_MockingboardC, CT_MouseInterface, CT_Phasor}},
      {5, {CT_Empty, CT_MockingboardC, CT_Z80, CT_SAM, CT_Disk2}},
      {6, {CT_Empty, CT_Disk2}},
      {7, {CT_Empty, CT_GenericHDD}},
    };

    const std::vector<SS_CARDTYPE> expansionCards =
      {CT_Empty, CT_LanguageCard, CT_Extended80Col, CT_Saturn128K, CT_RamWorksIII};
}

namespace sa2
{

  const std::string & getCardName(SS_CARDTYPE card)
  {
    return cards.at(card);
  }

  const std::string & getApple2Name(eApple2Type type)
  {
    return apple2Types.at(type);
  }

  const std::string & getCPUName(eCpuType cpu)
  {
    return cpuTypes.at(cpu);
  }

  const std::string & getAppModeName(AppMode_e mode)
  {
    return appModes.at(mode);
  }

  const std::string & getVideoTypeName(VideoType_e type)
  {
    return videoTypes.at(type);
  }

  const std::vector<SS_CARDTYPE> & getCardsForSlot(size_t slot)
  {
    return cardsForSlots.at(slot);
  }

  const std::vector<SS_CARDTYPE> & getExpansionCards()
  {
    return expansionCards;
  }

  const std::map<eApple2Type, std::string> & getAapple2Types()
  {
    return apple2Types;
  }

  const std::string & getDiskStatusName(Disk_Status_e status)
  {
    return statuses.at(status);
  }

  void insertCard(size_t slot, SS_CARDTYPE card)
  {
    CardManager & cardManager = GetCardMgr();

    switch (slot)
    {
      case 7:
      {
        const bool enabled = card == CT_GenericHDD;
        REGSAVE(REGVALUE_HDD_ENABLED, enabled);
        HD_SetEnabled(enabled);
      }
      default:
      {
        // we do not use REGVALUE_SLOT5 as they are not "runtime friendly"
        const std::string label = "Slot " + std::to_string(slot);
        REGSAVE(label.c_str(), (DWORD)card);
        cardManager.Insert(slot, card);
        break;
      }
    };
  }

  void setVideoStyle(Video & video, const VideoStyle_e style, const bool enabled)
  {
    VideoStyle_e currentVideoStyle = video.GetVideoStyle();
    if (enabled)
    {
      currentVideoStyle = VideoStyle_e(currentVideoStyle | style);
    }
    else
    {
      currentVideoStyle = VideoStyle_e(currentVideoStyle & (~style));
    }
    video.SetVideoStyle(currentVideoStyle);
  }

}