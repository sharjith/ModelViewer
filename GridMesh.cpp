#include "GridMesh.h"

GridMesh::GridMesh(QOpenGLShaderProgram* prog, const QString name, unsigned int slices, unsigned int stacks) : TriangleMesh(prog, name),
_slices(slices),
_stacks(stacks)
{
}

GridMesh::~GridMesh()
{
}

bool GridMesh::intersectsWithRay(const QVector3D& rayPos, const QVector3D& rayDir, QVector3D& outIntersectionPoint)
{
	bool intersects = false;

	size_t offset = (_stacks) * 3;
	for (size_t i = 0; i < _trsfpoints.size() - offset - 3; i += 3)
	{
		if (i == offset || (i > offset && i % offset == 0))
			continue;
		QVector3D v0(_trsfpoints[i + 0], _trsfpoints[i + 1], _trsfpoints[i + 2]);
		QVector3D v1(_trsfpoints[i + 3], _trsfpoints[i + 4], _trsfpoints[i + 5]);
		QVector3D v2(_trsfpoints[offset + i + 0], _trsfpoints[offset + i + 1], _trsfpoints[offset + i + 2]);
		intersects = rayIntersectsTriangle(rayPos, rayDir, v0, v1, v2, outIntersectionPoint);
		if (intersects)
			break;

		QVector3D v3(_trsfpoints[offset + i + 0], _trsfpoints[offset + i + 1], _trsfpoints[offset + i + 2]);
		QVector3D v4(_trsfpoints[i + 3], _trsfpoints[i + 4], _trsfpoints[i + 5]);
		QVector3D v5(_trsfpoints[offset + i + 3], _trsfpoints[offset + i + 4], _trsfpoints[offset + i + 5]);
		intersects = rayIntersectsTriangle(rayPos, rayDir, v3, v4, v5, outIntersectionPoint);
		if (intersects)
			break;
	}

	return intersects;
}
