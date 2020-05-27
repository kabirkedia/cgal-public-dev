#pragma once
#include <qopenglwidget.h>
#include <qmatrix4x4.h>
#include <qopenglshaderprogram.h>
#include <qopenglbuffer.h>
#include <qopengltexture.h>
#include <QMouseEvent>

class Widget :
	public QOpenGLWidget
{
public:
	Widget(QWidget* parent);
	~Widget();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void initShader();
	void initGeometry(float w);

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

private:
	QMatrix4x4 mMatrix;
	QMatrix4x4 pMatrix;
	QMatrix4x4 vMatrix;
	QOpenGLShaderProgram shaderProgram;
	QOpenGLTexture* texture;
	QOpenGLBuffer arrayBuffer;
	QOpenGLBuffer indexBuffer;

	QVector2D mousePosition;
	QQuaternion rotation;
};

struct Vertex {
	Vertex() {};
	Vertex(QVector3D position, QVector2D texCoord, QVector3D normal) :
		position(position), texCoord(texCoord), normal(normal) {};
	QVector3D position;
	QVector2D texCoord;
	QVector3D normal;
};