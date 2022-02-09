#include <SFML/Graphics.hpp>

#include "FrameBuffer.h"
#include "Rasterizer.h"
#include "CS250Parser.h"
#include "Math/Matrix4.h"
#include "Math/Point4.h"

//------------
//Functions
//------------
void Tank_Initialize();
void Tank_Update();
void Transformations();
void ModelToWorld();


//------------
//Globals
//------------
CS250Parser* parser;
enum obj { body, turret, joint, gun, wheel1, wheel12, wheel13, wheel14, TOTAL };
Matrix4 transf_body, m2w_body, Tb, Rb, Sb, persp_proj, viewport;

const int vp_width = 1280;
const int vp_height= 720;
float view_width, view_height;

const int window_limits[4] = { 360,-360,-640,640 }; //top, bot, left, right
const int max_vtx = 8;
const int max_faces = 12;


int main()
{

    sf::RenderWindow window(sf::VideoMode(vp_width, vp_height), "SFML works!");

    FrameBuffer::Init(vp_width, vp_height);

    // Generate image and texture to display
    sf::Image   image;
    sf::Texture texture;
    sf::Sprite  sprite;
    texture.create(vp_width, vp_height);
    image.create(vp_width, vp_height, sf::Color::Black);

    Tank_Initialize();

    // Init the clock
    sf::Clock clock;
    while (window.isOpen())
    {
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

void Tank_Initialize()
{
    //Read file
    parser = new CS250Parser;
    parser->LoadDataFromFile("input.txt");

    parser->objects;

    //Set viewport size
    view_width = parser->right - parser->left;
    view_height = parser->top - parser->bottom;

}

void Tank_Update()
{
    //Need to calculate the matrices first
    ModelToWorld();
    Transformations();

    //Vertices
    Rasterizer::Vertex vtx[3];

    //Vertices of the cube
    for (int i = 0; i < max_faces; i++)
    {
        auto face = parser->faces[i];

    //Get vertices: position and color
        vtx[0].position = parser->vertices[face.indices[0]];
        vtx[0].color = parser->colors[i];
    //Transform vertices
        vtx[0].position = m2w_body * vtx[0].position;
        vtx[0].position = persp_proj * vtx[0].position;

        //Perspective division
        vtx[0].position.x = vtx[0].position.x / vtx[0].position.w;
        vtx[0].position.y = vtx[0].position.y / vtx[0].position.w;
        vtx[0].position.z = vtx[0].position.z / vtx[0].position.w;
        vtx[0].position.w = vtx[0].position.w / vtx[0].position.w;

        vtx[0].position = viewport * vtx[0].position;


        //Get vertices: position and color
        vtx[1].position = parser->vertices[face.indices[1]];
        vtx[1].color = parser->colors[i];
        //Transform vertices
        vtx[1].position = m2w_body * vtx[1].position;
        vtx[1].position = persp_proj * vtx[1].position;

        //Perspective division
        vtx[1].position.x = vtx[1].position.x / vtx[1].position.w;
        vtx[1].position.y = vtx[1].position.y / vtx[1].position.w;
        vtx[1].position.z = vtx[1].position.z / vtx[1].position.w;
        vtx[1].position.w = vtx[1].position.w / vtx[1].position.w;

        vtx[1].position = viewport * vtx[1].position;



        //Get vertices: position and color
        vtx[2].position = parser->vertices[face.indices[2]];
        vtx[2].color = parser->colors[i];
        //Transform vertices
        vtx[2].position = m2w_body * vtx[2].position;
        vtx[2].position = persp_proj * vtx[2].position;

        //Perspective division
        vtx[2].position.x = vtx[2].position.x / vtx[2].position.w;
        vtx[2].position.y = vtx[2].position.y / vtx[2].position.w;
        vtx[2].position.z = vtx[2].position.z / vtx[2].position.w;
        vtx[2].position.w = vtx[2].position.w / vtx[2].position.w;

        vtx[2].position = viewport * vtx[2].position;


        Rasterizer::DrawTriangleSolid(vtx[0], vtx[1], vtx[2]);
    }



   /* for (int y = window_limits[0]; y < window_limits[1]; y++)
    {
        for (int x = window_limits[0]; x < window_limits[3]; x++)
        {
            /*Point4 p(x, y, -122);

            const float persp_div = (parser->focal / p.z);

            p = m2w_body * p;

            //Perspective division
            for (int i = 0; i < 4; i++)
            {
                p.v[i] *= persp_div;
            }

            FrameBuffer::SetPixel(p.x, p.y, 255, 0, 0);*//*
        }
    }
    */
}

void Transformations()
{
    //Viewport tr
    viewport.Identity();
    viewport.m[0][0] = vp_width / view_width;
    viewport.m[0][3] = vp_width / 2;
    viewport.m[1][1] = -vp_height / view_height;
    viewport.m[1][3] = vp_height/ 2;

    //Perspective projection
    persp_proj.Identity();
    persp_proj.m[3][2] = -1 / parser->focal;
    persp_proj.m[3][3] = 0;

}

void ModelToWorld()
{
    //Translation
    Tb.Identity();
    Tb.m[0][3] = parser->objects[body].pos.x;
    Tb.m[1][3] = parser->objects[body].pos.y;
    Tb.m[2][3] = parser->objects[body].pos.z;

    //Rotation z-axis
    Rb.Identity();
    Rb.m[0][0] = cos(parser->objects[body].rot.x);   //Angle ??
    Rb.m[0][1] = sin(parser->objects[body].rot.x);
    Rb.m[1][0] = -sin(parser->objects[body].rot.y);
    Rb.m[1][1] = cos(parser->objects[body].rot.y);

    //Scale
    Sb.Identity();
    Sb.m[0][0] = parser->objects[body].sca.x;
    Sb.m[1][1] = parser->objects[body].sca.y;
    Sb.m[2][2] = parser->objects[body].sca.z;

    m2w_body = Tb * Rb * Sb;
}