//******************************************************************************
//  Program: HAUNTED HOUSE
//
//  Author: Jon Baker
//  Email: jb239812@ohio.edu
//
//  Description: This file contains GLUT specific code to open a window and
//       allow interaction. It instantiates model classes declared in model.h
//
//      The goal of this program is to experiment with a number of textures
//      being used in the representation of a surface as part of a model.
//
//  Date: 10 October 2019
//******************************************************************************

#include "resources/sub.hpp"
#include <stdio.h>


Sub * submodel;
float scale = 1;
int t = 0;





//stuff to handle the player's movement
bool outside = true;  //if you're outside, show the hull like we've been looking at, if false, you're inside

JonDefault::state player_current_state = JonDefault::floor1; //need to know the y offsets for that, move them from sub.hpp to common.hpp

glm::vec3 player_position = JonDefault::player_position;
glm::vec3 player_forward = JonDefault::player_forward;   //+z
glm::vec3 player_left = JonDefault::player_left;     //+x
glm::vec3 player_up = JonDefault::player_up;      //+y

bool is_ok(glm::vec3 point);





//DEBUG STUFF

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "    GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}



void init()
{
  cout << "initializing models ...";
  submodel = new Sub();
  cout << " done." << endl;

  submodel->set_view(JonDefault::view);

  submodel->set_proj(JonDefault::proj);

  submodel->set_scale(scale);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_LINE_SMOOTH);

  glClearColor(0.068f, 0.168f, 0.268f, 1.0f);

}

//---------------------------------------------------------------------------

void display()
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // display functions go here
  submodel->display();

  // glFlush();
  glutSwapBuffers();
  glutPostRedisplay();

}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {

    case 033:
      exit(EXIT_SUCCESS);
      break;



    case 'y':
      submodel->toggle_hull();
      break;













    //these should be dropped in the interior view

    case 'q':   //YAW CONTROLS
      if(outside)
        submodel->adjust_yaw_rate(0.1);
      break;
    case 'w':
      if(outside)
        submodel->adjust_yaw_rate(-0.1);
      else
      {//you're inside, so this is forward

      }

      break;


    case 'a':   //PITCH CONTROLS
      if(outside)
        submodel->adjust_pitch_rate(0.1);
      else
      {//you're inside, this is left

      }
      break;


    case 's':
      if(outside)
        submodel->adjust_pitch_rate(-0.1);
      else
      {//you're inside, this is 'back'

      }
      break;





    case 'd':
      if(!outside)
      {//no collision here, it's just right

      }
      break;



    case 'z':   //ROLL CONTROLS
      if(outside)
        submodel->adjust_roll_rate(0.1);
      break;
    case 'x':
      if(outside)
        submodel->adjust_roll_rate(-0.1);
      break;


            //SCALE ADJUSTMENT
    case 'e':
      scale -= 0.01;
      submodel->set_scale(scale);
      break;

    case 'r':
      scale += 0.01;
      submodel->set_scale(scale);
      break;





    case 'p':
      //change perspective
      outside = !outside;

      if(outside)
      {//you're outside

      }
      else
      {//you're inside

      }




    case 'f':
      // glutFullScreen();
      glutFullScreenToggle(); //specific to freeglut - otherwise keep a bool, return to windowed with window functions
      break;

  }
  glutPostRedisplay();
}

//----------------------------------------------------------------------------



void mouse( int button, int state, int x, int y )
{
  if ( state == GLUT_DOWN )
	{
		switch( button )
		{
		    case GLUT_LEFT_BUTTON:    cout << "left" << endl;   break;
		    case GLUT_MIDDLE_BUTTON:  cout << "middle" << endl; break;
		    case GLUT_RIGHT_BUTTON:   cout << "right" << endl;  break;
		}

    if(button == GLUT_LEFT_BUTTON)
    {

      //clear the screen
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      //selection handling code - using input x and y
      y = glutGet( GLUT_WINDOW_HEIGHT ) - y;

      unsigned char pixel[4];
      glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

      cout << "color at click is r:" << (int)pixel[0] << " g:" << (int)pixel[1] << " b:" << (int)pixel[2] << endl;


      //clear the screen
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glutPostRedisplay();

    }
  }
}

//----------------------------------------------------------------------------

void timer(int)
{
  t++;
  submodel->set_time(t);
  submodel->update_rotation();


	glutPostRedisplay();
	glutTimerFunc(1000.0/60.0, timer, 0);
}

//----------------------------------------------------------------------------



void idle( void )
{
	// glutPostRedisplay();
}


//----------------------------------------------------------------------------
//is this an ok move?
bool is_ok(glm::vec3 point)
{
  switch(player_current_state)
  {
    case JonDefault::floor1:  //check bounds of first floor

    if(point.y != JonDefault::floor1yoffset)
      return false;

    if(point.z < JonDefault::room1start)
      return false;

    if(point.z > JonDefault::room3end)
      return false;

    if(point.x > JonDefault::radius)
      return false;

    if(point.x < -1.0f*JonDefault::radius)
      return false;

    if(point.z > JonDefault::room1end && point.z < JonDefault::room2start)
    {
      if(point.x < -0.2f*JonDefault::radius || point.x > 0.2f*JonDefault::radius)
      {
        return false;
      }
    }

    if(point.z > JonDefault::room2end && point.z < JonDefault::room3start)
    {
      if(point.x < -0.2f*JonDefault::radius || point.x > 0.2f*JonDefault::radius)
      {
        return false;
      }
    }

    return true;
    break;








    case JonDefault::floor2:  //check bounds of second floor

    if(point.y != JonDefault::floor2yoffset)
      return false;

    break;





    case JonDefault::floor3:  //check bounds of third floor

    if(point.y != JonDefault::floor3yoffset)
      return false;

    break;





    case JonDefault::onetotwo:
    case JonDefault::twotothree:
    case JonDefault::threetotwo:
    case JonDefault::twotoone:
      //no movement allowed while changing states
      return false;
    break;

  }
}


//----------------------------------------------------------------------------


int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  // glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  //doesn't look as good
  glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

  glutInitContextVersion( 4, 5 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

  glutInitWindowSize(720,480);
  glutCreateWindow("A VERY SCARY SUBMARINE");

  glewInit();


  //DEBUG
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );

  cout << endl << endl << " GL_DEBUG_OUTPUT ENABLED " << endl;



  init();

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc( mouse );
  glutIdleFunc( idle );
  glutTimerFunc(1000.0/60.0, timer, 0);





//ENTER MAIN LOOP
  glutMainLoop();
  return(EXIT_SUCCESS);
}
