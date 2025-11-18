#pragma once

#include <filesystem>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

/* 

Az http://www.opengl-tutorial.org/ oldal alapján.

*/

// Segéd osztályok

struct VertexPosColor
{
    glm::vec3 position;
    glm::vec3 color;
};

struct VertexPosTex
{
    glm::vec3 position;
    glm::vec2 texcoord;
};

struct VertexPos2DTex
{
    glm::vec2 position;
    glm::vec2 texcoord;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct VertexMergedNorm {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 mergedNormal;
    glm::vec2 texcoord;
};

struct ImageRGBA
{
    typedef glm::u8vec4 TexelRGBA;

    static_assert( sizeof( TexelRGBA ) == sizeof( std::uint32_t ) );


    std::vector<TexelRGBA> texelData;
    unsigned int width		   = 0;
    unsigned int height		   = 0;

    bool Allocate( unsigned int _width, unsigned int _height )
    {
        width = _width;
        height = _height;

        texelData.resize( width * height );

        return !texelData.empty();
    }

    bool Assign( const std::uint32_t* _TexelData, unsigned int _width, unsigned int _height )
    {
        width = _width;
        height = _height;

        const TexelRGBA* _data = reinterpret_cast<const TexelRGBA*>( _TexelData );

        texelData.assign( _data, _data + width * height );

        return !texelData.empty();
    }

    TexelRGBA GetTexel( unsigned int x, unsigned int y ) const
    {
        return texelData[y * width + x];
    }

    void SetTexel( unsigned int x, unsigned int y,const TexelRGBA& texel )
    {
        texelData[y * width + x] = texel;
    }

    const TexelRGBA* data() const
    {
        return texelData.data();
    }
};

template<typename VertexT>
struct MeshObject
{
    std::vector<VertexT> vertexArray;
    std::vector<GLuint>  indexArray;
};

struct OGLObject
{
    GLuint  vaoID = 0; // vertex array object erőforrás azonosító
    GLuint  vboID = 0; // vertex buffer object erőforrás azonosító
    GLuint  iboID = 0; // index buffer object erőforrás azonosító
    GLsizei count = 0; // mennyi indexet/vertexet kell rajzolnunk
};


struct VertexAttributeDescriptor
{
	GLuint index = -1;
    GLuint strideInBytes = 0;
	GLint  numberOfComponents = 0;
	GLenum glType = GL_NONE;
};

// Segéd függvények

GLuint AttachShader( const GLuint programID, GLenum shaderType, const std::filesystem::path& _fileName );
GLuint AttachShaderCode( const GLuint programID, GLenum shaderType, std::string_view shaderCode );
void LinkProgram( const GLuint programID, bool OwnShaders = true );


template <typename VertexT>
[[nodiscard]] OGLObject CreateGLObjectFromMesh( const MeshObject<VertexT>& mesh, std::initializer_list<VertexAttributeDescriptor> vertexAttrDescList )
{
	OGLObject meshGPU = { 0 };


	// hozzunk létre egy új VBO erőforrás nevet
	glCreateBuffers(1, &meshGPU.vboID);

	// töltsük fel adatokkal a VBO-t
	glNamedBufferData(meshGPU.vboID,	// a VBO-ba töltsünk adatokat
					   mesh.vertexArray.size() * sizeof(VertexT),		// ennyi bájt nagyságban
					   mesh.vertexArray.data(),	// erről a rendszermemóriabeli címről olvasva
					   GL_STATIC_DRAW);	// úgy, hogy a VBO-nkba nem tervezünk ezután írni és minden kirajzoláskor felhasnzáljuk a benne lévő adatokat

	// index puffer létrehozása
	glCreateBuffers(1, &meshGPU.iboID);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshGPU.iboID);
	glNamedBufferData(meshGPU.iboID, mesh.indexArray.size() * sizeof(GLuint), mesh.indexArray.data(), GL_STATIC_DRAW);

	meshGPU.count = static_cast<GLsizei>(mesh.indexArray.size());

	// 1 db VAO foglalasa
	glCreateVertexArrays(1, &meshGPU.vaoID);
	// a frissen generált VAO beallitasa aktívnak

	glVertexArrayVertexBuffer( meshGPU.vaoID, 0, meshGPU.vboID, 0, sizeof( VertexT ) );

	// attribútumok beállítása
	for ( const auto& vertexAttrDesc: vertexAttrDescList )
	{
		glEnableVertexArrayAttrib( meshGPU.vaoID, vertexAttrDesc.index ); // engedélyezzük az attribútumot
		glVertexArrayAttribBinding( meshGPU.vaoID, vertexAttrDesc.index, 0 ); // melyik VBO-ból olvassa az adatokat

		glVertexArrayAttribFormat(
			meshGPU.vaoID,						  // a VAO-hoz tartozó attribútumokat állítjuk be
			vertexAttrDesc.index,				  // a VB-ben található adatok közül a soron következő "indexű" attribútumait állítjuk be
			vertexAttrDesc.numberOfComponents,	  // komponens szam
			vertexAttrDesc.glType,				  // adatok tipusa
			GL_FALSE,							  // normalizalt legyen-e
			vertexAttrDesc.strideInBytes       // az attribútum hol kezdődik a sizeof(VertexT)-nyi területen belül
		);
	}
	glVertexArrayElementBuffer( meshGPU.vaoID, meshGPU.iboID );

	return meshGPU;
}

void CleanOGLObject( OGLObject& ObjectGPU );

[[nodiscard]] ImageRGBA ImageFromFile( const std::filesystem::path& fileName, bool needsFlip = true );
GLsizei NumberOfMIPLevels( const ImageRGBA& );

// uniform location lekérdezése
inline GLint ul( GLuint programID, const GLchar* uniformName ) noexcept
{
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
    return glGetUniformLocation( programID, uniformName );
}



