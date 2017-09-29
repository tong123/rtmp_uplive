/**
 * @file h264-enc.c
 * @brief use imx6 vpu do h264 encode test
 * 1. get options : input file , output file,
 * 2. init
 * 3. do encode
 * 4. output
 * @author wilge
 * @version 1.0.0
 * @date 2014-09-30
 */

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <semaphore.h>
#include "vpu_lib.h"
#include "vpu_io.h"

#define STREAM_BUF_SIZE		0x200000
//#define FRAME_BUF_SIZE      (518400) //( 720 * 480 * 3 / 2 )
#define FRAME_BUF_SIZE      (460800)

static int vpu_test_dbg_level = 0;
#if 0
#define dprintf(level, fmt, arg...)     if (vpu_test_dbg_level >= level) \
        printf("[DEBUG]\t%s:%d " fmt, __FILE__, __LINE__, ## arg)

#define err_msg(fmt, arg...) do { if (vpu_test_dbg_level >= 1)		\
    printf("[ERR]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); else \
    printf("[ERR]\t" fmt, ## arg);	\
    } while (0)
#define info_msg(fmt, arg...) do { if (vpu_test_dbg_level >= 1)		\
    printf("[INFO]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); else \
    printf("[INFO]\t" fmt, ## arg);	\
    } while (0)
#define warn_msg(fmt, arg...) do { if (vpu_test_dbg_level >= 1)		\
    printf("[WARN]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); else \
    printf("[WARN]\t" fmt, ## arg);	\
    } while (0)
#else

#define dprintf(level, fmt, arg...)
#define err_msg(fmt, arg...)
#define info_msg(fmt, arg...)
#define warn_msg(fmt, arg...)

#endif

static EncHandle handle = {0};
static EncOpenParam encop = {0};
static EncParam  enc_param = {0};
static EncOutputInfo outinfo = {0};
static EncInitialInfo initinfo = {0};
static EncExtBufInfo extbufinfo = {0};
static struct vpu_mem_desc enc_mem_desc;
static struct vpu_mem_desc *fb_mem_desc_array;
static FrameBuffer *fb_array;
static int fb_count;
static int f_yuv420;                                                                // 输入文件句柄
static int f_h264;                                                                  // 输出文件句柄
static unsigned char yuv_buf [ FRAME_BUF_SIZE ];
static unsigned char h264_buf [ FRAME_BUF_SIZE ];


static int framebuffer_alloc ( int cnt, int strideY, int height)
{
    int err;
    int divX, divY;
    int i;
    FrameBuffer *fb;

    info_msg(" function: %s \n", __FUNCTION__ );


    divX = divY = 2;
    fb_array = calloc( cnt, sizeof ( FrameBuffer ) );
    fb_mem_desc_array = calloc ( cnt, sizeof ( struct vpu_mem_desc ) );

    for ( i = 0; i < cnt; i ++ ) {
        /* alloc framebuffer memory */
        fb_mem_desc_array [ i ].size = ( strideY * height  + strideY / divX * height / divY * 2 );
        err = IOGetPhyMem ( fb_mem_desc_array + i );
        if ( err ) {
            printf ( " [ %d ] :Frame buffer allocation failure ! \n",  i );
            return -1;
        }

        fb_mem_desc_array [ i ].virt_uaddr = IOGetVirtMem ( fb_mem_desc_array + i ) ;
        if ( fb_mem_desc_array [ i ].virt_uaddr <= 0 ) {
            IOFreePhyMem ( &fb_mem_desc_array [ i ] );
            return -1;
        }

        /* init frame buffer */
        fb = fb_array + i;
        fb->myIndex = i;
        fb->strideY = strideY;
        fb->strideC = strideY / divX;
        fb->bufY = fb_mem_desc_array [ i ].phy_addr;
        fb->bufCr = fb->bufY + strideY * height;
        fb->bufCb = fb->bufCr + strideY / divX * height / divY;
    }

    return 0;
}


/**
 * @brief release allocated memory
 */
static void framebuffer_free ( void )
{
    int i;

    info_msg(" function: %s \n", __FUNCTION__ );

    for ( i = 0; i < fb_count; i++ ) {
        IOFreeVirtMem ( fb_mem_desc_array + i );
        IOFreePhyMem ( fb_mem_desc_array + i );
    }

    free ( fb_array );
    free ( fb_mem_desc_array );
}

static int encoder_open( void )
{
    int i;
    int err;
    RetCode ret;


    info_msg(" function: %s \n", __FUNCTION__ );
    /* Fill up parameters for encoding */
    enc_mem_desc.size = STREAM_BUF_SIZE;                                            // 申请缓存
    err = IOGetPhyMem ( &enc_mem_desc );
    enc_mem_desc.virt_uaddr = IOGetVirtMem ( &enc_mem_desc ) ;

    encop.bitstreamBuffer = enc_mem_desc.phy_addr;                                  // 比特流数据缓存 ,4字节对齐
    encop.bitstreamBufferSize = STREAM_BUF_SIZE;                                    // 缓存大小 必须是1024的倍数

    encop.bitstreamFormat = STD_AVC;                                                // 编码格式为h264
    encop.mapType = LINEAR_FRAME_MAP;                                               // 线性映射
    /* encop.linear2TiledEnable = enc->linear2TiledEnable; */
    encop.linear2TiledEnable = 0;                                                   // 允许vpu将线性转为平铺格式， 0 不允许转化

    /* width and height in command line means source image size */
    encop.picWidth = 640;                                                           // 图片的宽度
    encop.picHeight = 480;                                                          // 图片的高度

    /*Note: Frame rate cannot be less than 15fps per H.263 spec */
    encop.frameRateInfo = 25;                                                       // 帧率
    encop.bitRate = 0;                                                              // 目标比特率， 0表示不控制比特率
    /* encop.gopSize = enc->cmdl->gop; */
    encop.gopSize = 5;
    encop.slicemode.sliceMode = 0;	/* 0: 1 slice per picture; 1: Multiple slices per picture */
    encop.slicemode.sliceSizeMode = 0; /* 0: silceSize defined by bits; 1: sliceSize defined by MB number*/
    encop.slicemode.sliceSize = 4000;  /* Size of a slice in bits or MB numbers */

    encop.initialDelay = 0;                                                         //
    encop.vbvBufferSize = 0;        /* 0 = ignore 8 */                              // 该参数被忽略
    encop.intraRefresh = 0;                                                         // 内部宏块未使用
    encop.sliceReport = 0;                                                          // imx6 不支持
    encop.mbReport = 0;                                                             // imx6 不支持
    encop.mbQpReport = 0;                                                           // imx6 不支持
    encop.rcIntraQp = -1;                                                           // 量化参数由vpu来决定
    encop.userQpMax = 0;
    encop.userQpMin = 0;
    encop.userQpMinEnable = 0;
    encop.userQpMaxEnable = 0;

    encop.IntraCostWeight = 0;                                                      // 内部加权值， 默认为0
    encop.MEUseZeroPmv  = 0;                                                        // 0 会比1有更好的编码质量
    /* (3: 16x16, 2:32x16, 1:64x32, 0:128x64, H.263(Short Header : always 3) */
    encop.MESearchRange = 3;

    encop.userGamma = (Uint32)(0.75*32768);         /*  (0*32768 <= gamma <= 1*32768) */ // 平滑度的评估值， 默认为0.75 × 32768
    encop.RcIntervalMode= 1;        /* 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level */ // 编码率的控制模式
    encop.MbInterval = 0;                                                           // 内部宏块数目
    encop.avcIntra16x16OnlyModeEnable = 0;                                          // 16x16帧内预测块， 0 不使用该模式

    /* encop.ringBufferEnable = enc->ringBufferEnable = 0; */
    encop.ringBufferEnable = 0;                                                     // 0 不使用环形缓存， 使用基于帧的线性缓存
    encop.dynamicAllocEnable = 0;                                                   // imx6 不支持
    /* encop.chromaInterleave = enc->cmdl->chromaInterleave; */
    encop.chromaInterleave = 0;                                                     // CbCr 是否交叉， 1交错， 0不交错

    encop.EncStdParam.avcParam.avc_constrainedIntraPredFlag = 0;                    // 帧内预测允许， 0 disable， 1允许
    encop.EncStdParam.avcParam.avc_disableDeblk = 0;                                // 去块滤波禁止， 0 允许滤波， 1 禁止滤波， 2 禁止片滤波
    encop.EncStdParam.avcParam.avc_deblkFilterOffsetAlpha = 6;                      // 块滤波中Alpha的偏移值
    encop.EncStdParam.avcParam.avc_deblkFilterOffsetBeta = 0;                       // 块滤波中Beta的偏移值
    encop.EncStdParam.avcParam.avc_chromaQpOffset = 10;                             // 调节色度分量
    encop.EncStdParam.avcParam.avc_audEnable = 0;                                   // 允许插入 AUD RBSP， 0禁止插入， 1允许

    encop.EncStdParam.avcParam.interview_en = 0;                                    // 禁止访问预测， 0禁止，1允许
    encop.EncStdParam.avcParam.paraset_refresh_en = 0;                              // 禁止在固定框架前插入SPS/PPS
    encop.EncStdParam.avcParam.prefix_nal_en = 0;                                   // 禁止在第二个MVC视图前添加NAL前缀
    encop.EncStdParam.avcParam.mvc_extension = 0;                                   // imx6 不支持mvc 编码, 0=avc, 1=mvc
    encop.EncStdParam.avcParam.avc_frameCroppingFlag = 0;                           // 0 不允许裁剪，1允许裁剪
    encop.EncStdParam.avcParam.avc_frameCropLeft = 0;                               // 左侧裁剪区域
    encop.EncStdParam.avcParam.avc_frameCropRight = 0;                              // 右侧裁剪区域
    encop.EncStdParam.avcParam.avc_frameCropTop = 0;                                // 顶部裁剪行数
    encop.EncStdParam.avcParam.avc_frameCropBottom = 0;                             // 底部裁剪行数
    encop.EncStdParam.avcParam.avc_fmoEnable = 0,                                   // 0 禁止fmo， imx6 不支持fmo
    encop.EncStdParam.avcParam.avc_fmoSliceNum = 0,
    encop.EncStdParam.avcParam.avc_fmoType = 0,
    encop.EncStdParam.avcParam.avc_fmoSliceSaveBufSize = 0,

    ret = vpu_EncOpen(&handle, &encop);
    if (ret != RETCODE_SUCCESS) {
        err_msg("Encoder open failed %d\n", ret);
        return -1;
    }

    return 0;
}

/**
 * @brief 获取文件头信息
 *
 * @param head_buf [output] 数据缓存
 *
 * @return 数据长度
 */
//int get_encode_file_head ( void *head_buf )
//{
//    EncHeaderParam enchdr_param = {0};
//    void *ptr_buf = head_buf;
//    int size = 0;

//    if ( head_buf == NULL )
//        return 0;

//    enchdr_param.headerType = SPS_RBSP;
//    vpu_EncGiveCommand(handle, ENC_PUT_AVC_HEADER, &enchdr_param);
//    memcpy ( ptr_buf,
//            enc_mem_desc.virt_uaddr + enchdr_param.buf - enc_mem_desc.phy_addr,
//            enchdr_param.size );
//    ptr_buf += enchdr_param.size;
//    size += enchdr_param.size;
//    printf ( "SPS size=%d\n", size );
//    enchdr_param.headerType = PPS_RBSP;
//    vpu_EncGiveCommand(handle, ENC_PUT_AVC_HEADER, &enchdr_param);
//    memcpy ( ptr_buf,
//            enc_mem_desc.virt_uaddr + enchdr_param.buf - enc_mem_desc.phy_addr,
//            enchdr_param.size );
//    size += enchdr_param.size;
//    printf ( "PPS size=%d\n", enchdr_param.size );

//    return size;
//}

int get_encode_file_head( void *sps_buf, int *n_sps_size, void *pps_buf,  int *n_pps_size )
{
    EncHeaderParam enchdr_param = {0};
    void *sps_head_buf = sps_buf;
    void *pps_head_buf = pps_buf;

    int size = 0;

//    if ( head_buf == NULL )
//        return 0;

    enchdr_param.headerType = SPS_RBSP;
    vpu_EncGiveCommand(handle, ENC_PUT_AVC_HEADER, &enchdr_param);
    memcpy ( sps_head_buf,
            enc_mem_desc.virt_uaddr + enchdr_param.buf - enc_mem_desc.phy_addr,
            enchdr_param.size );
    *n_sps_size = enchdr_param.size;
    printf ( "SPS size=%d\n", size );
    enchdr_param.headerType = PPS_RBSP;
    vpu_EncGiveCommand(handle, ENC_PUT_AVC_HEADER, &enchdr_param);
    memcpy ( pps_head_buf,
            enc_mem_desc.virt_uaddr + enchdr_param.buf - enc_mem_desc.phy_addr,
            enchdr_param.size );
    *n_pps_size = enchdr_param.size;
    printf ( "PPS size=%d\n", enchdr_param.size );

    return size;
}


static void write_encode_file_head ( void )
{
    int len;

//    len = get_encode_file_head ( h264_buf );
//    write ( f_h264, h264_buf, len );
}

/**
 * @brief 执行h264 编码工作
 *
 * @param src_buf  输入缓存yuv420p buf
 * @param dest_buf 输出编码后缓存
 * @param src_size 输入缓存大小
 */
int h264_encode ( void *src_buf, void *dest_buf, int src_size )
{
    int ret;
    int loop_id = 0;
    printf("hello encode size fb_count %d, %d\n", src_size, fb_count );
    info_msg(" function: %s \n", __FUNCTION__ );
    memcpy ( ( void * )fb_mem_desc_array [ fb_count - 1 ].virt_uaddr, src_buf, src_size ) ;
//    printf("hello encode size %d\n", src_size );

    ret = vpu_EncStartOneFrame(handle, &enc_param);
    if (ret != RETCODE_SUCCESS) {
        err_msg("vpu_EncStartOneFrame failed Err code:%d\n",
                ret);
        return 0;
    }

    loop_id = 0;
    while (vpu_IsBusy()) {
        vpu_WaitForInt(200);

        if (loop_id == 20) {
            ret = vpu_SWReset(handle, 0);
            return 0;
        }
        loop_id ++;
    }


    ret = vpu_EncGetOutputInfo(handle, &outinfo);
    if (ret != RETCODE_SUCCESS) {
        err_msg("vpu_EncGetOutputInfo failed Err code: %d\n",
                ret);
        return 0;
    }

//    printf("outinfo.type:======================%d\n", outinfo.picType );
    memcpy ( dest_buf, ( void * ) ( enc_mem_desc.virt_uaddr + outinfo.bitstreamBuffer - enc_mem_desc.phy_addr ),
            outinfo.bitstreamSize );

    return outinfo.bitstreamSize;
}

/**
 * @brief 初始化vpu设备,以及相关参数
 */
int h264_init ( void )
{
    vpu_mem_desc	mem_desc = {0};
    vpu_mem_desc    scratch_mem_desc = {0};
    vpu_versioninfo ver;
    int err;
    int ret;

    info_msg(" function: %s \n", __FUNCTION__ );
    err = vpu_Init(NULL);                                                           // vpu 初始化
    if (err) {
        err_msg("VPU Init Failure.\n");
        return -1;
    }

    err = vpu_GetVersionInfo(&ver);                                                 // 获取vpu版本信息
    if (err) {
        err_msg("Cannot get version info, err:%d\n", err);
        vpu_UnInit();
        return -1;
    } else {
        info_msg("VPU firmware version: %d.%d.%d_r%d\n", ver.fw_major, ver.fw_minor,
                ver.fw_release, ver.fw_code);
        info_msg("VPU library version: %d.%d.%d\n", ver.lib_major, ver.lib_minor,
                ver.lib_release);
    }

    usleep ( 100000 );                                                              //  wait 100ms

    encoder_open ( );                                                               // 打开编码器


    ret = vpu_EncGetInitialInfo(handle, &initinfo);                                 // 获取初始化信息
    if (ret != RETCODE_SUCCESS) {
        err_msg("Encoder GetInitialInfo failed\n");
        return -1;
    }

    /* framebuffer init */
    fb_count = initinfo.minFrameBufferCount + 2 + 1;                                // 最小帧缓存数目 + 额外数据 2 + 源数据帧 1
    if ( framebuffer_alloc ( fb_count, 640, 480 ) < 0 ) {                           // fail to alloc frame buffer
        return -1;
    }

    ret = vpu_EncRegisterFrameBuffer( handle, fb_array,                             // 注册帧缓存
            initinfo.minFrameBufferCount,
            640, 640,
            fb_array [ initinfo.minFrameBufferCount ].bufY,
            fb_array [ initinfo.minFrameBufferCount + 1 ].bufY,
            &extbufinfo);

    if (ret != RETCODE_SUCCESS) {
        err_msg("Register frame buffer failed\n");
        return -1;
    }

    enc_param.sourceFrame = &fb_array [ fb_count - 1 ];

    /* enc_param.sourceFrame = &enc->fb[src_fbid]; */
    enc_param.quantParam = 30;                                                      // 量化参数
    enc_param.forceIPicture = 0;                                                    // 0 图片格式由vpu决定， 1 强制当前图片为 I 图片
    enc_param.skipPicture = 0;                                                      // 0 正常进行编码， 1生成跳转图片
    enc_param.enableAutoSkip = 1;                                                   // 当bitRate=0时该参数被忽略

    enc_param.encLeftOffset = 0;                                                    // 左侧裁剪
    enc_param.encTopOffset = 0;                                                     // 顶部裁剪


//    write_encode_file_head ( );

    return 0;
}

/**
 * @brief 相关参数信息释放
 */
void h264_exit ( void )
{
    int ret;

    info_msg(" function: %s \n", __FUNCTION__ );

    ret = vpu_EncClose ( handle );
    if ( ret == RETCODE_FRAME_NOT_COMPLETE ) {
        vpu_SWReset ( handle, 0 );
        vpu_EncClose ( handle );
    }

    framebuffer_free ( );
    IOFreeVirtMem ( &enc_mem_desc );
    IOFreePhyMem ( &enc_mem_desc );

    vpu_UnInit();
}


/**
 * @brief 初始化相关参数
 */
void files_init ( void )
{
    info_msg(" function: %s \n", __FUNCTION__ );
    f_yuv420 = open ( "input.yuv", O_RDONLY );                                      // 输入的yuv文件
    f_h264 = open ( "output.264", O_RDWR | O_CREAT, S_IWUSR | S_IWOTH | S_IWGRP );  // 输出的h264文件
}

/**
 * @brief 释放相关参数
 */
void files_close ( void )
{
    info_msg(" function: %s \n", __FUNCTION__ );
    close ( f_yuv420 );
    close ( f_h264 );
}


/**
 * @brief 测试demo
 *
 * @return
 */
int h264_test ( void )
{

    int len = 0;
    int ret = 0;
    // read file

    files_init ( );
    h264_init ( );

    do {
        len = read ( f_yuv420, yuv_buf, FRAME_BUF_SIZE );
        if ( len == FRAME_BUF_SIZE ) {
            ret = h264_encode ( yuv_buf, h264_buf, FRAME_BUF_SIZE );
            if ( ret > 0 ) {
                write ( f_h264, h264_buf, ret );
            }
        }
    } while ( len == FRAME_BUF_SIZE && ret > 0 );

    h264_exit ( );

    files_close ( );

    return 0;
}
