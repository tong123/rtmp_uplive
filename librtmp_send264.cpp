/**
 * Simplest Librtmp Send 264
 *
 * 雷霄骅，张晖
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序用于将内存中的H.264数据推送至RTMP流媒体服务器。
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "librtmp_send264.h"
#include "rtmp.h"
//#include "rtmp_sys.h"
#include "amf.h"
#include "sps_decode.h"
#include "unistd.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>

#ifdef WIN32     
#include <windows.h>  
#pragma comment(lib,"WS2_32.lib")   
#pragma comment(lib,"winmm.lib")  
#endif 
#define msleep(x) usleep(x*1000)
//定义包头长度，RTMP_MAX_HEADER_SIZE=18
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
//存储Nal单元数据的buffer大小
#define BUFFER_SIZE 32768
//搜寻Nal单元时的一些标志
#define GOT_A_NAL_CROSS_BUFFER BUFFER_SIZE+1
#define GOT_A_NAL_INCLUDE_A_BUFFER BUFFER_SIZE+2
#define NO_MORE_BUFFER_TO_READ BUFFER_SIZE+3
unsigned int n_first_buff_size = 0;
bool b_first_read = false;
/**
 * _NaluUnit
 * 内部结构体。该结构体主要用于存储和传递Nal单元的类型、大小和数据
 */ 
typedef struct _NaluUnit  
{  
	int type;  
    int size;  
	unsigned char *data;  
}NaluUnit;

/**
 * _RTMPMetadata
 * 内部结构体。该结构体主要用于存储和传递元数据信息
 */ 
typedef struct _RTMPMetadata  
{  
	// video, must be h264 type   
	unsigned int    nWidth;  
	unsigned int    nHeight;  
	unsigned int    nFrameRate;      
	unsigned int    nSpsLen;  
	unsigned char   *Sps;  
	unsigned int    nPpsLen;  
	unsigned char   *Pps;   
} RTMPMetadata,*LPRTMPMetadata;  

enum  
{  
	 VIDEO_CODECID_H264 = 7,  
};  

/**
 * 初始化winsock
 *					
 * @成功则返回1 , 失败则返回相应错误代码
 */ 
int InitSockets()    
{    
	#ifdef WIN32     
		WORD version;    
		WSADATA wsaData;    
		version = MAKEWORD(1, 1);    
		return (WSAStartup(version, &wsaData) == 0);    
	#else     
		return TRUE;    
	#endif     
}

/**
 * 释放winsock
 *					
 * @成功则返回0 , 失败则返回相应错误代码
 */ 
inline void CleanupSockets()    
{    
	#ifdef WIN32     
		WSACleanup();    
	#endif     
}    

//网络字节序转换
char * put_byte( char *output, uint8_t nVal )    
{    
	output[0] = nVal;    
	return output+1;    
}   

char * put_be16(char *output, uint16_t nVal )    
{    
	output[1] = nVal & 0xff;    
	output[0] = nVal >> 8;    
	return output+2;    
}  

char * put_be24(char *output,uint32_t nVal )    
{    
	output[2] = nVal & 0xff;    
	output[1] = nVal >> 8;    
	output[0] = nVal >> 16;    
	return output+3;    
}    
char * put_be32(char *output, uint32_t nVal )    
{    
	output[3] = nVal & 0xff;    
	output[2] = nVal >> 8;    
	output[1] = nVal >> 16;    
	output[0] = nVal >> 24;    
	return output+4;    
}    
char *  put_be64( char *output, uint64_t nVal )    
{    
	output=put_be32( output, nVal >> 32 );    
	output=put_be32( output, nVal );    
	return output;    
}  

char * put_amf_string( char *c, const char *str )    
{    
	uint16_t len = strlen( str );    
	c=put_be16( c, len );    
	memcpy(c,str,len);    
	return c+len;    
}    
char * put_amf_double( char *c, double d )    
{    
	*c++ = AMF_NUMBER;  /* type: Number */    
	{    
		unsigned char *ci, *co;    
		ci = (unsigned char *)&d;    
		co = (unsigned char *)c;    
		co[0] = ci[7];    
		co[1] = ci[6];    
		co[2] = ci[5];    
		co[3] = ci[4];    
		co[4] = ci[3];    
		co[5] = ci[2];    
		co[6] = ci[1];    
		co[7] = ci[0];    
	}    
	return c+8;    
}  


unsigned int  m_nFileBufSize; 
unsigned int  nalhead_pos;
RTMP* m_pRtmp;  
RTMPMetadata metaData;
unsigned char *m_pFileBuf;  
unsigned char *m_pFileBuf_tmp;
unsigned char* m_pFileBuf_tmp_old;	//used for realloc
unsigned char *m_sps_data;
unsigned char *m_pps_data;
unsigned char *m_idr_data;
unsigned int n_sps_size;
unsigned int n_pps_size;
unsigned int n_idr_size;
unsigned int n_frame_rate;
/**
 * 初始化并连接到服务器
 *
 * @param url 服务器上对应webapp的地址
 *					
 * @成功则返回1 , 失败则返回0
 */ 
int RTMP264_Connect(const char* url)  
{  
	nalhead_pos=0;
	m_nFileBufSize=BUFFER_SIZE;
	m_pFileBuf=(unsigned char*)malloc(BUFFER_SIZE);
	m_pFileBuf_tmp=(unsigned char*)malloc(BUFFER_SIZE);
    m_sps_data = ( unsigned char *)malloc(100);
    m_pps_data = ( unsigned char *)malloc(100);
    m_idr_data = ( unsigned char *)malloc(640*480*2);

	InitSockets();  

	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
	/*设置URL*/
	if (RTMP_SetupURL(m_pRtmp,(char*)url) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		return false;
	}
	/*设置可写,即发布流,这个函数必须在连接前使用,否则无效*/
	RTMP_EnableWrite(m_pRtmp);
	/*连接服务器*/
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE) 
	{
		RTMP_Free(m_pRtmp);
		return false;
	} 

	/*连接流*/
	if (RTMP_ConnectStream(m_pRtmp,0) == FALSE)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		return false;
	}
	return true;  
}  


/**
 * 断开连接，释放相关的资源。
 *
 */    
void RTMP264_Close()  
{  
	if(m_pRtmp)  
	{  
		RTMP_Close(m_pRtmp);  
		RTMP_Free(m_pRtmp);  
		m_pRtmp = NULL;  
	}  
	CleanupSockets();   
	if (m_pFileBuf != NULL)
	{  
		free(m_pFileBuf);
	}  
	if (m_pFileBuf_tmp != NULL)
	{  
		free(m_pFileBuf_tmp);
	}
} 

/**
 * 发送RTMP数据包
 *
 * @param nPacketType 数据类型
 * @param data 存储数据内容
 * @param size 数据大小
 * @param nTimestamp 当前包的时间戳
 *
 * @成功则返回 1 , 失败则返回一个小于0的数
 */
int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp)  
{  
	RTMPPacket* packet;
	/*分配包内存和初始化,len为包体长度*/
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size);
	memset(packet,0,RTMP_HEAD_SIZE);
	/*包体内存*/
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
	memcpy(packet->m_body,data,size);
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = nPacketType; /*此处为类型有两种一种是音频,一种是视频*/
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;
	packet->m_nChannel = 0x04;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	if (RTMP_PACKET_TYPE_AUDIO ==nPacketType && size !=4)
	{
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}
	packet->m_nTimeStamp = nTimestamp;
	/*发送*/
	int nRet =0;
	if (RTMP_IsConnected(m_pRtmp))
	{
		nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
	}
	/*释放内存*/
	free(packet);
	return nRet;  
}  

/**
 * 发送视频的sps和pps信息
 *
 * @param pps 存储视频的pps信息
 * @param pps_len 视频的pps信息长度
 * @param sps 存储视频的pps信息
 * @param sps_len 视频的sps信息长度
 *
 * @成功则返回 1 , 失败则返回0
 */
int SendVideoSpsPps(unsigned char *pps,int pps_len,unsigned char * sps,int sps_len)
{
	RTMPPacket * packet=NULL;//rtmp包结构
	unsigned char * body=NULL;
	int i;
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
	//RTMPPacket_Reset(packet);//重置packet状态
	memset(packet,0,RTMP_HEAD_SIZE+1024);
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (unsigned char *)packet->m_body;
	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++]   = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i],sps,sps_len);
	i +=  sps_len;

	/*pps*/
	body[i++]   = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i],pps,pps_len);
	i +=  pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;

	/*调用发送接口*/
	int nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE);
	free(packet);    //释放内存
	return nRet;
}

/**
 * 发送H264数据帧
 *
 * @param data 存储数据帧内容
 * @param size 数据帧的大小
 * @param bIsKeyFrame 记录该帧是否为关键帧
 * @param nTimeStamp 当前帧的时间戳
 *
 * @成功则返回 1 , 失败则返回0
 */
int SendH264Packet(unsigned char *data,unsigned int size,int bIsKeyFrame,unsigned int nTimeStamp)  
{  
    qDebug()<<"SendH264Packet: "<<data<<size;
	if(data == NULL && size<11){  
		return false;  
	}  

	unsigned char *body = (unsigned char*)malloc(size+9);  
	memset(body,0,size+9);

	int i = 0; 
	if(bIsKeyFrame){  
		body[i++] = 0x17;// 1:Iframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;  


		// NALU size   
		body[i++] = size>>24 &0xff;  
		body[i++] = size>>16 &0xff;  
		body[i++] = size>>8 &0xff;  
		body[i++] = size&0xff;
		// NALU data   
		memcpy(&body[i],data,size);  
		SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
	}else{  
		body[i++] = 0x27;// 2:Pframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;  


		// NALU size   
		body[i++] = size>>24 &0xff;  
		body[i++] = size>>16 &0xff;  
		body[i++] = size>>8 &0xff;  
		body[i++] = size&0xff;
		// NALU data   
		memcpy(&body[i],data,size);  
	}  
	

	int bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,body,i+size,nTimeStamp);  

	free(body);  
    qDebug()<<"SendH264Packet: "<<bRet;
	return bRet;  
} 

/**
 * 从内存中读取出第一个Nal单元
 *
 * @param nalu 存储nalu数据
 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据。
 *					2个参数功能：
 *					uint8_t *buf：外部数据送至该地址
 *					int buf_size：外部数据大小
 *					返回值：成功读取的内存大小
 * @成功则返回 1 , 失败则返回0
 */

void set_sps_into_buffer( unsigned char *buf, unsigned int buf_size )
{
    memcpy( m_sps_data, buf, buf_size );
    n_sps_size = buf_size;
}

void set_pps_into_buffer( unsigned char *buf, unsigned int buf_size )
{
    memcpy( m_pps_data, buf, buf_size );
    n_pps_size = buf_size;
}

void set_idr_into_buffer( unsigned char *buf, unsigned int buf_size )
{
    memcpy( m_idr_data, buf, buf_size );
    n_idr_size = buf_size;
}

int ReadSPSNaluFromBuf( NaluUnit &nalu )
{
    nalhead_pos = 0;
    if(nalhead_pos<n_sps_size)
    {
        if(m_sps_data[nalhead_pos++] == 0x00 &&
            m_sps_data[nalhead_pos++] == 0x00)
        {
            if(m_sps_data[nalhead_pos++] == 0x01)
                goto gotsps_head;
            else
            {
                //cuz we have done an i++ before,so we need to roll back now
                nalhead_pos--;
                if(m_sps_data[nalhead_pos++] == 0x00 &&
                    m_sps_data[nalhead_pos++] == 0x01)
                    goto gotsps_head;
                else
                    return -1;
            }
        } else
            return -1 ;
    } else {
        return -1;
    }
        //search for nal tail which is also the head of next nal
gotsps_head:
    nalu.type = m_sps_data[nalhead_pos]&0x1f;
    nalu.size = n_sps_size-nalhead_pos;
    qDebug()<<"ReadFirstNaluFromBuf: type, size"<<nalu.type<<nalu.size;
    nalu.data=m_sps_data+nalhead_pos;
    return 0;
}

int ReadPPSNaluFromBuf( NaluUnit &nalu )
{
    nalhead_pos = 0;
    if(nalhead_pos<n_pps_size)
    {
        if(m_pps_data[nalhead_pos++] == 0x00 &&
            m_pps_data[nalhead_pos++] == 0x00) {
            if(m_pps_data[nalhead_pos++] == 0x01)
                goto gotsps_head;
            else {
                //cuz we have done an i++ before,so we need to roll back now
                nalhead_pos--;
                if(m_pps_data[nalhead_pos++] == 0x00 &&
                    m_pps_data[nalhead_pos++] == 0x01)
                    goto gotsps_head;
                else
                    return -1;
            }
        } else
            return -1;
    } else {
        return -1;
    }
        //search for nal tail which is also the head of next nal
gotsps_head:
    nalu.type = m_pps_data[nalhead_pos]&0x1f;
    nalu.size = n_pps_size-nalhead_pos;
    qDebug()<<"ReadFirstNaluFromBuf: type, size"<<nalu.type<<nalu.size;
    nalu.data=m_pps_data+nalhead_pos;
    return 0;
}

int ReadIDRNaluFromBuf( NaluUnit &nalu )
{
    nalhead_pos = 0;
    if(nalhead_pos<n_idr_size)
    {
        if(m_idr_data[nalhead_pos++] == 0x00 &&
            m_idr_data[nalhead_pos++] == 0x00) {
            if(m_idr_data[nalhead_pos++] == 0x01)
                goto gotsps_head;
            else {
                //cuz we have done an i++ before,so we need to roll back now
                nalhead_pos--;
                if(m_idr_data[nalhead_pos++] == 0x00 &&
                    m_idr_data[nalhead_pos++] == 0x01)
                    goto gotsps_head;
                else
                    return -1;
            }
        } else
            return -1;
    } else {
        return -1;
    }
        //search for nal tail which is also the head of next nal
gotsps_head:
    nalu.type = m_idr_data[nalhead_pos]&0x1f;
    nalu.size = n_idr_size-nalhead_pos;
    qDebug()<<"ReadFirstNaluFromBuf: type, size"<<nalu.type<<nalu.size;
    nalu.data=m_idr_data+nalhead_pos;
    return 0;
}

/**
 * 从内存中读取出一个Nal单元
 *
 * @param nalu 存储nalu数据
 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据。
 *					2个参数功能：
 *					uint8_t *buf：外部数据送至该地址
 *					int buf_size：外部数据大小
 *					返回值：成功读取的内存大小
 * @成功则返回 1 , 失败则返回0
 */

int ReadOneNaluFromBuf(NaluUnit &nalu,int (*read_buffer)(uint8_t *buf, int buf_size))
{
    memset(m_pFileBuf_tmp,0,BUFFER_SIZE);
    memset( m_pFileBuf, 0, BUFFER_SIZE );
    int naltail_pos=0;
    int nalustart = 0;
    int n_ret = read_buffer( m_pFileBuf, BUFFER_SIZE );
    qint64 start = QDateTime::currentMSecsSinceEpoch();

    while( n_ret == 0 ) {
//        QThread::usleep(100);
        n_ret = read_buffer( m_pFileBuf, BUFFER_SIZE );
    }
    qint64 end = QDateTime::currentMSecsSinceEpoch();
    qDebug()<<"ReadOneNaluFromBuf end-start"<<end-start;
    qDebug()<<"read_buffer: n_ret "<<n_ret;
    qDebug()<<m_pFileBuf[0]<<m_pFileBuf[1]<<m_pFileBuf[2]<<m_pFileBuf[3]<<m_pFileBuf[4]<<m_pFileBuf[5];
    if(m_pFileBuf[naltail_pos++] == 0x00 ) {
        if( naltail_pos<m_nFileBufSize && m_pFileBuf[naltail_pos++] == 0x00 ) {
            if( naltail_pos<m_nFileBufSize && m_pFileBuf[naltail_pos++] == 0x01) {
                nalustart=3;
                goto gotnal ;
            } else {
                //cuz we have done an i++ before,so we need to roll back now
                naltail_pos--;
                if(naltail_pos<m_nFileBufSize && m_pFileBuf[naltail_pos++] == 0x00) {
                    if( naltail_pos<m_nFileBufSize && m_pFileBuf[naltail_pos++] == 0x01 ) {
                        nalustart=4;
                        goto gotnal;
                    } else {
                        return -1;
                    }
                } else {
                    return -1;
                }
            }
        } else{
            return -1;
        }
    } else {
        return -1;
    }
gotnal:
    qDebug()<<"read_buffer: n_ret,  nalustart: "<<n_ret<<nalustart;
    nalu.type = m_pFileBuf[nalustart]&0x1f;
    nalu.size=n_ret-nalustart;
    memcpy(m_pFileBuf_tmp,m_pFileBuf+nalustart,nalu.size);
    nalu.data=m_pFileBuf_tmp;
    return 0;
}

bool get_first_read_status( )
{
    return b_first_read;
}

void set_first_buff_size( int n_value )
{
    n_first_buff_size = n_value;
}

void set_frame_rate( int n_value )
{
    n_frame_rate = n_value;
}

/**
 * 将内存中的一段H.264编码的视频数据利用RTMP协议发送到服务器
 *
 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据。
 *					2个参数功能：
 *					uint8_t *buf：外部数据送至该地址
 *					int buf_size：外部数据大小
 *					返回值：成功读取的内存大小
 * @成功则返回1 , 失败则返回0
 */ 
int RTMP264_Send(int (*read_buffer)(unsigned char *buf, int buf_size))  
{    
	int ret;
	uint32_t now,last_update;
	  
	memset(&metaData,0,sizeof(RTMPMetadata));
	memset(m_pFileBuf,0,BUFFER_SIZE);
    b_first_read = true;
    while( n_first_buff_size == 0 ) {
        msleep(1);
    }
	NaluUnit naluUnit;  
	// 读取SPS帧   
    qDebug()<<"RTMP264_Send: ReadFirstNaluFromBuf==";

    ReadSPSNaluFromBuf( naluUnit );
	metaData.nSpsLen = naluUnit.size;  
	metaData.Sps=NULL;
	metaData.Sps=(unsigned char*)malloc(naluUnit.size);
	memcpy(metaData.Sps,naluUnit.data,naluUnit.size);
    qDebug()<<"nSpsLen: "<<naluUnit.size;
	// 读取PPS帧   
    ReadPPSNaluFromBuf( naluUnit );

	metaData.nPpsLen = naluUnit.size; 
	metaData.Pps=NULL;
	metaData.Pps=(unsigned char*)malloc(naluUnit.size);
	memcpy(metaData.Pps,naluUnit.data,naluUnit.size);
    qDebug()<<"nPpsLen: "<<naluUnit.size;

	// 解码SPS,获取视频图像宽、高信息   
	int width = 0,height = 0, fps=0;  
	h264_decode_sps(metaData.Sps,metaData.nSpsLen,width,height,fps);  
    qDebug()<<"RTMP264_Send: "<<width<<height;
	//metaData.nWidth = width;  
	//metaData.nHeight = height;  
	if(fps)
		metaData.nFrameRate = fps; 
	else
		metaData.nFrameRate = 25;
    metaData.nFrameRate = 10;
    n_frame_rate = 10;
	//发送PPS,SPS
	//ret=SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
	//if(ret!=1)
	//	return FALSE;
#if 1
	unsigned int tick = 0;  
	unsigned int tick_gap = 1000/metaData.nFrameRate; 
    ReadIDRNaluFromBuf( naluUnit );
	int bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
    int n_ret = SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick);
    while(n_ret)
	{    
got_sps_pps:
        printf("NALU size:%8d\n",naluUnit.size);
        qDebug()<<"NALU type: "<<naluUnit.type;
		last_update=RTMP_GetTime();
        qDebug()<<ReadOneNaluFromBuf(naluUnit,read_buffer);
        if(naluUnit.type == 0x07 || naluUnit.type == 0x08) {
            msleep(40);
			goto got_sps_pps;
        }
		bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
        if( n_frame_rate !=0 ) {
            tick_gap = 1000/n_frame_rate;
        } else {
            tick_gap = 1000;
        }
        tick +=tick_gap;
		now=RTMP_GetTime();
        qDebug()<<"msleep: "<<tick_gap+now-last_update<<bKeyframe;
//        QThread::usleep( tick_gap+now-last_update );
        n_ret = SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick);
    }
    qDebug()<<"run end";
	end:
	free(metaData.Sps);
	free(metaData.Pps);
#endif
	return TRUE;  
}  


