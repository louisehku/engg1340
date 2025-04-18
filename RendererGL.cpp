#include "RendererGL.h"



const char* RendererGL::vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec2 posIn;\n"
"uniform vec4 rectPixels;\n"
"uniform float rectOffset;\n"
"uniform vec2 windowWHPixels;\n"
"uniform float angleRad;\n"
"out vec2 posFrag;\n"

"void main()\n"
"{\n"
//Multiply each vertex by half the width and heigh of the rectangle and offset it by rectOffset.
"   vec2 posInScaled = posIn * rectPixels.zw / 2.0 + vec2(rectOffset, 0.0);\n"
//Rotate them.
"   vec2 posRot = vec2(posInScaled.x * cos(-angleRad) - posInScaled.y * sin(-angleRad),\n"
"                      posInScaled.x * sin(-angleRad) + posInScaled.y * cos(-angleRad));\n"
//Scale them so that they match the correct number of pixels on the window.
"   vec2 pos2 = posRot / windowWHPixels * 2.0;\n"

//Compute offsets that are needed to ensure the rectangle's upper left gets drawn in the correct spot.
"   float offsetWindow = -1.0;\n"
"   vec2 offsetRectUL = rectPixels.xy / windowWHPixels * 2.0;\n"
"   vec2 offsetRectWHDiv2 = (rectPixels.zw / 2.0) / windowWHPixels * 2.0;\n"

//Output positions for the vertices and fragment.
"   gl_Position = vec4(pos2.x + offsetWindow + offsetRectUL.x + offsetRectWHDiv2.x,\n"
"                      pos2.y - offsetWindow - offsetRectUL.y - offsetRectWHDiv2.y, 0.0, 1.0);\n"
"   posFrag = posIn;\n;"
"}\0";


const char* RendererGL::fragmentShaderSource = "#version 330 core\n"
"in vec2 posFrag;\n"
"uniform float sizePixel;\n"
"uniform vec4 drawColor;\n"
"uniform int shapeType;\n"
"out vec4 FragColor;\n"

//Functions for drawing each shape
"void fillOval() {\n"
"    float distance = 1.0 - sqrt(posFrag.x * posFrag.x + posFrag.y * posFrag.y) + sizePixel / 2.0;\n"
"    float alpha = clamp(distance / sizePixel, 0.0, 1.0);\n"
"    if (alpha < 0.01f)\n"
"        discard;\n"
"    FragColor  = vec4(drawColor.xyz, drawColor.w * alpha);\n"
"}\n"

"void fillRectangle() {\n"
"    FragColor = drawColor;\n"
"}\n"


//Draw the shape, default to rectangle
"void main()\n"
"{\n"
"    switch (shapeType) {\n"
"        case 1:  fillOval();  break;\n"
"        default: fillRectangle();  break;\n"
"    }\n"
"}\n\0";




RendererGL::RendererGL(SDL_Window* setWindow) :
	window(setWindow) {
	loadShader();
	loadBuffers();
	//Set the default color.
	setDrawColor(255, 255, 255, 255);
}


RendererGL::~RendererGL() {
	//Clean up.
	if (shaderProgramID > 0)
		glDeleteProgram(shaderProgramID);

	if (VAO > 0)
		glDeleteVertexArrays(1, &VAO);
	if (VBO > 0)
		glDeleteBuffers(1, &VBO);
	if (EBO > 0)
		glDeleteBuffers(1, &EBO);
}



void RendererGL::loadShader() {
	//Create the vertex shader.
	const GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShaderID);
	//Check for errors when creating the shader.
	if (!checkShaderForCompileErrors(vertexShaderID, "vertex shader")) {
		glDeleteShader(vertexShaderID);
		return;
	}

	//Create the fragment shader.
	const GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShaderID);
	//Check for errors when creating the shader.
	if (!checkShaderForCompileErrors(fragmentShaderID, "fragment shader")) {
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
		return;
	}

	//Create the shader program.
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShaderID);
	glAttachShader(shaderProgramID, fragmentShaderID);
	glLinkProgram(shaderProgramID);
	//Check for errors when creating the shader program.
	if (!checkShaderProgramForCompileErrors(shaderProgramID)) {
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
		glDeleteProgram(shaderProgramID);
		return;
	}

	std::cout << "Shader program created correctly" << std::endl;


	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(shaderProgramID);
}


bool RendererGL::checkShaderForCompileErrors(GLuint shader, std::string shaderType) {
	//Check the shader for errors.
	GLint statusShader;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &statusShader);
	if (statusShader == GL_FALSE) {
		//Get the error message and output it.
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		std::vector<char> errorMessage(length);
		glGetShaderInfoLog(shader, length, &length, &errorMessage[0]);
		std::cout << "ERROR: problem creating " << shaderType << std::endl << 
			&errorMessage[0] << std::endl;
		return false;
	}
	return true;
}


bool RendererGL::checkShaderProgramForCompileErrors(GLuint shaderProgram) {
	//Check the shader program for errors.
	GLint statusShaderProgram;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &statusShaderProgram);
	if (statusShaderProgram == GL_FALSE) {
		//Get the error message and output it.
		GLint length;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
		std::vector<char> errorMessage(length);
		glGetProgramInfoLog(shaderProgram, length, &length, &errorMessage[0]);
		std::cout << "ERROR: problem creating shader program " << std::endl << 
			&errorMessage[0] << std::endl;
		return false;
	}
	return true;
}


void RendererGL::loadBuffers() {
	float vertices[] = {
		1.0f,  1.0f,	// Right Top
		1.0f, -1.0f,	// Right Bottom
	   -1.0f, -1.0f,	// Left Bottom
	   -1.0f,  1.0f		// Left Top
	};

	unsigned int indices[] = {
		0, 1, 3,	//First Triangle
		1, 2, 3		//Second Triangle
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
}



void RendererGL::setDrawColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	glUniform4f(glGetUniformLocation(shaderProgramID, "drawColor"),
		(float)(r / 255.0f), (float)(g / 255.0f), (float)(b / 255.0f), (float)(a / 255.0f));
}


void RendererGL::fillRect(const SDL_Rect* rect, float angleRad, int rectOffset) {
	fillShape(rect, angleRad, rectOffset, 0);
}


void RendererGL::fillOval(const SDL_Rect* rect, float angleRad, int rectOffset) {
	fillShape(rect, angleRad, rectOffset, 1);
}


void RendererGL::fillShape(const SDL_Rect * rect, float angleRad, int rectOffset, int shapeType) {
	if (rect != nullptr && rect->w > 0 && rect->h > 0 && window != nullptr) {

		//Get the dimensions of the window.
		int windowWidth = 0, windowHeight = 0;
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		if (windowWidth > 0 && windowHeight > 0) {
			//Set the shader program's uniform's.
			glUniform4f(glGetUniformLocation(shaderProgramID, "rectPixels"),
				rect->x, rect->y, rect->w, rect->h);
			glUniform1f(glGetUniformLocation(shaderProgramID, "rectOffset"), rectOffset);
			glUniform2f(glGetUniformLocation(shaderProgramID, "windowWHPixels"),
				windowWidth, windowHeight);
			glUniform1f(glGetUniformLocation(shaderProgramID, "angleRad"), angleRad);

			//The approximate size of a pixel in OpenGL's coordinate system.
			glUniform1f(glGetUniformLocation(shaderProgramID, "sizePixel"),
				1.0f / rect->w + 1.0f / rect->h);
			glUniform1i(glGetUniformLocation(shaderProgramID, "shapeType"), shapeType);

			//Draw the shape.
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
}



void RendererGL::swapWindow() {
	SDL_GL_SwapWindow(window);
}