#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtGui>
#include <QtOpenGL>
#include <QOpenGLFunctions_4_5_Core>
#include <QImage>
#include <QColor>

#include <math.h>
#include "GLCamera.h"
#include "BoundingSphere.h"

/* Custom OpenGL Viewer Widget */

class TextRenderer;
class TriangleMesh;
class SphericalHarmonicsEditor;
class SuperToroidEditor;
class SuperEllipsoidEditor;
class SpringEditor;
class ClippingPlanesEditor;
class GraysKleinEditor;
class STLMesh;
class Plane;
class Cube;

class ModelViewer;

struct GLMaterialProps
{
	QVector4D ambientMaterial;
	QVector4D diffuseMaterial;
	QVector4D specularMaterial;
	QVector4D specularReflectivity;
	QVector4D emmissiveMaterial;
	float   shininess;
	float   opacity;

	bool bHasTexture;
};

enum class ViewMode { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK, ISOMETRIC, DIMETRIC, TRIMETRIC, NONE };
enum class ViewProjection { ORTHOGRAPHIC, PERSPECTIVE };
enum class DisplayMode { SHADED, WIREFRAME, WIRESHADED, REALSHADED };

class GLWidget : public QOpenGLWidget, QOpenGLFunctions_4_5_Core
{
	friend class ClippingPlanesEditor;
	Q_OBJECT
public:
	GLWidget(QWidget* parent = 0, const char* name = 0);
	~GLWidget();
	void updateView();

	void resizeView(int w, int h) { resizeGL(w, h); }
	void setViewMode(ViewMode mode);
	void setProjection(ViewProjection proj);

	void setMultiView(bool active) { _bMultiView = active; }
	void setRotationActive(bool active);
	void setPanningActive(bool active);
	void setZoomingActive(bool active);

	void fitAll();

	void beginWindowZoom();
    void performWindowZoom();

	void setDisplayList(const std::vector<int>& ids);
	void updateBoundingSphere();
	int getModelNum() const
	{
		return _modelNum;
	}

	void showClippingPlaneEditor(bool show);
	void showAxis(bool show);

	void showShadows(bool show);
	void showEnvironment(bool show);
	void showSkyBox(bool show);
	void showReflections(bool show);
	void showFloor(bool show);
    void showFloorTexture(bool show);
    void setFloorTexture(QImage img);

	std::vector<TriangleMesh*> getMeshStore() const { return _meshStore; }

	void addToDisplay(TriangleMesh*);
	void removeFromDisplay(int index);
	void centerScreen(int index);
	void select(int id);
	void deselect(int id);

	TriangleMesh* loadSTLMesh(QString fileName);
	TriangleMesh* loadOBJMesh(QString fileName);

	void setMaterialProps(const std::vector<int>& ids, const GLMaterialProps& mat);
    void setTransformation(const std::vector<int>& ids, const QVector3D& trans, const QVector3D& rot, const QVector3D& scale);
	void setTexture(const std::vector<int>& ids, const QImage& texImage);

public:
	float getXTran() const;
	void setXTran(const float& xTran);

	float getYTran() const;
	void setYTran(const float& yTran);

	float getZTran() const;
	void setZTran(const float& zTran);

	float getXRot() const;
	void setXRot(const float& xRot);

	float getYRot() const;
	void setYRot(const float& yRot);

	float getZRot() const;
	void setZRot(const float& zRot);

	float getXScale() const;
	void setXScale(const float& xScale);

	float getYScale() const;
	void setYScale(const float& yScale);

	float getZScale() const;
	void setZScale(const float& zScale);

	QVector4D getAmbientLight() const;
	void setAmbientLight(const QVector4D& ambientLight);

	QVector4D getDiffuseLight() const;
	void setDiffuseLight(const QVector4D& diffuseLight);

	QVector4D getSpecularLight() const;
	void setSpecularLight(const QVector4D& specularLight);

	QVector3D getLightPosition() const;
	void setLightPosition(const QVector3D& lightPosition);

	bool isShaded() const;
	void setDisplayMode(DisplayMode mode);

	bool isVertexNormalsShown() const;
	void setShowVertexNormals(bool showVertexNormals);

	bool isFaceNormalsShown() const;
	void setShowFaceNormals(bool showFaceNormals);

	std::vector<int> getDisplayedObjectsIds() const;

	BoundingSphere getBoundingSphere() const;

	QColor getBgTopColor() const;
	void setBgTopColor(const QColor& bgTopColor);

	QColor getBgBotColor() const;
	void setBgBotColor(const QColor& bgBotColor);

    void checkAndStopTimers();

signals:
	void windowZoomEnded();
	void rotationsSet();
	void zoomAndPanSet();
	void viewSet();
	void displayListSet();
	void objectSelectionChanged(int);

public slots:
	void animateViewChange();
	void animateFitAll();
	void animateWindowZoom();
	void animateCenterScreen();

private slots:
	void showContextMenu(const QPoint& pos);
	void centerDisplayList();
	void hideSelectedItem();
	void deleteSelectedItem();
	void showPropertiesPage();
	void showTransformationsPage();
	void setBackgroundColor();

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
    void keyPressEvent(QKeyEvent* event);
	void closeEvent(QCloseEvent* event);

private:
	DisplayMode _displayMode;
	QColor      _bgTopColor;
	QColor      _bgBotColor;
	bool _bWindowZoomActive;
	bool _bZoomView;
	bool _bPanView;
	bool _bRotateView;
	int _modelNum;
    QImage _texImage, _texBuffer, _floorTexImage;
	TextRenderer* _textRenderer;
	TextRenderer* _axisTextRenderer;
	QString _modelName;

	QVector3D _currentTranslation;
	QQuaternion _currentRotation;
	float _slerpStep;
	float _slerpFrac;

	float _currentViewRange;
	float _scaleFrac;

	float _viewRange;
	float _viewBoundingSphereDia;
	float _FOV;

	QPoint _leftButtonPoint;
	QPoint _rightButtonPoint;
	QPoint _middleButtonPoint;

	QRubberBand* _rubberBand;
	QVector3D _rubberBandPan;
	float _rubberBandZoomRatio;

	bool _bMultiView;

	bool _bShowAxis;

	float _clipXCoeff;
	float _clipYCoeff;
	float _clipZCoeff;

	float _clipDX;
	float _clipDY;
	float _clipDZ;

	bool _clipXEnabled;
	bool _clipYEnabled;
	bool _clipZEnabled;

	bool _clipXFlipped;
	bool _clipYFlipped;
	bool _clipZFlipped;

	bool _showVertexNormals;
	bool _showFaceNormals;

	bool _envMapEnabled;
	bool _shadowsEnabled;
	bool _reflectionsEnabled;
	bool _floorDisplayed;
    bool _floorTextureDisplayed;
	bool _skyBoxEnabled;

	unsigned int _shadowWidth;
	unsigned int _shadowHeight;

	float _xTran;
	float _yTran;
	float _zTran;

	float _xRot;
	float _yRot;
	float _zRot;

	float _xScale;
	float _yScale;
	float _zScale;

	QVector4D _ambientLight;
	QVector4D _diffuseLight;
	QVector4D _specularLight;

	QVector3D _lightPosition;
	QVector3D _prevLightPosition;

	QMatrix4x4 _lightSpaceMatrix;

	QMatrix4x4 _projectionMatrix, _viewMatrix, _modelMatrix;
	QMatrix4x4 _modelViewMatrix;
	QMatrix4x4 _viewportMatrix;

	QOpenGLShaderProgram* _fgShader;
	QOpenGLShaderProgram* _axisShader;
	QOpenGLShaderProgram* _vertexNormalShader;
	QOpenGLShaderProgram* _faceNormalShader;
	QOpenGLShaderProgram* _shadowMappingShader;
	QOpenGLShaderProgram* _skyBoxShader;

	unsigned int             _environmentMap;
	unsigned int             _shadowMap;
	unsigned int             _shadowMapFBO;
	float                    _floorSize;
	QVector3D                _floorCenter;

	QOpenGLShaderProgram     _textShader;

	QOpenGLShaderProgram     _bgShader;
	QOpenGLVertexArrayObject _bgVAO;

	QOpenGLShaderProgram     _bgSplitShader;
	QOpenGLVertexArrayObject _bgSplitVAO;
	QOpenGLBuffer _bgSplitVBO;

	QOpenGLVertexArrayObject _axisVAO;
	QOpenGLBuffer _axisVBO;
	QOpenGLBuffer _axisCBO;

	std::vector<TriangleMesh*> _meshStore;
	std::vector<int> _displayedObjectsIds;
	int _centerScreenObjectId;

	QVBoxLayout* _editorLayout;
	QFormLayout* _lowerLayout;
	QFormLayout* _upperLayout;

	SphericalHarmonicsEditor* _sphericalHarmonicsEditor;
	SuperToroidEditor* _superToroidEditor;
	SuperEllipsoidEditor* _superEllipsoidEditor;
	SpringEditor* _springEditor;
	GraysKleinEditor* _graysKleinEditor;
	ClippingPlanesEditor* _clippingPlanesEditor;

	ViewMode _viewMode;
	ViewProjection _projection;

    GLCamera* _primaryCamera;
    GLCamera* _orthoViewsCamera;

	QTimer* _animateViewTimer;
	QTimer* _animateFitAllTimer;
	QTimer* _animateWindowZoomTimer;
	QTimer* _animateCenterScreenTimer;

	BoundingSphere _boundingSphere;
	Plane* _floorPlane;
	Cube* _skyBox;

	QOpenGLShaderProgram     _debugShader;

	ModelViewer* _viewer;

	unsigned int quadVAO;
    unsigned int quadVBO;

private:
	void createShaderPrograms();
	void createGeometry();
    void loadEnvMap();
	void loadFloor();

	void drawMesh();
	void drawFloor();
	void drawSkyBox();
	void drawVertexNormals();
	void drawFaceNormals();
	void drawAxis();
	void drawCornerAxis();

    void render(GLCamera* camera);
	void renderToShadowBuffer();

	void gradientBackground(float top_r, float top_g, float top_b, float top_a,
		float bot_r, float bot_g, float bot_b, float bot_a);
	void splitScreen();

	void setRotations(float xRot, float yRot, float zRot);
	void setZoomAndPan(float zoom, QVector3D pan);
	void setView(QVector3D viewPos, QVector3D viewDir, QVector3D upDir, QVector3D rightDir);

    void convertClickToRay(const QPoint& pixel, const QRect& viewport, QVector3D& orig, QVector3D& dir);
	int mouseSelect(const QPoint& pixel);

    float highestModelZ();
	float lowestModelZ();

	void renderQuad();
    QRect getViewportFromPoint(const QPoint& pixel);
    QRect getClientRectFromPoint(const QPoint& pixel);

};

#endif
