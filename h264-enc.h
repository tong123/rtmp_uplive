/** 
 * @file h264-enc.h
 * @brief 
 * @author wilge
 * @version 1.0.0
 * @date 2014-10-14
 */


#ifndef     __H264_ENC_H__
#define     __H264_ENC_H__
/** 
 * @brief 初始化vpu设备,以及相关参数
 */
extern "C" {
int h264_init ( void );

/** 
 * @brief 相关参数信息释放
 */
void h264_exit ( void );


/** 
 * @brief 执行h264 编码工作
 * 
 * @param src_buf  输入缓存yuv420p buf
 * @param dest_buf 输出编码后缓存
 * @param src_size 输入缓存大小
 */
int h264_encode ( void *src_buf, void *dest_buf, int src_size ) ;

/** 
 * @brief 获取文件头信息
 * 
 * @param head_buf [output] 数据缓存
 * 
 * @return 数据长度
 */
//int get_encode_file_head ( void *head_buf );
int get_encode_file_head ( void *sps_buf, int *n_sps_size, void *pps_buf,  int *n_pps_size );

/** 
 * @brief 测试demo
 * 
 * @return 
 */
int h264_test ( void );
}
#endif /*    __H264_ENC_H__ */
