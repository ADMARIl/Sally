// File: Sally.cpp
//
// CMSC 341 Fall 2018 Project 2
//
// Implementation of member functions of Sally Forth interpreter
//
// Author: Andrew Ingson (aings1@umbc.edu)
// Date: 10/3/18

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <stdexcept>
#include <cstdlib>
using namespace std ;

#include "Sally.h"


// Basic Token constructor. Just assigns values.
//
Token::Token(TokenKind kind, int val, string txt) {
   m_kind = kind ;
   m_value = val ;
   m_text = txt ;
}


// Basic SymTabEntry constructor. Just assigns values.
//
SymTabEntry::SymTabEntry(TokenKind kind, int val, operation_t fptr) {
   m_kind = kind ;
   m_value = val ;
   m_dothis = fptr ;
}


// Constructor for Sally Forth interpreter.
// Adds built-in functions to the symbol table.
//
Sally::Sally(istream& input_stream) :
   istrm(input_stream)  // use member initializer to bind reference
{

   symtab["DUMP"]    =  SymTabEntry(KEYWORD,0,&doDUMP) ;

   symtab["+"]    =  SymTabEntry(KEYWORD,0,&doPlus) ;
   symtab["-"]    =  SymTabEntry(KEYWORD,0,&doMinus) ;
   symtab["*"]    =  SymTabEntry(KEYWORD,0,&doTimes) ;
   symtab["/"]    =  SymTabEntry(KEYWORD,0,&doDivide) ;
   symtab["%"]    =  SymTabEntry(KEYWORD,0,&doMod) ;
   symtab["NEG"]  =  SymTabEntry(KEYWORD,0,&doNEG) ;

   symtab["."]    =  SymTabEntry(KEYWORD,0,&doDot) ;
   symtab["SP"]   =  SymTabEntry(KEYWORD,0,&doSP) ;
   symtab["CR"]   =  SymTabEntry(KEYWORD,0,&doCR) ;

   // stack operations
   symtab["DUP"]  =  SymTabEntry(KEYWORD,0,&doDUP) ;
   symtab["DROP"] = SymTabEntry(KEYWORD,0,&doDROP) ;
   symtab["SWAP"] = SymTabEntry(KEYWORD,0,&doSWAP) ;
   symtab["ROT"] = SymTabEntry(KEYWORD,0,&doROT) ;
   // var operations
   symtab["SET"] = SymTabEntry(KEYWORD,0,&doSET) ;
   symtab["@"] = SymTabEntry(KEYWORD,0,&doAT) ;
   symtab["!"] = SymTabEntry(KEYWORD,0,&doSTORE) ;
   // comparision operations
   symtab["<"] = SymTabEntry(KEYWORD,0,&doLess) ;
   symtab["<="] = SymTabEntry(KEYWORD,0,&doLessOrEqual) ;
   symtab["=="] = SymTabEntry(KEYWORD,0,&doEqual) ;
   symtab["!="] = SymTabEntry(KEYWORD,0,&doNotEqual) ; 
   symtab[">="] = SymTabEntry(KEYWORD,0,&doGreaterOrEqual) ;
   symtab[">"] = SymTabEntry(KEYWORD,0,&doGreater) ;
   // logic operations
   symtab["AND"] = SymTabEntry(KEYWORD,0,&doAND) ;
   symtab["OR"] = SymTabEntry(KEYWORD,0,&doOR) ;
   symtab["NOT"] = SymTabEntry(KEYWORD,0,&doNOT) ;
   // if statements
   symtab["IFTHEN"] = SymTabEntry(KEYWORD,0,&doIFTHEN) ;
   symtab["ELSE"] = SymTabEntry(KEYWORD,0,&doELSE) ;
   symtab["ENDIF"] = SymTabEntry(KEYWORD,0,&doENDIF) ;
   // loop construct
   symtab["DO"] = SymTabEntry(KEYWORD,0,&doDO) ;
   symtab["UNTIL"] = SymTabEntry(KEYWORD,0,&doUNTIL) ;
}


// This function should be called when tkBuffer is empty.
// It adds tokens to tkBuffer.
//
// This function returns when an empty line was entered 
// or if the end-of-file has been reached.
//
// This function returns false when the end-of-file was encountered.
// 
// Processing done by fillBuffer()
//   - detects and ignores comments.
//   - detects string literals and combines as 1 token
//   - detetcs base 10 numbers
// 
//
bool Sally::fillBuffer() {
   string line ;     // single line of input
   int pos ;         // current position in the line
   int len ;         // # of char in current token
   long int n ;      // int value of token
   char *endPtr ;    // used with strtol()


   while(true) {    // keep reading until empty line read or eof

      // get one line from standard in
      //
      getline(istrm, line) ;   

      // if "normal" empty line encountered, return to mainLoop
      //
      if ( line.empty() && !istrm.eof() ) {
         return true ;
      }

      // if eof encountered, return to mainLoop, but say no more
      // input available
      //
      if ( istrm.eof() )  {
         return false ;
      }


      // Process line read

      pos = 0 ;                      // start from the beginning

      // skip over initial spaces & tabs
      //
      while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
         pos++ ; 
      }

      // Keep going until end of line
      //
      while (line[pos] != '\0') {

         // is it a comment?? skip rest of line.
         //
         if (line[pos] == '/' && line[pos+1] == '/') break ;

         // is it a string literal? 
         //
         if (line[pos] == '.' && line[pos+1] == '"') {

            pos += 2 ;  // skip over the ."
            len = 0 ;   // track length of literal

            // look for matching quote or end of line
            //
            while(line[pos+len] != '\0' && line[pos+len] != '"') {
               len++ ;
            }

            // make new string with characters from
            // line[pos] to line[pos+len-1]
            string literal(line,pos,len) ;  // copy from pos for len chars

            // Add to token list
            //
            tkBuffer.push_back( Token(STRING,0,literal) ) ;  

            // Different update if end reached or " found
            //
            if (line[pos+len] == '\0') {
               pos = pos + len ;
            } else {
               pos = pos + len + 1 ;
            }

         } else {  // otherwise "normal" token

            len = 0 ;  // track length of token

            // line[pos] should be an non-white space character
            // look for end of line or space or tab
            //
            while(line[pos+len] != '\0' && line[pos+len] != ' ' && line[pos+len] != '\t') {
               len++ ;
            }

            string literal(line,pos,len) ;   // copy form pos for len chars
            pos = pos + len ;

            // Try to convert to a number
            //
            n = strtol(literal.c_str(), &endPtr, 10) ;

            if (*endPtr == '\0') {
               tkBuffer.push_back( Token(INTEGER,n,literal) ) ;
            } else {
               tkBuffer.push_back( Token(UNKNOWN,0,literal) ) ;
            }
         }

         // skip over trailing spaces & tabs
         //
         while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
            pos++ ; 
         }

      }
   }
}



// Return next token from tkBuffer.
// Call fillBuffer() if needed.
// Checks for end-of-file and throws exception 
//
Token Sally::nextToken() {
      Token tk ;
      bool more = true ;

      while(more && tkBuffer.empty() ) {
         more = fillBuffer() ;
      }

      if ( !more && tkBuffer.empty() ) {
         throw EOProgram("End of Program") ;
      }

      tk = tkBuffer.front() ;
      tkBuffer.pop_front() ;
      return tk ;
}


// The main interpreter loop of the Sally Forth interpreter.
// It gets a token and either push the token onto the parameter
// stack or looks for it in the symbol table.
//
//
void Sally::mainLoop() {

   Token tk ;
   map<string,SymTabEntry>::iterator it ;

   try {
      while( 1 ) {
         tk = nextToken() ;

	 // check if we need to record the current token
	  if (recording == true) {
	    // add the token to the record buffer
	      doBuffer.push_back(tk);
	 }

	 if (tk.m_kind == INTEGER || tk.m_kind == STRING) {

            // if INTEGER or STRING just push onto stack
            params.push(tk) ;

         } else { 
            it = symtab.find(tk.m_text) ;
            
            if ( it == symtab.end() )  {   // not in symtab

               params.push(tk) ;

            } else if (it->second.m_kind == KEYWORD)  {

               // invoke the function for this operation
               //

               it->second.m_dothis(this) ;   
               
            } else if (it->second.m_kind == VARIABLE) {

               // variables are pushed as tokens
               //
               tk.m_kind = VARIABLE ;
               params.push(tk) ;

            } else {

               // default action
               //
               params.push(tk) ;

	    }
         }
	
      }

   } catch (EOProgram& e) {

      cerr << "End of Program\n" ;
      if ( params.size() == 0 ) {
         cerr << "Parameter stack empty.\n" ;
      } else {
         cerr << "Parameter stack has " << params.size() << " token(s).\n" ;
      }

   } catch (out_of_range& e) {

      cerr << "Parameter stack underflow??\n" ;

   } catch (...) {

      cerr << "Unexpected exception caught\n" ;

   }
}

// -------------------------------------------------------


void Sally::doPlus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for +.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value + p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doMinus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for -.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value - p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doTimes(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for *.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value * p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doDivide(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for /.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value / p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doMod(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for %.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value % p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doNEG(Sally *Sptr) {
   Token p ;

   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for NEG.") ;
   }
   p = Sptr->params.top() ;
   Sptr->params.pop() ;
   Sptr->params.push( Token(INTEGER, -p.m_value, "") ) ;
}


void Sally::doDot(Sally *Sptr) {

   Token p ;
   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for .") ;
   }

   p = Sptr->params.top() ;
   Sptr->params.pop() ;

   if (p.m_kind == INTEGER) {
      cout << p.m_value ;
   } else {
      cout << p.m_text ;
   }
}


void Sally::doSP(Sally *Sptr) {
   cout << " " ;
}


void Sally::doCR(Sally *Sptr) {
   cout << endl ;
}

void Sally::doDUMP(Sally *Sptr) {
   // do whatever for debugging
}

//
// NEW FUNCTIONS ADDED PAST HERE
//

// stack operations

void Sally::doDUP(Sally *Sptr) {
  Token p1; // create token and check if there is enough available
  
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need one parameter for DUP.");
  }

  // copy token on top and push a duplicate to the stack
  p1 = Sptr->params.top();
  Sptr->params.push( Token(p1.m_kind,p1.m_value,p1.m_text) );
  
}

void Sally::doDROP(Sally *Sptr) {
  // check if there are enough params in the buffer
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need one parameter for DROP.");
  }
  // remove top param from buffer
  Sptr->params.pop();
}

void Sally::doSWAP(Sally *Sptr) {
  // create var tokens and see if there are enough in buffer
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for SWAP.");
  }

  // copy the params sitting at the top
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();
  // push them back in reverse order
  Sptr->params.push(p1);
  Sptr->params.push(p2);
}

void Sally::doROT(Sally *Sptr) {
  // create 3 var tokens and see if ther are enough is buffer
  Token p1, p2, p3;

  if (Sptr->params.size() < 3) {
    throw out_of_range("Need three parameters for SWAP.");
  }

  // copy top three params
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();
  p3 = Sptr->params.top();
  Sptr->params.pop();
  // push back with the last being first now
  Sptr->params.push(p2);
  Sptr->params.push(p1);
  Sptr->params.push(p3);
}

// var operations

void Sally::doSET(Sally *Sptr) {
  map<string,SymTabEntry>::iterator it ;
  // create 2 var tokens and check if there are enough in buffer
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for SET.");
  }
  // copy top 2 params
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //  cout << "p1: " << p1.m_value << endl;
  //cout << "p2: " << p2.m_value << endl;

  // find the FORTH variable we are looking for
  it = Sptr->symtab.find(p1.m_text) ;

  // if we think it doesnt exist, create it and add it to the SymTab
  if (it == (Sptr->symtab.end())) {
    p1.m_value = p2.m_value;
    Sptr->symtab[p1.m_text] = SymTabEntry(p1.m_kind,p1.m_value,NULL);
    //cout << p1.m_text << Sptr->symtab[p1.m_text].m_value;
  } else {
    // if it already exists print an error
    cout << "Variable already exists.\n";
  }
}

void Sally::doAT(Sally *Sptr) {
  map<string,SymTabEntry>::iterator it ;
  // create var token and check if there are enough in buffer
  Token p1;

  if (Sptr->params.size() < 1) {
    throw out_of_range("Need one parameter for AT.");
  }
  // copy top param
  p1 = Sptr->params.top();
  Sptr->params.pop();
  // find the var using iterator
  it = Sptr->symtab.find(p1.m_text);

  // if it doesnt exist, print an error
  if (it == (Sptr->symtab.end())) {
    cout << "Not found.\n";
  } else {
    // push found value to param
    Sptr->params.push( Token(INTEGER, Sptr->symtab[p1.m_text].m_value, "") );
  }
  
}

void Sally::doSTORE(Sally *Sptr) {
  map<string,SymTabEntry>::iterator it ;
  // create two var tokens and see if there are enough in buffer
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for STORE.");
  }
  // copy top 2 params
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();
  // find var using iterator
  it = Sptr->symtab.find(p1.m_text) ;
  // if it doesnt exist, say so
  if (it == (Sptr->symtab.end())) {
    cout << "Not found.\n";
  } else {
    // if it does exist, add it to the table
    p1.m_value = p2.m_value;
    Sptr->symtab[p1.m_text] = SymTabEntry(p1.m_kind, p1.m_value, NULL);
  }
}

// comparison operations

void Sally::doLess(Sally *Sptr) {
  Token p1, p2; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for <.");
  }

  // grab top two params off the stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // push answer to the stack
  int answer = (p2.m_value < p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

void Sally::doLessOrEqual(Sally *Sptr) {
  Token p1, p2; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for <=.");
  }

  // grab top two params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // push answer to the stack
  int answer = (p2.m_value <= p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

void Sally::doEqual(Sally *Sptr) {
  Token p1, p2; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for ==.");
  }
  // grab top two params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // push answer to the stack
  int answer = (p2.m_value == p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

void Sally::doNotEqual(Sally *Sptr) {
  Token p1, p2; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for !=.");
  }

  // grab params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // push answer to stack
  int answer = (p2.m_value != p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

void Sally::doGreaterOrEqual(Sally *Sptr) {

  Token p1, p2; // create temp tokens
  
  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for >=");
  }

  // grab params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // push answer to the stack
  int answer = (p2.m_value >= p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
  //cout << "GREATEROFEQUAL\n";
}

void Sally::doGreater(Sally *Sptr) {
  Token p1, p2; // create temp tokens
  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for >.");
  }

  // grab params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // calculate answer and push it to param stack
  int answer = (p2.m_value > p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// logic operations

void Sally::doAND(Sally *Sptr) {
  Token p1, p2; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for AND.");
  }

  // grab params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // calculate answer and push it to param stack
  int answer = (p2.m_value && p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

void Sally::doOR(Sally *Sptr) {
  Token p1, p2; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for OR.");
  }

  // grab params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  p2 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // calculate answer and push it to param stack
  int answer = (p2.m_value || p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

void Sally::doNOT(Sally *Sptr) {
  Token p1; // create temp tokens

  // check if param stack has enough params for us
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need one parameter for NOT.");
  }

  // grab params off stack and remove them
  p1 = Sptr->params.top() ;
  Sptr->params.pop() ;
  // calculate answer and push it to param stack
  int answer = (!p1.m_value) ;
  Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

// if statements

void Sally::doIFTHEN(Sally *Sptr) {
  Token p1; // create temp tokens
  // map<string,SymTabEntry>::iterator it ;
  
  // check if param stack has enough params for us
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need one parameter for IFTHEN.");
  }

  // get top token off of stack
  p1 = Sptr->params.top();
  // see if the token is false and we need to delete if commands
  if (p1.m_value == false) {
    // remove param since we dont need it anymore
    Sptr->params.pop();
    // get next token
    p1 = Sptr->nextToken();
    // vars to count conditionals
    int nestIF = 1;
    int nestELSE = 0;
    // while loop to count them
    while ((p1.m_text != "ELSE") || (nestIF != nestELSE)) {
      p1 = Sptr->nextToken();
      if (p1.m_text == "IFTHEN")
	nestIF++;
      if (p1.m_text == "ELSE")
	nestELSE++;
      					     
    }
  }
}


void Sally::doELSE(Sally *Sptr) {
  Token p1; // create temp tokens
  //map<string,SymTabEntry>::iterator it ;
  
  // check if param stack has enough params for us
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need one parameter for ELSE.");
  }
  // grab top param off of stack
  p1 = Sptr->params.top();
  Sptr->params.pop();
  // see if that param is true and we need to delete
  if (p1.m_value == true) {
    // advance the token
    p1 = Sptr->nextToken();
    // create vars
    int nestENDIF = 0;
    int nestELSE = 1;
    // while loop to count conditionals
    while ((p1.m_text != "ENDIF") || (nestENDIF != nestELSE)) {
      p1 = Sptr->nextToken();
      if (p1.m_text == "ENDIF")
	nestENDIF++;
      if (p1.m_text == "ELSE")
	nestELSE++;
      					     
    }
  }
}

void Sally::doENDIF(Sally *Sptr) {
  // we proooooooooabbbbbbbbbbbbblllllllllllyyyy dont need anything here
  /*
                   ▄              ▄
                  ▌▒█           ▄▀▒▌
                  ▌▒▒█        ▄▀▒▒▒▐
                 ▐▄▀▒▒▀▀▀▀▄▄▄▀▒▒▒▒▒▐
               ▄▄▀▒░▒▒▒▒▒▒▒▒▒█▒▒▄█▒▐
             ▄▀▒▒▒░░░▒▒▒░░░▒▒▒▀██▀▒▌
            ▐▒▒▒▄▄▒▒▒▒░░░▒▒▒▒▒▒▒▀▄▒▒▌
            ▌░░▌█▀▒▒▒▒▒▄▀█▄▒▒▒▒▒▒▒█▒▐
           ▐░░░▒▒▒▒▒▒▒▒▌██▀▒▒░░░▒▒▒▀▄▌
           ▌░▒▄██▄▒▒▒▒▒▒▒▒▒░░░░░░▒▒▒▒▌
          ▌▒▀▐▄█▄█▌▄░▀▒▒░░░░░░░░░░▒▒▒▐
          ▐▒▒▐▀▐▀▒░▄▄▒▄▒▒▒▒▒▒░▒░▒░▒▒▒▒▌
          ▐▒▒▒▀▀▄▄▒▒▒▄▒▒▒▒▒▒▒▒░▒░▒░▒▒▐
           ▌▒▒▒▒▒▒▀▀▀▒▒▒▒▒▒░▒░▒░▒░▒▒▒▌
           ▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▄▒▒▐
            ▀▄▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▄▒▒▒▒▌
              ▀▄▒▒▒▒▒▒▒▒▒▒▄▄▄▀▒▒▒▒▄▀
                ▀▄▄▄▄▄▄▀▀▀▒▒▒▒▒▄▄▀
                   ▒▒▒▒▒▒▒▒▒▒▀▀

   */
}

// loop construct

void Sally::doDO(Sally *Sptr) {
  // set recording flag to true
  Sptr->recording = true;
}

void Sally::doUNTIL(Sally *Sptr) {
  // set recording to false since we have everything we need
  Sptr->recording = false;
  Token p1;
  // create tempbuffer equal to dobuffer because splice is weird
  list<Token> tempBuffer;
  tempBuffer = Sptr->doBuffer;

  // check if we have enough parameters
  if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for UNTIL.") ;
   }

  // pull top param off of stack and save it for later
  p1 = Sptr->params.top();
  Sptr->params.pop();

  // if the until is true, end it
  if (p1.m_value == 1) {
    Sptr->recording = false;
  } else {
    // if not append the recording to tk buffer and keep going
    Sptr->tkBuffer.splice(Sptr->tkBuffer.begin(), tempBuffer);
  }
}
