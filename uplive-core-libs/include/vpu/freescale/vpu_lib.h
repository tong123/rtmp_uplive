/*
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 *
 * Copyright (C) 2004-2013 Freescale Semiconductor, Inc.
 */

/* The following programs are the sole property of Freescale Semiconductor Inc.,
 * and contain its proprietary and confidential information. */

/*!
 * @file vpu_lib.h
 *
 * @brief header file for codec API funcitons for VPU
 *
 * @ingroup VPU
 */

#ifndef __VPU__LIB__H
#define __VPU__LIB__H

typedef unsigned char Uint8;
typedef unsigned long Uint32;
typedef unsigned short Uint16;
typedef Uint32 PhysicalAddress;
typedef Uint32 VirtualAddress;

#define STREAM_FULL_EMPTY_CHECK_DISABLE 0
#define BUF_PIC_FLUSH			1
#define BUF_PIC_RESET			0

#define BIT_REG_MARGIN			0x4000

#define PRJ_TRISTAN     		0xF000
#define PRJ_TRISTAN_REV			0xF001
#define PRJ_PRISM_CX			0xF002
#define PRJ_SHIVA       		0xF003
#define PRJ_PRISM_EX			0xF004
#define PRJ_BODA_CX_4			0xF005
#define PRJ_CODA_DX_6M			0xF100
#define PRJ_CODA_DX_8			0xF306
#define PRJ_BODA_DX_4V			0xF405
#define PRJ_BODADX7X			0xF009
#define	PRJ_CODAHX_14			0xF00A
#define PRJ_CODA7541			0xF012
#define PRJ_CODA_960			0xF020

#define MAX_NUM_INSTANCE		32

#define DC_TABLE_INDEX0		    0
#define AC_TABLE_INDEX0		    1
#define DC_TABLE_INDEX1		    2
#define AC_TABLE_INDEX1		    3

typedef enum {
	STD_MPEG4 = 0,
	STD_H263 = 1,
	STD_AVC = 2,
	STD_VC1 = 3,
	STD_MPEG2 = 4,
	STD_DIV3 =5,
	STD_RV = 6,
	STD_MJPG = 7,
	STD_AVS = 8,
	STD_VP8 = 9
} CodStd;

typedef enum {
	RETCODE_SUCCESS = 0,
	RETCODE_FAILURE = -1,
	RETCODE_INVALID_HANDLE = -2,
	RETCODE_INVALID_PARAM = -3,
	RETCODE_INVALID_COMMAND = -4,
	RETCODE_ROTATOR_OUTPUT_NOT_SET = -5,
	RETCODE_ROTATOR_STRIDE_NOT_SET = -11,
	RETCODE_FRAME_NOT_COMPLETE = -6,
	RETCODE_INVALID_FRAME_BUFFER = -7,
	RETCODE_INSUFFICIENT_FRAME_BUFFERS = -8,
	RETCODE_INVALID_STRIDE = -9,
	RETCODE_WRONG_CALL_SEQUENCE = -10,
	RETCODE_CALLED_BEFORE = -12,
	RETCODE_NOT_INITIALIZED = -13,
	RETCODE_DEBLOCKING_OUTPUT_NOT_SET = -14,
	RETCODE_NOT_SUPPORTED = -15,
	RETCODE_REPORT_BUF_NOT_SET = -16,
	RETCODE_FAILURE_TIMEOUT = -17,
	RETCODE_MEMORY_ACCESS_VIOLATION = -18,
	RETCODE_JPEG_EOS = -19,
	RETCODE_JPEG_BIT_EMPTY = -20
} RetCode;

typedef enum {
	LINEAR_FRAME_MAP = 0,
	TILED_FRAME_MB_RASTER_MAP = 1,
	TILED_FIELD_MB_RASTER_MAP = 2,
	TILED_MAP_TYPE_MAX
} GDI_TILED_MAP_TYPE;

typedef enum {
	ENABLE_ROTATION,
	DISABLE_ROTATION,
	ENABLE_MIRRORING,
	DISABLE_MIRRORING,
	ENABLE_DERING,
	DISABLE_DERING,
	SET_MIRROR_DIRECTION,
	SET_ROTATION_ANGLE,
	SET_ROTATOR_OUTPUT,
	SET_ROTATOR_STRIDE,
	ENC_GET_SPS_RBSP,
	ENC_GET_PPS_RBSP,
	DEC_SET_SPS_RBSP,
	DEC_SET_PPS_RBSP,
	ENC_PUT_MP4_HEADER,
	ENC_PUT_AVC_HEADER,
	ENC_SET_SEARCHRAM_PARAM,
	ENC_GET_VOS_HEADER,
	ENC_GET_VO_HEADER,
	ENC_GET_VOL_HEADER,
	ENC_GET_JPEG_HEADER,
	ENC_SET_INTRA_MB_REFRESH_NUMBER,
	DEC_SET_DEBLOCK_OUTPUT,
	ENC_ENABLE_HEC,
	ENC_DISABLE_HEC,
	ENC_SET_SLICE_INFO,
	ENC_SET_GOP_NUMBER,
	ENC_SET_INTRA_QP,
	ENC_SET_BITRATE,
	ENC_SET_FRAME_RATE,
	ENC_SET_REPORT_MBINFO,
	ENC_SET_REPORT_MVINFO,
	ENC_SET_REPORT_SLICEINFO,
	DEC_SET_REPORT_BUFSTAT,
	DEC_SET_REPORT_MBINFO,
	DEC_SET_REPORT_MVINFO,
	DEC_SET_REPORT_USERDATA,
	SET_DBK_OFFSET,
	SET_WRITE_MEM_PROTECT,

	ENC_SET_SUB_FRAME_SYNC,
	ENC_ENABLE_SUB_FRAME_SYNC,
	ENC_DISABLE_SUB_FRAME_SYNC,

	DEC_SET_FRAME_DELAY
} CodecCommand;

typedef struct {
	Uint32 strideY;
	Uint32 strideC;
	int myIndex;
	PhysicalAddress bufY;
	PhysicalAddress bufCb;
	PhysicalAddress bufCr;
	PhysicalAddress bufMvCol;
} FrameBuffer;

typedef struct {
	Uint32 left;
	Uint32 top;
	Uint32 right;
	Uint32 bottom;
} Rect;

typedef enum {
	MIRDIR_NONE,
	MIRDIR_VER,
	MIRDIR_HOR,
	MIRDIR_HOR_VER
} MirrorDirection;

typedef enum {
	FORMAT_420,
	FORMAT_422,
	FORMAT_224,
	FORMAT_444,
	FORMAT_400
} ChromaFormat;

typedef struct {
	int DbkOffsetA;
	int DbkOffsetB;
	int DbkOffsetEnable;
} DbkOffset;

/* VP8 specific display information */
typedef struct {
	unsigned hScaleFactor : 2;
	unsigned vScaleFactor : 2;
	unsigned picWidth     : 14;
	unsigned picHeight    : 14;
} Vp8ScaleInfo;

/* Decode struct and definition */
typedef struct CodecInst DecInst;
typedef DecInst *DecHandle;

typedef struct {
	CodStd bitstreamFormat;
	PhysicalAddress bitstreamBuffer;
	Uint8 *pBitStream;
	int bitstreamBufferSize;
	int qpReport;
	int mp4DeblkEnable;
	int reorderEnable;
	int chromaInterleave;
	int filePlayEnable;     /* Not used on mx6 */
	int picWidth;
	int picHeight;
	int avcExtension;	/* Not used on none mx6 */
	int dynamicAllocEnable; /* Not used on mx6 */
	int streamStartByteOffset;
	int mjpg_thumbNailDecEnable; /* Not used on mx6 */
	PhysicalAddress psSaveBuffer;
	int psSaveBufferSize;
	int mp4Class;

	int mapType;
	int tiled2LinearEnable;
	int bitstreamMode;
	int jpgLineBufferMode; /* mx6 */

} DecOpenParam;

typedef struct {
	int frameBufStatBufSize;  /* Size of buffer to save Frame Buffer Status */
	int mbInfoBufSize;        /* Size of buffer to save Mb information for Error Concealment  */
	int mvInfoBufSize;        /* Size of buffer to save Motion vector information */
} DecReportBufSize;

typedef struct {
	Uint8 *pHeader;
	int headerSize;
} JpegHeaderBufInfo;

typedef struct {
	int picWidth;
	int picHeight;
	Uint32 frameRateInfo;   /* Not used on mx6 */
	Uint32 frameRateRes;    /* Not used on none mx6 */
	Uint32 frameRateDiv;    /* Not used on none mx6 */

	Rect picCropRect;

	int mp4_dataPartitionEnable;
	int mp4_reversibleVlcEnable;
	int mp4_shortVideoHeader;
	int h263_annexJEnable;

	int minFrameBufferCount;
	int frameBufDelay;
	int nextDecodedIdxNum;
	int normalSliceSize;
	int worstSliceSize;
	int mjpg_thumbNailEnable;
	int mjpg_sourceFormat;

	int streamInfoObtained;
	int profile;
	int level;
	int interlace;
	int constraint_set_flag[4];
	int direct8x8Flag;
	int vc1_psf;
	int aspectRateInfo;
	Uint32 errorcode;

	int bitRate;
	Vp8ScaleInfo vp8ScaleInfo;
	int mjpg_ecsPtr;

	DecReportBufSize reportBufSize;
} DecInitialInfo;

typedef struct {
	PhysicalAddress bufferBase;
	int bufferSize;
} ExtBufCfg;

typedef struct {
	int sliceMode;
	int sliceSizeMode;
	int sliceSize;
} EncSliceMode;

typedef struct {
	PhysicalAddress subSampBaseAMvc;
	PhysicalAddress subSampBaseBMvc;
	ExtBufCfg scratchBuf;
} EncExtBufInfo;

typedef struct {
	PhysicalAddress sliceSaveBuffer;
	int sliceSaveBufferSize;
} DecAvcSliceBufInfo;

typedef struct {
	int maxMbX;
	int maxMbY;
	int maxMbNum;
} DecMaxFrmInfo;

typedef struct {
	ExtBufCfg avcSliceBufInfo;
	ExtBufCfg vp8MbDataBufInfo;
	DecMaxFrmInfo	maxDecFrmInfo;
} DecBufInfo;

typedef enum {
	PARA_TYPE_FRM_BUF_STATUS = 1,
	PARA_TYPE_MB_PARA = 2,
	PARA_TYPE_MV = 4,
} ExtParaType;

typedef struct {
	int prescanEnable; /* Not used on mx6 */
	int prescanMode;   /* Not used on mx6 */
	int dispReorderBuf;
	int iframeSearchEnable;
	int skipframeMode;
	int skipframeNum;
	int chunkSize;      /* Not used on mx6 */
	int picStartByteOffset;   /* Not used on mx6 */
	PhysicalAddress picStreamBufferAddr;  /* Not used on mx6 */
	int mjpegScaleDownRatioWidth;  /* mx6 */
	int mjpegScaleDownRatioHeight;  /* mx6 */

	PhysicalAddress phyJpgChunkBase;
	unsigned char *virtJpgChunkBase;

} DecParam;

typedef	struct {
	int enable;
	int size;
	union {
	    int mvNumPerMb;
	    int userDataNum;
	};
	union {
	    int reserved;
	    int userDataBufFull;
	};
	Uint8 *addr;
} DecReportInfo;

/* VP8 specific header information */
typedef struct {
	unsigned showFrame     : 1;
	unsigned versionNumber : 3;
	unsigned refIdxLast    : 8;
	unsigned refIdxAltr    : 8;
	unsigned refIdxGold    : 8;
} Vp8PicInfo;

/* MVC specific picture information */
typedef struct {
	int viewIdxDisplay;
	int viewIdxDecoded;
} MvcPicInfo;

/* AVC specific SEI information (frame packing arrangement SEI) */
typedef struct {
	unsigned exist;
	unsigned frame_packing_arrangement_id;
	unsigned frame_packing_arrangement_cancel_flag;
	unsigned quincunx_sampling_flag;
	unsigned spatial_flipping_flag;
	unsigned frame0_flipped_flag;
	unsigned field_views_flag;
	unsigned current_frame_is_frame0_flag;
	unsigned frame0_self_contained_flag;
	unsigned frame1_self_contained_flag;
	unsigned frame_packing_arrangement_ext_flag;
	unsigned frame_packing_arrangement_type;
	unsigned content_interpretation_type;
	unsigned frame0_grid_position_x;
	unsigned frame0_grid_position_y;
	unsigned frame1_grid_position_x;
	unsigned frame1_grid_position_y;
	unsigned frame_packing_arrangement_repetition_period;
} AvcFpaSei;

typedef struct {
	int indexFrameDisplay;
	int indexFrameDecoded;
	int NumDecFrameBuf;
	int picType;
	int picTypeFirst;    /* Not used on none mx6 */
	int idrFlg;	     /* Not used on none mx6 */
	int numOfErrMBs;
	Uint32 *qpInfo;      /* Not used on mx5 and mx6 */
	int hScaleFlag;
	int vScaleFlag;
	int indexFrameRangemap;
	int prescanresult;    /* Not used on mx6 */
	int notSufficientPsBuffer;
	int notSufficientSliceBuffer;
	int decodingSuccess;
	int interlacedFrame;
	int mp4PackedPBframe;
	int h264Npf;

	int pictureStructure;
	int topFieldFirst;
	int repeatFirstField;
	union {
	    int progressiveFrame;
	    int vc1_repeatFrame;
	};
	int fieldSequence;

	int decPicHeight;
	int decPicWidth;
	Rect decPicCrop;

	int aspectRateInfo;
	Uint32 frameRateRes;   /* Not used on none mx6 */
	Uint32 frameRateDiv;   /* Not used on none mx6 */
	Vp8ScaleInfo vp8ScaleInfo;
	Vp8PicInfo vp8PicInfo;
	MvcPicInfo mvcPicInfo;  /* Not used on none mx6 */
	AvcFpaSei avcFpaSei;

	int frameStartPos;   /* Not used on none mx6 */
	int frameEndPos;     /* Not used on none mx6 */
	int consumedByte;    /* Not used on none mx6 */

	DecReportInfo mbInfo;        /* Not used on mx6 */
	DecReportInfo mvInfo;        /* Not used on mx6 */
	DecReportInfo frameBufStat;  /* Not used on mx6 */
	DecReportInfo userData;
} DecOutputInfo;

/* encode struct and definition */
typedef struct CodecInst EncInst;
typedef EncInst *EncHandle;

typedef struct {
	int mp4_dataPartitionEnable;
	int mp4_reversibleVlcEnable;
	int mp4_intraDcVlcThr;
	int mp4_hecEnable;
	int mp4_verid;
} EncMp4Param;

typedef struct {
	int h263_annexIEnable;
	int h263_annexJEnable;
	int h263_annexKEnable;
	int h263_annexTEnable;
} EncH263Param;

typedef struct {
	int avc_constrainedIntraPredFlag;
	int avc_disableDeblk;
	int avc_deblkFilterOffsetAlpha;
	int avc_deblkFilterOffsetBeta;
	int avc_chromaQpOffset;
	int avc_audEnable;
	int avc_fmoEnable;
	int avc_fmoSliceNum;
	int avc_fmoType;
	int avc_fmoSliceSaveBufSize;
	int avc_frameCroppingFlag;
	int avc_frameCropLeft;
	int avc_frameCropRight;
	int avc_frameCropTop;
	int avc_frameCropBottom;

	int mvc_extension;
	int interview_en;
	int paraset_refresh_en;
	int prefix_nal_en;
} EncAvcParam;

typedef struct {
	int mjpg_sourceFormat;
	int mjpg_restartInterval;
	int mjpg_thumbNailEnable;
	int mjpg_thumbNailWidth;
	int mjpg_thumbNailHeight;
	Uint8 *mjpg_hufTable;   /* Not used on mx6 */
	Uint8 *mjpg_qMatTable;  /* Not used on mx6 */

	Uint8 huffVal[4][162];  /* mx6 */
	Uint8 huffBits[4][256]; /* mx6 */
	Uint8 qMatTab[4][64];   /* mx6 */
	Uint8 cInfoTab[4][6];   /* mx6 */

} EncMjpgParam;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32 bitstreamBufferSize;
	CodStd bitstreamFormat;

	int picWidth;
	int picHeight;
	Uint32 frameRateInfo;
	int bitRate;
	int initialDelay;
	int vbvBufferSize;
	int enableAutoSkip;
	int gopSize;
	int linear2TiledEnable;
	int mapType;

	EncSliceMode slicemode;
	int intraRefresh;

	int sliceReport;
	int mbReport;
	int mbQpReport;

	int rcIntraQp;
	int chromaInterleave;
	int dynamicAllocEnable; /* Not used for mx6 */
	int ringBufferEnable;

	union {
		EncMp4Param mp4Param;
		EncH263Param h263Param;
		EncAvcParam avcParam;
		EncMjpgParam mjpgParam;
	} EncStdParam;

	int userQpMin;
	int userQpMax;
	int userQpMinEnable;
	int userQpMaxEnable;

	Uint32 userGamma;
	int RcIntervalMode;		/* 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level */
	int MbInterval;			/* use when RcintervalMode is 3 */
	int avcIntra16x16OnlyModeEnable;

	int MESearchRange;      // 3: 16x16, 2:32x16, 1:64x32, 0:128x64, H.263(Short Header : always 3)
	int MEUseZeroPmv;       // 0: PMV_ENABLE, 1: PMV_DISABLE
	int IntraCostWeight;    // Additional weight of Intra Cost for mode decision to reduce Intra MB density
} EncOpenParam;

typedef struct {
        int sliceInfoBufSize;                 /* Slice Info */
        int mbInfoBufSize;               /* Mb Param for Error Concealment */
        int mvInfoBufSize;                /* Motion vector */
} EncReportBufSize;

typedef struct {
	int minFrameBufferCount;
	EncReportBufSize reportBufSize;
} EncInitialInfo;

typedef struct {
	FrameBuffer *sourceFrame;
	int encTopOffset;
	int encLeftOffset;
	int forceIPicture;
	int skipPicture;
	int quantParam;
	PhysicalAddress picStreamBufferAddr;
	int picStreamBufferSize;
	int enableAutoSkip;
} EncParam;

typedef	struct {
	int	enable;
	int	type;
	int	size;
	Uint8   *addr;
} EncReportInfo;

typedef struct {
	PhysicalAddress bitstreamBuffer;
	Uint32 bitstreamSize;
	int bitstreamWrapAround;
	int skipEncoded;
	int picType;
	int numOfSlices;
	int reconFrameIndex;

	Uint32 *pSliceInfo;
	Uint32 *pMBInfo;
	Uint32 *pMBQpInfo;

	EncReportInfo mbInfo;
	EncReportInfo mvInfo;
	EncReportInfo sliceInfo;
} EncOutputInfo;

typedef struct {
	Uint32 *paraSet;
	Uint8 *pParaSet;
	int size;
} EncParamSet;

typedef struct {
	PhysicalAddress searchRamAddr;
	int SearchRamSize;
} SearchRamParam;

typedef struct {
	PhysicalAddress buf;
	int size;
	int headerType;
	int userProfileLevelEnable;
	int userProfileLevelIndication;
} EncHeaderParam;

typedef enum {
	VOL_HEADER,		/* video object layer header */
	VOS_HEADER,		/* visual object sequence header */
	VIS_HEADER		/* video object header */
} Mp4HeaderType;

typedef enum {
	SPS_RBSP,
	PPS_RBSP,
	END_SEQ_RBSP,
	END_STREAM_RBSP,
	SPS_RBSP_MVC,
	PPS_RBSP_MVC
} AvcHeaderType;

typedef struct {
	Uint32 gopNumber;
	Uint32 intraQp;
	Uint32 bitrate;
	Uint32 framerate;
} stChangeRcPara;

/*
 * The firmware version is retrieved from bitcode table.
 *
 * The library version convention:
 * lib_major increases when a new platform support is added
 * lib_minor increases when one firmware is upgraded
 * lib_release increases when bug fixes, excluding the above cases
 */
typedef struct vpu_versioninfo {
	int fw_major;		/* firmware major version */
	int fw_minor;		/* firmware minor version */
	int fw_release;		/* firmware release version */
	int fw_code;		/* firmware checkin code number */
	int lib_major;		/* library major version */
	int lib_minor;		/* library minor version */
	int lib_release;	/* library release version */
} vpu_versioninfo;

#define VPU_FW_VERSION(major, minor, release)	 \
	(((major) << 12) + ((minor) << 8) + (release))

#define VPU_LIB_VERSION(major, minor, release)	 \
	(((major) << 12) + ((minor) << 8) + (release))

/*
 * Revision History:
 * v5.3.0 [2011.07.30] Add mx6q vpu support
 * v5.2.0 [2011.07.04] Upgrading mx5x f/w to infinite instances support
 * v5.1.5 [2011.06.16] Remove code for not mx5x platforms
 * v5.0.1 [2010.03.03] Integrate mx53 vpu
 * v4.7.1 [2009.09.18] remove share memory file and update SWReset function
 * v4.7.0 [2009.08.03] upgrade mx51 fw to v1.2.0
 * v4.3.2 [2008.10.28] support loopback on MX51
 * v4.2.2 [2008.09.03] support encoder on MX51
 * v4.0.2 [2008.08.21] add the IOClkGateSet() for power saving.
 */
#define VPU_LIB_VERSION_CODE	VPU_LIB_VERSION(5, 4, 16)

extern unsigned int system_rev;

#define CHIP_REV_1_0            	0x10
#define CHIP_REV_2_0			0x20
#define CHIP_REV_2_1            	0x21

#define mxc_cpu()               (system_rev >> 12)
#define mxc_is_cpu(part)        ((mxc_cpu() == (unsigned int)part) ? 1 : 0)
#define mxc_cpu_rev()           (system_rev & 0xFF)
#define mxc_cpu_is_rev(rev)     \
        ((mxc_cpu_rev() == (unsigned int)rev) ? 1 : ((mxc_cpu_rev() < (unsigned int)rev) ? -1 : 2))
#define MXC_REV(type)                           \
static inline int type## _rev (int rev)         \
{                                               \
        return (type() ? mxc_cpu_is_rev(rev) : 0);      \
}

#define cpu_is_mx27()		mxc_is_cpu(0x27)
#define cpu_is_mx51()		mxc_is_cpu(0x51)
#define cpu_is_mx53()		mxc_is_cpu(0x53)
#define cpu_is_mx5x()		(mxc_is_cpu(0x51) || mxc_is_cpu(0x53))
#define cpu_is_mx6q()		mxc_is_cpu(0x63)
#define cpu_is_mx6dl()		mxc_is_cpu(0x61)
#define cpu_is_mx6x()		(mxc_is_cpu(0x63) || mxc_is_cpu(0x61))

MXC_REV(cpu_is_mx27);

RetCode vpu_Init(void *);
void vpu_UnInit(void);
RetCode vpu_GetVersionInfo(vpu_versioninfo * verinfo);

RetCode vpu_EncOpen(EncHandle *, EncOpenParam *);
RetCode vpu_EncClose(EncHandle);
RetCode vpu_EncGetInitialInfo(EncHandle, EncInitialInfo *);
RetCode vpu_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray,
				   int num, int frameBufStride, int sourceBufStride,
				   PhysicalAddress subSampBaseA, PhysicalAddress subSampBaseB,
				   EncExtBufInfo * pBufInfo);
RetCode vpu_EncGetBitstreamBuffer(EncHandle handle, PhysicalAddress * prdPrt,
				  PhysicalAddress * pwrPtr, Uint32 * size);
RetCode vpu_EncUpdateBitstreamBuffer(EncHandle handle, Uint32 size);
RetCode vpu_EncStartOneFrame(EncHandle handle, EncParam * param);
RetCode vpu_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info);
RetCode vpu_EncGiveCommand(EncHandle handle, CodecCommand cmd, void *parameter);

RetCode vpu_DecOpen(DecHandle *, DecOpenParam *);
RetCode vpu_DecClose(DecHandle);
RetCode vpu_DecSetEscSeqInit(DecHandle handle, int escape);
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info);
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,
				   FrameBuffer * bufArray, int num, int stride,
				   DecBufInfo * pBufInfo);
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle, PhysicalAddress * paRdPtr,
				  PhysicalAddress * paWrPtr, Uint32 * size);
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size);
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam * param);
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info);
RetCode vpu_DecBitBufferFlush(DecHandle handle);
RetCode vpu_DecClrDispFlag(DecHandle handle, int index);
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *parameter);

int vpu_IsBusy(void);
int jpu_IsBusy(void);
int vpu_WaitForInt(int timeout_in_ms);
RetCode vpu_SWReset(DecHandle handle, int index);
int vpu_GetXY2AXIAddr(DecHandle handle, int ycbcr, int posY, int posX, int stride,
                   unsigned int addrY, unsigned int addrCb, unsigned int addrCr);

void SaveGetEncodeHeader(EncHandle handle, int encHeaderType, char *filename);

#endif
