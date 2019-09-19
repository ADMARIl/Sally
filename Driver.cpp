// File: Driver.cpp
//
// CMSC 341 Fall 2018 Project 2
//
// Simple driver program to call the Sally Forth interpreter
// 
// Author: Andrew Ingson (aings1@umbc.edu)
// Date: 10/9/18


#include <iostream>
#include <fstream>
using namespace std ;

#include "Sally.h"

int main() {
   string fname = "test.sally";

   ifstream ifile(fname.c_str()) ;

   Sally S(ifile) ;

   S.mainLoop() ;

   ifile.close() ;
   return 0 ;
}
