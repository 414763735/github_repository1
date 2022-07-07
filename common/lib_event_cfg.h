#ifndef _LIB_EVENT_CFG_H_
#define _LIB_EVENT_CFG_H_

typedef enum
{
    LibEvent_ID__NONE = 0,
    
    LibEvent_ID__AmplifierStatusUpdate,
    
    LibEvent_ID__SysVolumeUpdate,
    LibEvent_ID__SysMuteSwitch,
    LibEvent_ID__SysMuteON,
    LibEvent_ID__SysMuteOFF,
    
    LibEvent_ID__NosignalMute,
    LibEvent_ID__NosignalMuteRelease,    
    LibEvent_ID__TemporaryMute,
    LibEvent_ID__TemporaryMuteRelease,
    
    LibEvent_ID__PrintVideoInfo,
    LibEvent_ID__HdmiRxSigDetected,
    LibEvent_ID__HdmiRxSigLost,
    LibEvent_ID__OsdNetworkUpdate,
    
    LibEvent_ID__SetHdmiCecOn,
    LibEvent_ID__SetHdmiCecOff,
    
    LibEvent_ID__LightUpStandbyLed,
    LibEvent_ID__LightUpWorkingLed,
    
    LibEvent_ID__BTPairing,
    
    LibEvent_ID__FactoryReset,
    LibEvent_ID__DelayPowerUp,
    LibEvent_ID__LibreReboot,
    LibEvent_ID__Dwam83Reboot,

		LibEvent_ID__VersionDisplay,
		LibEvent_ID__WriteEq,
		LibEvent_ID__SetLedIntensity,
		LibEvent_ID__SetCrossoveer,
    
}LibEventId_t;

#endif

