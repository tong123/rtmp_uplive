#include "image-item.h"
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QImageReader>
#include <QDateTime>
#include <QApplication>
#include <QDesktopWidget>
#define ImageW 640
#define ImageH 480
//CImageItem::CImageItem( QQuickItem *parent ) : QQuickItem( parent )
CImageItem::CImageItem( QQuickItem *parent ) : QQuickPaintedItem( parent )
{
    mp_buffer = new unsigned char[640*480*4];
    n_start_frame_count = 0;
    n_frame_rate = 0;
    n_width = 640;
    n_height = 480;
}

CImageItem::~CImageItem()
{
}

void CImageItem::paint( QPainter *painter )
{
    m_mutex.lock();
    QRect a(0,0,640,480);
    m_ir_img = m_ir_img.scaled( 640, 480 );
    painter->drawImage( a, m_ir_img );
    m_mutex.unlock();
}

void CImageItem::img_trigged_slot( unsigned int puc_out_buf, int n_data_len )
{
    for( int i=0; i<n_data_len; i++ ) {
        mp_buffer[i] = *( (unsigned char*)puc_out_buf + i );
    }
    m_ir_img =  QImage( (const uchar *)mp_buffer, 640, 480, QImage::Format_RGB32 );
    update();
    m_mutex.unlock();
}

uchar* CImageItem::rgb_to_rgb( QImage &image32, int n_width, int n_height )
{
    uchar* imagebits_32 = (uchar* )image32.constBits();         //获取图像首地址，32位图
    int n_line_num = 0;
    int w_32 = image32.bytesPerLine();
    for( int i=0; i<n_height; i++ ) {
        n_line_num = i*w_32;
        for( int j=0; j<n_width; j++ ) {
            int r_32 = imagebits_32[ n_line_num+ j * 4 + 2 ];
            int g_32 = imagebits_32[ n_line_num+ j * 4 + 1 ];
            int b_32 = imagebits_32[ n_line_num+ j * 4 ];
            imagebits_32[ n_line_num+ j * 4 ] = r_32;
            imagebits_32[ n_line_num+ j * 4 + 2 ] = b_32;
        }
    }
    return imagebits_32;
//    return NULL;
}
