#include "common.hpp"
//this is based on the project from this summer - here implemented with polygons, and rendered using perspective projection
// - from what I can gather, we're going to be wildly more efficient with polygons than with the voxel scheme

  //thoughts
    //little propeller, connected out the back by the crankshaft - maybe add a transmission?


    //the engine parts will be closed meshes (pistons, etc), and we'll be using the discarding of back faces to
    //draw the cylinders, the exhaust pipes if we get to that, perhaps with a uniform toggle to enable/disable
    //and representing the cycle with a color mixed with calculated lighting for the sleeve - denser during some
    //parts of the cycle than others.











class engine
{
public:

  //the engine consists of a number of parts -

    //crankshaft - connects to the driveshaft to take power from the engine
    //conrods    - connects the crankshaft to the pistons - bottom end located by crank, top located by piston
    //pistons    - the bottom of the combustion chamber - doing piston rings is way easier with polygons
    //valves     - this is new, I want to simplify the representation of the cycle
    //cams       - these are also new, and their rotation is half the speed of the crankshaft
    //intake     - a way to represent air and fuel coming into the engine - scoop out the top?
    //exhaust    - a way to represent the exhaust gasses coming out of the engine - may include piping



  // I want to do coloration of the gas, based on this - https://iquilezles.org/www/articles/palettes/palettes.htm
  //  probably going to represent it as disks (disqes, short for disquettes), past the valve,

  //calls all the add functions, so there is geometry to draw all of the engine components
  void init(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors);


  //calls glDrawArrays(GL_TRIANGLES, ____start, num_pts_____); for each of the things, for each cylinder
    //so there's a fair few, num_cylinders many for pistons, con rods, and the sets of valves, then one crank, and 4 cams
  void draw();


  void set_theta(float in) {theta = in;}

private:
  int crank_start, num_pts_crank;
  int conrod_start, num_pts_conrod;
  int piston_start, num_pts_piston;
  int intake_valves_start, num_pts_intake_valves;
  int exhaust_valves_start, num_pts_exhaust_valves;
  int cams_start, num_pts_cams;
  // int exhaust_start, num_pts_exhaust;


  float theta;

  //translation vectors & rotation amounts are held here, vec3s and floats




  //call these during init - set the integer values to keep track of the indices, same way as in ship.h
  void add_crank(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors);
  void add_conrod(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors);
  void add_piston(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors);
  void add_valves(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors);
  void add_cams(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors);
  // void add_exhaust(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);


  //we need to come up with ways to reduce the complexity of this


};




void engine::init(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors)
{
  theta = 0.0f;

  int temp = points.size();

  add_crank(points, normals, colors);
  add_conrod(points, normals, colors);
  add_piston(points, normals, colors);
  add_valves(points, normals, colors);
  add_cams(points, normals, colors);

  cout << "engine contains " << points.size() - temp << " points" << endl;
}






void engine::draw()
{

  GLint id;
  glGetIntegerv(GL_CURRENT_PROGRAM,&id);

  glUniform1i(glGetUniformLocation(id, "type"), 2);

  glm::mat4 r = glm::rotate((float)theta,glm::vec3(0.0f,0.0f,1.0f));
  glm::mat4 t = glm::translate(glm::vec3(0.0f,0.2f*sin(theta),0.0f));

  glUniformMatrix4fv(glGetUniformLocation(id, "rot8"), 1, GL_FALSE, glm::value_ptr(r));
  glUniformMatrix4fv(glGetUniformLocation(id, "transl8"), 1, GL_FALSE, glm::value_ptr(t));


  glDrawArrays(GL_TRIANGLES, crank_start, num_pts_crank);
  glDrawArrays(GL_TRIANGLES, conrod_start, num_pts_conrod);
  glDrawArrays(GL_TRIANGLES, piston_start, num_pts_piston);
  glDrawArrays(GL_TRIANGLES, intake_valves_start, num_pts_intake_valves);
  glDrawArrays(GL_TRIANGLES, exhaust_valves_start, num_pts_exhaust_valves);
  glDrawArrays(GL_TRIANGLES, cams_start, num_pts_cams);

}









void engine::add_crank(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors)
{
  crank_start = points.size();

  glm::vec3 a,b,c;

  a = glm::vec3(1,0.3,1);
  b = glm::vec3(0,0.3,0);
  c = glm::vec3(0,0.3,1);

  glm::vec3 norm = glm::cross(b - a, b - c);

  points.push_back(a);
  points.push_back(b);
  points.push_back(c);

  normals.push_back(norm);
  normals.push_back(norm);
  normals.push_back(norm);

  colors.push_back(glm::vec4(0,0,0,1));
  colors.push_back(glm::vec4(0,1,0,1));
  colors.push_back(glm::vec4(0,1,1,1));

  num_pts_crank = points.size() - crank_start;
}

void engine::add_conrod(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors)
{
  conrod_start = points.size();

  num_pts_conrod = points.size() - conrod_start;
}

void engine::add_piston(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors)
{
  piston_start = points.size();

  num_pts_piston = points.size() - piston_start;
}

void engine::add_valves(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors)
{
  intake_valves_start = points.size();

  num_pts_intake_valves = points.size() - intake_valves_start;



  exhaust_valves_start = points.size();

  num_pts_exhaust_valves = points.size() - exhaust_valves_start;
}

void engine::add_cams(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& colors)
{
  cams_start = points.size();

  num_pts_cams = points.size() - cams_start;
}
