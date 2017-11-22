/*********************************************************************************
*
*  Copyright (C) 2014 Hisilicon Technologies Co., Ltd.  All rights reserved. 
*
*  This program is confidential and proprietary to Hisilicon Technologies Co., Ltd.
*  (Hisilicon), and may not be copied, reproduced, modified, disclosed to
*  others, published or used, in whole or in part, without the express prior
*  written permission of Hisilicon.
*
***********************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/time.h>
#include "hi_osal.h"
#include "drv_struct_ext.h"

#include "hi_debug.h"
#include "mpi_log.h"
#include "hi_mpi_mem.h"


static HI_U32 g_LogModInit = 0;
static HI_S32 g_s32DbgDevFd = -1;
static LOG_CONFIG_INFO_S *g_pLogConfigInfo;  /*the pointer of print control info*//*CNcomment:��ӡ������Ϣ���û�ָ̬��*/
//static HI_U32 g_u32LogInfoPhyAddr = 0;

/** Map the debug id to mod id*/
typedef struct hiID_MAP_S
{
    HI_U32 u32DebugID;      /**< DebugID from X5HD *//**<CNcomment:Debug ID ��X5HDһ��*/
    HI_U32 u32ModID;          /**< ModID from V1R1 *//**<CNcomment: ModID��V1R1һ��*/
} ID_MAP_S;

static ID_MAP_S g_u32DebugIdModIdMap[HI_DEBUG_ID_BUTT] = {
    {HI_DEBUG_ID_COMMON, HI_ID_SYS}, {HI_DEBUG_ID_DEMUX, HI_ID_DEMUX}, 
    {HI_DEBUG_ID_ADEC, HI_ID_ADEC}, {HI_DEBUG_ID_AENC, HI_ID_AENC}, {HI_DEBUG_ID_AO, HI_ID_AO}, 
    {HI_DEBUG_ID_AI, HI_ID_AI}, {HI_DEBUG_ID_VDEC, HI_ID_VDEC}, {HI_DEBUG_ID_VENC, HI_ID_VENC}, 
    {HI_DEBUG_ID_VO, HI_ID_VO}, {HI_DEBUG_ID_VI, HI_ID_VI}, {HI_DEBUG_ID_DISP, HI_ID_DISP},
    {HI_DEBUG_ID_PNG, HI_ID_PNG}, {HI_DEBUG_ID_FB, HI_ID_FB}, {HI_DEBUG_ID_FIRMWARE, HI_ID_VFMW}, 
    {HI_DEBUG_ID_VP, HI_ID_VP}, {HI_DEBUG_ID_AVPLAY, HI_ID_AVPLAY}, {HI_DEBUG_ID_PVR, HI_ID_PVR},
    {HI_DEBUG_ID_SYNC, HI_ID_SYNC}, {HI_DEBUG_ID_VSYNC, HI_ID_VSYNC}, {HI_DEBUG_ID_ASYNC, HI_ID_ASYNC}, 
    {HI_DEBUG_ID_SIO, HI_ID_SIO}, {HI_DEBUG_ID_SIO_AI, HI_ID_SIO_AI}, {HI_DEBUG_ID_SIO_AO, HI_ID_SIO_AO}, 
    {HI_DEBUG_ID_HDMI, HI_ID_HDMI}, {HI_DEBUG_ID_ECS, HI_ID_I2C}, {HI_DEBUG_ID_TUNER, HI_ID_TUNER}, 
    {HI_DEBUG_ID_CA, HI_ID_CA}, {HI_DEBUG_ID_C51, HI_ID_PMOC}, {HME_LOG_API, HI_ID_VOIP_HME}, 
    {HME_LOG_ENGINE, HI_ID_VOIP_HME}, {HI_DEBUG_ID_DEBUG, HI_ID_SYS}, {HI_DEBUG_ID_USR, HI_ID_SYS}, 
    {HI_DEBUG_ID_SUBT, HI_ID_SUBT}, {HI_DEBUG_ID_TTX, HI_ID_TTX}, {HI_DEBUG_ID_MMZ,HI_ID_SYS}, 
    {HI_DEBUG_ID_CC, HI_ID_CC}, {HI_DEBUG_ID_BUTT, HI_ID_BUTT}
};

char *DebugLevelName[HI_LOG_LEVEL_BUTT+1] = {
    "FATAL",
    "ERROR" ,
    "WARN",
    "INFO",
    "DEBUG",
    "BUTT"
};

static pthread_mutex_t   s_LogMutex = PTHREAD_MUTEX_INITIALIZER;

#define HI_LOG_LOCK()     (void)pthread_mutex_lock(&s_LogMutex)
#define HI_LOG_UNLOCK()   (void)pthread_mutex_unlock(&s_LogMutex)

static HI_S32 MPILogDeviceOpen(HI_CHAR  *pathname,const HI_S32 Flags)
{
    HI_S32 DbgDevFd;
    struct stat st;

    DbgDevFd  = open(pathname,Flags,0);
    if(-1 == DbgDevFd)
    {
        return HI_FAILURE;
    }

    if (-1 == fstat(DbgDevFd, &st))
    {
        fprintf(stderr, "Cannot identiy '%s'.\n", pathname);
        close(DbgDevFd);
        return HI_FAILURE;
    }

    if (!S_ISCHR (st.st_mode))
    {
        fprintf(stderr, "%s is no device\n", pathname);
        close(DbgDevFd);
        return HI_FAILURE;
    }

    return DbgDevFd;
}

static HI_S32 MPILogDeviceClose(HI_S32 FileHandle)
{
    HI_S32 ret;

    ret = close(FileHandle);
    if(HI_SUCCESS !=ret)
    {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
HI_S32 MPILogBufferRead(HI_U8 *pBuf,  HI_U32 BufLen, HI_U32 *pCopyLen)
{
    HI_S32 ret;
    LOG_BUF_READ_S LogRead;

    if(g_s32DbgDevFd == -1)
    {
        HI_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    LogRead.pMsgAddr = pBuf;
    LogRead.BufLen = BufLen;
    LogRead.CopyedLen = 0;
    ret = ioctl(g_s32DbgDevFd, UMAP_CMPI_LOG_READ_LOG, &LogRead);
    if(ret != HI_SUCCESS)
    {
        HI_ERR_LOG("%s ERROR: ioctl UMAP_CMPI_LOG_READ_LOG error!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    *pCopyLen = LogRead.CopyedLen;
    return HI_SUCCESS;
}

HI_S32 MPILogBufferWrite(HI_U8 *Buf,  HI_U32 MsgLen)
{
    HI_S32 ret;
    LOG_BUF_WRITE_S LogWrite;

    if(g_s32DbgDevFd == -1)
    {
        HI_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    LogWrite.pMsgAddr = Buf;
    LogWrite.MsgLen = MsgLen;
    ret = ioctl(g_s32DbgDevFd, UMAP_CMPI_LOG_WRITE_LOG, &LogWrite);
    if(ret != HI_SUCCESS)
    {
        HI_ERR_LOG("%s ERROR: ioctl UMAP_CMPI_LOG_WRITE_LOG error!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}

HI_S32 LOGPrintToBuf(const char *format, ...)
{
    char    log_str[LOG_MAX_TRACE_LEN] = {0};
    HI_S32  MsgLen;
    va_list args;

    if(0 == g_LogModInit)
    {
        HI_ERR_LOG("log device not init(usr)!\n");
        return HI_FAILURE;
    }

    va_start(args, format);
    MsgLen = HI_OSAL_Vsnprintf(log_str, LOG_MAX_TRACE_LEN-1, format, args);
    va_end(args);

    if (MsgLen >= (LOG_MAX_TRACE_LEN-1))
    {
        log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
        log_str[LOG_MAX_TRACE_LEN-2] = '\n';
        log_str[LOG_MAX_TRACE_LEN-3] = '.';
        log_str[LOG_MAX_TRACE_LEN-4] = '.';
        log_str[LOG_MAX_TRACE_LEN-5] = '.';
    }
    
    if ((MsgLen > 0 ) && (MsgLen < LOG_MAX_TRACE_LEN))
    {
        return MPILogBufferWrite((HI_U8 *)log_str, (HI_U32)(MsgLen));
    }
    else
    {
        return HI_FAILURE;
    }
}
#endif

/* HI_TRUE: need print
*  HI_FALSE: not print
*/
HI_BOOL LOGLevelChk(HI_S32 s32Levle, HI_U32 u32ModId, HI_BOOL* pUsrModeIDFlag)
{
    if (u32ModId < LOG_CONFIG_BUF_SIZE/sizeof(LOG_CONFIG_INFO_S))
    {
        /* log module has Initialized yet */
        if (g_LogModInit && s32Levle <= g_pLogConfigInfo[u32ModId].u8LogLevel)
        {
            return HI_TRUE;
        }

        /* log module has not Initialized yet */
        if(!g_LogModInit && s32Levle <= HI_LOG_LEVEL_DEFAULT)
        {
            return HI_TRUE;
        }
        
    }

    return HI_FALSE;
}

/*
  0 serial port
  1 network
*/
HI_U32 LOGPrintPosGet(HI_U32 u32ModId)
{
    HI_U32 u32Pos = 0;

    if (u32ModId >= LOG_CONFIG_BUF_SIZE/sizeof(LOG_CONFIG_INFO_S))
    {
        return LOG_OUTPUT_SERIAL;
    }
   
    if(0 == g_LogModInit)
    {        
        return LOG_OUTPUT_SERIAL;
    }

    if (g_pLogConfigInfo->u8UdiskFlag == 1)
    {
        return LOG_OUTPUT_UDISK;
    }

    u32Pos = g_pLogConfigInfo[u32ModId].u8LogPrintPos;

    return u32Pos;
}

inline HI_U32 LOGGetTimeMs(HI_VOID)
{
    struct timeval tv;
    (HI_VOID)gettimeofday(&tv , NULL);
    return (((HI_U32)tv.tv_sec)*1000 + ((HI_U32)tv.tv_usec)/1000);
}



HI_VOID HI_LogOut(HI_U32 u32Level, HI_MOD_ID_E enModId, HI_U8 *pFuncName, HI_U32 u32LineNum, const char *format, ...)
{
    HI_U32 TimeMs = 0;
    char  log_str[LOG_MAX_TRACE_LEN];
    HI_S32  MsgLen = 0;
    HI_BOOL bLevel = HI_FALSE;
    va_list args = {0};
    HI_BOOL bUsrModeFlag = HI_FALSE;
 
    HI_LOG_LOCK();

    bLevel = LOGLevelChk((HI_S32)u32Level, (HI_U32)enModId, &bUsrModeFlag);
    if (bLevel)
    {

        va_start(args, format);
        MsgLen = HI_OSAL_Vsnprintf(log_str, LOG_MAX_TRACE_LEN, format, args);
        va_end(args);

        if (MsgLen >= LOG_MAX_TRACE_LEN)
        {
            log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
            log_str[LOG_MAX_TRACE_LEN-2] = '\n';
            log_str[LOG_MAX_TRACE_LEN-3] = '.';
            log_str[LOG_MAX_TRACE_LEN-4] = '.';
            log_str[LOG_MAX_TRACE_LEN-5] = '.';
        }

        /* log module has Initialized. */
        if (g_LogModInit)
        {
            /*get the time, unit ms*/           
            TimeMs = LOGGetTimeMs();

            if (HI_FALSE == bUsrModeFlag)
            {
                HI_S32 nPos = (HI_S32)LOGPrintPosGet((HI_U32)enModId);

                switch(nPos)
                {
                    case LOG_OUTPUT_SERIAL://output to serial port
                    default:
                        HI_PRINT(PRINT_FMT);
                        break;
                    case LOG_OUTPUT_NETWORK://output to network
                    case LOG_OUTPUT_UDISK:
#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
                        LOGPrintToBuf(PRINT_FMT);
#endif
                        break;
                }
            }
            else
            {
                HI_U8* pModuleName = (HI_U8*)"ABC";//HI_ModuleMGR_GetModuleName(enModId);
                HI_PRINT ("<\33[32m%s\33[0m>:[%s - %d]:%s\n", pModuleName, pFuncName, u32LineNum, log_str);
            }

            HI_LOG_UNLOCK();
            
            return;
        }
        else /* log module has not Initialized. */
        {
             if (bUsrModeFlag == HI_FALSE)
            {
                 HI_PRINT("[%s-Unknow]: %s[%d]:%s", DebugLevelName[u32Level],  pFuncName,
                            u32LineNum, log_str);
            }

            HI_LOG_UNLOCK();
            
            return;
        }
    }

    HI_LOG_UNLOCK();

    return;
}

HI_S32 HI_MPI_LogInit(HI_VOID)
{
    HI_S32 ret;
    HI_S8 pathname[60];
    HI_U32 PhyAddr;
    HI_U32 *VirAddr;

    HI_LOG_LOCK();

    if(-1 == g_s32DbgDevFd)
    {
        HI_OSAL_Snprintf((char*)pathname, sizeof(pathname), "/dev/%s", UMAP_DEVNAME_LOG);
        g_s32DbgDevFd = MPILogDeviceOpen((char*)pathname,O_RDWR|O_NONBLOCK);
        if(HI_FAILURE ==  g_s32DbgDevFd)
        {
            HI_LOG_UNLOCK();
            
            HI_ERR_LOG("mpi log device open failed\n");
            return HI_FAILURE;
        }

        ret = ioctl(g_s32DbgDevFd, UMAP_CMPI_LOG_INIT, &PhyAddr);
        if (ret != 0)
        {
            (HI_VOID)MPILogDeviceClose(g_s32DbgDevFd);
            HI_LOG_UNLOCK();
            
            HI_ERR_LOG("log init ioctl failed\n");           
            
            return HI_FAILURE;
        }

        VirAddr = (HI_U32*)HI_MMAP(PhyAddr, LOG_CONFIG_BUF_SIZE);
        if(NULL ==  VirAddr)
        {
            (HI_VOID)MPILogDeviceClose(g_s32DbgDevFd);

            HI_LOG_UNLOCK();
            
            HI_ERR_LOG("HI_MPI_MMZ_Map failed\n");
            
            return HI_FAILURE;
        }

        g_pLogConfigInfo = (LOG_CONFIG_INFO_S *)VirAddr;
        g_LogModInit = 1;
    }

    HI_LOG_UNLOCK();

    HI_INFO_LOG("mpi log init OK\n");

    return HI_SUCCESS;
 }

HI_VOID HI_MPI_LogDeInit(HI_VOID)
{
    HI_LOG_LOCK();
    
    if(-1 != g_s32DbgDevFd)
    {
        g_LogModInit = 0;
        if (HI_SUCCESS != HI_MUNMAP((void*)g_pLogConfigInfo))
        {
            HI_LOG_UNLOCK();
            HI_WARN_LOG("log info umap failed\n");
            HI_LOG_LOCK();
        }
        (HI_VOID)ioctl(g_s32DbgDevFd, UMAP_CMPI_LOG_EXIT);
        (HI_VOID)MPILogDeviceClose(g_s32DbgDevFd);

        g_s32DbgDevFd = -1;
    }

    HI_LOG_UNLOCK();
}

HI_S32 HI_MPI_LogRead(HI_U8 *pBuf,  HI_U32 BufLen, HI_U32 *pCopyLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    HI_LOG_LOCK();
    
    if(0 == g_LogModInit)
    {
        HI_LOG_UNLOCK();
        
        HI_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    HI_LOG_UNLOCK();
    
#if defined(LOG_NETWORK_SUPPORT) || defined(LOG_UDISK_SUPPORT)
    s32Ret = MPILogBufferRead(pBuf, BufLen, pCopyLen);
#endif

    return s32Ret;
}

HI_S32 HI_MPI_LogLevelSet(HI_U32 u32ModId,  HI_LOG_LEVEL_E enLogLevel)
{
    if ((u32ModId >=  HI_DEBUG_ID_BUTT) || (enLogLevel >= HI_LOG_LEVEL_BUTT))
    {
        return HI_FAILURE;
    }
   
    HI_LOG_LOCK();
    
    if(0 == g_LogModInit)
    {
        HI_LOG_UNLOCK();
        
        HI_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    if (u32ModId == HI_DEBUG_ID_ECS)
    {
	 g_pLogConfigInfo[HI_ID_I2C].u8LogLevel = (HI_U8)enLogLevel;
	 g_pLogConfigInfo[HI_ID_SCI].u8LogLevel = (HI_U8)enLogLevel;
	 g_pLogConfigInfo[HI_ID_IR].u8LogLevel      = (HI_U8)enLogLevel;
	 g_pLogConfigInfo[HI_ID_RTC].u8LogLevel   = (HI_U8)enLogLevel;
	 g_pLogConfigInfo[HI_ID_WDG].u8LogLevel  = (HI_U8)enLogLevel;
	 g_pLogConfigInfo[HI_ID_GPIO].u8LogLevel =  (HI_U8)enLogLevel;
	 g_pLogConfigInfo[HI_ID_KEYLED].u8LogLevel =  (HI_U8)enLogLevel;
    }
    else
    {
    	g_pLogConfigInfo[g_u32DebugIdModIdMap[u32ModId].u32ModID].u8LogLevel = (HI_U8)enLogLevel;
    }
    HI_LOG_UNLOCK();

    return HI_SUCCESS;
}

/** Sets the log level of a module by Mod ID */
HI_S32 HI_MPI_LogLevelSetByModId(HI_U32 u32ModId,  HI_LOG_LEVEL_E enLogLevel)
{
    if ((u32ModId >= LOG_CONFIG_BUF_SIZE/sizeof(LOG_CONFIG_INFO_S)) || (enLogLevel >= HI_LOG_LEVEL_BUTT))
    {
        return HI_FAILURE;
    }

    HI_LOG_LOCK();
    
    if(0 == g_LogModInit)
    {
        HI_LOG_UNLOCK();
        
        HI_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return HI_FAILURE;
    }
    g_pLogConfigInfo[u32ModId].u8LogLevel = (HI_U8)enLogLevel;

    HI_LOG_UNLOCK();

    return HI_SUCCESS;
}

HI_S32 HI_MPI_LogPrintPosSet(HI_U32 u32ModId, LOG_OUTPUT_POS_E enPrintPos)
{
    if (u32ModId >= LOG_CONFIG_BUF_SIZE/sizeof(LOG_CONFIG_INFO_S))
    {
        return HI_FAILURE;
    }

    HI_LOG_LOCK();
    
    if(0 == g_LogModInit)
    {
        HI_LOG_UNLOCK();
        
        HI_ERR_LOG("%s ERROR: device not opened!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    if (enPrintPos != LOG_OUTPUT_BUTT)
    {
        g_pLogConfigInfo[u32ModId].u8LogPrintPos = (HI_U8)enPrintPos;
    }
    else
    {
        g_pLogConfigInfo[u32ModId].u8LogPrintPos = LOG_OUTPUT_POS_DEFAULT;
    }

    HI_LOG_UNLOCK();

    return HI_SUCCESS;
}


HI_S32 HI_MPI_LogPathSet(const HI_CHAR* pszLogPath)
{
    HI_S32 s32Ret = HI_FAILURE;
    LOG_PATH_S stLogPath;

    HI_LOG_LOCK();

    if (pszLogPath == NULL || g_s32DbgDevFd == -1)
    {
        HI_LOG_UNLOCK();

        HI_ERR_LOG(" params invalid or log non-initialized!\n");
        
        return s32Ret;
    }

    stLogPath.pszPath = (HI_CHAR*)pszLogPath;
    stLogPath.u32PathLen = strlen((HI_CHAR*)pszLogPath) + 1;

    //1.first, check the path
    s32Ret = access(pszLogPath, W_OK | F_OK);
    if (s32Ret != 0)
    {        
        HI_LOG_UNLOCK();

        HI_ERR_LOG("pszFilePath %s non-exist or non-writeable\n", pszLogPath);

        return HI_FAILURE;
    }
    
    s32Ret = ioctl(g_s32DbgDevFd, UMAP_CMPI_LOG_SET_PATH, &stLogPath);
    HI_LOG_UNLOCK();

    return s32Ret;
}

HI_S32 HI_MPI_StorePathSet(const HI_CHAR* pszPath)
{
    HI_S32 s32Ret = HI_FAILURE;
    STORE_PATH_S stStorePath;

    HI_LOG_LOCK();

    if (pszPath == HI_NULL || g_s32DbgDevFd == -1)
    {
        HI_LOG_UNLOCK();
        HI_ERR_LOG(" params invalid or log non-initialized!\n");
        return s32Ret;
    }
    
    stStorePath.pszPath = (HI_CHAR*)pszPath;
    stStorePath.u32PathLen = strlen((HI_CHAR*)pszPath) + 1;    

    /* check the path */
    s32Ret = access(pszPath, W_OK);
    if (s32Ret != 0)
    {        
        HI_LOG_UNLOCK();

        HI_ERR_LOG("pszPath %s non-writeable\n", pszPath);

        return HI_FAILURE;
    }
    
    s32Ret = ioctl(g_s32DbgDevFd, UMAP_CMPI_LOG_SET_STORE_PATH, &stStorePath);
    HI_LOG_UNLOCK();
    
    return s32Ret;
}

