
#include "TankFunctions.h"  //Header file


/**
* @brief Tank_Initialize: initialize tank object
*
* @param (void)
*/
void Tank::Tank_Initialize()
{
    //Read input file
    parser = new CS250Parser;
    parser->LoadDataFromFile("input.txt");

    //Set viewport size
    view_width = parser->right - parser->left;
    view_height = parser->top - parser->bottom;

    TOTAL_obj = parser->objects.size();

    //Number of faces per cube and number of vertices per face
    max_faces = parser->faces.size();

    //Get view matrix
    Viewport_Transformation();
    Perspective_Projection();

    //Get the color of each face
    //They are the same for all the cubes
    for (int j = 0; j < max_faces; j++)
    {
        //Normalize color
        color[j] = parser->colors[j];
        color[j].r = color[j].r / 255;
        color[j].g = color[j].g / 255;
        color[j].b = color[j].b / 255;
    }
}


/**
* @brief Tank_Update: renders the current state of the tank
*
* @param (void)
*/
void Tank::Tank_Update()
{
    //Get inputs from the user
    draw_mode_solid = GetInput();
    

    //Calculate the new state of each object
    for (int obj = 0; obj < TOTAL_obj; obj++)
    {     
        //Need to calculate the model to world matrix first
        //Because it is the same for the whole object
        Matrix4 m2w = ModelToWorld(parser->objects[obj], true);

        //Vertices of the cube
        for (int i = 0; i < max_faces; i++)
        {
            auto face = parser->faces[i];
            Rasterizer::Vertex vtx[3];      //Each vertex of the triangle

            for (int j = 0; j < 3; j++)
            {
                //Get vertices: color
                vtx[j].color = color[i];

                //Get vertices: position
                vtx[j].position = parser->vertices[face.indices[j]];

                //Transform vertices: perspective division and model to world
                vtx[j].position = persp_proj * m2w * vtx[j].position;

                //Transform vertices:: perspective division
                vtx[j].position.x = vtx[j].position.x / vtx[j].position.w;
                vtx[j].position.y = vtx[j].position.y / vtx[j].position.w;
                vtx[j].position.z = vtx[j].position.z / vtx[j].position.w;
                vtx[j].position.w = vtx[j].position.w / vtx[j].position.w;

                //Transform vertices:: view transformation
                vtx[j].position = viewport * vtx[j].position;
            }

            //Draw the object
            if (draw_mode_solid)
                Rasterizer::DrawTriangleSolid(vtx[0], vtx[1], vtx[2]);
            else
            {
                //Every line composing the triangle
                Rasterizer::DrawMidpointLine(vtx[0], vtx[1]);
                Rasterizer::DrawMidpointLine(vtx[1], vtx[2]);
                Rasterizer::DrawMidpointLine(vtx[2], vtx[0]);
            }
        }

    }

}


/**
* @brief Viewport_Transformation: calculate the viewport transformation matrix
*
* @param (void)
*/
void Tank::Viewport_Transformation()
{
    //Viewport transformation
    viewport.Identity();
    viewport.m[0][0] =  WIDTH / view_width;
    viewport.m[0][3] =  WIDTH / 2.f;
    viewport.m[1][1] = -HEIGHT / view_height;
    viewport.m[1][3] =  HEIGHT / 2.f;

}


/**
* @brief Perspective_Projection: calculate the perspective projection matrix
*
* @param (void)
*/
void Tank::Perspective_Projection()
{
    //Perspective projection
    persp_proj.Identity();
    persp_proj.m[3][2] = -1 / parser->focal;
    persp_proj.m[3][3] = 0;
}

/**
* @brief ModelToWorld:  calculate the model to world matrix of the object
*
* @param obj:           object to find
* @return               the transform of the found object
*/
CS250Parser::Transform* Tank::FindObject(std::string obj)
{
    for (int i = 0; i < TOTAL_obj; i++)
    {
        //Find the object
        if (!strcmp(obj.c_str(), parser->objects[i].name.c_str()))
            return &parser->objects[i];
    }

    //If it is never found
    return nullptr;
}



/**
* @brief ModelToWorld:  calculate the model to world matrix of the object
*
* @param obj:           object to calculate the matrix for
* @param scale:         whether to calculate the scale
* @return               model to world matrix
*/
/**
* @brief ModelToWorld:  calculate the model to world matrix of the object
*
* @param obj:           object to calculate the matrix for
* @param scale:         whether to calculate the scale
* @return               model to world matrix
*/
Matrix4 Tank::ModelToWorld(CS250Parser::Transform obj, bool scale)
{
    //Translation
    Matrix4 Transl;
    {
        Transl.Identity();
        Transl.m[0][3] = obj.pos.x;
        Transl.m[1][3] = obj.pos.y;
        Transl.m[2][3] = obj.pos.z;
    }

    //Rotation
    Matrix4 Rot, RotX, RotY, RotZ;
    Vector4 angle = obj.rot;
    {
        //Rotation x-axis
        RotX.Identity();
        RotX.m[1][1] = cos(angle.x);
        RotX.m[1][2] = -sin(angle.x);
        RotX.m[2][1] = sin(angle.x);
        RotX.m[2][2] = cos(angle.x);

        //Rotation y-axis
        RotY.Identity();
        RotY.m[0][0] = cos(angle.y);
        RotY.m[0][2] = sin(angle.y);
        RotY.m[2][0] = -sin(angle.y);
        RotY.m[2][2] = cos(angle.y);

        //Rotation z-axis
        RotZ.Identity();
        RotZ.m[0][0] = cos(angle.z);
        RotZ.m[0][1] = -sin(angle.z);
        RotZ.m[1][0] = sin(angle.z);
        RotZ.m[1][1] = cos(angle.z);

        //Concatenate rotations
        Rot = RotZ * RotY * RotX;
    }

    //Scale
    Matrix4 Scale;
    Scale.Identity();

    if (scale)
    {
        Scale.m[0][0] = obj.sca.x;
        Scale.m[1][1] = obj.sca.y;
        Scale.m[2][2] = obj.sca.z;
    }

    //Complete concatenation for the m2w matrix
    Matrix4 m2w = Transl * Rot * Scale;

    //If there is a parent, multiply its the M2W matrix
    if (strcmp(obj.parent.c_str(), "None"))
    {
        CS250Parser::Transform* parent = FindObject(obj.parent);

        if (parent)
            m2w = ModelToWorld(*parent, false) * m2w;
    }

    return m2w;
}



/**
* @brief GetInput:  change tank with input from user
*
* @return           whether to draw the tank as solid
*/
bool Tank::GetInput()
{
    //Tank body rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        FindObject("body")->rot.y += 0.05f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        FindObject("body")->rot.y -= 0.05f;


    //Turret rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        FindObject("turret")->rot.y += 0.05f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        FindObject("turret")->rot.y -= 0.05f;


    //Gun rotation
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        FindObject("joint")->rot.x += 0.05f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        FindObject("joint")->rot.x -= 0.05f;



    //Move tank forward
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        //Move body
        FindObject("body")->pos.z += 1.f * cos(FindObject("body")->rot.y);
        FindObject("body")->pos.x += 1.f * sin(FindObject("body")->rot.y);

        //Turn wheels
        FindObject("wheel1")->rot.x += 0.1f;
        FindObject("wheel2")->rot.x += 0.1f;
        FindObject("wheel3")->rot.x += 0.1f;
        FindObject("wheel4")->rot.x += 0.1f;

    }

    //Check solid/wireframe mode
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
        return false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
        return true;


    return draw_mode_solid;
}
