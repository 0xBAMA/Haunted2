
#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <iostream>
using std::cout;
using std::endl;

#include <random>



// #include "engine.hpp"   //engine class
// #include "accoutrement.h"    //accoutrement class
// #include """


#define POINT_SPRITE_PATH "resources/textures/height/sphere_small.png"

#define WATER_HEIGHT_TEXTURE "resources/textures/height/water_height.png"
#define WATER_NORMAL_TEXTURE "resources/textures/normal/water_normal.png"
#define WATER_COLOR_TEXTURE "resources/textures/water_color.png"

//************************************************

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>


// GLUT
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>



// Shader Compilation
#include "shaders/Shader.h"

//**********************************************

#include "../resources/LodePNG/lodepng.h"
// Good, simple png library


#include "../resources/perlin.h"
//perlin noise generation

//**********************************************

// #define GLM_MESSAGES
// #define GLM_SWIZZLE
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "glm/glm.hpp" //general vector types
// #include "glm/gtc/swizzle.hpp"    //swizzling
#include "glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "glm/gtc/type_ptr.hpp" //to send matricies gpu-side
#include "glm/gtx/transform.hpp"





namespace JonDefault{


    glm::mat4 view = glm::lookAt(
        glm::vec3(-1.3f, 0.7f, -1.7f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 proj = glm::perspective(glm::radians(65.0f), 1366.0f / 768.0f, 0.25f, 4.0f);

    glm::vec3 player_position = glm::vec3(0,0.1,0);
    glm::vec3 player_forward = glm::vec3(0,0,1);   //+z
    glm::vec3 player_left = glm::vec3(1,0,0);     //+x
    glm::vec3 player_up = glm::vec3(0,1,0);      //+y

    typedef enum state_t
    {//control of the player's position
      floor1=1,
      floor2=2,
      floor3=3,
      onetotwo=4,
      twotothree=5,
      threetotwo=6,
      twotoone=7
    } state;


    float room1start = -0.558f;
    float room1end = -0.29f;

    float room2start = -0.27f;
    float room2end = 0.064f;

    float room3start = 0.082f;
    float room3end = 0.624f;




    float tallroom1start = -0.596f;
    float tallroom1end = -0.154f;

    float tallroom2start = -0.132f;
    float tallroom2end = 0.414f;


    

    float floor1yoffset = 0.072f;
    float floor2yoffset = -0.062f;
    float floor3yoffset = -0.196f;

    float xoffset = 0.05f;
    float yoffset = 0.08f;
    float zoffset = 0.55f;
    float radius  = 0.17f;


}

//******************************************************************************
//  Function: planetest
//
//  NOTE: This comes up with some frequency. I'll be carrying this along for a while.
//
//  Purpose:
//      Return true if the test point is below the plane. Return false if the
//      test point is below the plane. Above and below are determined with
//      respect to the normal specified by plane_norm. This is used to confirm
//      that computed normals are actually pointing outwards, by testing a
//      point that is known to be inside the shape against the computed normal.
//      If the point is below the plane specified by plane_norm and plane_point,
//      we know that that normal will be valid. Otherwise, it needs to be
//      inverted.
//
//  Parameters:
//      plane_point - the point from which the normal will originate
//      plane_norm - what direction is 'up'?
//      test_point - you want to know if this is above or below the plane
//
//  Preconditions:
//      plane_norm must be given as a nonzero vector
//
//  Postconditions:
//      true or false is returned to tell the user the restult of their query
//      TRUE IF YOUR TEST POINT IS BELOW THE PLANE
//
//******************************************************************************

bool planetest(glm::vec3 plane_point, glm::vec3 plane_norm, glm::vec3 test_point)
{
  double result, a, b, c, x, x1, y, y1, z, z1;

  a  =  plane_norm.x;   b  =  plane_norm.y;  c  =  plane_norm.z;
  x  =  test_point.x;   y  =  test_point.y;  z  =  test_point.z;
  x1 = plane_point.x;   y1 = plane_point.y;  z1 = plane_point.z;

  //equation of a plane is:
    // a (x-x1) + b (y-y1) + c (z-z1) = 0;

  result = a * (x-x1) + b * (y-y1) + c * (z-z1);

  return (result < 0) ? true:false;
}


//function: capsule sdf
float capsdf(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r)
{
  glm::vec3 ab = b-a;
  glm::vec3 ap = p-a;

  float t = glm::dot(ab,ap)/glm::dot(ab,ab);

  t = glm::clamp(t, 0.0f, 1.0f);

  glm::vec3 c = a + (t * ab);

  return glm::length(p-c) - r;
}


#endif
