//******************************************************************************
//  Program: Haunted
//
//  Author: Jon Baker
//  Email: jb239812@ohio.edu

//
//  Date: 6 November 2019
//******************************************************************************

#include "common.hpp"
#include "accoutrement.hpp"
#include "engine.hpp"


//******************************************************************************
//  Class: Sub
//
//  Purpose:  To represent the ship on the GPU, and everything that goes along
//        with displaying this ship to the user.
//
//  Functions:
//
//    Constructor:
//        Takes no arguments, calls generate_points() to create geometry. Then
//        buffers all this data to the GPU memory. Textures are handled by
//        the class, and each panel has its own display function. This allows
//        the 6 panels to be drawn in depth-order (back to front)
//
//    Setters:
//        Used to update the values of the uniform variables.
//
//    Generate Points:
//        Creates a square for each panel, subdivides the faces several times,
//        and creates triangles to span the shape. This data is used to populate
//        the vectors containing point data.
//
//    Display functions
//        The top level display funciton orders the panels by depth, then draws
//        them in order, from farthest to nearest. It calls the functions specific
//        to each panel, for each panel.
//
//        For each panel, it makes sure the correct shader is being used, that
//        the correct buffers are bound, that the vertex attributes are set up,
//        and that all the latest values of the uniform variables are sent to the
//        GPU. In addition to this, make sure that all the textures are bound the
//        correct texture units.
//******************************************************************************


class Sub{

public:
  Sub();

  void display();

  void draw_hull_func();
  void draw_rooms_func();

  // void display_panel(int num);  //display the appropriate side, 1-6 - holdover from SpAce


  void adjust_roll_rate(float adj)  {roll_rate += adj;}
  void adjust_pitch_rate(float adj) {pitch_rate += adj;}
  void adjust_yaw_rate(float adj)   {yaw_rate += adj;}

  // void adjust_roll(int degrees);   //upcoming, allows for manual control over orientation of the ship - when used, zero out the rate value
  // void adjust_pitch(int degrees);
  // void adjust_yaw(int degrees);

  void update_rotation();

  void set_proj(glm::mat4 proj);
  void set_view(glm::mat4 view);
  void set_scale(float scale);
  void set_time(int tin)          {t = tin; glUniform1i(t_loc,t);}

  void toggle_hull()              {draw_hull = !draw_hull;}
  void toggle_room(int n)         {draw_room[n] = !draw_room[n];}



private:


  void generate_points();
  void subd_square(glm::vec3 a, glm::vec2 at, glm::vec3 b, glm::vec2 bt, glm::vec3 c, glm::vec2 ct, glm::vec3 d, glm::vec2 dt, glm::vec3 norm, float clow, float chigh);

  //pitch, yaw, roll

  float roll_rate, pitch_rate, yaw_rate;

//BUFFER, VAO
  GLuint vao;
  GLuint buffer;

// //TEXTURES - load them all in the init, then bind the appropriate ones in the associated display functions
//   GLuint panel_height[8];
//   GLuint panel_color[8];
//   GLuint panel_normal[8];



  GLuint point_sprite_tex;



//SHADERS
  GLuint sub_shader;
  // GLuint axes_shader;
  GLuint room_shader;



//UNIFORMS
  GLuint yawpitchroll_loc;
  glm::vec3 yawpitchroll;

  GLuint proj_loc;
  glm::mat4 proj;

  GLuint view_loc;
  glm::mat4 view;


  GLuint eye_position_loc, light_position_loc;
  glm::vec3 eye_position, light_position, original_light_position;


  GLuint scale_loc;
  GLfloat scale;

  GLuint t_loc;
  int t;



//The vertex data
  std::vector<glm::vec3> points;    //add the 1.0 w value in the shader
  std::vector<glm::vec2> texcoords; //texture coordinates
  std::vector<glm::vec3> normals;   //used for displacement along the normals
  std::vector<glm::vec4> colors;    //support alpha

//vertex attribs
  GLuint points_attrib;
  GLuint texcoords_attrib;
  GLuint normals_attrib;
  GLuint colors_attrib;



  bool draw_hull;       //draw for the hull
  bool draw_room[9];    //draw for each of the rooms

  int hull_start, hull_num; //start of hull geometry, number of verticies in the hull geometry
  int room_start[9], room_num[9]; //start of room geometry, number of verticies in the room geometry, per each of the 9 rooms

  typedef struct triangle_t{
    glm::vec3 points[3];
    glm::vec2 texcoords[3];
    glm::vec4 colors[3];
    glm::vec3 normals[3];

    bool done;
  } triangle;

  std::vector<triangle> triangles;

  accoutrement bits;  //holds buttons, keys, the ghost
  // roommodel rooms;    //holds the rooms separate from the hull
  engine sub_engine;  //does animation for the engine

};


// //******************************************************************************

Sub::Sub()
{

    //initialize all the vectors
    points.clear();
    texcoords.clear();
    normals.clear();
    colors.clear();

    //fill those vectors with geometry
    generate_points();




    draw_hull = true;

    for(int i = 0; i < 9; i++)
      draw_room[i] = true;

  //SETTING UP GPU STUFF


    //SHADERS (COMPILE, USE)

    cout << " compiling ship shaders" << endl;
    Shader s("resources/shaders/hull_vert.glsl", "resources/shaders/hull_frag.glsl");

    sub_shader = s.Program;


    //VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //BUFFER, SEND DATA
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glUseProgram(sub_shader);


  //POPULATE THE ARRAYS

    const GLuint num_bytes_points = sizeof(glm::vec3) * points.size();
    const GLuint num_bytes_texcoords = sizeof(glm::vec2) * texcoords.size();
    const GLuint num_bytes_normals = sizeof(glm::vec3) * normals.size();
    const GLuint num_bytes_colors = sizeof(glm::vec4) * colors.size();

    GLint num_bytes = num_bytes_points + num_bytes_texcoords + num_bytes_normals + num_bytes_colors;

    glBufferData(GL_ARRAY_BUFFER, num_bytes, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes_points, &points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points, num_bytes_texcoords, &texcoords[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_texcoords, num_bytes_normals, &normals[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_texcoords + num_bytes_normals, num_bytes_colors, &colors[0]);

    //VERTEX ATTRIBS
      //todo - https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
          //  - http://docs.gl/gl4/glVertexAttribPointer

      //these are the per-vertex attributes (point location, texture coordinate, normal vector, and, at least initially, a color value)

    points_attrib = glGetAttribLocation(sub_shader, "vPosition");
    texcoords_attrib = glGetAttribLocation(sub_shader, "vTexCoord");
    normals_attrib = glGetAttribLocation(sub_shader, "vNormal");
    colors_attrib = glGetAttribLocation(sub_shader, "vColor");

    glEnableVertexAttribArray(points_attrib);
    glEnableVertexAttribArray(texcoords_attrib);
    glEnableVertexAttribArray(normals_attrib);
    glEnableVertexAttribArray(colors_attrib);

    cout << "setting up points attrib" << endl;
    glVertexAttribPointer(points_attrib, 3, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (0)));
    cout << "setting up texcoords attrib" << endl;
    glVertexAttribPointer(texcoords_attrib, 2, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (num_bytes_points)));
    cout << "setting up normals attrib" << endl;
    glVertexAttribPointer(normals_attrib, 3, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (num_bytes_points + num_bytes_texcoords)));
    cout << "setting up colors attrib" << endl;
    glVertexAttribPointer(colors_attrib, 4, GL_FLOAT, false, 0, (static_cast<const char*>(0) + (num_bytes_points + num_bytes_texcoords + num_bytes_normals)));




    //UNIFORMS
    yawpitchroll_loc = glGetUniformLocation(sub_shader, "yawpitchroll");
    yawpitchroll = glm::vec3(0,0,0);
    glUniform3fv(yawpitchroll_loc, 1, glm::value_ptr(yawpitchroll));

    proj_loc = glGetUniformLocation(sub_shader, "proj");
    glUniformMatrix4fv(proj_loc, 1, GL_TRUE, glm::value_ptr(proj));
    // glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));

    view_loc = glGetUniformLocation(sub_shader, "view");
    glUniformMatrix4fv(view_loc, 1, GL_TRUE, glm::value_ptr(view));
    // glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

    scale = 1.0;
    scale_loc = glGetUniformLocation(sub_shader, "scale");
    glUniform1fv(scale_loc, 1, &scale);

    t = 0;
    t_loc = glGetUniformLocation(sub_shader, "t");
    glUniform1i(t_loc,t);



    eye_position_loc = glGetUniformLocation(sub_shader, "eye_position");
    eye_position = glm::vec3(-1.3f, 1.0f, -1.7f);
    glUniform3fv(eye_position_loc, 1, glm::value_ptr(eye_position));


    light_position_loc = glGetUniformLocation(sub_shader, "light_position");
    light_position = original_light_position = glm::vec3(0,0,0);
    glUniform3fv(light_position_loc, 1, glm::value_ptr(light_position));




    //And now, the generation and loading of a great many textures
    // glEnable(GL_TEXTURE_2D);

    // glGenTextures(8, &panel_height[0]);
    // glGenTextures(8, &panel_color[0]);
    // glGenTextures(8, &panel_normal[0]);

    // load_textures();


    glUniform1i(glGetUniformLocation(sub_shader, "type"), 0);















    glPointSize(6.0f);
}

// //******************************************************************************


  //****************************************************************************
  //  Function: Sub::generate_points()
  //
  //  Purpose:
  //    This function produces all the data for representing this object.
  //****************************************************************************

void Sub::generate_points()
{
//GENERATING GEOMETRY

  hull_start = points.size();

  triangles.clear();

  glm::vec3 a,b,c,d;
  glm::vec2 at,bt,ct,dt;

  glm::vec3 norm = glm::vec3(0,0,1);

  float scale = 0.5f;
  float xfactor = 1.0f;

  a = glm::vec3(xfactor*scale,scale,scale);
  b = glm::vec3(-xfactor*scale,scale,scale);
  c = glm::vec3(xfactor*scale,-scale,scale);
  d = glm::vec3(-xfactor*scale,-scale,scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);

  a = glm::vec3(-xfactor*scale,-scale,-scale);
  b = glm::vec3(-xfactor*scale,scale,-scale);
  c = glm::vec3(xfactor*scale,-scale,-scale);
  d = glm::vec3(xfactor*scale,scale,-scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(0,0,-1);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);








  a = glm::vec3(-xfactor*scale,scale,-scale);
  b = glm::vec3(-xfactor*scale,scale,scale);
  c = glm::vec3(xfactor*scale,scale,-scale);
  d = glm::vec3(xfactor*scale,scale,scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(0,1,0);


  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);

  a = glm::vec3(xfactor*scale,-scale,scale);
  b = glm::vec3(-xfactor*scale,-scale,scale);
  c = glm::vec3(xfactor*scale,-scale,-scale);
  d = glm::vec3(-xfactor*scale,-scale,-scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(0,-1,0);


  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);










  a = glm::vec3(xfactor*scale,scale,scale);
  b = glm::vec3(xfactor*scale,-scale,scale);
  c = glm::vec3(xfactor*scale,scale,-scale);
  d = glm::vec3(xfactor*scale,-scale,-scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(1,0,0);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);

  a = glm::vec3(-xfactor*scale,-scale,-scale);
  b = glm::vec3(-xfactor*scale,-scale,scale);
  c = glm::vec3(-xfactor*scale,scale,-scale);
  d = glm::vec3(-xfactor*scale,scale,scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(-1,0,0);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);



  // glm::vec3 acap,bcap;
  // float r;
  //
  // // acap = glm::vec3(0.5, 0, 0);
  // // bcap = glm::vec3(-0.5, 0,0);
  // // r = 0.1618;
  //
  // acap = glm::vec3( 0.3,0,0);
  // bcap = glm::vec3(-0.3,0,0);
  // r = 0.5;
  //
  //
  // int passes = 0;
  // int count = 0;
  //
  //
  // bool some_not_done = true;
  //
  // while(some_not_done)
  // {
  //   some_not_done = false;
  //   count = 0;
  //   for(auto &x : triangles)
  //   {
  //     if(!x.done)
  //     {
  //       //shrink each point in, if it's still outside the shape
  //       float d0,d1,d2;
  //
  //       d0 = capsdf(x.points[0],acap,bcap,r);
  //       d1 = capsdf(x.points[1],acap,bcap,r);
  //       d2 = capsdf(x.points[2],acap,bcap,r);
  //
  //
  //       if(abs(d0)>0.0001)
  //       {
  //         if(d0 > 0)
  //           x.points[0] *= 0.99999;
  //
  //         if(d0 < 0)
  //           x.points[0] /= 0.5;
  //       }
  //
  //
  //
  //       if(abs(d1)>0.0001)
  //       {
  //         if(d1 > 0)
  //           x.points[1] *= 0.99999;
  //
  //         if(d1 < 0)
  //           x.points[1] /= 0.5;
  //       }
  //
  //
  //
  //       if(abs(d2)>0.0001)
  //       {
  //         if(d2 > 0)
  //           x.points[2] *= 0.99999;
  //
  //         if(d2 < 0)
  //           x.points[2] /= 0.5;
  //       }
  //
  //       glm::vec3 mid = (x.points[0] + x.points[1] + x.points[2]) / 3.0f;
  //
  //       if((abs(capsdf(x.points[0], acap, bcap, r)) < 0.0001) && (abs(capsdf(x.points[1], acap, bcap, r)) < 0.0001) && (abs(capsdf(x.points[2], acap, bcap, r)) < 0.0001))
  //       {
  //         x.done = true;
  //
  //         // glm::vec3 norm = glm::normalize(glm::cross(x.points[0]-x.points[1], x.points[0]-x.points[2]));
  //         // if(!planetest(mid, norm, glm::vec3(0,0,0)))
  //         //    norm = norm * -1.0f;
  //
  //         float d = capsdf(mid, acap, bcap, r);
  //         glm::vec2 e = glm::vec2(0.01, 0);
  //
  //         // glm::vec3 norm = glm::normalize(glm::vec3(d,d,d) -
  //         //   glm::vec3(capsdf(glm::vec3(mid.x-e.x,mid.y-e.y,mid.z-e.y), acap, bcap, r),
  //         //             capsdf(glm::vec3(mid.x-e.y,mid.y-e.x,mid.z-e.y), acap, bcap, r),
  //         //             capsdf(glm::vec3(mid.x-e.y,mid.y-e.y,mid.z-e.x), acap, bcap, r)));
  //         // x.normals[0] = norm;
  //         // x.normals[1] = norm;
  //         // x.normals[2] = norm;
  //
  //         x.normals[0] = glm::normalize(glm::vec3(d,d,d) -
  //           glm::vec3(capsdf(glm::vec3(x.points[0].x-e.x,x.points[0].y-e.y,x.points[0].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[0].x-e.y,x.points[0].y-e.x,x.points[0].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[0].x-e.y,x.points[0].y-e.y,x.points[0].z-e.x), acap, bcap, r)));
  //
  //         x.normals[1] = glm::normalize(glm::vec3(d,d,d) -
  //           glm::vec3(capsdf(glm::vec3(x.points[1].x-e.x,x.points[1].y-e.y,x.points[1].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[1].x-e.y,x.points[1].y-e.x,x.points[1].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[1].x-e.y,x.points[1].y-e.y,x.points[1].z-e.x), acap, bcap, r)));
  //
  //         x.normals[2] = glm::normalize(glm::vec3(d,d,d) -
  //           glm::vec3(capsdf(glm::vec3(x.points[2].x-e.x,x.points[2].y-e.y,x.points[2].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[2].x-e.y,x.points[2].y-e.x,x.points[2].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[2].x-e.y,x.points[2].y-e.y,x.points[2].z-e.x), acap, bcap, r)));
  //
  //
  //
  //
  //       }
  //     }
  //
  //     if(!x.done)
  //     {
  //       some_not_done = true;
  //       count++;
  //     }
  //   }
  //   passes++;
  //
  //   cout << "\rpass number " << passes  << " touched " << count << "                      ";
  // }





  float xoffset, yoffset, zoffset, radius;


//OFFICIAL SUBMARINE DIMENSIONS
  xoffset = JonDefault::xoffset;
  yoffset = JonDefault::yoffset;
  zoffset = JonDefault::zoffset;
  radius  = JonDefault::radius;

  bool panels = true;




// //CUBOID
//   xoffset = 0.2f;
//   yoffset = 0.2f;
//   zoffset = 0.2f;
//   radius  = 0.1f;
//
//   bool panels = false;







  for(auto x : triangles)
  {

    glm::vec3 midp = (radius*glm::normalize(x.points[0]) +
                        radius*glm::normalize(x.points[1]) +
                           radius*glm::normalize(x.points[2]))/3.0f;

    if(midp.x > 0 && midp.y  > 0 )
    {
      if(midp.z > 0)
      {
        // colors.push_back(glm::vec4(0.5,0.0,0.5,1.0));
        // colors.push_back(glm::vec4(0.5,0.0,0.5,1.0));
        // colors.push_back(glm::vec4(0.5,0.0,0.5,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,yoffset,zoffset));
      }
      else
      {
        // colors.push_back(glm::vec4(0.15,0.0,0.15,1.0));
        // colors.push_back(glm::vec4(0.15,0.0,0.15,1.0));
        // colors.push_back(glm::vec4(0.15,0.0,0.15,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,yoffset,-zoffset));
      }
    }
    else if ( midp.x > 0 && midp.y  < 0 )
    {
      if(midp.z > 0)
      {
        // colors.push_back(glm::vec4(0.1,0.6,0.1,1.0));
        // colors.push_back(glm::vec4(0.1,0.6,0.1,1.0));
        // colors.push_back(glm::vec4(0.1,0.6,0.1,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,-yoffset,zoffset));
      }
      else
      {
        // colors.push_back(glm::vec4(0.03,0.18,0.03,1.0));
        // colors.push_back(glm::vec4(0.03,0.18,0.03,1.0));
        // colors.push_back(glm::vec4(0.03,0.18,0.03,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,-yoffset,-zoffset));
      }
    }
    else if ( midp.x < 0 && midp.y  > 0 )
    {
      if(midp.z > 0)
      {
        // colors.push_back(glm::vec4(0.1,0.1,0.3,1.0));
        // colors.push_back(glm::vec4(0.1,0.1,0.3,1.0));
        // colors.push_back(glm::vec4(0.1,0.1,0.3,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,yoffset,zoffset));
      }
      else
      {
        // colors.push_back(glm::vec4(0.03,0.03,0.09,1.0));
        // colors.push_back(glm::vec4(0.03,0.03,0.09,1.0));
        // colors.push_back(glm::vec4(0.03,0.03,0.09,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,yoffset,-zoffset));
      }
    }
    else if ( midp.x < 0 && midp.y  < 0 )
    {
      if(midp.z > 0)
      {
        // colors.push_back(glm::vec4(0.5,0.1,0.0,1.0));
        // colors.push_back(glm::vec4(0.5,0.1,0.0,1.0));
        // colors.push_back(glm::vec4(0.5,0.1,0.0,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,-yoffset,zoffset));
      }
      else
      {
        // colors.push_back(glm::vec4(0.15,0.03,0.0,1.0));
        // colors.push_back(glm::vec4(0.15,0.03,0.0,1.0));
        // colors.push_back(glm::vec4(0.15,0.03,0.0,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,-yoffset,-zoffset));
      }
    }
    else
    {
      // points.push_back(radius*glm::normalize(x.points[0]));
      // points.push_back(radius*glm::normalize(x.points[1]));
      // points.push_back(radius*glm::normalize(x.points[2]));
      colors.push_back(x.colors[0]);
      colors.push_back(x.colors[1]);
      colors.push_back(x.colors[2]);
    }



    texcoords.push_back(x.texcoords[0]);
    texcoords.push_back(x.texcoords[1]);
    texcoords.push_back(x.texcoords[2]);



    // normals.push_back(x.normals[0]);
    // normals.push_back(x.normals[1]);
    // normals.push_back(x.normals[2]);

    normals.push_back(glm::normalize(x.points[0]));
    normals.push_back(glm::normalize(x.points[1]));
    normals.push_back(glm::normalize(x.points[2]));


  }


  float twopi = 2*3.1415926535;
  float rot_inc = twopi/400;

  //cylinders +/- x
  for(float rot = -rot_inc; rot <= 5*twopi; rot += rot_inc)
  {
    float xcur = radius * cos(rot);
    float xprev = radius * cos(rot-rot_inc);

    float ycur = radius * sin(rot);
    float yprev = radius * sin(rot-rot_inc);

    norm = glm::normalize(glm::vec3(xcur, ycur, 0));


    if(xcur > 0)
    {
      if(ycur > 0)
      {
        points.push_back(glm::vec3(xcur+xoffset,ycur+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur+yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev+yoffset,-zoffset));

        points.push_back(glm::vec3(xprev+xoffset,yprev+yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur+yoffset,zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur+xoffset,ycur-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur-yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev-yoffset,-zoffset));

        points.push_back(glm::vec3(xprev+xoffset,yprev-yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur-yoffset,zoffset));
      }

    }
    else
    {
      if(ycur > 0)
      {
        points.push_back(glm::vec3(xcur-xoffset,ycur+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur+yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev+yoffset,-zoffset));

        points.push_back(glm::vec3(xprev-xoffset,yprev+yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur+yoffset,zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur-xoffset,ycur-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur-yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev-yoffset,-zoffset));

        points.push_back(glm::vec3(xprev-xoffset,yprev-yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur-yoffset,zoffset));
      }
    }

    for(int i = 0; i < 6; i++)
      normals.push_back(norm);

    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));

    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));

    // texcoords.push_back();   //not really defined
  }





  //cylinders +/- y
  for(float rot = -rot_inc; rot <= 5*twopi; rot += rot_inc)
  {
    float xcur = radius * cos(rot);
    float xprev = radius * cos(rot-rot_inc);

    float zcur = radius * sin(rot);
    float zprev = radius * sin(rot-rot_inc);

    norm = glm::normalize(glm::vec3(xcur, 0, zcur));


    if(xcur > 0)
    {
      if(zcur > 0)
      {
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xcur+xoffset,-yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev+zoffset));

        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur+zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,-yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev-zoffset));

        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur-zoffset));
      }

    }
    else
    {
      if(zcur > 0)
      {
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xcur-xoffset,-yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev+zoffset));

        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur+zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,-yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev-zoffset));

        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur-zoffset));
      }
    }

    for(int i = 0; i < 6; i++)
      normals.push_back(norm);

    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));

    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));

    // texcoords.push_back();   //not really defined
  }






  //cylinders +/- z
  for(float rot = -rot_inc; rot <= 5*twopi; rot += rot_inc)
  {
    float ycur = radius * cos(rot);
    float yprev = radius * cos(rot-rot_inc);

    float zcur = radius * sin(rot);
    float zprev = radius * sin(rot-rot_inc);

    norm = glm::normalize(glm::vec3(0, ycur, zcur));


    if(ycur > 0)
    {
      if(zcur > 0)
      {

        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,ycur+yoffset,zcur+zoffset));

        points.push_back(glm::vec3(xoffset,yprev+yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur+zoffset));

      }
      else
      {

        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,ycur+yoffset,zcur-zoffset));

        points.push_back(glm::vec3(xoffset,yprev+yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur-zoffset));

      }

    }
    else
    {
      if(zcur > 0)
      {
        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,ycur-yoffset,zcur+zoffset));

        points.push_back(glm::vec3(xoffset,yprev-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur+zoffset));

      }
      else
      {

        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,ycur-yoffset,zcur-zoffset));

        points.push_back(glm::vec3(xoffset,yprev-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur-zoffset));
      }
    }

    for(int i = 0; i < 6; i++)
      normals.push_back(norm);

    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));

    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));
    colors.push_back(glm::vec4(0,0,0,1));

    // texcoords.push_back();   //not really defined
  }

  //panels (+x, -x, +y, -y, +z, -z)


  if(panels)
  {
    //+x
    points.push_back(glm::vec3(radius+xoffset,yoffset,-zoffset));
    points.push_back(glm::vec3(radius+xoffset,yoffset,zoffset));
    points.push_back(glm::vec3(radius+xoffset,-yoffset,zoffset));

    points.push_back(glm::vec3(radius+xoffset,-yoffset,zoffset));
    points.push_back(glm::vec3(radius+xoffset,-yoffset,-zoffset));
    points.push_back(glm::vec3(radius+xoffset,yoffset,-zoffset));

    for(int i = 0; i < 6; i++)
      normals.push_back(glm::vec3(1.0,0,0));

    for(int i = 0; i < 6; i++)
      colors.push_back(glm::vec4(0,0,0,1));

    //-x
    points.push_back(glm::vec3(-1.0f*(radius+xoffset),yoffset,zoffset));
    points.push_back(glm::vec3(-1.0f*(radius+xoffset),yoffset,-zoffset));
    points.push_back(glm::vec3(-1.0f*(radius+xoffset),-yoffset,zoffset));

    points.push_back(glm::vec3(-1.0f*(radius+xoffset),-yoffset,-zoffset));
    points.push_back(glm::vec3(-1.0f*(radius+xoffset),-yoffset,zoffset));
    points.push_back(glm::vec3(-1.0f*(radius+xoffset),yoffset,-zoffset));

    for(int i = 0; i < 6; i++)
      normals.push_back(glm::vec3(-1.0,0,0));

    for(int i = 0; i < 6; i++)
      colors.push_back(glm::vec4(0,0,0,1));


    //+y
    points.push_back(glm::vec3(xoffset,radius+yoffset,zoffset));
    points.push_back(glm::vec3(xoffset,radius+yoffset,-zoffset));
    points.push_back(glm::vec3(-xoffset,radius+yoffset,zoffset));

    points.push_back(glm::vec3(-xoffset,radius+yoffset,-zoffset));
    points.push_back(glm::vec3(-xoffset,radius+yoffset,zoffset));
    points.push_back(glm::vec3(xoffset,radius+yoffset,-zoffset));

    for(int i = 0; i < 6; i++)
      normals.push_back(glm::vec3(0,1.0,0));

    for(int i = 0; i < 6; i++)
      colors.push_back(glm::vec4(0,0,0,1));


    //-y
    points.push_back(glm::vec3(xoffset,-1.0f*(radius+yoffset),-zoffset));
    points.push_back(glm::vec3(xoffset,-1.0f*(radius+yoffset),zoffset));
    points.push_back(glm::vec3(-xoffset,-1.0f*(radius+yoffset),zoffset));

    points.push_back(glm::vec3(-xoffset,-1.0f*(radius+yoffset),zoffset));
    points.push_back(glm::vec3(-xoffset,-1.0f*(radius+yoffset),-zoffset));
    points.push_back(glm::vec3(xoffset,-1.0f*(radius+yoffset),-zoffset));

    for(int i = 0; i < 6; i++)
      normals.push_back(glm::vec3(0,-1.0,0));

    for(int i = 0; i < 6; i++)
      colors.push_back(glm::vec4(0,0,0,1));


    //+z
    points.push_back(glm::vec3(xoffset,-yoffset,radius+zoffset));
    points.push_back(glm::vec3(xoffset,yoffset,radius+zoffset));
    points.push_back(glm::vec3(-xoffset,yoffset,radius+zoffset));

    points.push_back(glm::vec3(-xoffset,yoffset,radius+zoffset));
    points.push_back(glm::vec3(-xoffset,-yoffset,radius+zoffset));
    points.push_back(glm::vec3(xoffset,-yoffset,radius+zoffset));

    for(int i = 0; i < 6; i++)
      normals.push_back(glm::vec3(0,0,1.0));

    for(int i = 0; i < 6; i++)
      colors.push_back(glm::vec4(0,0,0,1));


    //-z
    points.push_back(glm::vec3(xoffset,-yoffset,-1.0f*(radius+zoffset)));
    points.push_back(glm::vec3(-xoffset,yoffset,-1.0f*(radius+zoffset)));
    points.push_back(glm::vec3(xoffset,yoffset,-1.0f*(radius+zoffset)));

    points.push_back(glm::vec3(-xoffset,yoffset,-1.0f*(radius+zoffset)));
    points.push_back(glm::vec3(xoffset,-yoffset,-1.0f*(radius+zoffset)));
    points.push_back(glm::vec3(-xoffset,-yoffset,-1.0f*(radius+zoffset)));

    for(int i = 0; i < 6; i++)
      normals.push_back(glm::vec3(0,0,-1.0));

    for(int i = 0; i < 6; i++)
      colors.push_back(glm::vec4(0,0,0,1));

  }



  hull_num = points.size() - hull_start;
  cout << "hull starts at " << hull_start << " and is " << hull_num << " verticies" << endl;











  //ROOMS DEFINED HERE

    //the space alotted for rooms is 2*(xoffset+(cos(45)*radius - epsilon)) by 2*(yoffset+(cos(45)*radius - epsilon)) by 2*(zoffset+(cos(45)*radius - epsilon))

    // NOTE: check that, I was thinking 2d when I did these calculations

    //so basically if xoffset=yoffset=zoffset=0, you have a cube whose corners just almost touch the inside of the sphere
    //however- we use the defined xoffset, yoffset, zoffset, radius, from before


    //with that being said - it is worth considering we don't want to fill that space - and with the note above,
    //you can sort of see why we want to kind of limit it a little more than you would otherwise

    //we can split up the width:  v-- the interstitial space will give you separation for the doors (sliding hatches)
    //  |------------------------|-|-----------------------------|-|------------------------|

    //we may want to vary this split, per floor

    //        --using the captain's wheel will spin it, allowing the player to change the orientation of the ship
    //        v               in incrments - two buttons, on opposite sides of the wheel - buttons may become important
    //|--captain's-wheel--|-|--terminal--|-|-----start--|
    //|--^----|-|-------2-floors---------|-|-engine-hi--| < top parts of the engine (cams, valves, some pistons visible)
    //|--^----|-|-----of-computers-------|-|-engine-low-| < bottom parts of engine (crank, con rods, pistons)

  //******************************************************************************


  float inc = 0.01f;


  float room1start, room1end;
  float room2start, room2end;
  float room3start, room3end;

  float tallroom1start, tallroom1end;
  float tallroom2start, tallroom2end;

  room1start =  JonDefault::room1start;
  room1end =  JonDefault::room1end;

  room2start =  JonDefault::room2start;
  room2end = JonDefault::room2end;

  room3start = JonDefault::room3start;
  room3end = JonDefault::room3end;

  tallroom1start =  JonDefault::tallroom1start;
  tallroom1end =  JonDefault::tallroom1end;

  tallroom2start =  JonDefault::tallroom2start;
  tallroom2end = JonDefault::tallroom2end;

  float floor1yoffset = JonDefault::floor1yoffset;
  float floor2yoffset = JonDefault::floor2yoffset;
  float floor3yoffset = JonDefault::floor3yoffset;



  //ROOM 1
  room_start[0] = points.size();

  //first the round part of the room
  for(float i = 0.0f; i < 3.141592654; i += inc)
  {
    //add a 'slat' about the cylinder - the ends are at room1start and room1end on the z axis
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room1start));
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room1end));
    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room1end));

    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room1start));
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room1start));
    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room1end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }

  }

  //then the bottom, just a plane
  points.push_back(glm::vec3(radius, floor1yoffset, room1start));
  points.push_back(glm::vec3(-radius, floor1yoffset, room1start));
  points.push_back(glm::vec3(radius, floor1yoffset, room1end));

  points.push_back(glm::vec3(-radius, floor1yoffset, room1start));
  points.push_back(glm::vec3(-radius, floor1yoffset, room1end));
  points.push_back(glm::vec3(radius, floor1yoffset, room1end));




//walkway
  points.push_back(glm::vec3(0.2f*radius, floor1yoffset, room1end));
  points.push_back(glm::vec3(-0.2f*radius, floor1yoffset, room1end));
  points.push_back(glm::vec3(0.2f*radius, floor1yoffset, room2start));

  points.push_back(glm::vec3(-0.2f*radius, floor1yoffset, room1end));
  points.push_back(glm::vec3(-0.2f*radius, floor1yoffset, room2start));
  points.push_back(glm::vec3(0.2f*radius, floor1yoffset, room2start));




//walkway
  points.push_back(glm::vec3(0.2f*radius, floor1yoffset, room2end));
  points.push_back(glm::vec3(-0.2f*radius, floor1yoffset, room2end));
  points.push_back(glm::vec3(0.2f*radius, floor1yoffset, room3start));

  points.push_back(glm::vec3(-0.2f*radius, floor1yoffset, room2end));
  points.push_back(glm::vec3(-0.2f*radius, floor1yoffset, room3start));
  points.push_back(glm::vec3(0.2f*radius, floor1yoffset, room3start));




  for(int j = 0; j < 18; j++)
  {
    normals.push_back(glm::vec3(0,1,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }


  //then the end caps

  room_num[0] = points.size() - room_start[0];

  //******************************************************************************

  //ROOM 2
  room_start[1] = points.size();
  //GENER8
  //first the round part of the room
  for(float i = 0.0f; i < 3.141592654; i += inc)
  {
    //add a 'slat' about the cylinder - the ends are at room1start and room1end on the z axis
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room2start));
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room2end));
    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room2end));

    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room2start));
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room2start));
    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room2end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }
  }

  //then the bottom, just a plane
  points.push_back(glm::vec3(radius, floor1yoffset, room2start));
  points.push_back(glm::vec3(-radius, floor1yoffset, room2start));
  points.push_back(glm::vec3(radius, floor1yoffset, room2end));

  points.push_back(glm::vec3(-radius, floor1yoffset, room2start));
  points.push_back(glm::vec3(-radius, floor1yoffset, room2end));
  points.push_back(glm::vec3(radius, floor1yoffset, room2end));

  for(int j = 0; j < 6; j++)
  {
    normals.push_back(glm::vec3(0,1,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }

  //then the end caps

  room_num[1] = points.size() - room_start[1];

  //******************************************************************************

  //ROOM 3
  room_start[2] = points.size();
  //GENER8
  //first the round part of the room
  for(float i = 0.0f; i < 3.141592654; i += inc)
  {
    //add a 'slat' about the cylinder - the ends are at room1start and room1end on the z axis
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room3start));
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room3end));
    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room3end));

    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room3start));
    points.push_back(glm::vec3(radius*cos(i), radius*0.75*sin(i) + floor1yoffset, room3start));
    points.push_back(glm::vec3(radius*cos(i+inc), radius*0.75*sin(i+inc) + floor1yoffset, room3end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }
  }

  //then the bottom, just a plane
  points.push_back(glm::vec3(radius, floor1yoffset, room3start));
  points.push_back(glm::vec3(-radius, floor1yoffset, room3start));
  points.push_back(glm::vec3(radius, floor1yoffset, room3end));

  points.push_back(glm::vec3(-radius, floor1yoffset, room3start));
  points.push_back(glm::vec3(-radius, floor1yoffset, room3end));
  points.push_back(glm::vec3(radius, floor1yoffset, room3end));

  for(int j = 0; j < 6; j++)
  {
    normals.push_back(glm::vec3(0,1,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }

  //then the end caps

  room_num[2] = points.size() - room_start[2];

  //******************************************************************************

  //ROOM 4
  room_start[3] = points.size();
  //GENER8

  for(float i = 0.125*twopi; i <= 0.875*twopi; i+= 0.001)
  {

  }


  room_num[3] = points.size() - room_start[3];









  //******************************************************************************
  //ROOM 5
  room_start[4] = points.size();
  //GENER8


  points.push_back(glm::vec3(0.3f*radius, floor2yoffset, tallroom2start));
  points.push_back(glm::vec3(-0.3f*radius, floor2yoffset, tallroom2start));
  points.push_back(glm::vec3(0.3f*radius, floor2yoffset, tallroom2end));

  points.push_back(glm::vec3(-0.3f*radius, floor2yoffset, tallroom2start));
  points.push_back(glm::vec3(-0.3f*radius, floor2yoffset, tallroom2end));
  points.push_back(glm::vec3(0.3f*radius, floor2yoffset, tallroom2end));

  for(int j = 0; j < 6; j++)
  {
    normals.push_back(glm::vec3(0,1,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }


  room_num[4] = points.size() - room_start[4];


  //******************************************************************************


  //ROOM 6
  room_start[5] = points.size();
  //GENER8

  points.push_back(glm::vec3(0.75f*radius, floor2yoffset, tallroom1start-0.75*(tallroom1start-tallroom1end)));
  points.push_back(glm::vec3(-0.75f*radius, floor2yoffset, tallroom1start-0.75*(tallroom1start-tallroom1end)));
  points.push_back(glm::vec3(0.75f*radius, floor2yoffset, tallroom1end));

  points.push_back(glm::vec3(-0.75f*radius, floor2yoffset, tallroom1start-0.75*(tallroom1start-tallroom1end)));
  points.push_back(glm::vec3(-0.75f*radius, floor2yoffset, tallroom1end));
  points.push_back(glm::vec3(0.75f*radius, floor2yoffset, tallroom1end));

  for(int j = 0; j < 6; j++)
  {
    normals.push_back(glm::vec3(0,1,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }

  room_num[5] = points.size() - room_start[5];








  //******************************************************************************

  //ROOM 7
  room_start[6] = points.size();
  //GENER8
  room_num[6] = points.size() - room_start[6];

  //******************************************************************************

  //ROOM 8
  room_start[7] = points.size();
  //GENER8



  // //TEMPORARY
  //   points.push_back(glm::vec3(radius, floor3yoffset, tallroom1start));
  //   points.push_back(glm::vec3(-radius, floor3yoffset, tallroom1start));
  //   points.push_back(glm::vec3(radius, floor3yoffset, tallroom1end));
  //
  //   points.push_back(glm::vec3(-radius, floor3yoffset, tallroom1start));
  //   points.push_back(glm::vec3(-radius, floor3yoffset, tallroom1end));
  //   points.push_back(glm::vec3(radius, floor3yoffset, tallroom1end));
  //
  //   for(int j = 0; j < 6; j++)
  //     normals.push_back(glm::vec3(0,1,0));
  // //TEMPORARY






  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom1end));

  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom1end));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom1end));


  for(int i = 0; i < 6; i++)
  {
    normals.push_back(glm::vec3(0,-1,0));
    colors.push_back(glm::vec4(1,0,0,1));

  }








  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius, tallroom1start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius, tallroom1end));

  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius, tallroom1end));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom1end));


  for(int i = 0; i < 6; i++)
  {
    normals.push_back(glm::vec3(-1,0,0));
    colors.push_back(glm::vec4(1,0,0,1));

  }


  float temp;
  for(float i = 0.0f; i > -3.141592654/2; i -= inc)
  {
    //add a 'slat' about the cylinder - the ends are at room1start and room1end on the z axis
    points.push_back(glm::vec3(radius*0.25*cos(i) + 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom1start));
    points.push_back(glm::vec3(radius*0.25*cos(i) + 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom1end));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) + 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom1end));

    points.push_back(glm::vec3(radius*0.25*cos(i+inc) + 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom1start));
    points.push_back(glm::vec3(radius*0.25*cos(i) + 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom1start));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) + 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom1end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }

    temp = i;
  }

  // points.push_back(glm::vec3());


  points.push_back(glm::vec3(radius*0.25*cos(temp) + 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom1start));
  points.push_back(glm::vec3(radius*0.25*cos(temp) - 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom1start));
  points.push_back(glm::vec3(radius*0.25*cos(temp) + 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom1end));

  points.push_back(glm::vec3(radius*0.25*cos(temp) + 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom1end));
  points.push_back(glm::vec3(radius*0.25*cos(temp) - 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom1start));
  points.push_back(glm::vec3(radius*0.25*cos(temp) - 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom1end));



  for(int j = 0; j < 6; j++)
  {
    normals.push_back(glm::vec3(-cos(temp),-sin(temp),0));
    colors.push_back(glm::vec4(1,0,0,1));
  }



  for(float i = temp; i > -3.141592654; i-=inc)
  {
    points.push_back(glm::vec3(radius*0.25*cos(i) - 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom1start));
    points.push_back(glm::vec3(radius*0.25*cos(i) - 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom1end));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) - 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom1end));

    points.push_back(glm::vec3(radius*0.25*cos(i+inc) - 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom1start));
    points.push_back(glm::vec3(radius*0.25*cos(i) - 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom1start));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) - 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom1end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }

  }


  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius, tallroom1end));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius, tallroom1start));

  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom1start));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom1end));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius, tallroom1end));


  for(int i = 0; i < 6; i++)
  {
    normals.push_back(glm::vec3(1,0,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }








  room_num[7] = points.size() - room_start[7];

  //******************************************************************************

  //ROOM 9
  room_start[8] = points.size();
  //GENER8



  // points.push_back(glm::vec3(radius, floor3yoffset, tallroom2start));
  // points.push_back(glm::vec3(-radius, floor3yoffset, tallroom2start));
  // points.push_back(glm::vec3(radius, floor3yoffset, tallroom2end));
  //
  // points.push_back(glm::vec3(-radius, floor3yoffset, tallroom2start));
  // points.push_back(glm::vec3(-radius, floor3yoffset, tallroom2end));
  // points.push_back(glm::vec3(radius, floor3yoffset, tallroom2end));
  //
  // for(int j = 0; j < 6; j++)
  //   normals.push_back(glm::vec3(0,1,0));


// #HEADING




  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom2end));

  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom2end));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom2end));


  for(int i = 0; i < 6; i++)
  {
    normals.push_back(glm::vec3(0,-1,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }





  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius, tallroom2start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius, tallroom2end));

  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius, tallroom2end));
  points.push_back(glm::vec3(radius, floor3yoffset+0.25*radius+0.2, tallroom2end));


  for(int i = 0; i < 6; i++)
  {
    normals.push_back(glm::vec3(-1,0,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }


  for(float i = 0.0f; i > -3.141592654/2; i -= inc)
  {
    //add a 'slat' about the cylinder - the ends are at room1start and room1end on the z axis
    points.push_back(glm::vec3(radius*0.25*cos(i) + 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom2start));
    points.push_back(glm::vec3(radius*0.25*cos(i) + 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom2end));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) + 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom2end));

    points.push_back(glm::vec3(radius*0.25*cos(i+inc) + 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom2start));
    points.push_back(glm::vec3(radius*0.25*cos(i) + 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom2start));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) + 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom2end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }

    temp = i;
  }

  // points.push_back(glm::vec3());


  points.push_back(glm::vec3(radius*0.25*cos(temp) + 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom2start));
  points.push_back(glm::vec3(radius*0.25*cos(temp) - 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom2start));
  points.push_back(glm::vec3(radius*0.25*cos(temp) + 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom2end));

  points.push_back(glm::vec3(radius*0.25*cos(temp) + 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom2end));
  points.push_back(glm::vec3(radius*0.25*cos(temp) - 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom2start));
  points.push_back(glm::vec3(radius*0.25*cos(temp) - 0.75*radius, radius*0.25*sin(temp) + floor3yoffset+0.25*radius, tallroom2end));



  for(int j = 0; j < 6; j++)
  {
    normals.push_back(glm::vec3(-cos(temp),-sin(temp),0));
    colors.push_back(glm::vec4(1,0,0,1));
  }



  for(float i = temp; i > -3.141592654; i-=inc)
  {
    points.push_back(glm::vec3(radius*0.25*cos(i) - 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom2start));
    points.push_back(glm::vec3(radius*0.25*cos(i) - 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom2end));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) - 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom2end));

    points.push_back(glm::vec3(radius*0.25*cos(i+inc) - 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom2start));
    points.push_back(glm::vec3(radius*0.25*cos(i) - 0.75*radius, radius*0.25*sin(i) + floor3yoffset+0.25*radius, tallroom2start));
    points.push_back(glm::vec3(radius*0.25*cos(i+inc) - 0.75*radius, radius*0.25*sin(i+inc) + floor3yoffset+0.25*radius, tallroom2end));

    for(int j = 0; j < 6; j++)
    {
      normals.push_back(glm::vec3(-cos(i),-sin(i),0));
      colors.push_back(glm::vec4(1,0,0,1));
    }

  }


  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius, tallroom2end));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius, tallroom2start));

  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom2start));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius+0.2, tallroom2end));
  points.push_back(glm::vec3(-radius, floor3yoffset+0.25*radius, tallroom2end));


  for(int i = 0; i < 6; i++)
  {
    normals.push_back(glm::vec3(1,0,0));
    colors.push_back(glm::vec4(1,0,0,1));
  }



  room_num[8] = points.size() - room_start[8];


  cout << "the rooms collectively have " << points.size() - room_start[0] << " verticies" << endl;


  //THIS IS GOING TO CHANGE - HOWEVER, FOR TIME'S SAKE, THIS IS GOING TO BE FUNCTIONING AS A SCENE CLASS
//THE SCENE CLASS INSTANTIATES ALL THE OBJECTS WHICH HOLD GEOMETRY - WE'RE INHERENTLY SORT OF SINGLE THREADED HERE, BECAUSE OF THE
//NATURE OF OPENGL, SO A TREE STRUCTURE MAKES A LOT OF SENSE - SOME KIND OF TRAVERSAL TO PLACE ALL THE THINGS IN THE WORLD





  sub_engine.init(points, normals, colors); //I think this might be a good pattern, to pass the vectors by reference, keep one vertex array



}

// //******************************************************************************

void Sub::subd_square(glm::vec3 a, glm::vec2 at, glm::vec3 b, glm::vec2 bt, glm::vec3 c, glm::vec2 ct, glm::vec3 d, glm::vec2 dt, glm::vec3 norm, float clow, float chigh)
{

  float thresh = 0.01;
  if(glm::distance(a, b) < thresh || glm::distance(a,c) < thresh || glm::distance(a,d) < thresh)
  {//add points


    triangle temp1, temp2;

    //initially not done, want to still move these around since they're just at the initial positions.
    temp1.done = false;
    temp2.done = false;

    temp1.points[0] = a;
    temp1.points[1] = b;
    temp1.points[2] = c;

    temp2.points[0] = c;
    temp2.points[1] = b;
    temp2.points[2] = d;




    //random number generation
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(clow, chigh);

    //colors
    temp1.colors[0] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp1.colors[1] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp1.colors[2] = glm::vec4(dist(mt), dist(mt), 0, 1);

    temp2.colors[0] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp2.colors[1] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp2.colors[2] = glm::vec4(dist(mt), dist(mt), 0, 1);


    //texcoords

    temp1.texcoords[0] = at;
    temp1.texcoords[1] = bt;
    temp1.texcoords[2] = ct;

    temp2.texcoords[0] = bt;
    temp2.texcoords[1] = ct;
    temp2.texcoords[2] = dt;


    //this scheme uses one point at the center - it was for the spaceship but might be useful elsewhere.
    // glm::vec2 tc = (at + bt + ct + dt)/4.0f;
    //
    // temp1.texcoords[0] = tc;
    // temp1.texcoords[1] = tc;
    // temp1.texcoords[2] = tc;
    //
    // temp2.texcoords[0] = tc;
    // temp2.texcoords[1] = tc;
    // temp2.texcoords[2] = tc;


    //normals
    temp1.normals[0] = norm;
    temp1.normals[1] = norm;
    temp1.normals[2] = norm;

    temp2.normals[0] = norm;
    temp2.normals[1] = norm;
    temp2.normals[2] = norm;

    triangles.push_back(temp1);
    triangles.push_back(temp2);




  }
  else
  { //recurse
    glm::vec3 center = (a + b + c + d) / 4.0f;    //center of the square
    glm::vec2 centert = (at + bt + ct + dt) / 4.0f;

    glm::vec3 bdmidp = (b + d) / 2.0f;            //midpoint between b and d
    glm::vec2 bdmidpt = (bt + dt) / 2.0f;

    glm::vec3 abmidp = (a + b) / 2.0f;            //midpoint between a and b
    glm::vec2 abmidpt = (at + bt) / 2.0f;

    glm::vec3 cdmidp = (c + d) / 2.0f;            //midpoint between c and d
    glm::vec2 cdmidpt = (ct + dt) / 2.0f;

    glm::vec3 acmidp = (a + c) / 2.0f;            //midpoint between a and c
    glm::vec2 acmidpt = (at + ct) / 2.0f;

    subd_square(abmidp, abmidpt, b, bt, center, centert, bdmidp, bdmidpt, norm, clow, chigh);
    subd_square(a, at, abmidp, abmidpt, acmidp, acmidpt, center, centert, norm, clow, chigh);
    subd_square(center, centert, bdmidp, bdmidpt, cdmidp, cdmidpt, d, dt, norm, clow, chigh);
    subd_square(acmidp, acmidpt, center, centert, c, ct, cdmidp, cdmidpt, norm, clow, chigh);
  }
}

// //******************************************************************************

void Sub::display()
{

  glBindVertexArray(vao);
  glUseProgram(sub_shader);

  light_position = original_light_position + glm::vec3(2*cos(0.005*t),-2,2*sin(0.01*t));
  glUniform3fv(light_position_loc, 1, glm::value_ptr(light_position));




  GLint id;
  glGetIntegerv(GL_CURRENT_PROGRAM,&id);


  glUniform1i(glGetUniformLocation(id, "type"), 0);
  draw_hull_func();

  glUniform1i(glGetUniformLocation(id, "type"), 1);
  draw_rooms_func();

  //draw_decor_func();

  //draw_engine_func();




  //draw_accoutremont_func();
    //the doors will have windows, which might require alpha

}

void Sub::draw_hull_func()
{
  if(draw_hull)
  {
    //the hull is pretty simple
    glUseProgram(sub_shader);
    glDrawArrays(GL_TRIANGLES, hull_start, hull_num);
  }
}

void Sub::draw_rooms_func()
{
  for(int i = 0; i < 9; i++)
    if(draw_room[i])
    {
      //set up lights, on a per-room basis - perhaps pass in the neighboring lights, so as to have light spillover

      //swap textures here, probably a switch statement

      // glUseProgram(room_shader);  //they all use the same shader, using the per-vertex normals, texcoords,
                                    //positions (colors ignored) - and reference the same texture UNITS
                                    // - not the same textures, but as far as the shader is concerned, the
                                    //behavior is identical and it doesn't need to know anything about it

                                    //we'll follow up on this, I asked on /r/opengl

      glDrawArrays(GL_TRIANGLES, room_start[i], room_num[i]);
    }
}

// //******************************************************************************

void Sub::update_rotation()
{
  yawpitchroll[0] += (3.14/180.0) * yaw_rate;
  yawpitchroll[1] += (3.14/180.0) * pitch_rate;
  yawpitchroll[2] += (3.14/180.0) * roll_rate;

  if(abs(yawpitchroll[0]) > 2*3.14)
    if(yawpitchroll[0] > 0) //positive overflow
      yawpitchroll[0] -= 2*3.14;
    else                    //negative "underflow"
      yawpitchroll[0] += 2*3.14;

  if(abs(yawpitchroll[1]) > 2*3.14)
    if(yawpitchroll[1] > 0)
      yawpitchroll[1] -= 2*3.14;
    else
      yawpitchroll[1] += 2*3.14;

  if(abs(yawpitchroll[2]) > 2*3.14)
    if(yawpitchroll[2] > 0)
      yawpitchroll[2] -= 2*3.14;
    else
      yawpitchroll[2] += 2*3.14;

  glUniform3fv(yawpitchroll_loc, 1, glm::value_ptr(yawpitchroll));


  // view = view * glm::rotate(1.0f/200.0f,glm::vec3(view[0][0], view[0][1], view[0][2]));
  // glUniformMatrix4fv(view_loc, 1, GL_TRUE, glm::value_ptr(view));


}

// //******************************************************************************

void Sub::set_proj(glm::mat4 in)
{
  proj = in;
  glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
  // glUniformMatrix4fv(proj_loc, 1, GL_TRUE, glm::value_ptr(proj));
}

// //******************************************************************************

void Sub::set_view(glm::mat4 in)
{
  view = in;
  glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
}

// //******************************************************************************


void Sub::set_scale(float in)
{
  scale = in;
  glUniform1fv(scale_loc, 1, &scale);
}

// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
