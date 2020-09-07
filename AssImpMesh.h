#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TriangleMesh.h"

using namespace std;

struct Vertex
{
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
};

struct Texture
{
	GLuint id;
	string type;
	aiString path;
};

class AssImpMesh : public TriangleMesh
{
public:
	/*  Mesh Data  */
	vector<Vertex> _vertices;
	vector<GLuint> _indices;
	vector<Texture> _textures;
	GLMaterialProps _materials;

	/*  Functions  */
	// Constructor
	AssImpMesh(QOpenGLShaderProgram* shader, vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, GLMaterialProps materials) : TriangleMesh(shader, "AssImpMesh"),
		_materials(materials)
	{
		_vertices = vertices;
		_indices = indices;
		_textures = textures;

		// Now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}

	// Render the mesh
	/*void render()
	{
		// Bind appropriate textures
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;

		for( GLuint i = 0; i < this->textures.size( ); i++ )
		{
			glActiveTexture( GL_TEXTURE0 + i ); // Active proper texture unit before binding
			// Retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = this->textures[i].type;

			if( name == "texture_diffuse" )
			{
				ss << diffuseNr++; // Transfer GLuint to stream
			}
			else if( name == "texture_specular" )
			{
				ss << specularNr++; // Transfer GLuint to stream
			}

			number = ss.str( );
			// Now set the sampler to the correct texture unit
			_prog->bind();
			_prog->setUniformValue((name + number).c_str(), i);
			// And finally bind the texture
			glBindTexture( GL_TEXTURE_2D, this->textures[i].id );
		}

		// Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
		_prog->setUniformValue("material.shininess", 16.0f );

		// Draw mesh
		glBindVertexArray( this->VAO );
		glDrawElements( GL_TRIANGLES, this->indices.size( ), GL_UNSIGNED_INT, 0 );
		glBindVertexArray( 0 );

		// Always good practice to set everything back to defaults once configured.
		for ( GLuint i = 0; i < this->textures.size( ); i++ )
		{
			glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, 0 );
		}
	}*/

	void render()
	{
		if (!_vertexArrayObject.isCreated())
			return;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texImage.width(), _texImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _texImage.bits());
		glGenerateMipmap(GL_TEXTURE_2D);

		_prog->bind();
		_prog->setUniformValue("texUnit", 0);
		_prog->setUniformValue("material.emission", _emmissiveMaterial.toVector3D());
		_prog->setUniformValue("material.ambient", _ambientMaterial.toVector3D());
		_prog->setUniformValue("material.diffuse", _diffuseMaterial.toVector3D());
		_prog->setUniformValue("material.specular", _specularMaterial.toVector3D());
		_prog->setUniformValue("material.shininess", _shininess);
        _prog->setUniformValue("material.metallic", _metallic);
		_prog->setUniformValue("texEnabled", _bHasTexture);
		_prog->setUniformValue("alpha", _opacity);
		_prog->setUniformValue("selected", _selected);
		_prog->setUniformValue("hasDiffuseTexture", false);
		_prog->setUniformValue("hasSpecularTexture", false);

		// Bind appropriate textures
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;

		for (GLuint i = 0; i < _textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
			// Retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = _textures[i].type;

			if (name == "texture_diffuse")
			{
				ss << diffuseNr++; // Transfer GLuint to stream
				_prog->setUniformValue("hasDiffuseTexture", true);
			}
			else if (name == "texture_specular")
			{
				ss << specularNr++; // Transfer GLuint to stream
				_prog->setUniformValue("hasSpecularTexture", true);
			}			

			number = ss.str();
			// Now set the sampler to the correct texture unit
			_prog->bind();
			_prog->setUniformValue((name + number).c_str(), i);
			// And finally bind the texture
			glBindTexture(GL_TEXTURE_2D, _textures[i].id);
		}

		if (_opacity < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		_vertexArrayObject.bind();
		glDrawElements(GL_TRIANGLES, _nVerts, GL_UNSIGNED_INT, 0);
		_vertexArrayObject.release();
		_prog->release();
		glDisable(GL_BLEND);

		// Always good practice to set everything back to defaults once configured.
		for (GLuint i = 0; i < _textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

private:
	/*  Functions    */
	// Initializes all the buffer objects/arrays
	void setupMesh()
	{
		std::vector<float> points;
		std::vector<float> normals;
		std::vector<float> texCoords;

		for (Vertex v : _vertices)
		{
			points.push_back(v.Position.x);
			points.push_back(v.Position.y);
			points.push_back(v.Position.z);

			normals.push_back(v.Normal.x);
			normals.push_back(v.Normal.y);
			normals.push_back(v.Normal.z);

			texCoords.push_back(v.TexCoords.x);
			texCoords.push_back(v.TexCoords.y);
		}

		_ambientMaterial = _materials.ambientMaterial;
		_diffuseMaterial = _materials.diffuseMaterial;
		_specularMaterial = _materials.specularMaterial;
		_emmissiveMaterial = _materials.emmissiveMaterial;
		_shininess = _materials.shininess;
		_opacity = _materials.opacity;
		_bHasTexture = _materials.bHasTexture;

		initBuffers(&_indices, &points, &normals, &texCoords);
		computeBoundingSphere(points);
	}
};


