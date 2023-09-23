#define RWNX_VERS_REV "241c091M (master)"
#define RWNX_VERS_MOD "20220606-005-6.4.3.0"
#define RWNX_VERS_BANNER "rwnx " RWNX_VERS_MOD " - - " RWNX_VERS_REV

#if defined(AICWF_SDIO_SUPPORT)
#define DRV_TYPE_NAME   "compatible(sdio)"
#elif defined(AICWF_USB_SUPPORT)
#define DRV_TYPE_NAME   "compatible(usb)"
#else
#define DRV_TYPE_NAME   "compatible(unknow)"
#endif

#define DRV_RELEASE_TAG "aic-rwnx-" DRV_TYPE_NAME "-" RWNX_VERS_MOD
