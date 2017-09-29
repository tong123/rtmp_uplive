#ifndef IMAGEENCODE_H
#define IMAGEENCODE_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>

#include "ir-view.h"
#include "librtmp_send264.h"

class EncodeThread;
class ImageEncode : public QObject
{
    Q_OBJECT
public:
    explicit ImageEncode( IrView *p_ir_view );
    ~ImageEncode();

signals:
    void encode_image_changed( unsigned char *p_buf, unsigned int n_size );
    void show_frame_rate_info( QString s_frame_rate );

public slots:
    void on_raw_image_changed_slot( const uchar* p_buf );
private:
    IrView *mp_ir_view;
    EncodeThread *mp_encode_thread;

};

class EncodeThread: public QThread
{
    Q_OBJECT
public:
    explicit EncodeThread( ImageEncode *p_image_encode );
    ~EncodeThread();
    void run();
    void set_raw_data( const uchar* p_buf );
    void set_sps_pss_into_buffer( );
    void Bitmap2Yuv420p_calc2( unsigned char *p_yuv, unsigned char *p_rgb, int width, int height );
signals:
private:
    ImageEncode *mp_image_encode;
    unsigned char *p_raw_data;
    unsigned char *mp_sps_pps_data;
    unsigned char *mp_yuv_data;
    unsigned char *mp_encode_data;
    unsigned char *mp_packet_data;
    unsigned char *mp_sps_data;
    unsigned char *mp_pps_data;
    unsigned int mn_sps_pps_size;
    unsigned int mn_encode_data_size;
    unsigned char *mp_first_frame_data;
    unsigned int mn_first_frame_size;
    QMutex m_mutex;
    bool b_first_flag;
    bool b_first_frame_status;
    int n_count;
    unsigned char m_type[6];
    int mn_sps_size;
    int mn_pps_size;
    qint64 mn_prev_time;
    qint64 mn_current_time;
    int mn_frame_rate;
};

#endif // IMAGEENCODE_H
