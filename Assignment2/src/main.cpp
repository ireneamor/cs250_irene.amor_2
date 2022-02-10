#include <SFML/Graphics.hpp>

#include "FrameBuffer.h"
#include "Rasterizer.h"
#include "CS250Parser.h"
#include "Math/Matrix4.h"
#include "Math/Point4.h"

//------------
//Functions
//------------
void Tank_Initialize(float WIDTH, float HEIGHT);
Matrix4 Viewport_Transformation(float width, float height, float v_width, float v_height);
Matrix4 Perspective_Projection();
Matrix4 ModelToWorld(int obj, bool scale);
void Tank_Update();
bool GetInput();


//------------
//Globals
//------------
CS250Parser* parser;
enum obj { body, turret, joint, gun, wheel1, wheel2, wheel3, wheel4, TOTAL };

Matrix4 viewport;
Matrix4 persp_proj;

const int max_vtx = 8;
const int max_faces = 12;

int body_rot;

bool draw_mode_solid = true;


int main()
{
    const int WIDTH = 1280;
    const int HEIGHT = 720;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");

    FrameBuffer::Init(WIDTH, HEIGHT);

    // Generate image and texture to display
    sf::Image   image;
    sf::Texture texture;
    sf::Sprite  sprite;
    texture.create(WIDTH, HEIGHT);
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    Tank_Initialize(WIDTH, HEIGHT);

    while (window.isOpen())
    {
        FrameBuffer::Clear(sf::Color::White.r, sf::Color::White.g, sf::Color::White.b);

        // Handle input
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        // Fill framebuffer
        Tank_Update();

        // Show image on screen
        FrameBuffer::ConvertFrameBufferToSFMLImage(image);

        texture.update(image);
        sprite.setTexture(texture);
        window.draw(sprite);
        window.display();
    }
    
    FrameBuffer::Free();

    return 2;
}

void Tank_Initialize(float WIDTH, float HEIGHT)
{
    //Read file
    parser = new CS250Parser;
    parser->LoadDataFromFile("input.txt");

    parser->objects;

    //Set viewport size
    float view_width = parser->right - parser->left;
    float view_height = parser->top - parser->bottom;

    //Get view matrix
    viewport = Viewport_Transformation(WIDTH, HEIGHT, view_width, view_height);
    persp_proj = Perspective_Projection();

}

void Tank_Update()
{
    draw_mode_solid = GetInput();
    Matrix4 m2w_body = ModelToWorld(body, false);

    //Calculate for each object
    for (int obj = 0; obj < TOTAL; obj++)
    {
        //Need to calculate the model to world matrix first
        Matrix4 m2w = ModelToWorld(obj, true);
        
        if (obj == body)
            m2w_body.Identity();


        //Vertices of the cube
        for (int i = 0; i < max_faces; i++)
        {
            auto face = parser->faces[i];
            //Vertices
            Rasterizer::Vertex vtx[3];

            for (int j = 0; j < 3; j++)
            {
                //Get vertices: position and color
                vtx[j].position = parser->vertices[face.indices[j]];

                //Color
                vtx[j].color = parser->colors[i];
                vtx[j].color.r = vtx[j].color.r / 255;
                vtx[j].color.g = vtx[j].color.g / 255;
                vtx[j].color.b = vtx[j].color.b / 255;

                //Transform vertices
                vtx[j].position = persp_proj * m2w_body * m2w * vtx[j].position;

                //Perspective division
                vtx[j].position.x = vtx[j].position.x / vtx[j].position.w;
                vtx[j].position.y = vtx[j].position.y / vtx[j].position.w;
                vtx[j].position.z = vtx[j].position.z / vtx[j].position.w;
                vtx[j].position.w = vtx[j].position.w / vtx[j].position.w;

                vtx[j].position = viewport * vtx[j].position;
            }
            if(draw_mode_solid)
                Rasterizer::DrawTriangleSolid(vtx[0], vtx[1], vtx[2]);
            else
            {
                Rasterizer::DrawMidpointLine(vtx[0], vtx[1]);
                Rasterizer::DrawMidpointLine(vtx[1], vtx[2]);
                Rasterizer::DrawMidpointLine(vtx[2], vtx[0]);
            }
        }

    }

}

Matrix4 Viewport_Transformation(float width, float height, float v_width, float v_height)
{
    //Viewport tr
    Matrix4 viewport;
    viewport.Identity();
    viewport.m[0][0] =  width / v_width;
    viewport.m[0][3] =  width / 2;
    viewport.m[1][1] = -height / v_height;
    viewport.m[1][3] =  height / 2;

    return viewport;
}

Matrix4 Perspective_Projection()
{
    //Perspective projection
    Matrix4 persp_proj;
    persp_proj.Identity();
    persp_proj.m[3][2] = -1 / parser->focal;
    persp_proj.m[3][3] = 0;

    return persp_proj;
}

Matrix4 ModelToWorld(int obj, bool scale)
{
    //Translation
    Matrix4 Transl;
    Transl.Identity();
    Transl.m[0][3] = parser->objects[obj].pos.x;
    Transl.m[1][3] = parser->objects[obj].pos.y;
    Transl.m[2][3] = parser->objects[obj].pos.z;



    Matrix4 Rot, RotX, RotY, RotZ;
    Vector4 angle = parser->objects[obj].rot;
    //Rotation x-axis
    RotX.Identity();
    RotX.m[1][1] =  cos(angle.x);   //Angle ??
    RotX.m[1][2] = -sin(angle.x);
    RotX.m[2][1] =  sin(angle.x);
    RotX.m[2][2] =  cos(angle.x);

    //Rotation y-axis
    RotY.Identity(); 
    RotY.m[0][0] =  cos(angle.y);   //Angle ??
    RotY.m[0][2] = -sin(angle.y);
    RotY.m[2][0] =  sin(angle.y);
    RotY.m[2][2] =  cos(angle.y);
    //Rotation z-axis
    RotZ.Identity(); 
    RotZ.m[0][0] =  cos(angle.z);   //Angle ??
    RotZ.m[0][1] = -sin(angle.z);
    RotZ.m[1][0] =  sin(angle.z);
    RotZ.m[1][1] =  cos(angle.z);

    Rot = RotZ * RotY * RotX;



    //Scale
    Matrix4 Scale;
    Scale.Identity();

    if (scale)
    {
        Scale.m[0][0] = parser->objects[obj].sca.x;
        Scale.m[1][1] = parser->objects[obj].sca.y;
        Scale.m[2][2] = parser->objects[obj].sca.z;
    }


    Matrix4 m2w_body = Transl * Rot * Scale;

    //Multiply by parent
    if (obj == gun)
        m2w_body = ModelToWorld(joint, false) * m2w_body;
    if (obj == joint)
        m2w_body = ModelToWorld(turret, false) * m2w_body;
    

    return m2w_body;
}

bool GetInput()
{
    //Tank body rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        parser->objects[body].rot.y -= 0.1f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        parser->objects[body].rot.y += 0.1f;


    //Turret rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        parser->objects[turret].rot.y -= 0.1f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        parser->objects[turret].rot.y += 0.1f;


    //Gun rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        parser->objects[joint].rot.x += 0.1f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        parser->objects[joint].rot.x -= 0.1f;


    //Move tank forward
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        parser->objects[body].pos.x--;

        parser->objects[wheel1].rot.x += 0.1f;
        parser->objects[wheel2].rot.x += 0.1f;
        parser->objects[wheel3].rot.x += 0.1f;
        parser->objects[wheel4].rot.x += 0.1f;

    }

    //Check solid/wireframe mode
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
        return false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
        return true;

    return draw_mode_solid;
}
