//#include <windows.h> // remove this for mac
//
//#include <GL/gl.h> // These are in a different spot for mac and glut is already installed
//#include <GL/glu.h>
//#include <glut.h>
//#include <gmtl/gmtl.h> // This is a header-only library in the project folders
//
//#include <iostream>
//#include <sstream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include <cmath>
//
//
//#define MAX_PARTICLES 1000
//
//class  ParticleSystem
//{
//	
//public:
//	 ParticleSystem();
//	~ ParticleSystem();
//
//	
//	void CreateParticleSystem();
//
//private:
//
//};
//
// ParticleSystem:: ParticleSystem()
//{
//	 Particle *ParticlesList[MAX_PARTICLES];
//	 for (int i = 0; i < MAX_PARTICLES; i++){
//		 		 
//		 ParticlesList[i] = new Particle();
//
//	 }
//
//
//}
//
// void CreateParticleSystem()
// {
//	 
// }
// ParticleSystem::~ ParticleSystem()
//{
//}
// 
//
//
// class Particle{
//	 float lifetime;                       // total lifetime of the particle
//	 float decay;                          // decay speed of the particle
//	 float r, g, b;                          // color values of the particle
//	 float xpos, ypos, zpos;                 // position of the particle
//	 float xspeed, yspeed, zspeed;           // speed of the particle
//	 boolean active;                       // is particle active or not?
// public:
//	 Particle();
//	 ~Particle();
//
// private:
// };
//
// Particle::Particle()
// {
//	 this->lifetime = (float)gmtl::Math::rangeRandom(0, 500000) / 500000.0;
//	 this->decay = 0.001;
//	 /*this->r = 0.7;
//	 particle[i].g = 0.7;
//	 particle[i].b = 1.0;*/
//	 this->xpos = 0.0;
//	 this->ypos = 0.0;
//	 this->zpos = 0.0;
//	 this->xspeed = 0.0005 - (float)gmtl::Math::rangeRandom(0, 100) / 100000.0;
//	 this->yspeed = 0.01 - (float)gmtl::Math::rangeRandom(0, 100) / 100000.0;
//	 this->zspeed = 0.0005 - (float)gmtl::Math::rangeRandom(0, 100) / 100000.0;
//	 this->active = true;
// }
//
