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
      submodel->adjust_yaw_rate(0.1);
      break;
    case 'w':
      submodel->adjust_yaw_rate(-0.1);
      break;

    case 'a':   //PITCH CONTROLS
      submodel->adjust_pitch_rate(0.1);
      break;
    case 's':
      submodel->adjust_pitch_rate(-0.1);
      break;

    case 'z':   //ROLL CONTROLS
      submodel->adjust_roll_rate(0.1);
      break;
    case 'x':
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
