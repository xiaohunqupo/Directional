#include <iostream>
#include <Eigen/Core>
#include <igl/unproject_onto_mesh.h>
#include <directional/TriMesh.h>
#include <directional/IntrinsicFaceTangentBundle.h>
#include <directional/CartesianField.h>
#include <directional/readOFF.h>
#include <directional/read_raw_field.h>
#include <directional/write_raw_field.h>
#include <directional/curl_matching.h>
#include <directional/combing.h>
#include <directional/setup_integration.h>
#include <directional/integrate.h>
#include <directional/cut_mesh_with_singularities.h>
#include <directional/branched_isolines.h>
#include <directional/directional_viewer.h>

#define NUM_N 4

int N[NUM_N];
int currN = 0;
directional::TriMesh meshWhole, meshCut[NUM_N];
directional::IntrinsicFaceTangentBundle ftb;
directional::CartesianField rawField[NUM_N], combedField[NUM_N];
Eigen::MatrixXd NFunction[NUM_N], NCornerFunction[NUM_N];
directional::DirectionalViewer viewer;

typedef enum {FIELD, INTEGRATION} ViewingModes;
ViewingModes viewingMode=FIELD;

void update_viewer()
{
  for (int i=0;i<NUM_N;i++){
    viewer.toggle_field(false,i);
    viewer.toggle_singularities(false,i);
    viewer.toggle_seams(false,i);
    viewer.toggle_isolines(false,i);
  }
  if (viewingMode==FIELD){
    viewer.toggle_field(true,currN);
    viewer.toggle_singularities(true,currN);
    viewer.toggle_seams(true,currN);
    viewer.toggle_isolines(false,currN);
  } else {
    viewer.toggle_field(false,currN);
    viewer.toggle_singularities(false,currN);
    viewer.toggle_seams(false,currN);
    viewer.toggle_isolines(true,currN);
  }
}


// Handle keyboard input
bool key_down(igl::opengl::glfw::Viewer& viewer, int key, int modifiers)
{
  switch (key)
  {
      // Select vector
    case '1': viewingMode = FIELD; break;
    case '2': viewingMode = INTEGRATION; break;
    case '3': currN=(currN+1)%NUM_N; break;
  }
  update_viewer();
  return true;
}


int main()
{
  std::cout <<
  "  1  Loaded field" << std::endl <<
  "  2  Show isoline mesh" << std::endl <<
  "  3  change between different N" << std::endl;
  
  directional::readOFF(TUTORIAL_SHARED_PATH "/vase.off", meshWhole);
  ftb.init(meshWhole);
  directional::read_raw_field(TUTORIAL_SHARED_PATH "/vase-2.rawfield", ftb, N[0], rawField[0]);
  directional::read_raw_field(TUTORIAL_SHARED_PATH "/vase-4.rawfield", ftb, N[1], rawField[1]);
  directional::read_raw_field(TUTORIAL_SHARED_PATH "/vase-7.rawfield", ftb, N[2], rawField[2]);
  directional::read_raw_field(TUTORIAL_SHARED_PATH "/vase-11.rawfield", ftb, N[3], rawField[3]);
  
  //combing and cutting
  for (int i=0;i<NUM_N;i++){
    directional::principal_matching(rawField[i]);
    
    
    directional::IntegrationData intData(N[i]);
    std::cout<<"Setting up Integration N="<<N[i]<<std::endl;
    directional::setup_integration(rawField[i], intData, meshCut[i], combedField[i]);
    
    intData.verbose=false;
    intData.integralSeamless=true;
    intData.roundSeams=false;
  
    std::cout<<"Solving integration N=" << N[i]<<std::endl;
    directional::integrate(combedField[i],  intData, meshCut[i], NFunction[i],NCornerFunction[i]);
    
    std::cout<<"Done!"<<std::endl;
    
    viewer.set_mesh(meshWhole,i);
    viewer.set_field(combedField[i], directional::DirectionalViewer::indexed_glyph_colors(combedField[i].extField), i);
    viewer.set_seams(combedField[i].matching, i);
    viewer.set_isolines(meshCut[i],NFunction[i],i);

    
  }
  
  update_viewer();
  viewer.callback_key_down = &key_down;
  viewer.launch();
}


