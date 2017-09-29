#ifndef IMAGESEND_H
#define IMAGESEND_H

#include "image-encode.h"
#include <QObject>
#include <QThread>


class SendThread;
class ImageSend: public QObject
{
    Q_OBJECT
public:
    explicit ImageSend( ImageEncode *p_image_encode );
public slots:
    void on_encode_image_changed_slot( unsigned char *p_buf, unsigned int n_size );
private:
    ImageEncode *mp_image_encode;
    SendThread *mp_send_thread;
    bool b_first_start;
};

class SendThread : public QThread
{
    Q_OBJECT
public:
    explicit SendThread( ImageSend *p_image_send );
    void run();
    void lock( ) { m_fill_mutex.lock(); }
    void unlock() { m_fill_mutex.unlock(); }
    int put_frame( unsigned char *puc_buf, unsigned int ui_data_len );
    unsigned int get_used_data_len( );
    unsigned int get_remain_data_len( );
    int get_data( unsigned char *puc_buf, unsigned int ui_data_len );

    typedef struct MEDIA_BUF_INFO{
        unsigned int ui_real_data_len;    /* 当前buf中的真实数据的大小 */
        unsigned int ui_buf_data_len;     /* 视频注入buf的大小 */
        unsigned char *puc_buf;             /* 视频注入的buf */
        unsigned int ui_buf_read_ptr;     /* buf的读指针 */
        unsigned int ui_buf_write_ptr;    /* buf的读指针 */
    }MEDIA_BUF_INFO_S;
    MEDIA_BUF_INFO_S mst_media_buf_info;

private:
   ImageSend *mp_image_send;
   QMutex m_fill_mutex;
   unsigned int n_count;
};
#endif // IMAGESEND_H
