#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix, iMatrix, bMatrix;

float player_x = 0; 
float boulder_x = 0;
float boulder_rotate = 0;

GLuint boulderTextureID;
GLuint indianaTextureID;

//when using images, you should use something that is 64x64, or 128x128, 1024x1024
//avoid odd resolutions and things that aren't multiplies of 16
GLuint LoadTexture(const char* filePath) { //returns ID number of a texture
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha); //loading the png file off the hard drive and into ram

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID); //create the texture ID
    glBindTexture(GL_TEXTURE_2D, textureID); //now bind that texture ID

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image); //here is the raw pixel data, put it onto the video card and heres the id number

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //use the nearest filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image); //freeing from our main memory since it's already on the video card memory
    return textureID; //now it returns the texture id
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Raiders of the Lost Ark!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl"); // Setting up the shader program
    //loads a shader program that can handle textured polygons

    viewMatrix = glm::mat4(1.0f);
    bMatrix = glm::mat4(1.0f); //applies this identity matrix to the triangle
    iMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); 


    //this is the drawing of the object
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(1.0f, 0.0f, 0.0f, 1.0f); //set the color to red

    glUseProgram(program.programID); //use this shader program

    glClearColor(0.56f, 0.50f, 0.31f, 1.0f); //whenever you go to draw something, use this background color
    glEnable(GL_BLEND);

    //good for setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    boulderTextureID = LoadTexture("boulder.png"); //put in the address of the picture
    indianaTextureID = LoadTexture("indiana.png");
}

void ProcessInput() { //this basically looks for if the window closed or if something happened
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

float lastTicks = 0.0f; //time since last frame

//update only runs as fast as your computer can run it
void Update() { //if something is input then it will update the screen, this is empty for now 
    
    float ticks = (float)SDL_GetTicks() / 1000.0f; //num of milliseconds since sdl has been initialized
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    player_x += 1.45f * deltaTime;
    boulder_x += 0.59f * deltaTime;
    boulder_rotate += -90.0f * deltaTime;

    //identity
    bMatrix = glm::mat4(1.0f); 
    bMatrix = glm::translate(bMatrix, glm::vec3(-5.0f, 0.4f, 0.0f));
    bMatrix = glm::scale(bMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
    bMatrix = glm::translate(bMatrix, glm::vec3(boulder_x, 0.0f, 0.0f)); 
    bMatrix = glm::rotate(bMatrix, glm::radians(boulder_rotate), glm::vec3(0.0f, 0.0f, 1.0f));

    iMatrix = glm::mat4(1.0f);
    iMatrix = glm::translate(iMatrix, glm::vec3(-2.0f, 0.0f, 0.0f));
    iMatrix = glm::translate(iMatrix, glm::vec3(player_x, 0.0f, 0.0f));

    //if you put the rotate line of code above the translate, then it will spiral
    //if you put the translate line of code above the rotate, then it will rotate while moving right
    //modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.01f, 0.0f)); //X Y Z
    //modelMatrix = glm::rotate(modelMatrix, glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //modelMatrix = glm::scale(modelMatrix, glm::vec3(1.01f, 1.01f, 1.0f));
} 

void Render() {
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 }; //this are the vertices of the triangle
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glClear(GL_COLOR_BUFFER_BIT); //clear the color and use the background color

    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);


    program.SetModelMatrix(bMatrix); //here is the matrix to use when drawing the next thing
    glBindTexture(GL_TEXTURE_2D, boulderTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(iMatrix); //here is the matrix to use when drawing the next thing
    glBindTexture(GL_TEXTURE_2D, indianaTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow); //swap window basically pushes it to the screen
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) { //this is happening as fast as the computer can handle (framerate)
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}