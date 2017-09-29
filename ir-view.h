#ifndef IRVIEW_H
#define IRVIEW_H

#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QThread>

#include <QList>
#include <QMutex>
#include "pci-irlib-machine.h"
#include "config.h"

class IrShowThread;
class IrView : public QObject
{
    Q_OBJECT
public:
    friend class IrShowThread;
    explicit IrView(QObject *parent = 0);
    ~IrView();
    YHANDLE get_ir_capture() { return mh_ir_capture; }

    static void draw_image( void *p_user_data, unsigned short *pw_ad, unsigned short w_width, unsigned short w_height, DataStatus status );
    void draw_image_real( );

    void stop_video();
    void restore_video();
    bool get_started();
    void laser_on();
    void laser_off();
private:

    YHANDLE mh_ir_capture;
    YHANDLE mh_ir;
    YHANDLE mh_data;
    YHANDLE mh_corret;
    int mn_ir_capture_timer_id;
    QImage m_image;
    unsigned short mw_width;
    unsigned short mw_height;
    static void on_cb_correct_over( void *p_user_data );
    void init_device();
    void start_capture( bool b_first );
    bool mb_captured;
    bool mb_caputure_init;
    bool mb_shooted;
    bool mb_stop_video;
    bool mb_resized;
    bool mb_inited_anas;
    bool mb_called_init_fun;
    bool mb_laser_on;
    QString ms_img_fn;
    QMutex mo_mutex;

protected:
    void resizeEvent( QResizeEvent *event );

    void * get_capture_handle() {
        #ifdef ENABLE_IRCAPTURE
        return mh_ir_capture;
        #else
        return NULL;
        #endif
    }
    void * get_ir_handle() {
        #ifdef ENABLE_IRCAPTURE
        return mh_ir;
        #else
        return NULL;
        #endif
    }
    void * get_data_show_handle() {
        #ifdef ENABLE_IRCAPTURE
        return mh_data;
        #else
        return NULL;
        #endif
    }
    QImage &get_image() { return m_image;}

private:
    IrShowThread *mp_ir_show_thread;
signals:
    void image_changed( unsigned int puc_out_buf, int n_data_len );
    void raw_image_changed( const uchar *puc_out_buf ) ;

};

class IrShowThread : public QThread
{
    Q_OBJECT
public:
    friend class IrView;
    IrShowThread( void *);
    void run() ;
    QMutex m_mutex;
private:
    IrView *m_handle;
};

#endif // IRVIEW_H
