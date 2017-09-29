#include "image-send.h"
#include <stdio.h>
#include "librtmp_send264.h"
#include <QMutex>

FILE *fp_send1;

//读文件的回调函数
//we use this callback function to read data from buffer
//int read_buffer1(unsigned char *buf, int buf_size ){
//    if(!feof(fp_send1)){
//        int true_size=fread(buf,1,buf_size,fp_send1);
//        return true_size;
//    }else{
//        return -1;
//    }
//}
#define BUFFER_SIZE 32768
#define VIDEO_BUFFER_SIZE 640 * 480 * 40 * 4

unsigned char *p_puc_buf = new unsigned char[2*BUFFER_SIZE];
unsigned char *p_tmp_puc_buf = new unsigned char[2*BUFFER_SIZE];

unsigned int n_buf_len = 0;
QMutex mutex;
SendThread *p_send_thread = NULL;

int read_buffer1(unsigned char *buf, int buf_size )
{
    int i_read_len = 0;
    p_send_thread->lock();
    i_read_len = p_send_thread->get_data(buf, buf_size);
    p_send_thread->unlock();
//    if (i_read_len == 0) { QThread::msleep(1); }
    return i_read_len;
}

ImageSend::ImageSend( ImageEncode *p_image_encode  )
{
    mp_send_thread = new SendThread( this );
    p_send_thread = mp_send_thread;
    mp_image_encode = p_image_encode;
    connect( mp_image_encode, SIGNAL( encode_image_changed(unsigned char*, unsigned int) ), this, SLOT( on_encode_image_changed_slot(unsigned char*, unsigned int) ) );
    b_first_start = true;

}

void ImageSend::on_encode_image_changed_slot( unsigned char *p_buf, unsigned int n_size )
{
    mp_send_thread->put_frame( p_buf, n_size );
}

SendThread::SendThread( ImageSend *p_image_send )
{
    mp_image_send = p_image_send;

    mst_media_buf_info.puc_buf = new unsigned char[VIDEO_BUFFER_SIZE];
    mst_media_buf_info.ui_buf_read_ptr = 0;
    mst_media_buf_info.ui_buf_write_ptr = 0;
    mst_media_buf_info.ui_real_data_len = 0;
    mst_media_buf_info.ui_buf_data_len = VIDEO_BUFFER_SIZE;

    n_count = 0;
//    init_uplive_thread( );
    start();
//    publish();
}

void SendThread::run()
{
    //初始化并连接到服务器
    RTMP264_Connect("rtmp://直播域名/live/stream");
    qDebug()<<"connect success";
    //发送
    RTMP264_Send(read_buffer1);
    //断开连接并释放相关资源
    RTMP264_Close();
}

unsigned int SendThread::get_used_data_len( )
{
    return mst_media_buf_info.ui_real_data_len;
}

unsigned int SendThread::get_remain_data_len()
{
    return (mst_media_buf_info.ui_buf_data_len - mst_media_buf_info.ui_real_data_len);
}

int SendThread::put_frame( unsigned char *puc_buf, unsigned int ui_data_len)
{
    unsigned char *puc_data_write_ptr = NULL;
    unsigned int ui_buf_left_len = 0;
    this->lock();
    qDebug()<<" put_frame=== "<<ui_data_len;
    if (ui_data_len > get_remain_data_len()) {
        this->unlock();
        return 1;
    }
    puc_data_write_ptr = &mst_media_buf_info.puc_buf[mst_media_buf_info.ui_buf_write_ptr];
    ui_buf_left_len = mst_media_buf_info.ui_buf_data_len - mst_media_buf_info.ui_buf_write_ptr;

    if (ui_data_len > ui_buf_left_len) {
        /* 循环写 */
        unsigned int ui_from_start_len = ui_data_len - ui_buf_left_len;

        memcpy(puc_data_write_ptr, puc_buf, ui_buf_left_len);

        puc_data_write_ptr = &mst_media_buf_info.puc_buf[0];
        memcpy(puc_data_write_ptr, puc_buf + ui_buf_left_len, ui_from_start_len);
    } else {
        memcpy(puc_data_write_ptr, puc_buf, ui_data_len);
    }
    mst_media_buf_info.ui_real_data_len += ui_data_len;
    mst_media_buf_info.ui_buf_write_ptr = (mst_media_buf_info.ui_buf_write_ptr + ui_data_len) % mst_media_buf_info.ui_buf_data_len;
    this->unlock();
    return 0;
}

int SendThread::get_data( unsigned char *puc_buf, unsigned int ui_data_len)
{
    unsigned char *puc_data_read_ptr = NULL;
    unsigned int ui_buf_left_len = 0;
    unsigned int ui_buf_real_read_len = ui_data_len;

    if (ui_data_len > get_used_data_len()) { ui_buf_real_read_len = get_used_data_len(); }

    puc_data_read_ptr = &mst_media_buf_info.puc_buf[mst_media_buf_info.ui_buf_read_ptr];
    ui_buf_left_len = mst_media_buf_info.ui_buf_data_len - mst_media_buf_info.ui_buf_read_ptr;

    if (ui_buf_left_len < ui_buf_real_read_len) {
        /* 循环读 */
        unsigned int ui_from_start_len = ui_buf_real_read_len - ui_buf_left_len;

        memcpy(puc_buf, puc_data_read_ptr, ui_buf_left_len);

        puc_data_read_ptr = &mst_media_buf_info.puc_buf[0];
        memcpy(puc_buf + ui_buf_left_len, puc_data_read_ptr, ui_from_start_len);
    } else {
        memcpy(puc_buf, puc_data_read_ptr, ui_buf_real_read_len);
    }

    mst_media_buf_info.ui_real_data_len -= ui_buf_real_read_len;// 可读数据长度
    mst_media_buf_info.ui_buf_read_ptr = (mst_media_buf_info.ui_buf_read_ptr + ui_buf_real_read_len) % mst_media_buf_info.ui_buf_data_len;//读指针偏移量

    return ui_buf_real_read_len;
}

