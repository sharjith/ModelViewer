#include "MeshProperties.h"
#include "TriangleMesh.h"

MeshProperties::MeshProperties(TriangleMesh *mesh, QObject *parent) : QObject(parent), _mesh(mesh)
{
    _meshPoints = _mesh->getPoints();
    calculateSurfaceAreaAndVolume();
}

TriangleMesh *MeshProperties::mesh() const
{
    return _mesh;
}

void MeshProperties::setMesh(TriangleMesh *mesh)
{
    _mesh = mesh;
    _meshPoints.clear();
    _meshPoints = _mesh->getPoints();
    calculateSurfaceAreaAndVolume();
}

std::vector<float> MeshProperties::meshPoints() const
{
    return _meshPoints;
}

float MeshProperties::surfaceArea() const
{
    return _surfaceArea;
}

float MeshProperties::volume() const
{
    return _volume;
}

void MeshProperties::calculateSurfaceAreaAndVolume()
{
    _surfaceArea = 0;
    _volume = 0;
    for (size_t i = 0; i < _meshPoints.size(); i += 9)
    {
        QVector3D p1(_meshPoints[i + 0], _meshPoints[i + 1], _meshPoints[i + 2]);
        QVector3D p2(_meshPoints[i + 3], _meshPoints[i + 4], _meshPoints[i + 5]);
        QVector3D p3(_meshPoints[i + 6], _meshPoints[i + 7], _meshPoints[i + 8]);

        _volume += QVector3D::dotProduct(p1, (QVector3D::crossProduct(p2, p3))) / 6.0f;

        _surfaceArea += QVector3D::crossProduct(p2 - p1, p3 - p1).length() * 0.5;
    }
    _volume = fabs(_volume);
}