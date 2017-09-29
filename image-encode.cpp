#include "image-encode.h"
#include "h264-enc.h"
#include "pox_system.h"
#include <QDateTime>

ImageEncode::ImageEncode( IrView *p_ir_view ) : QObject(p_ir_view)
{    
    mp_encode_thread = new EncodeThread( this );
    connect( p_ir_view, SIGNAL( raw_image_changed(const uchar *) ), this, SLOT( on_raw_image_changed_slot( const uchar*) ) );
}

ImageEncode::~ImageEncode()
{

}

void ImageEncode::on_raw_image_changed_slot( const uchar* p_buf )
{
    mp_encode_thread->set_raw_data( p_buf );
    mp_encode_thread->start( );
}

EncodeThread::EncodeThread( ImageEncode *p_image_encode )
{
    mp_image_encode = p_image_encode;
    mp_sps_pps_data = new unsigned char[100];
    mp_yuv_data = new unsigned char[ 640 * 480 * 3/2 ];
    mp_encode_data = new unsigned char[ 640*480*2 ];
    mp_first_frame_data = new unsigned char[ 640*480*2 ];
    mp_packet_data = new unsigned char[ DATA_PACKET_SIZE ];
    mp_sps_data = new unsigned char[100];
    mp_pps_data = new unsigned char[100];
    mn_sps_size = 0;
    mn_pps_size = 0;
    mn_encode_data_size = 0;
    mn_sps_pps_size = 0;
    b_first_flag = true;
    b_first_frame_status = true;
    n_count = 0;
    mn_frame_rate = 0;
    h264_init( );
    set_sps_pss_into_buffer( );
}

void EncodeThread::set_sps_pss_into_buffer( )
{
    get_encode_file_head (  mp_sps_data, &mn_sps_size, mp_pps_data, &mn_pps_size );
    qDebug()<<"mn_sps_size, mn_pps_size: "<<mn_sps_size<<mn_pps_size;
    qDebug()<<mp_sps_data[0]<<mp_sps_data[1]<<mp_sps_data[2]<<mp_sps_data[3]<<mp_sps_data[4];
    qDebug()<<mp_pps_data[0]<<mp_pps_data[1]<<mp_pps_data[2]<<mp_pps_data[3]<<mp_pps_data[4];

}

EncodeThread::~EncodeThread()
{

}


void EncodeThread::run()
{
    m_mutex.lock();
    n_count++;
    qint64 start = QDateTime::currentMSecsSinceEpoch();
    memset( mp_encode_data, 0, sizeof(640*480*2) );
    memset( mp_packet_data, 0, DATA_PACKET_SIZE );
    mn_encode_data_size = h264_encode( (void*)mp_yuv_data, (void*)mp_encode_data, IR_IMAGE_WIDTH*IR_IMAGE_HEIGHT*3/2 );
    qDebug()<<"mn_encode_data_size: "<<mn_encode_data_size;
    memcpy( m_type, mp_encode_data, 5 );
    qDebug()<<mp_encode_data[0]<<mp_encode_data[1]<<mp_encode_data[2]<<mp_encode_data[3]<<mp_encode_data[4]<<mp_encode_data[5];
    if( b_first_frame_status ) {
        memset( mp_first_frame_data, 0, 640*480*2 );
        memcpy( mp_first_frame_data, mp_encode_data, mn_encode_data_size );

        mn_first_frame_size = mn_encode_data_size;
        b_first_frame_status = false;
    }
    if( get_first_read_status()  ) {
        if( b_first_flag ) {
            set_sps_into_buffer( mp_sps_data, mn_sps_size );
            set_pps_into_buffer( mp_pps_data, mn_pps_size );
            set_idr_into_buffer( mp_first_frame_data, mn_first_frame_size );
            set_first_buff_size( 1 );
            b_first_flag = false;
        } else {
            memcpy( mp_packet_data, mp_encode_data, mn_encode_data_size );
            emit mp_image_encode->encode_image_changed( mp_packet_data, mn_encode_data_size );
        }

    }
    if( mn_frame_rate == 0 ) {
        mn_prev_time = QDateTime::currentMSecsSinceEpoch();
        mn_frame_rate++;
    } else {
        mn_current_time = QDateTime::currentMSecsSinceEpoch();
        if( mn_current_time - mn_prev_time >= 1000 ) {
            set_frame_rate( mn_frame_rate );
            emit mp_image_encode->show_frame_rate_info( QString("%1").arg( mn_frame_rate ) );
            mn_frame_rate = 0;
        } else {
            mn_frame_rate++;
        }
    }
    qint64 end = QDateTime::currentMSecsSinceEpoch();
    m_mutex.unlock();

    qDebug()<<"encode data end-start:"<<end-start;
}

void EncodeThread::set_raw_data( const uchar* p_buf )
{
    p_raw_data = (unsigned char *)p_buf;
    Bitmap2Yuv420p_calc2( (unsigned char *)mp_yuv_data, (unsigned char *)p_buf, IR_IMAGE_WIDTH, IR_IMAGE_HEIGHT );
}

void EncodeThread::Bitmap2Yuv420p_calc2( unsigned char *p_yuv, unsigned char *p_rgb, int n_width, int n_height )
{
    int image_size = n_width * n_height;
    int upos = image_size;
    int vpos = upos + upos / 4;
    int i = 0;

    for( int n_line = 0; n_line < n_height; ++n_line ) {
        if( !(n_line % 2 ) ) {
            for( int x = 0; x < n_width; x += 2 ) {
                unsigned char r = p_rgb[4 * i];
                unsigned char g = p_rgb[4 * i + 1];
                unsigned char b = p_rgb[4 * i + 2];
                p_yuv[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
                p_yuv[upos++] = ((-38*r + -74*g + 112*b) >> 8) + 128;
                p_yuv[vpos++] = ((112*r + -94*g + -18*b) >> 8) + 128;
                r = p_rgb[4 * i];
                g = p_rgb[4 * i + 1];
                b = p_rgb[4 * i + 2];
                p_yuv[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        } else{
            for( int x = 0; x < n_width; x += 1 ) {
                unsigned char r = p_rgb[4 * i];
                unsigned char g = p_rgb[4 * i + 1];
                unsigned char b = p_rgb[4 * i + 2];
                p_yuv[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
    }
}




