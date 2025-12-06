#include "../Headers/include_all.h"

#include "GLUtils.hpp"

#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <set>
#include <utility>

#include <SDL2/SDL_image.h>

/* 

Az http://www.opengl-tutorial.org/ oldal alapján.

*/

static void loadShaderCode( std::string& shaderCode, const std::filesystem::path& _fileName )
{
	// shaderkod betoltese _fileName fajlbol
	shaderCode = "";

	// _fileName megnyitasa
	std::ifstream shaderStream( _fileName );
	if ( !shaderStream.is_open() )
	{
		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR,
						SDL_LOG_PRIORITY_ERROR,
						"Error while opening shader code file %s!", _fileName.string().c_str());
		return;
	}

	// file tartalmanak betoltese a shaderCode string-be
	std::string line = "";
	while ( std::getline( shaderStream, line ) )
	{
		shaderCode += line + "\n";
	}

	shaderStream.close();
}

/* STABLE VERSION
// Ez egy nagyon primitív preprocesszor!!! Csak az #include-okat kezeli és nincs benne védelem a többszörös include-ok ellen
static void preprocessShaderCode(std::string& shaderCode, const std::filesystem::path& includerPath) {

	// query directory from parent path
	const std::filesystem::path& parentPath = includerPath.parent_path();

	std::regex includeRegex(R"(#include\s*\"([^\"]+)\")");
	std::smatch match;

	std::string processedCode;
	std::string::const_iterator searchStart(shaderCode.cbegin());

	while (std::regex_search(searchStart, shaderCode.cend(), match, includeRegex)) {
		processedCode.append(searchStart, match[0].first); // Append up to include line

		// build include path
		std::stringstream includeFile;
		includeFile << parentPath.string() << "/" << match[1].str();

		Log::logToConsole("Include found: ", match[1].str(), "; Full path: ", includeFile.str());

		std::string includedCode;
		loadShaderCode(includedCode, includeFile.str());

		// Recursively preprocess the included file
		preprocessShaderCode(includedCode, includeFile.str());

		processedCode.append(includedCode); // Replace include with content
		searchStart = match[0].second; // Continue after the match
	}

	processedCode.append(searchStart, shaderCode.cend()); // Append remaining code
	shaderCode = std::move(processedCode);
}
*/

// Segédfüggvény a rekurzióhoz és az állapottartáshoz
static void preprocessShaderCodeRecursive(std::string& sourceCode, const std::filesystem::path& currentPath, std::set<std::string>& includedFiles) {

	// Regex for includes
	std::regex includeRegex(R"(#include\s*\"([^\"]+)\")");
	std::smatch match;

	std::string processedCode;
	std::string::const_iterator searchStart(sourceCode.cbegin());

	int currentLine = 1;

	while (std::regex_search(searchStart, sourceCode.cend(), match, includeRegex)) {

		// string before match
		std::string segment(searchStart, match[0].first);
		processedCode.append(segment);

		currentLine += std::count(segment.begin(), segment.end(), '\n');

		// set include path
		std::filesystem::path includePath = currentPath.parent_path() / match[1].str();

		// build absolute path for duplication filtering
		std::error_code ec;
		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(includePath, ec);
		std::string uniqueKey = canonicalPath.string();

		if (!ec && includedFiles.find(uniqueKey) == includedFiles.end()) {
			// not yet included

			includedFiles.insert(uniqueKey);
			Log::logToConsole("Include found: ", match[1].str());

			std::string includedCode;
			loadShaderCode(includedCode, includePath.string());

			if (!includedCode.empty()) {
				// recursion on included file
				preprocessShaderCodeRecursive(includedCode, includePath, includedFiles);

				// insert #line directive
				processedCode.append("\n#line 1\n");
				processedCode.append(includedCode);

				// reset line on return
				processedCode.append("\n#line " + std::to_string(currentLine + 1) + "\n");
			}
		}
		else {
			// already included
			processedCode.append("// Skipped duplicate include: " + match[1].str() + "\n");
		}

		searchStart = match[0].second;
	}

	processedCode.append(searchStart, sourceCode.cend());
	sourceCode = std::move(processedCode);
}

static void preprocessShaderCode(std::string& shaderCode, const std::filesystem::path& includerPath) {
	std::set<std::string> includedFiles;

	std::error_code ec;
	includedFiles.insert(std::filesystem::weakly_canonical(includerPath, ec).string());

	preprocessShaderCodeRecursive(shaderCode, includerPath, includedFiles);
}

GLuint AttachShader( const GLuint programID, GLenum shaderType, const std::filesystem::path& _fileName )
{
    // shaderkod betoltese _fileName fajlbol
    std::string shaderCode;
    loadShaderCode( shaderCode, _fileName );

	// preprocessor futtatása az include-ok kicserélésére
	preprocessShaderCode(shaderCode, _fileName);

    return AttachShaderCode( programID, shaderType, shaderCode );
}

GLuint AttachShaderCode( const GLuint programID, GLenum shaderType, std::string_view shaderCode )
{
	if (programID == 0)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_ERROR,
						SDL_LOG_PRIORITY_ERROR,
						"Program needs to be inited before loading!");
		return 0;
	}

	// shader letrehozasa
	GLuint shaderID = glCreateShader( shaderType );

	// kod hozzarendelese a shader-hez
	const char* sourcePointer = shaderCode.data();
	GLint sourceLength = static_cast<GLint>( shaderCode.length() );

	glShaderSource( shaderID, 1, &sourcePointer, &sourceLength );

	// shader leforditasa
	glCompileShader( shaderID );

	// ellenorizzuk, h minden rendben van-e
	GLint result = GL_FALSE;
	int infoLogLength;

	// forditas statuszanak lekerdezese
	glGetShaderiv( shaderID, GL_COMPILE_STATUS, &result );
	glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &infoLogLength );

	if ( GL_FALSE == result || infoLogLength != 0 )
	{
		// hibauzenet elkerese es kiirasa
		std::string ErrorMessage( infoLogLength, '\0' );
		glGetShaderInfoLog( shaderID, infoLogLength, NULL, ErrorMessage.data() );

		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR,
						( result ) ? SDL_LOG_PRIORITY_WARN : SDL_LOG_PRIORITY_ERROR,
						"[glCompileShader]: %s", ErrorMessage.data() );
	}

	// shader hozzarendelese a programhoz
	glAttachShader( programID, shaderID );

	return shaderID;

}

void LinkProgram(const GLuint programID, bool OwnShaders)
{
	// Require a valid program handle
	if (programID == 0)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_ERROR,
			SDL_LOG_PRIORITY_ERROR,
			"LinkProgram: programID is 0 (invalid)!");
		return;
	}

	// Ensure there is a current GL context on this thread
	if (SDL_GL_GetCurrentContext() == nullptr)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_ERROR,
			SDL_LOG_PRIORITY_ERROR,
			"LinkProgram: No current OpenGL context on this thread!");
		return;
	}

	// Link the program
	glLinkProgram(programID);

	// Query link status and info log length safely
	GLint linkStatus = GL_FALSE;
	GLint infoLogLength = 0;

	glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

	// Only request the info log if the driver returned a positive length
	if (infoLogLength > 0)
	{
		std::string ErrorMessage(static_cast<size_t>(infoLogLength), '\0');
		glGetProgramInfoLog(programID, infoLogLength, nullptr, ErrorMessage.data());

		SDL_LogMessage(SDL_LOG_CATEGORY_ERROR,
			(linkStatus == GL_TRUE) ? SDL_LOG_PRIORITY_WARN : SDL_LOG_PRIORITY_ERROR,
			"[glLinkProgram]: %s", ErrorMessage.c_str());
	}
	else if (linkStatus != GL_TRUE)
	{
		// Linking failed but no log provided
		SDL_LogMessage(SDL_LOG_CATEGORY_ERROR,
			SDL_LOG_PRIORITY_ERROR,
			"[glLinkProgram]: Linking failed, no info log available.");
	}

	// If the program "owns" shaders, delete attached shaders.
	if (OwnShaders)
	{
		GLint attachedShaders = 0;
		glGetProgramiv(programID, GL_ATTACHED_SHADERS, &attachedShaders);

		if (attachedShaders > 0)
		{
			std::vector<GLuint> shaders(static_cast<size_t>(attachedShaders));
			GLsizei actuallyRetrieved = 0;
			glGetAttachedShaders(programID, attachedShaders, &actuallyRetrieved, shaders.data());

			for (GLuint shader : shaders)
			{
				glDeleteShader(shader);
			}
		}
	}
}

static inline ImageRGBA::TexelRGBA* get_image_row( ImageRGBA& image, int rowIndex )
{
	return &image.texelData[  rowIndex * image.width ];
}

static void invert_image_RGBA(ImageRGBA& image)
{
	int height_div_2 = image.height / 2;


	for ( int index = 0; index < height_div_2; index++ )
	{
		std::uint32_t* lower_data  =reinterpret_cast<std::uint32_t*>(get_image_row( image, index) );
		std::uint32_t* higher_data =reinterpret_cast<std::uint32_t*>(get_image_row( image, image.height - 1 - index ) );

		for ( unsigned int rowIndex = 0; rowIndex < image.width; rowIndex++ )
		{
			lower_data[ rowIndex ] ^= higher_data[ rowIndex ];
			higher_data[ rowIndex ] ^= lower_data[ rowIndex ];
			lower_data[ rowIndex ] ^= higher_data[ rowIndex ];
		}
	}
}

GLsizei NumberOfMIPLevels( const ImageRGBA& image )
{
	GLsizei targetlevel = 1;
	unsigned int index = std::max( image.width, image.height );

	while (index >>= 1) ++targetlevel;

	return targetlevel;
}

[[nodiscard]] ImageRGBA ImageFromFile( const std::filesystem::path& fileName, bool needsFlip )
{
	ImageRGBA img;

	// Kép betöltése
	std::unique_ptr<SDL_Surface, decltype( &SDL_FreeSurface )> loaded_img( IMG_Load( fileName.string().c_str() ), SDL_FreeSurface );
	if ( !loaded_img )
	{
		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR, 
						SDL_LOG_PRIORITY_ERROR,
						"[ImageFromFile] Error while loading image file: %s", fileName.string().c_str());

		Log::errorToConsole(SDL_LOG_CATEGORY_ERROR,
							SDL_LOG_PRIORITY_ERROR,
							"[ImageFromFile] Error while loading image file: ", fileName.string().c_str());

		return img;
	}

	// Uint32-ben tárolja az SDL a színeket, ezért számít a bájtsorrend
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#else
	Uint32 format = SDL_PIXELFORMAT_RGBA8888;
#endif

	// Átalakítás 32bit RGBA formátumra, ha nem abban volt
	std::unique_ptr<SDL_Surface, decltype( &SDL_FreeSurface )> formattedSurf( SDL_ConvertSurfaceFormat( loaded_img.get(), format, 0 ), SDL_FreeSurface );

	if (!formattedSurf)
	{
		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR, 
						SDL_LOG_PRIORITY_ERROR,
						"[ImageFromFile] Error while processing texture");

		Log::errorToConsole(SDL_LOG_CATEGORY_ERROR,
							SDL_LOG_PRIORITY_ERROR,
							"[ImageFromFile] Error while processing texture");

		return img;
	}

	// Rakjuk át az SDL Surface-t az ImageRGBA-ba
	img.Assign( reinterpret_cast<const std::uint32_t*>(formattedSurf->pixels), formattedSurf->w, formattedSurf->h );

	// Áttérés SDL koordinátarendszerről ( (0,0) balfent ) OpenGL textúra-koordinátarendszerre ( (0,0) ballent )

	if ( needsFlip ) invert_image_RGBA( img );

	return img;
}

void CleanOGLObject( OGLObject& ObjectGPU )
{
	glDeleteBuffers(1,      &ObjectGPU.vboID);
	ObjectGPU.vboID = 0;
	glDeleteBuffers(1,      &ObjectGPU.iboID);
	ObjectGPU.iboID = 0;
	glDeleteVertexArrays(1, &ObjectGPU.vaoID);
	ObjectGPU.vaoID = 0;
}