#ifndef _LIB_SEQ_CFG_H_
#define _LIB_SEQ_CFG_H_

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/
#define LIB_SEQ_PRIOR__HIGH                            0
#define LIB_SEQ_PRIOR__LOW                             255

#define LIB_SEQ_PRIOR__SYS_LIB_EVT                     2
#define LIB_SEQ_PRIOR__LED_UI_POLLING                  3
#define LIB_SEQ_PRIOR__SYS_AUDIO_POLLING               4
#define LIB_SEQ_PRIOR__APP_HDMI_POLLING                5
#define LIB_SEQ_PRIOR__CEC_MAIN_POLLING                6
#define LIB_SEQ_PRIOR__APP_BT_POLLING                  6
#define LIB_SEQ_PRIOR__SYS_MAIN_POLLING                7 // must have higher priority 
#define LIB_SEQ_PRIOR__SYS_VOLUME_POLLING              8
#define LIB_SEQ_PRIOR__OSD_DISPLAY_POLLING             20

#define LIB_SEQ_PRIOR__VFD_DISPLAY_POLLING             4

#define LIB_SEQ_PRIOR__GPIO_POLLING                    4
#define LIB_SEQ_PRIOR__SYS_APP_EVT                     21

#define LIB_SEQ_PRIOR__APP_IR_POLLING                  22
#define LIB_SEQ_PRIOR__D83_TXRX_POLLING                23
#define LIB_SEQ_PRIOR__SYS_TUNER_POLLING               24


#define LIB_SEQ_PRIOR__APP_CEC_POLLING                 253

#define LIB_SEQ_PRIOR__SYS_POWER_POLLING               254
#define LIB_SEQ_PRIOR__APP_DELAY_EVT_POLLING           254

#define LIB_SEQ_PRIOR__SYS_TRIGGER_POLLING             255
#define LIB_SEQ_PRIOR__AMPLIFIER_POLLING               255
#define LIB_SEQ_PRIOR__APP_NVR_POLLING                 255
#define LIB_SEQ_PRIOR__CONSOLE_POLLING                 255
#define LIB_SEQ_PRIOR__LIB_NVR                         255
#define LIB_SEQ_PRIOR__DRV_CLK_DET                     255
#define LIB_SEQ_PRIOR__APP_CLOCK                       255
#define LIB_SEQ_PRIOR__APP_EVENT                       255

#endif /* LIB_SEQ_CFG_H */

