

#ifndef __FSP_HEADER__
#define __FSP_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#include "public.h"
#include "vfmw.h"
#include "syn_cmn.h"

#define USE_FSP  1


#define FSP_OK     0
#define FSP_ERR   -1

#define FS_DISP_STATE_NOT_USED        0
#define FS_DISP_STATE_ALLOCED         1
#define FS_DISP_STATE_DEC_COMPLETE    2
#define FS_DISP_STATE_WAIT_DISP       3

#define MAX_PMV_NUM          MAX_PMV_SLOT_NUM


#define  FSP_ASSERT_RET( cond, else_print )								\
    do {																	\
        if( !(cond) )														\
        {																	\
            dprint(PRN_FATAL,"fsp.c,L%d: %s\n", __LINE__, else_print );		\
            return FSP_ERR;													\
        }																	\
    }while(0)

#define  FSP_ASSERT( cond, else_print )									\
    do {																	\
        if( !(cond) )														\
        {																	\
            dprint(PRN_FATAL,"fsp.c,L%d: %s\n", __LINE__, else_print );		\
            return;															\
        }																	\
    }while(0)

/* ����֡������� */
typedef enum
{
    FSP_PHY_FS_TYPE_DECODE,
    FSP_PHY_FS_TYPE_DISPLAY,
    FSP_PHY_FS_TYPE_BUTT
} FSP_PHY_FS_TYPE_E;

/* ����֡��Ĵ洢��ʽ */
typedef enum
{
    FSP_PHY_FS_STORE_TYPE_FRAME,
    FSP_PHY_FS_STORE_TYPE_FIELD,
    FSP_PHY_FS_STORE_TYPE_BUTT
} FSP_PHY_FS_STORE_TYPE_E;

typedef enum
{
    LFS_PMV_STATE_NOT_USE,
    LFS_PMV_STATE_ALLOCATED,
    LFS_PMV_STATE_VALID,
    LFS_PMV_STATE_BUTT
} LFS_PMV_STATE_E;

/* FSPʵ��������Ϣ */
typedef struct hiFSP_INST_CFG_S
{
    SINT8     s32DispLoopEnable;
    SINT8     s32TfEnable;
    SINT8     s32RprEnable;
    SINT8     s8SubLevelEnable;
    SINT8     s32UserDec;

    SINT8     s32ExpectedDecFsNum;
    SINT8     s32ExpectedTfFsNum;
    SINT8     s32ExpectedDispFsNum;
    SINT8     s32ExpectedPmvNum;

    SINT32    s32DecFsWidth;
    SINT32    s32DecFsHeight;

    SINT32    s32TfFsWidth;
    SINT32    s32TfFsHeight;

    SINT32    s32DispFsWidth;
    SINT32    s32DispFsHeight;
} FSP_INST_CFG_S;


/* ����֡�������� */
typedef struct hiFSP_PHY_FS_S
{
    SINT8     IsDecRef;
    SINT8     IsTfRef;
    SINT8     DispState;

    SINT32    PhyFsID;
    FSP_PHY_FS_STORE_TYPE_E  eStoreType;
    SINT32    PhyAddr;
    SINT32    LineNumAddr;
    SINT32    Stride;
    SINT32    FsWidth;
    SINT32    FsHeight;
    SINT32    ChromOffset;
    SINT32    LumaPixelOffset;
    SINT32    ChromaPixelOffset;
    UINT32    u32Tick;
    SINT32    ImgSize; //l00273086
} FSP_PHY_FS_S;

/* �߼�֡�������� */
typedef struct hiFSP_LOGIC_FS_S
{
    SINT8             s32IsRef;
    SINT8             s32DispState;
    SINT8             IsDummyFs;
    SINT8             s32LogicFsID;
    SINT8             PmvIdc;
    IMAGE             stDispImg;

    UINT32            u32Tick;
    LFS_PMV_STATE_E   eLfsPmvState;

    FSP_PHY_FS_S      *pstDecodeFs;
    FSP_PHY_FS_S      *pstDispOutFs;
    FSP_PHY_FS_S      *pstTfOutFs;

    SINT32            PmvAddr;
    SINT32            HalfPmvOffset;

    UINT32            TR;
} FSP_LOGIC_FS_S;

/* ֡�汸����Ϣ�ṹ�� */
typedef struct hiFSP_PRE_INF_S
{
    SINT32           s32PreWidth;      // ��¼ǰһ�εĿ��߼�����֡���������� 4+64 ƽ���л���
    SINT32           s32PreHeight;
    SINT32           s32PreDecNum;   // ����֡���С�ɿ��߾�����ͬʱҪ��¼�����������жϽ���֡���ַ�Ƿ����仯
    SINT32           s32PreDecSlotLen;
    SINT32           s32PreExpectNum; //Ϊ�˱��ֹ��ܵĶ����ԣ�������һ����������ֻ����DSD��Ŀ������ʶ��dpb size�ı仯

} FSP_PRE_INF_S;

/* FSPʵ��״̬���� */
typedef enum hiFSP_INST_STATE_E
{
    FSP_INST_STATE_NULL,
    FSP_INST_STATE_INUSE,
    FSP_INST_STATE_BUTT
} FSP_INST_STATE_E;


/* ֡���ʵ�� */
typedef struct hiFSP_INST_S
{
    /* ����֡��ʵ�� */
    SINT8             s32InstID;
    SINT8             s8LowBufEnable;
    FSP_INST_STATE_E  eFspInstState;

    SINT8             s8SizeChangeFlag;

    SINT8             s8TotalValidFsNum;
    SINT8             s32DecFsNum;
    SINT8             s32DispFsNum;
    SINT8             s32TfFsNum;

    SINT8             s32PmvNum;
    SINT8             IsPmvInUse[MAX_PMV_NUM];

    UINT32            u32InstTick;

    FSP_PRE_INF_S     stPreInfo;
    FSP_INST_CFG_S    stInstCfg;

    FSP_LOGIC_FS_S    stLogicFs[MAX_FRAME_NUM];// 64
    FSP_PHY_FS_S      stDummyPhyFs;

    FSP_PHY_FS_S      stDecFs[MAX_FRAME_NUM]; //64
    FSP_PHY_FS_S      stDispFs[MAX_FRAME_NUM]; //64
    FSP_PHY_FS_S      stTfFs[2];

    /* PMV�洢ʵ�� */
    SINT32            s32HalfPmvOffset;
    SINT32            PmvAddr[MAX_PMV_NUM]; //20
    SINT32            ImgSlotLen;  //l00273086
    SINT32            TotalMemUsed; //l00273086
    SINT32            MemAddr;//l00273086
    SINT32            MemSize;//l00273086
    UINT32            StrideByte;//l00273086
} FSP_INST_S;

typedef struct hiFSP_ATTR_S
{
    SINT8             s8LowBufEnable;
} FSP_ATTR_S;

/* ֡���ʵ��������Ϣ */
typedef struct FSP_INST_INFO_S
{
    /* PMV�洢ʵ�� */
    SINT8             s32PmvNum;
    /* ����֡��ʵ�� */
    SINT8             s32DecFsNum;
    SINT8             s32DispFsNum;
    SINT8             s32TfFsNum;
    SINT8             s32InstID;

    FSP_PHY_FS_S      stDecFs[MAX_FRAME_NUM];

    SINT32            s32DecFsSize;
} FSP_INST_INFO_S;

/***********************************************************************
    ȫ�ֳ�ʼ��
 ***********************************************************************/
VOID FSP_Init(VOID);

/***********************************************************************
    ȫ���˳�
 ***********************************************************************/
VOID  FSP_Exit(VOID);

/***********************************************************************
    ֡��ʵ����������
 ***********************************************************************/
SINT32 FSP_ConfigAttribute(SINT32 InstID, FSP_ATTR_S *pstAttr);

/***********************************************************************
    ʹ��ʵ��
    �ɹ�����ʵ��FSP_OK��ʧ�ܷ���FSP_ERR
 ***********************************************************************/
SINT32  FSP_EnableInstance(SINT32  InstID);

/***********************************************************************
    ����ʵ��
 ***********************************************************************/
VOID  FSP_DisableInstance(SINT32 InstID);

/***********************************************************************
    ��λʵ��
    �ɹ�����ʵ��FSP_OK��ʧ�ܷ���FSP_ERR
 ***********************************************************************/
SINT32  FSP_ResetInstance(SINT32  InstID);

/***********************************************************************
    ���ʵ��
    �ڼ�������£��������������ش���Э�����ܻ�Ҫ��������н�������
    �ģ����а������FSPʵ����������֡�涼���Ϊ���ο�������ʾ���ȴ���һ��
    ����ʹ�á�
    ��FSP_ResetInstance()��ͬ���ǣ������������������Ϣ��Ҳ�����֡��ķ���
    ��Ϣ�������������֡��ռ����Ϣ��
    �ɹ�����ʵ��FSP_OK��ʧ�ܷ���FSP_ERR
 ***********************************************************************/
SINT32  FSP_EmptyInstance(SINT32  InstID);

//SINT32 FSP_GetUsedBufferNum(SINT32 InstID);
/***********************************************************************
    ���û�б��ŵ�VO QUEUE ���߼�֡
 ***********************************************************************/
SINT32 FSP_ClearNotInVoQueue(SINT32 InstID, IMAGE_VO_QUEUE *pstVoQue);

/***********************************************************************
    ����ʵ��
 ***********************************************************************/
SINT32  FSP_ConfigInstance( SINT32 InstID,  FSP_INST_CFG_S *pstFspInstCfg );

/***********************************************************************
    ֡��ָ�
 ***********************************************************************/
SINT32  FSP_PartitionFsMemory( SINT32 InstID, SINT32 MemPhyAddr, SINT32 MemSize, SINT32 *UsedMemSize );

/***********************************************************************
    ��ȡ����֡��ַ��
    VDH���ý���֡��ַʱ��Ҫ��"����֡��ַ�� + ֡ID"�ķ�ʽ��
 ***********************************************************************/
//VOID  FSP_GetDecodeFrameTable(SINT32 InstID, SINT32 PhyAddr[], SINT32 *pFsNum);

/***********************************************************************
    ��ȡ����PMV��ַ��
    VDH����PMV��ַʱ��Ҫ��"PMV��ַ�� + PMV ID"�ķ�ʽ��
 ***********************************************************************/
//VOID  FSP_GetDecodePmvTable(SINT32 InstID, SINT32 PhyAddr[], SINT32 *pPmvNum);

/***********************************************************************
    ��ȡ�µ��߼�֡
    InstID: ʵ��ID
    NeedPhyFs: ��־�Ƿ���Ҫ��������֡�档H.264�ڽ���gapʱҲҪ����֡�棬
               ������֡�治��Ҫ����ʵ������֡���Դ�����.

    �ɹ������߼�֡��ID��ʧ�ܷ���-1
 ***********************************************************************/
SINT32  FSP_NewLogicFs( SINT32 InstID, SINT32 NeedPhyFs );

/***********************************************************************
    ����߼�֡
    ���Ҫ���ⶪ��һ���Ѿ����뵽���߼�֡�����Էֱ��������Ϊ������ο���
    ������ʾ������TF�ο��ȣ�����õ��ô˺�����ʹ�ü����ܱ�֤������߼�
    ֡�Ϲ�������Դ��ȫ�ͷŵ�
    HoldDispFs: ָʾ�Ƿ���Ҫ��������ʾ����ʾ֡�棬1��ʾ����߼�֡�ڵ���ʾ
                ֡���Ѳ�����ʾ��������������������߼�֡���������
                ����HoldDispFsΪ��ֵ����ǰ�߼�֡�Ĳο�����һ���ᱻ���.
 ***********************************************************************/
VOID FSP_ClearLogicFs( SINT32 InstID, SINT32 LogicFrameID, SINT32 HoldDispFs);

/***********************************************************************
    �����߼�֡�Ĳο�����, ��������֡��TF�ο�֡
    IsRef = 1���Ϊ���ο�����֮���߼�֡���Ϊ���ο�
 ***********************************************************************/
VOID  FSP_SetRef( SINT32 InstID, SINT32 LogicFrameID, SINT32 IsRef );

/***********************************************************************
    ��ȡ�߼�֡�Ľ���ο�����
 ***********************************************************************/
SINT32  FSP_GetRef( SINT32 InstID, SINT32 LogicFrameID );

/***********************************************************************
    �����߼�֡�Ƿ�ȴ���ʾ
 ***********************************************************************/
VOID  FSP_SetDisplay( SINT32 InstID, SINT32 LogicFrameID, SINT32 IsWaitDisp );

/***********************************************************************
    ��ȡ�߼�֡�Ƿ�ȴ���ʾ
 ***********************************************************************/
SINT32  FSP_GetDisplay( SINT32 InstID, SINT32 LogicFrameID );

/***********************************************************************
    ��������֡��Ĵ洢��ʽ(��֡�洢/�����洢)
 ***********************************************************************/
SINT32  FSP_SetStoreType( SINT32 InstID, FSP_PHY_FS_TYPE_E ePhyFsType, SINT32 PhyFsID, FSP_PHY_FS_STORE_TYPE_E eStoreType);

/***********************************************************************
    ��ȡ����֡��Ĵ洢��ʽ(��֡�洢/�����洢)
 ***********************************************************************/
FSP_PHY_FS_STORE_TYPE_E  FSP_GetStoreType( SINT32 InstID, FSP_PHY_FS_TYPE_E ePhyFsType, SINT32 PhyFsID);

/***********************************************************************
    ɾ��ָ���߼�֡��PMV��
    PMV����ÿһ������֡�󶨣��ڽ�һ֡���Ϊ�ο�ʱ������PMV��Ҳ����������
    ����ÿһ���ο�֡����ҪPMV��(ǰ��ο�֡��PMV���豣��)��������һ���ο�
    ֡��PMV�鲻��Ҫ����ʱ����ɾ�����ɱ�֤���̶ȵؽ�ʡ�ڴ档
    ���PMV���㹻(����MPEG2Э����3��PMV��)����������ʹ��Ҳ������������.
 ***********************************************************************/
VOID  FSP_RemovePmv( SINT32 InstID, SINT32 LogicFrameID );


/***********************************************************************
    ��ȡ�߼�֡
 ***********************************************************************/
FSP_LOGIC_FS_S  *FSP_GetLogicFs( SINT32 InstID, SINT32 LogicFrameID);

/***********************************************************************
    ��ȡ�߼�֡�����ͼ��(ָ��IMAGE�ṹ���ָ��)
 ***********************************************************************/
IMAGE  *FSP_GetFsImagePtr( SINT32 InstID, SINT32 LogicFrameID);

/***********************************************************************
    ��ȡ��������֡��
 ***********************************************************************/
//FSP_PHY_FS_S  *FSP_GetDecPhyFs( SINT32 InstID, SINT32 LogicFrameID);

/***********************************************************************
    ��ȡ�������֡��
 ***********************************************************************/
FSP_PHY_FS_S  *FSP_GetDispPhyFs( SINT32 InstID, SINT32 LogicFrameID);

/***********************************************************************
    ��ȡTF�ο�����֡��
 ***********************************************************************/
FSP_PHY_FS_S  *FSP_GetTfPhyFs( SINT32 InstID, SINT32 LogicFrameID);

/***********************************************************************
    ��ȡPMV��ַ
 ***********************************************************************/
SINT32 FSP_GetPmvAddr( SINT32 InstID, SINT32 LogicFrameID );

/***********************************************************************
    �ж��Ƿ������뵽��֡
    ��������뵽������1�����򷵻�0
 ***********************************************************************/
SINT32  FSP_IsNewFsAvalible(SINT32 InstID);

/***********************************************************************
 ����Ƿ�����֡��(��Ҫ����ʾ֡��)й©
 ***********************************************************************/
SINT32  FSP_IsFsLeak(SINT32 InstID);

/***********************************************************************
    ��ȡDecFs��ַ��
    264Э���õ�ַ��+ID�ķ�ʽ���߼����õ�ַ��Ϣ��������Ҫ��ȡ����֡���ַ��
 ***********************************************************************/
VOID  FSP_GetDecFsAddrTab( SINT32 InstID, SINT32 *DecFsNum, SINT32 *DecFsAddrTab , SINT32 *LineNumAddrTab);

/***********************************************************************
    ��ȡPMV��ַ��
    264Э���õ�ַ��+ID�ķ�ʽ���߼�����PMV��ַ��Ϣ��������Ҫ��ȡPMV��ַ��
 ***********************************************************************/
VOID  FSP_GetPmvAddrTab( SINT32 InstID, SINT32 *PmvNum, SINT32 *PmvAddrTab );

/***********************************************************************
    ��ӡFSP�ڲ�״̬
    ����ʵʱ����ʱ�������ض�����̽��FSP�ڲ�״̬
 ***********************************************************************/
VOID FSP_PrintContext(SINT32 InstID);

/***********************************************************************
    ��ȡ��ǰ֡��ʵ�������Ϣ
 ***********************************************************************/
VOID  FSP_GetInstInfo( SINT32 InstID, FSP_INST_INFO_S *pFspInstInfo );

/***********************************************************************
    ͨ��������ַ��ȡ�߼�֡��ID
 ***********************************************************************/
SINT32  FSP_GetLogicIdByPhyAddr( SINT32 InstID, SINT32 PhyAddr);

/***********************************************************************
    ��ȡ��ձ��
 ***********************************************************************/
//UINT8 FSP_GetQueSwitchFlag(SINT32 InstID);

/***********************************************************************
    ��ȡ������ձ��
 ***********************************************************************/
//VOID FSP_SetQueSwitchFlag(SINT32 InstID);

/***********************************************************************
    ��ȡ�����߼�֡��ID
    �ο�֡��λʱ��
 ***********************************************************************/
SINT32 FSP_GetNullLogicFs(SINT32 InstID);

/***********************************************************************
    ��ȡ��ǰ֡��ʹ�÷�ʽ
    1:��״֡�棬0:ԭ֡�棬-1:��δʹ��
 ***********************************************************************/
SINT32 FSP_GetFspType(SINT32 InstID);

/***********************************************************************
    �ⲿ֡�����ýӿ�
 ***********************************************************************/
#ifdef VFMW_BVT_SUPPORT
SINT32 FSP_SetFrameInterface(FSP_FRAME_INTF_S *pFspFrameIntf);

/***********************************************************************
	�����߼�֡��stride�������Ϣ���ýӿ�
 ***********************************************************************/
SINT32 FSP_UdateInfo(SINT32 InstID, UINT32 Width, UINT32 Height);
#endif

#ifdef __cplusplus
}
#endif

#endif
