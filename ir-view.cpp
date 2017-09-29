#include "ir-view.h"
#include <QDebug>
#include <QTimerEvent>
#include <QApplication>
#include <QFile>
#include <QProcess>
#include "math.h"
#ifdef Q_OS_LINUX
#include "stdlib.h"
#endif


IrShowThread::IrShowThread( void *h_hand )
    : m_handle( NULL )
{
    m_handle = ( IrView *)h_hand;
}

void IrShowThread::run()
{
    m_mutex.lock();
    yf_capture_ex_get_data( m_handle->get_capture_handle(), m_handle->get_ir_handle(), m_handle->get_data_show_handle() );
    bool b_ret = yf_data_ex_data_calc_pixel( m_handle->get_data_show_handle(), m_handle->get_image().bits() );
    emit m_handle->image_changed( (unsigned int)m_handle->get_image().constBits(), m_handle->get_image().byteCount() );
    emit m_handle->raw_image_changed( m_handle->get_image().constBits() );
    m_mutex.unlock();
}

IrView::IrView(QObject *parent) :
    QObject(parent)
{
    mb_captured = false;
    mb_caputure_init = false;
    mb_shooted = false;
    mb_stop_video = false;
    mb_resized = false;
    mb_inited_anas = false;

    mb_laser_on = true;
    mp_ir_show_thread = new IrShowThread( this );

#ifdef ENABLE_IRCAPTURE
    mh_ir_capture = NULL;
    mw_width =  0;
    mw_height = 0;
#endif
    mb_called_init_fun = false;

    init_device();
}

IrView::~IrView()
{
    if( mb_captured ) { yf_capture_ex_stop( mh_ir_capture ); }
    if( mh_ir ) { yf_ir_uninit( mh_ir ); }
    if( mb_caputure_init ) { yf_capture_ex_uninit( mh_ir_capture ); }
}

void IrView::stop_video()
{
    mb_stop_video = true;
    if( mb_captured ) {
        yf_capture_ex_stop( mh_ir_capture );
    }
}

void IrView::restore_video()
{
    mb_stop_video = false;
    start_capture( false );
}

bool IrView::get_started()
{
    #ifdef ENABLE_IRCAPTURE
    return mb_captured;
    #endif
    return false;
}

void IrView::laser_on()
{
    bool b_ret = yf_capture_ex_laser_on( mh_ir_capture );
    qDebug() << "IrView::laser_on b_ret=" << b_ret;
    if( b_ret ) {
        mb_laser_on = true;
    }
}

void IrView::laser_off()
{
    bool b_ret = yf_capture_ex_laser_off( mh_ir_capture );
    qDebug() << "IrView::laser_off b_ret=" << b_ret;
    if( b_ret ) {
        mb_laser_on = false;
    }
}

void IrView::init_device()
{
    mb_caputure_init = yf_capture_ex_init( mh_ir_capture );

    if( !mb_caputure_init ) {
        qWarning() << "yf_capture_ex_init fail";
    } else {
        yf_capture_ex_get_image_resolution( mh_ir_capture, mw_width, mw_height );
        qDebug()<<"yf_capture_ex_get_image_resolution: "<<mw_width<<mw_height;
        if( !yf_ir_init( mh_ir ) ) {
            qWarning() << "yf_ir_init fail";
        } else {
            if( !yf_data_init( mh_ir, mw_width, mw_height, mh_data ) ) {
                qWarning() << "yf_ir_init fail";
            } else {
                yf_data_set_imaging_flash_alg( mh_data, FlashFix );
                yf_data_set_enforce_auto( mh_data, false );
            }
        }
//        yf_capture_get_correct_handle( mh_ir_capture , mh_corret );
        bool b_inited = yf_capture_ex_set_callback_corrected( mh_ir_capture, draw_image, this );
        if( !b_inited ) { qDebug() << "IrView::init_device yf_capture_ex_set_callback_corrected fail";  }

        yf_capture_set_cb_correct_over( mh_ir_capture, on_cb_correct_over, (YHANDLE)this );
        m_image = QImage( mw_width, mw_height, QImage::Format_RGB32 );

        start_capture( true );
    }
}


void IrView::draw_image( void *p_user_data, unsigned short *pw_ad, unsigned short w_width, unsigned short w_height, DataStatus status )
{
    IrView *p_this = ( IrView* )p_user_data;
   p_this->draw_image_real( );
}

void IrView::draw_image_real()
{
    mp_ir_show_thread->start();
}

void IrView::on_cb_correct_over( void *p_user_data )
{

}

void IrView::start_capture( bool b_first )
{
    mb_captured = yf_capture_ex_start( mh_ir_capture );
    if( !b_first ) {
        return ;
    }
    yf_capture_ex_one_pt_correct( mh_ir_capture, true );
    if( !mb_captured ) {
        qWarning() << "yf_capture_ex_start fail";
    } else {
    }
}



