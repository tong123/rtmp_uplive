#ifndef CIMAGEITEM_H
#define CIMAGEITEM_H

#include <QtQuick/QQuickPaintedItem>
#include <QImage>
#include <QMutex>
#include <QQuickItem>

#include <QSGGeometry>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QSGTexture>
#include <QSGSimpleMaterialShader>
#include <QtQuick/QQuickWindow>

#define RIGHT_AREA_WIDTH 640
#define RIGHT_AREA_HIGHT 480
class QQuickWindow;

struct State
{
    virtual ~State() {
        delete texture;
    }
    QSGTexture *get_texture() const {
        return texture;
    }
    QColor color;
    QSGTexture *texture;
};

class Shader : public QSGSimpleMaterialShader<State>
{
    QSG_DECLARE_SIMPLE_SHADER(Shader, State)
public:
    const char *vertexShader() const {
        return  "attribute vec4 a_position;"
                "attribute vec2 a_texCoord;"
                "uniform highp mat4 qt_Matrix;"
                "varying vec2 v_texCoord;"
                "void main() {"
                "   v_texCoord  = a_texCoord;"
                "   gl_Position = qt_Matrix*a_position;"
                "} ";
    }

    const char *fragmentShader() const {
        return  "varying vec2 v_texCoord;"
                "uniform lowp float qt_Opacity;"
                "uniform sampler2D s_texture;"
                "void main() {"
                "  gl_FragColor = qt_Opacity*texture2D( s_texture, v_texCoord );"
                "}";
    }

    QList<QByteArray> attributes() const {
        return QList<QByteArray>() << "a_position" << "a_texCoord";
    }

    void updateState(const State *, const State *) {
//        program()->bind();
//       state->texture->bind();
    }

    void resolveUniforms() {
    }

private:

};

class ImageNode : public QSGGeometryNode
{
public:
    ImageNode(QQuickWindow *window) :
        m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4),
        mp_material(0)
    {
//        QImage img = QImage(  QString("/home/fish/qtample/ffmpeg_client/")  + "wheat.jpg" ).scaled( 640, 480 );
        QImage img = QImage(":/wheat.jpg" ).scaled( 640, 480 );

        QSGTexture *p_texture = window->createTextureFromImage( img );
        p_texture->setFiltering( QSGTexture::Nearest );
        p_texture->setHorizontalWrapMode( QSGTexture::Repeat );
        p_texture->setVerticalWrapMode( QSGTexture::Repeat );
        p_texture->bind();

        mp_material = Shader::createMaterial();
        mp_material->state()->texture = p_texture;


        setMaterial( mp_material );
        setFlag( OwnsMaterial );
        setGeometry( &m_geometry );
    }

    void setRect(const QRectF &bounds) {
        QSGGeometry::updateTexturedRectGeometry( geometry(), bounds, QRectF(0, 0, 1, 1) );
        markDirty(QSGNode::DirtyGeometry|QSGNode::DirtyMaterial);
    }

    QSGSimpleMaterial<State> *get_material() const {
        return mp_material;
    }
    int get_material_texture_id() {
        if ( mp_material )
            return mp_material->state()->texture->textureId();
        return -1;
    }
private:
    QSGGeometry               m_geometry;
    QSGSimpleMaterial<State> *mp_material;
};

class CImageItem : public QQuickPaintedItem
//class CImageItem : public QQuickItem
{
    Q_OBJECT
public:
    CImageItem( QQuickItem *parent = 0 );
    ~CImageItem();
//    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

    void paint( QPainter *painter );

    uchar* rgb_to_rgb( QImage &image32, int n_width, int n_height );

public slots:
//    void img_trigged_slot(QString puc_out_buf, int n_data_len );
    void img_trigged_slot(unsigned int puc_out_buf, int n_data_len );
signals:
    void show_frame_rate_info( QString s_frame_rate );
private:
    QImage m_ir_img;
    QImage m_img;
    QImage big_img;
    QMutex m_mutex;
    unsigned char *mp_buffer;
    unsigned int n_start_frame_count;
    unsigned int n_frame_rate;
    qint64 n_prev_time;
    qint64 n_current_time;
    unsigned int n_width;
    unsigned int n_height;
};

#endif // CIMAGEITEM_H
