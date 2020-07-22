#pragma once

#include <vector>
#include "Drawable.h"
#include "BoundingSphere.h"

class TriangleMesh : public Drawable 
{

public:
    TriangleMesh(QOpenGLShaderProgram* prog, const QString name);

    virtual ~TriangleMesh();

    virtual void setName(const QString& name)
    {
        _name = name;
    }

    virtual void render();
    virtual BoundingSphere getBoundingSphere() const { return _boundingSphere; }

    virtual QOpenGLVertexArrayObject& getVAO();
    virtual QString getName() const
    {
        return _name;
    }

    QVector4D ambientMaterial() const;
    void setAmbientMaterial(const QVector4D &ambientMaterial);

    QVector4D diffuseMaterial() const;
    void setDiffuseMaterial(const QVector4D &diffuseMaterial);

    QVector4D specularMaterial() const;
    void setSpecularMaterial(const QVector4D &specularMaterial);

    QVector4D emmissiveMaterial() const;
    void setEmmissiveMaterial(const QVector4D &emmissiveMaterial);

    QVector4D specularReflectivity() const;
    void setSpecularReflectivity(const QVector4D &specularReflectivity);

    GLfloat opacity() const;
    void setOpacity(const GLfloat &opacity);

    GLfloat shininess() const;
    void setShininess(const GLfloat &shininess);

    GLboolean hasTexture() const;
    void enableTexture(const GLboolean &bHasTexture);

    void setTexureImage(const QImage &texImage);

    QMatrix4x4 getTransformation() const;
    void setTransformation(const QMatrix4x4 &transformation);

    std::vector<GLfloat> getPoints() const;

    std::vector<GLfloat> getNormals() const;

protected: // methods
    virtual void initBuffers(
            std::vector<GLuint> * indices,
            std::vector<GLfloat> * points,
            std::vector<GLfloat> * normals,
            std::vector<GLfloat> * texCoords = nullptr,
            std::vector<GLfloat> * tangents = nullptr
            );

    virtual void deleteBuffers();

    void computeBoundingSphere(std::vector<GLfloat> points);

protected:

    QOpenGLBuffer _indexBuffer;
    QOpenGLBuffer _positionBuffer;
    QOpenGLBuffer _normalBuffer;
    QOpenGLBuffer _texCoordBuffer;
    QOpenGLBuffer _tangentBuf;

    QOpenGLBuffer _coordBuf;

    GLuint _nVerts;     // Number of vertices
    QOpenGLVertexArrayObject _vertexArrayObject;        // The Vertex Array Object

    // Vertex buffers
    std::vector<QOpenGLBuffer> _buffers;

    BoundingSphere _boundingSphere;

    QString _name;

    QVector4D _ambientMaterial;
    QVector4D _diffuseMaterial;
    QVector4D _specularMaterial;
    QVector4D _emmissiveMaterial;
    QVector4D _specularReflectivity;
    GLfloat _opacity;
    GLfloat _shininess;

    QImage _texImage, _texBuffer;
    GLuint _texture;
    GLboolean _bHasTexture;

    std::vector<GLfloat> _points;
    std::vector<GLfloat> _normals;
    std::vector<GLfloat> _trsfpoints;
    std::vector<GLfloat> _trsfnormals;
    QMatrix4x4 _transformation;
};
